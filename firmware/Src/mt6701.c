#include "mt6701.h"

/*
 * MT6701 magnetic angle sensor — I2C readout.
 *
 * Register 0x03: angle MSB (bits [13:6] of 14-bit angle)
 * Register 0x04: angle LSB in bits [7:2], status/parity in bits [1:0]
 *
 * One transaction: write reg 0x03 then read 2 bytes (auto-increment).
 * HAL_I2C_Mem_Read packages start+addr+reg+restart+addr+read+stop atomically.
 */
HAL_StatusTypeDef MT6701_ReadAngleRaw(I2C_HandleTypeDef *hi2c, uint16_t *angle,
                                      uint8_t *raw_hi, uint8_t *raw_lo)
{
    uint8_t hi_byte;
    uint8_t lo_byte;
    HAL_StatusTypeDef st;

    /* Fast-fail if the peripheral is stuck. HAL_I2C_Mem_Read otherwise blocks
     * up to 25 ms inside I2C_TIMEOUT_BUSY_FLAG waiting for SR2.BUSY to clear. */
    if (hi2c->State != HAL_I2C_STATE_READY) return HAL_BUSY;
    if ((hi2c->Instance->SR2 & I2C_SR2_BUSY) != 0U) return HAL_BUSY;

    st = HAL_I2C_Mem_Read(hi2c, MT6701_I2C_ADDR, MT6701_REG_ANGLE_H,
                          I2C_MEMADD_SIZE_8BIT, &hi_byte, 1,
                          MT6701_I2C_TIMEOUT_MS);
    if (st != HAL_OK) return st;

    /* No between-read BUSY fast-fail: STOP takes ~10 us to complete on the
     * wire after HAL returns HAL_OK from the first read, so SR2.BUSY is
     * still 1 for that window and a fast-fail here would abort valid inits.
     * HAL's own 25 ms internal BUSY wait catches truly-stuck cases. */

    st = HAL_I2C_Mem_Read(hi2c, MT6701_I2C_ADDR, MT6701_REG_ANGLE_H + 1,
                          I2C_MEMADD_SIZE_8BIT, &lo_byte, 1,
                          MT6701_I2C_TIMEOUT_MS);
    if (st != HAL_OK) return st;

    if (raw_hi) *raw_hi = hi_byte;
    if (raw_lo) *raw_lo = lo_byte;
    *angle = (uint16_t)(((uint16_t)hi_byte << 6) | (lo_byte >> 2));
    return HAL_OK;
}

HAL_StatusTypeDef MT6701_ReadAngle(I2C_HandleTypeDef *hi2c, uint16_t *angle)
{
    return MT6701_ReadAngleRaw(hi2c, angle, NULL, NULL);
}

HAL_StatusTypeDef MT6701_StartReadIT(I2C_HandleTypeDef *hi2c, uint8_t *buf)
{
    if (hi2c->State != HAL_I2C_STATE_READY) return HAL_BUSY;
    if ((hi2c->Instance->SR2 & I2C_SR2_BUSY) != 0U) return HAL_BUSY;

    /* Single 2-byte read of registers 0x03 (angle MSB) and 0x04 (angle LSB).
     * DMA path: DMA controller moves the bytes, HAL only handles SB/ADDR/BTF
     * events. */
    return HAL_I2C_Mem_Read_DMA(hi2c, MT6701_I2C_ADDR, MT6701_REG_ANGLE_H,
                                I2C_MEMADD_SIZE_8BIT, buf, 2);
}

/* ==================================================================
 * Bare-metal I2C1 state-machine driver for MT6701 Y.
 *
 * Bypasses HAL entirely for the read path — no HAL_I2C_Mem_Read_DMA (which
 * blocks the caller ~200-500 us polling SB/ADDR/TXE/BTF flags before it
 * finally arms the DMA), no HAL callbacks, no main-loop recovery except a
 * lightweight watchdog for stuck transactions.
 *
 * Sequence (RM0008 §26.3.3 Master Receiver, case N=2):
 *
 *   Write phase (send memory address = 0x03):
 *     1. Set START. Wait SB.
 *     2. Write DR = slave_addr|W. Wait ADDR.
 *     3. Read SR2 to clear ADDR. Write DR = 0x03. Wait BTF.
 *     4. Set START (REPEAT — also clears BTF).
 *
 *   Read phase (N=2 pattern):
 *     5. Wait SB. Set POS=1, ACK=1. Write DR = slave_addr|R. Wait ADDR.
 *     6. Clear ACK, then read SR2 to clear ADDR — done atomically inside
 *        __disable_irq() so nothing delays the ACK->NACK transition and
 *        causes an unwanted third byte to get ACKed.
 *     7. Wait BTF. Set STOP. Read DR twice for the two bytes. Clear POS.
 *
 * Only ~6 EV interrupts per read, each < 5 us. Zero blocking inside TIM7.
 * ================================================================== */
/* State enum + shared includes — must be visible to BOTH the Y and X bare-
 * metal blocks below, so cannot live inside the `#if defined(MT6701_BAREMETAL_Y)`
 * guard. Anything else that both drivers need (buzzerTimer extern) goes here
 * too. Gated by "at least one of Y/X bare-metal is enabled" so we do not pull
 * util.h into the file when neither is on. */
#if (defined(MT6701_BAREMETAL_Y) && defined(ENCODER_Y)) || \
    (defined(MT6701_BAREMETAL_X) && defined(ENCODER_X))
#include "defines.h"
#include "util.h"
extern volatile uint32_t  buzzerTimer;

typedef enum {
    MT_BM_IDLE = 0,
    MT_BM_SB_W,     /* START issued, wait SB=1 */
    MT_BM_ADDR_W,   /* slave_addr|W sent, wait ADDR=1 */
    MT_BM_BTF_MEM,  /* memaddr sent, wait BTF=1 */
    MT_BM_SB_R,     /* REPEAT START, wait SB=1 */
    MT_BM_ADDR_R,   /* slave_addr|R sent (POS+ACK), wait ADDR=1 */
    MT_BM_BTF_DATA, /* ACK cleared + ADDR cleared, wait BTF=1 */
} mt6701_bm_state_t;
#endif

#if defined(MT6701_BAREMETAL_Y) && defined(ENCODER_Y)

extern I2C_HandleTypeDef hi2c1;
extern SensorState        encoder_y;
extern volatile uint32_t  mt6701_y_cplt_count;
extern volatile uint32_t  mt6701_y_err_count;
extern          uint8_t   mt6701_y_rxbuf[2];        /* moved out of static in util.c */
void            MT6701_Y_ProcessReading(void);      /* now non-static in util.c */

static volatile mt6701_bm_state_t bm_state       = MT_BM_IDLE;
static volatile uint32_t          bm_state_ts    = 0;
static volatile uint8_t           bm_needs_recover = 0;

/* Diagnostic counters — help tell if we ever leave IDLE or get stuck. */
volatile uint32_t mt6701_y_bm_state_max     = 0;
volatile uint32_t mt6701_y_bm_watchdog_hits = 0;
/* Per-state visit counters. A runaway IRQ loop shows up as one of these
 * counting way past the completion count (mt6701_y_cplt_count). Indices:
 *  [0]=IDLE (never should hit — case is default)
 *  [1]=SB_W  [2]=ADDR_W  [3]=BTF_MEM
 *  [4]=SB_R  [5]=ADDR_R  [6]=BTF_DATA
 *  [7]=default (spurious). */
volatile uint32_t mt6701_y_bm_state_visits[8] = {0};

/* Soft abort — the fast, common recovery path. Drain DR (clears BTF/RxNE),
 * send STOP, disable EV+ER IRQs, back to IDLE. Costs ~5 µs. Kick will
 * re-enable the IRQs when it starts the next transaction (~1 ms later).
 *
 * ITEVTEN is disabled here — even after DR drain, the peripheral can latch
 * a spurious flag (STOP completion, residual BTF from a byte that finishes
 * clocking in after the abort) that would trigger an EV IRQ with
 * state=IDLE. That falls to default and triggers a hard abort, throwing
 * away 25 ms every time. Keeping IRQs off between abort and next Kick
 * eliminates that class of spurious IRQ. */
static inline void bm_soft_abort(void)
{
    (void)I2C1->DR;
    (void)I2C1->DR;
    I2C1->CR1 |= I2C_CR1_STOP;
    I2C1->CR1 &= ~I2C_CR1_POS;
    I2C1->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
    bm_state = MT_BM_IDLE;
}

/* Hard abort — for cases the soft path cannot fix (stuck BUSY after several
 * kicks). Soft already disables IRQs; additionally flag main loop to do a
 * full I2C1_Init (SWRST + bit-bang + reconfigure). Kick re-enables the IRQs
 * on the first transaction after recovery. */
static inline void bm_hard_abort(void)
{
    bm_soft_abort();
    bm_needs_recover = 1;
}

void MT6701_Y_Bare_EV_IRQ(void)
{
    uint32_t sr1 = I2C1->SR1;

    /* Track worst-case state age for debugging. */
    if (bm_state > mt6701_y_bm_state_max) mt6701_y_bm_state_max = bm_state;
    if (bm_state < 8) mt6701_y_bm_state_visits[bm_state]++;

    /* Race guard: EV and ER IRQs share priority 2 — if the peripheral raised
     * an error at the same time as a normal event, EV can run first and
     * advance the state machine BEFORE the ER handler gets to abort.
     * Processing normal transitions with an error pending corrupts the
     * transaction; check for errors first and abort so the state machine
     * stays coherent. Count the error HERE: clearing the flags (and the
     * soft_abort disabling ITERREN) means the ER handler will never see it
     * — without this increment the stats silently underreport. ER's own
     * `if (!err) return` guarantees no double count. */
    if (sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR)) {
        I2C1->SR1 = (uint16_t)(sr1 & ~(I2C_SR1_BERR | I2C_SR1_ARLO
                                       | I2C_SR1_AF | I2C_SR1_OVR));
        bm_soft_abort();
        mt6701_y_err_count++;
        if (encoder_y.i2c_fail_count < 255) encoder_y.i2c_fail_count++;
        return;
    }

    switch (bm_state) {
    case MT_BM_SB_W:
        if (sr1 & I2C_SR1_SB) {
            I2C1->DR   = MT6701_I2C_ADDR;                       /* write bit = 0 */
            bm_state    = MT_BM_ADDR_W;
            bm_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_ADDR_W:
        if (sr1 & I2C_SR1_ADDR) {
            (void)I2C1->SR2;                                    /* clear ADDR */
            I2C1->DR   = MT6701_REG_ANGLE_H;                    /* 0x03 */
            bm_state    = MT_BM_BTF_MEM;
            bm_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_BTF_MEM:
        if (sr1 & I2C_SR1_BTF) {
            I2C1->CR1  |= I2C_CR1_START;                        /* REPEAT start */
            /* Writing START does NOT clear BTF — BTF only clears when the
             * START actually hits the wire (~2-5 µs later at 400 kHz) or on
             * a DR access. EV IRQ is level-triggered, so without this dummy
             * DR read it re-fires ~4x per read with state=SB_R and BTF=1
             * (measured: state_visits[SB_R] was exactly 4x the neighbours).
             * DR read in transmitter phase returns stale RX data, harmless. */
            (void)I2C1->DR;
            bm_state    = MT_BM_SB_R;
            bm_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_SB_R:
        if (sr1 & I2C_SR1_SB) {
            /* CRITICAL: only read SR1 (via switch prologue) then write DR.
             * Any other register access between the two — including a CR1
             * modify — prevents the peripheral from clearing SB on STM32F1
             * and traps EV IRQ in a loop. POS is set once in Kick (before
             * START); it is a no-op during the write phase per RM0008. */
            I2C1->DR    = MT6701_I2C_ADDR | 1U;
            bm_state    = MT_BM_ADDR_R;
            bm_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_ADDR_R:
        if (sr1 & I2C_SR1_ADDR) {
            /* N=2 pattern per AN2824. Sequence matters:
             *   1. Cli()
             *   2. Clear ACK (CR1 modify).
             *   3. Re-read SR1 then SR2 as an ADJACENT pair — the CR1 write
             *      above interposes between the prologue SR1 read and the SR2
             *      read, and STM32F1 needs a fresh SR1+SR2 read pair with no
             *      other peripheral access between to reliably clear ADDR.
             *   4. Sei()
             * The whole thing runs with IRQs off so nothing delays the
             * ACK->NACK transition (else the peripheral would clock in an
             * extra byte and ACK it). POS is already 1 from Kick. */
            __disable_irq();
            I2C1->CR1  &= ~I2C_CR1_ACK;
            (void)I2C1->SR1;
            (void)I2C1->SR2;
            __enable_irq();
            bm_state    = MT_BM_BTF_DATA;
            bm_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_BTF_DATA:
        if (sr1 & I2C_SR1_BTF) {
            /* Per RM0008: set STOP BEFORE reading DR (STOP is transmitted
             * after byte 2 clocks in from the shift register). */
            I2C1->CR1  |= I2C_CR1_STOP;
            mt6701_y_rxbuf[0] = (uint8_t)I2C1->DR;
            mt6701_y_rxbuf[1] = (uint8_t)I2C1->DR;
            I2C1->CR1  &= ~I2C_CR1_POS;                         /* leave the bit clean */
            /* INVARIANT: IRQs enabled <=> transaction in flight. Disabling
             * here (like soft_abort does) guarantees no EV/ER can fire while
             * state=IDLE — which (a) removes the spurious default-case hard
             * aborts from latched flags, and (b) makes the CR1/CR2
             * read-modify-writes in Kick (TIM7, lower priority) safe from
             * preemption by this handler. Kick re-enables on next start. */
            I2C1->CR2  &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
            mt6701_y_cplt_count++;
            bm_state = MT_BM_IDLE;
            MT6701_Y_ProcessReading();                          /* raw -> ENCODER_COUNT */
        }
        break;

    default:
        /* Spurious EV IRQ (state was IDLE — nothing to do). Belt-and-braces:
         * if this fires it means either a leftover flag or lost sync, both
         * indicate the peripheral is in a weird state we cannot fix from
         * here — hard abort and let main loop rebuild it. */
        I2C1->SR1 = (uint16_t)(sr1 & ~(I2C_SR1_BERR | I2C_SR1_ARLO
                                       | I2C_SR1_AF | I2C_SR1_OVR));
        (void)I2C1->SR2;
        bm_hard_abort();
        mt6701_y_err_count++;
        break;
    }
}

void MT6701_Y_Bare_ER_IRQ(void)
{
    uint32_t sr1 = I2C1->SR1;
    uint32_t err = sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR);
    if (!err) return;

    /* Clear error flags — SR1 error bits are RC_W0. */
    I2C1->SR1 = (uint16_t)(sr1 & ~err);

    /* Soft abort — 99% of errors under motor EMI are 1-2 corrupted bits on
     * the wire (AF/OVR/BERR). STOP + drain DR is enough; next TIM7 tick
     * kicks a fresh transaction ~1 ms later. Only if BUSY stays stuck does
     * Kick escalate to a hard abort + main-loop I2C1_Init (~25 ms). */
    bm_soft_abort();

    mt6701_y_err_count++;
    if (encoder_y.i2c_fail_count < 255) encoder_y.i2c_fail_count++;
}

/* Called from TIM7 IRQ (1 kHz). If IDLE, arm a new read; else check watchdog. */
void MT6701_Y_Bare_Kick(uint8_t *buf)
{
    if (!encoder_y.ini) return;
    if (bm_needs_recover) return;

    if (bm_state == MT_BM_IDLE) {
        /* BUSY=1 in IDLE typically clears in 1-2 ticks after a soft abort
         * (the STOP finishes transmitting on the wire). Count consecutive
         * stuck ticks; only escalate to hard recovery after ~5 ms — that
         * length means the F1 errata (2.14.7) latched BUSY and a full
         * I2C1_Init is the only way to shake it loose. Counter resets
         * once BUSY goes back to 0. */
        static uint8_t busy_stuck_ticks = 0;
        if ((I2C1->SR2 & I2C_SR2_BUSY) != 0U) {
            if (++busy_stuck_ticks > 5u) {
                busy_stuck_ticks = 0;
                bm_hard_abort();
            }
            return;
        }
        busy_stuck_ticks = 0;

        (void)buf;                                               /* rxbuf is a fixed global */
        /* Set ACK + POS up-front. POS only affects master-receive (per RM0008
         * §26.6.1), so it is a no-op during the write phase and takes effect
         * automatically on the read phase after the RESTART. Setting it here
         * lets SB_R touch ONLY DR — the "read SR1 -> write DR" pair required
         * to clear SB on STM32F1 must not have any other register access
         * between them (verified via post-mortem: a CR1 write there left SB
         * stuck and trapped EV IRQ in a level-triggered loop). */
        I2C1->CR1  = I2C1->CR1 | I2C_CR1_ACK | I2C_CR1_POS;
        /* Enable EVT + ERR IRQs. BUF disabled — we drive state on BTF only. */
        I2C1->CR2  = (I2C1->CR2 | I2C_CR2_ITEVTEN | I2C_CR2_ITERREN) & ~I2C_CR2_ITBUFEN;
        I2C1->CR1 |= I2C_CR1_START;
        bm_state    = MT_BM_SB_W;
        bm_state_ts = buzzerTimer;
        return;
    }

    /* Watchdog: transaction stuck > 20 ms. Flag main loop to do a full
     * peripheral re-init — we've hit a case bare-metal alone can't clear
     * (usually a hard bus lockup that needs SWRST + GPIO bit-bang). */
    if ((buzzerTimer - bm_state_ts) > 320u) {
        bm_needs_recover = 1;
        mt6701_y_bm_watchdog_hits++;
    }
}

uint8_t MT6701_Y_Bare_NeedsRecover(void) { return bm_needs_recover; }

void MT6701_Y_Bare_ResetState(void)
{
    bm_state         = MT_BM_IDLE;
    bm_needs_recover = 0;
    bm_state_ts      = buzzerTimer;
}

#endif  /* MT6701_BAREMETAL_Y */

/* ==================================================================
 * Bare-metal I2C2 state-machine driver for MT6701 X.
 *
 * Direct mirror of the Y driver above — same state machine, same sequence
 * (RM0008 §26.3.3 Master Receiver, case N=2), same soft/hard abort split.
 * Only the peripheral instance (I2C2 instead of I2C1) and the target
 * variables (encoder_x, mt6701_x_*) differ. Keep the two implementations
 * in sync: any bug fix on one side almost certainly applies to the other.
 * ================================================================== */
#if defined(MT6701_BAREMETAL_X) && defined(ENCODER_X)

/* Shared includes (defines.h, util.h, buzzerTimer, enum) live above the Y
 * block since both drivers need them. */

extern I2C_HandleTypeDef hi2c2;
extern SensorState        encoder_x;
extern volatile uint32_t  mt6701_x_cplt_count;
extern volatile uint32_t  mt6701_x_err_count;
extern          uint8_t   mt6701_x_rxbuf[2];
void            MT6701_X_ProcessReading(void);

static volatile mt6701_bm_state_t bmx_state         = MT_BM_IDLE;
static volatile uint32_t          bmx_state_ts      = 0;
static volatile uint8_t           bmx_needs_recover = 0;

volatile uint32_t mt6701_x_bm_state_max     = 0;
volatile uint32_t mt6701_x_bm_watchdog_hits = 0;
volatile uint32_t mt6701_x_bm_state_visits[8] = {0};

static inline void bmx_soft_abort(void)
{
    (void)I2C2->DR;
    (void)I2C2->DR;
    I2C2->CR1 |= I2C_CR1_STOP;
    I2C2->CR1 &= ~I2C_CR1_POS;
    I2C2->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
    bmx_state = MT_BM_IDLE;
}

static inline void bmx_hard_abort(void)
{
    bmx_soft_abort();
    bmx_needs_recover = 1;
}

void MT6701_X_Bare_EV_IRQ(void)
{
    uint32_t sr1 = I2C2->SR1;

    if (bmx_state > mt6701_x_bm_state_max) mt6701_x_bm_state_max = bmx_state;
    if (bmx_state < 8) mt6701_x_bm_state_visits[bmx_state]++;

    /* Error-first guard + counting — see the Y handler for full rationale. */
    if (sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR)) {
        I2C2->SR1 = (uint16_t)(sr1 & ~(I2C_SR1_BERR | I2C_SR1_ARLO
                                       | I2C_SR1_AF | I2C_SR1_OVR));
        bmx_soft_abort();
        mt6701_x_err_count++;
        if (encoder_x.i2c_fail_count < 255) encoder_x.i2c_fail_count++;
        return;
    }

    switch (bmx_state) {
    case MT_BM_SB_W:
        if (sr1 & I2C_SR1_SB) {
            I2C2->DR    = MT6701_I2C_ADDR;
            bmx_state    = MT_BM_ADDR_W;
            bmx_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_ADDR_W:
        if (sr1 & I2C_SR1_ADDR) {
            (void)I2C2->SR2;
            I2C2->DR    = MT6701_REG_ANGLE_H;
            bmx_state    = MT_BM_BTF_MEM;
            bmx_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_BTF_MEM:
        if (sr1 & I2C_SR1_BTF) {
            I2C2->CR1   |= I2C_CR1_START;
            /* Dummy DR read clears BTF — see the Y handler for why. */
            (void)I2C2->DR;
            bmx_state    = MT_BM_SB_R;
            bmx_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_SB_R:
        if (sr1 & I2C_SR1_SB) {
            I2C2->DR     = MT6701_I2C_ADDR | 1U;
            bmx_state    = MT_BM_ADDR_R;
            bmx_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_ADDR_R:
        if (sr1 & I2C_SR1_ADDR) {
            __disable_irq();
            I2C2->CR1   &= ~I2C_CR1_ACK;
            (void)I2C2->SR1;
            (void)I2C2->SR2;
            __enable_irq();
            bmx_state    = MT_BM_BTF_DATA;
            bmx_state_ts = buzzerTimer;
        }
        break;

    case MT_BM_BTF_DATA:
        if (sr1 & I2C_SR1_BTF) {
            I2C2->CR1   |= I2C_CR1_STOP;
            mt6701_x_rxbuf[0] = (uint8_t)I2C2->DR;
            mt6701_x_rxbuf[1] = (uint8_t)I2C2->DR;
            I2C2->CR1   &= ~I2C_CR1_POS;
            /* INVARIANT: IRQs enabled <=> transaction in flight — see Y. */
            I2C2->CR2   &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
            mt6701_x_cplt_count++;
            bmx_state = MT_BM_IDLE;
            MT6701_X_ProcessReading();
        }
        break;

    default:
        I2C2->SR1 = (uint16_t)(sr1 & ~(I2C_SR1_BERR | I2C_SR1_ARLO
                                       | I2C_SR1_AF | I2C_SR1_OVR));
        (void)I2C2->SR2;
        bmx_hard_abort();
        mt6701_x_err_count++;
        break;
    }
}

void MT6701_X_Bare_ER_IRQ(void)
{
    uint32_t sr1 = I2C2->SR1;
    uint32_t err = sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR);
    if (!err) return;

    I2C2->SR1 = (uint16_t)(sr1 & ~err);
    bmx_soft_abort();

    mt6701_x_err_count++;
    if (encoder_x.i2c_fail_count < 255) encoder_x.i2c_fail_count++;
}

void MT6701_X_Bare_Kick(uint8_t *buf)
{
    if (!encoder_x.ini) return;
    if (bmx_needs_recover) return;

    if (bmx_state == MT_BM_IDLE) {
        static uint8_t busy_stuck_ticks = 0;
        if ((I2C2->SR2 & I2C_SR2_BUSY) != 0U) {
            if (++busy_stuck_ticks > 5u) {
                busy_stuck_ticks = 0;
                bmx_hard_abort();
            }
            return;
        }
        busy_stuck_ticks = 0;

        (void)buf;
        I2C2->CR1  = I2C2->CR1 | I2C_CR1_ACK | I2C_CR1_POS;
        I2C2->CR2  = (I2C2->CR2 | I2C_CR2_ITEVTEN | I2C_CR2_ITERREN) & ~I2C_CR2_ITBUFEN;
        I2C2->CR1 |= I2C_CR1_START;
        bmx_state    = MT_BM_SB_W;
        bmx_state_ts = buzzerTimer;
        return;
    }

    if ((buzzerTimer - bmx_state_ts) > 320u) {
        bmx_needs_recover = 1;
        mt6701_x_bm_watchdog_hits++;
    }
}

uint8_t MT6701_X_Bare_NeedsRecover(void) { return bmx_needs_recover; }

void MT6701_X_Bare_ResetState(void)
{
    bmx_state         = MT_BM_IDLE;
    bmx_needs_recover = 0;
    bmx_state_ts      = buzzerTimer;
}

#endif  /* MT6701_BAREMETAL_X */

#include "mt6701_pwm.h"

/* ---------------------------------------------------------------------
 * MT6701 PWM-mode input-capture drivers.
 *   X: PB10 -> TIM2_CH3 (rising) + TIM2_CH4 (falling, indirect on TI3).
 *   Y: PB6  -> TIM4_CH1 (rising) + TIM4_CH2 (falling, indirect on TI1).
 *
 * Both capture BOTH edges of one input pin (the standard STM32 way to
 * measure pulse width on a non-CH1/CH2 or CH1/CH2 pin): one channel takes
 * its own TIx directly, the paired channel takes the same TIx indirectly.
 *
 * Framed 12-bit decode (MT6701 datasheet Figure 16). The PWM frame is
 * 4119 PWM clocks:
 *     [ 16 clocks HIGH  (start pattern) ]
 *     [ 4096 clocks     (12-bit angle data window) ]
 *     [ 8 clocks LOW    (end pattern) ]
 * The measured HIGH pulse spans the start pattern plus the leading `angle`
 * data clocks:  t_high = (16 + angle_12bit) clocks. So:
 *     high_clocks = t_high * 4119 / t_period
 *     angle_12bit = high_clocks - 16               (0..4095)
 * Resolution is a genuine 12 bits (4096 steps/rev). angle_12bit is used
 * directly as the count value — the whole firmware now runs at
 * ENCODER_?_CPR = MT6701_CPR = 4096, one consistent scale, no fake bits.
 *
 * Timer: TIM2/TIM4 free-run at 54 MHz (APB1-timer 108 MHz / PSC 2),
 * ARR=0xFFFF. One frame ~= 54320 ticks, inside 16 bits. 16-bit modular
 * subtraction absorbs the CNT rollover (period < 65536 ticks).
 *
 * FLAG HANDLING: the interrupt flags are cleared EXPLICITLY by writing SR
 * at the end of each handler. Relying on the "reading CCRx clears CCxIF"
 * side effect alone once left a flag asserted and self-retriggered the ISR
 * into a boot-hanging storm — never again.
 * --------------------------------------------------------------------- */

#if defined(MT6701_MODE_PWM_X) || defined(MT6701_MODE_PWM_Y)

#define MT6701_PWM_TIMER_PSC       (2u - 1u)   /* divide by (PSC+1) = 2 */
#define MT6701_PWM_PERIOD_MIN      45000u      /* ~83% of 54320 nominal */
#define MT6701_PWM_PERIOD_MAX      65000u      /* ~120%, stays < 65536 */
#define MT6701_PWM_FRAME_CLOCKS    4119u       /* total clocks per frame */
#define MT6701_PWM_START_CLOCKS    16u         /* always-high start pattern */
#define MT6701_PWM_ANGLE_MAX       4095u       /* 12-bit data max */

/* Slew guard: physically-impossible jump per frame. At N_MOT_MAX=200 rpm the
 * shaft moves ~14 counts per 994 Hz frame; 512 counts (1/8 rev, ~7400 rpm)
 * is far above any real motion, so a bigger jump is a capture hiccup, not
 * the rotor. Escape after N rejects so a genuine relocation isn't wedged. */
#define MT6701_PWM_SLEW_MAX   512
#define MT6701_PWM_SLEW_ESC   4

/* Shared framed decode. Returns native 12-bit angle (0..4095), or -1 if the
 * period is out of range or pulse exceeds period (missed edge / boundary). */
static inline int32_t mt6701_pwm_decode(uint16_t pulse, uint16_t period)
{
    if (period < MT6701_PWM_PERIOD_MIN || period > MT6701_PWM_PERIOD_MAX) return -1;
    if (pulse > period) return -1;
    uint32_t high_clocks = ((uint32_t)pulse * MT6701_PWM_FRAME_CLOCKS) / period;
    int32_t angle12 = (int32_t)high_clocks - (int32_t)MT6701_PWM_START_CLOCKS;
    if (angle12 < 0) angle12 = 0;
    if (angle12 > (int32_t)MT6701_PWM_ANGLE_MAX) angle12 = (int32_t)MT6701_PWM_ANGLE_MAX;
    return angle12;
}

#endif  /* PWM_X || PWM_Y */

/* =====================================================================
 * X axis — PB10 / TIM2_CH3 (rising) + TIM2_CH4 (falling, indirect TI3).
 * ===================================================================== */
#if defined(MT6701_MODE_PWM_X)

extern uint8_t mt6701_x_rxbuf[2];
void MT6701_X_ProcessReading(void);

static volatile uint16_t sx_prev_rising  = 0;
static volatile uint8_t  sx_have_prev    = 0;
static volatile uint16_t sx_last_period  = 0;
static volatile uint8_t  sx_period_valid = 0;
static volatile uint16_t sx_last_pulse   = 0;
static volatile uint16_t sx_raw_angle    = 0;
static volatile uint32_t sx_frame_count  = 0;
static volatile uint32_t sx_err_count    = 0;
static volatile uint32_t sx_irq_count    = 0;
static volatile uint32_t sx_slew_count   = 0;   /* readings rejected by slew guard */
static          uint8_t  sx_slew_streak  = 0;
static          uint8_t  sx_seeded       = 0;

static inline void x_apply_rising(uint16_t r)
{
    if (sx_have_prev) {
        uint16_t period = (uint16_t)(r - sx_prev_rising);
        if (period >= MT6701_PWM_PERIOD_MIN && period <= MT6701_PWM_PERIOD_MAX) {
            sx_last_period = period; sx_period_valid = 1;
        } else { sx_period_valid = 0; sx_err_count++; }
    }
    sx_prev_rising = r; sx_have_prev = 1;
}
static inline void x_apply_falling(uint16_t f)
{
    if (sx_have_prev && sx_period_valid) {
        uint16_t pulse = (uint16_t)(f - sx_prev_rising);
        int32_t a = mt6701_pwm_decode(pulse, sx_last_period);
        if (a < 0) { sx_err_count++; return; }
        /* Slew guard: reject a jump > 1/8 rev (shortest path on the 4096
         * circle, so the 4095<->0 wrap reads as ~0). Escape after a few
         * rejects so a genuine relocation isn't wedged forever. */
        if (sx_seeded) {
            int32_t d = a - (int32_t)sx_raw_angle;
            if (d < -2048) d += 4096; else if (d > 2048) d -= 4096;
            if (d < 0) d = -d;
            if (d > MT6701_PWM_SLEW_MAX && sx_slew_streak < MT6701_PWM_SLEW_ESC) {
                sx_slew_streak++; sx_slew_count++; return;
            }
        }
        sx_slew_streak = 0; sx_seeded = 1;
        sx_last_pulse = pulse; sx_raw_angle = (uint16_t)a; sx_frame_count++;
    }
}

void MT6701_X_Pwm_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* PB10 input pull-down (CRH pin10 = 0x8; BRR selects pull-down). */
    GPIOB->CRH = (GPIOB->CRH & ~(0xFu << 8)) | (0x8u << 8);
    GPIOB->BRR = GPIO_PIN_10;

    /* TIM2 partial-remap-2 => CH3=PB10, CH4=PB11 (bits [9:8]=10). */
    AFIO->MAPR = (AFIO->MAPR & ~(0x3u << 8)) | (0x2u << 8);

    __HAL_RCC_TIM2_CLK_ENABLE();
    TIM2->CR1  = 0;
    TIM2->PSC  = MT6701_PWM_TIMER_PSC;
    TIM2->ARR  = 0xFFFFu;
    TIM2->CNT  = 0;
    TIM2->EGR  = TIM_EGR_UG;

    /* CCMR2: CC3S=01(TI3) IC3F=3, CC4S=10(TI3) IC4F=3.
     * IC filter = 3 => ~148 ns (fCK_INT, N=8). Must stay well BELOW the
     * shortest real pulse/gap or the frame extremes break: at angle 0 the
     * HIGH pulse is only 16 PWM clocks (~3.9 us) and at angle 4095 the LOW
     * end-pattern gap is 8 clocks (~2 us). The old 4.7 us filter (IC=15)
     * ate those, corrupting readings exactly at the 0<->4095 wrap. 148 ns
     * still rejects sub-100 ns edge ringing on the clean push-pull line. */
    TIM2->CCER  = 0;
    TIM2->CCMR2 = (0x01u << 0) | (0x03u << 4)
                | (0x02u << 8) | (0x03u << 12);
    /* CC3 rising, CC4 falling, both enabled. */
    TIM2->CCER  = TIM_CCER_CC3E | TIM_CCER_CC4E | TIM_CCER_CC4P;
    TIM2->SR    = 0;
    TIM2->DIER  = TIM_DIER_CC3IE | TIM_DIER_CC4IE;
    TIM2->CR1   = TIM_CR1_CEN;

    HAL_NVIC_SetPriority(TIM2_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void MT6701_X_Pwm_IRQ(void)
{
    uint32_t sr = TIM2->SR;
    sx_irq_count++;
    uint8_t got_r = (sr & TIM_SR_CC3IF) != 0;
    uint8_t got_f = (sr & TIM_SR_CC4IF) != 0;
    /* Read a CCR ONLY when its flag is set. The read clears CCxIF for the
     * edge we consume; an edge arriving mid-ISR keeps its flag for the next
     * ISR. A blanket "clear all" at the end would instead wipe a CCxIF set
     * after we sampled `sr`, dropping that edge and leaving prev_rising one
     * frame stale — the source of the intermittent ~780-count-low glitches. */
    uint16_t r = got_r ? (uint16_t)TIM2->CCR3 : 0;   /* rising  */
    uint16_t f = got_f ? (uint16_t)TIM2->CCR4 : 0;   /* falling */

    /* Both edges coalesced (frame extremes / preemption): apply in
     * chronological order so each falling pairs with its frame's rising. */
    if (got_r && got_f) {
        if ((uint16_t)(f - r) <= (uint16_t)(r - f)) { x_apply_rising(r); x_apply_falling(f); }
        else                                        { x_apply_falling(f); x_apply_rising(r); }
    } else if (got_r) {
        x_apply_rising(r);
    } else if (got_f) {
        x_apply_falling(f);
    }
    /* Clear exactly the CCxIF bits we OBSERVED in `sr` (storm-safe: never
     * leave a flag we already saw), plus overcapture. A flag that arrived
     * mid-ISR is NOT in `sr`, so this write preserves it (rc_w0: writing 1
     * keeps it) and the next ISR handles it — no dropped edges. */
    TIM2->SR = (uint16_t)~((sr & (TIM_SR_CC3IF | TIM_SR_CC4IF))
                           | TIM_SR_CC3OF | TIM_SR_CC4OF);
}

void MT6701_X_Pwm_Feed(void)
{
    uint16_t raw = sx_raw_angle;                 /* 0..4095 native 12-bit */
    mt6701_x_rxbuf[0] = (uint8_t)(raw >> 6);
    mt6701_x_rxbuf[1] = (uint8_t)((raw << 2) & 0xFCu);
    MT6701_X_ProcessReading();
}

uint16_t MT6701_X_Pwm_GetRawAngle(void)         { return sx_raw_angle; }
uint32_t MT6701_X_Pwm_GetFrameCount(void)       { return sx_frame_count; }
uint32_t MT6701_X_Pwm_GetErrCount(void)         { return sx_err_count; }
uint32_t MT6701_X_Pwm_GetSlewCount(void)        { return sx_slew_count; }
uint32_t MT6701_X_Pwm_GetIrqCount(void)         { return sx_irq_count; }
uint16_t MT6701_X_Pwm_GetLastPeriodTicks(void)  { return sx_last_period; }
uint16_t MT6701_X_Pwm_GetLastPulseTicks(void)   { return sx_last_pulse; }

#endif  /* MT6701_MODE_PWM_X */

/* =====================================================================
 * Y axis — PB6 / TIM4_CH1 (rising) + TIM4_CH2 (falling, indirect TI1).
 * TIM4 default mapping already puts CH1=PB6, CH2=PB7 — no AFIO remap.
 * ===================================================================== */
#if defined(MT6701_MODE_PWM_Y)

extern uint8_t mt6701_y_rxbuf[2];
void MT6701_Y_ProcessReading(void);

static volatile uint16_t sy_prev_rising  = 0;
static volatile uint8_t  sy_have_prev    = 0;
static volatile uint16_t sy_last_period  = 0;
static volatile uint8_t  sy_period_valid = 0;
static volatile uint16_t sy_last_pulse   = 0;
static volatile uint16_t sy_raw_angle    = 0;
static volatile uint32_t sy_frame_count  = 0;
static volatile uint32_t sy_err_count    = 0;
static volatile uint32_t sy_irq_count    = 0;
static volatile uint32_t sy_slew_count   = 0;
static          uint8_t  sy_slew_streak  = 0;
static          uint8_t  sy_seeded       = 0;

static inline void y_apply_rising(uint16_t r)
{
    if (sy_have_prev) {
        uint16_t period = (uint16_t)(r - sy_prev_rising);
        if (period >= MT6701_PWM_PERIOD_MIN && period <= MT6701_PWM_PERIOD_MAX) {
            sy_last_period = period; sy_period_valid = 1;
        } else { sy_period_valid = 0; sy_err_count++; }
    }
    sy_prev_rising = r; sy_have_prev = 1;
}
static inline void y_apply_falling(uint16_t f)
{
    if (sy_have_prev && sy_period_valid) {
        uint16_t pulse = (uint16_t)(f - sy_prev_rising);
        int32_t a = mt6701_pwm_decode(pulse, sy_last_period);
        if (a < 0) { sy_err_count++; return; }
        if (sy_seeded) {
            int32_t d = a - (int32_t)sy_raw_angle;
            if (d < -2048) d += 4096; else if (d > 2048) d -= 4096;
            if (d < 0) d = -d;
            if (d > MT6701_PWM_SLEW_MAX && sy_slew_streak < MT6701_PWM_SLEW_ESC) {
                sy_slew_streak++; sy_slew_count++; return;
            }
        }
        sy_slew_streak = 0; sy_seeded = 1;
        sy_last_pulse = pulse; sy_raw_angle = (uint16_t)a; sy_frame_count++;
    }
}

void MT6701_Y_Pwm_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* PB6 input pull-down (CRL pin6 bits [27:24] = 0x8; BRR selects PD). */
    GPIOB->CRL = (GPIOB->CRL & ~(0xFu << 24)) | (0x8u << 24);
    GPIOB->BRR = GPIO_PIN_6;

    __HAL_RCC_TIM4_CLK_ENABLE();
    TIM4->CR1  = 0;
    TIM4->PSC  = MT6701_PWM_TIMER_PSC;
    TIM4->ARR  = 0xFFFFu;
    TIM4->CNT  = 0;
    TIM4->EGR  = TIM_EGR_UG;

    /* CCMR1: CC1S=01(TI1) IC1F=3, CC2S=10(TI1) IC2F=3.
     * IC filter ~148 ns — see the X init for why it must stay well below
     * the ~2 us minimum pulse/gap at the frame extremes. */
    TIM4->CCER  = 0;
    TIM4->CCMR1 = (0x01u << 0) | (0x03u << 4)
                | (0x02u << 8) | (0x03u << 12);
    /* CC1 rising, CC2 falling, both enabled. */
    TIM4->CCER  = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC2P;
    TIM4->SR    = 0;
    TIM4->DIER  = TIM_DIER_CC1IE | TIM_DIER_CC2IE;
    TIM4->CR1   = TIM_CR1_CEN;

    HAL_NVIC_SetPriority(TIM4_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

void MT6701_Y_Pwm_IRQ(void)
{
    uint32_t sr = TIM4->SR;
    sy_irq_count++;
    uint8_t got_r = (sr & TIM_SR_CC1IF) != 0;
    uint8_t got_f = (sr & TIM_SR_CC2IF) != 0;
    /* Conditional CCR read + overcapture-only clear — see MT6701_X_Pwm_IRQ
     * for the full rationale (avoids dropping mid-ISR edges that left
     * prev_rising stale and produced ~780-count-low glitches). */
    uint16_t r = got_r ? (uint16_t)TIM4->CCR1 : 0;
    uint16_t f = got_f ? (uint16_t)TIM4->CCR2 : 0;

    if (got_r && got_f) {
        if ((uint16_t)(f - r) <= (uint16_t)(r - f)) { y_apply_rising(r); y_apply_falling(f); }
        else                                        { y_apply_falling(f); y_apply_rising(r); }
    } else if (got_r) {
        y_apply_rising(r);
    } else if (got_f) {
        y_apply_falling(f);
    }
    /* Clear observed CCxIF + overcapture; preserve mid-ISR arrivals. See
     * MT6701_X_Pwm_IRQ for the full rationale (storm-safe, drop-free). */
    TIM4->SR = (uint16_t)~((sr & (TIM_SR_CC1IF | TIM_SR_CC2IF))
                           | TIM_SR_CC1OF | TIM_SR_CC2OF);
}

void MT6701_Y_Pwm_Feed(void)
{
    uint16_t raw = sy_raw_angle;                 /* 0..4095 native 12-bit */
    mt6701_y_rxbuf[0] = (uint8_t)(raw >> 6);
    mt6701_y_rxbuf[1] = (uint8_t)((raw << 2) & 0xFCu);
    MT6701_Y_ProcessReading();
}

uint16_t MT6701_Y_Pwm_GetRawAngle(void)         { return sy_raw_angle; }
uint32_t MT6701_Y_Pwm_GetFrameCount(void)       { return sy_frame_count; }
uint32_t MT6701_Y_Pwm_GetErrCount(void)         { return sy_err_count; }
uint32_t MT6701_Y_Pwm_GetSlewCount(void)        { return sy_slew_count; }
uint32_t MT6701_Y_Pwm_GetIrqCount(void)         { return sy_irq_count; }
uint16_t MT6701_Y_Pwm_GetLastPeriodTicks(void)  { return sy_last_period; }
uint16_t MT6701_Y_Pwm_GetLastPulseTicks(void)   { return sy_last_pulse; }

#endif  /* MT6701_MODE_PWM_Y */

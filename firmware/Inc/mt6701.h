#ifndef MT6701_H
#define MT6701_H

#include "stm32f1xx_hal.h"
#include "config.h"
#include <stdint.h>

#define MT6701_I2C_ADDR_7BIT    0x06
#define MT6701_I2C_ADDR         (MT6701_I2C_ADDR_7BIT << 1)
#define MT6701_REG_ANGLE_H      0x03
/* Effective resolution used for the count scale. PWM output mode delivers
 * 12 bits (4096 steps/rev); the whole firmware runs at that scale. If an
 * axis is reverted to I2C (14-bit), set this back to 14 and ENCODER_CPR /
 * ENCODER_?_PPR in config.h back to the 16384 scale. */
#define MT6701_RESOLUTION_BITS  12
#define MT6701_CPR              (1u << MT6701_RESOLUTION_BITS)   // 4096

/* Bounded timeout for HAL_I2C_Mem_Read internal flag waits. */
#define MT6701_I2C_TIMEOUT_MS   1u

/* Blocking readouts — only used by Encoder_?_Init at boot. */
HAL_StatusTypeDef MT6701_ReadAngle(I2C_HandleTypeDef *hi2c, uint16_t *angle);
HAL_StatusTypeDef MT6701_ReadAngleRaw(I2C_HandleTypeDef *hi2c, uint16_t *angle,
                                      uint8_t *raw_hi, uint8_t *raw_lo);

/* Non-blocking DMA-mode start: kicks off a 2-byte read from register 0x03.
 * Completion via HAL_I2C_MemRxCpltCallback (weak, override in util.c).
 * Errors via HAL_I2C_ErrorCallback. Returns HAL_BUSY fast if bus stuck. */
HAL_StatusTypeDef MT6701_StartReadIT(I2C_HandleTypeDef *hi2c, uint8_t *buf);

/* Kick a new read from TIM7 IRQ. Guards on busy/recovery internally. */
#ifdef ENCODER_X
void MT6701_X_KickIT(void);
#endif
#ifdef ENCODER_Y
void MT6701_Y_KickIT(void);
#endif

/* ------------------------------------------------------------------
 * Bare-metal I2C1 state-machine driver for MT6701 on Y.
 * Enabled by MT6701_BAREMETAL_Y in config.h.
 *
 * Contract:
 *   - I2C1_EV_IRQHandler / I2C1_ER_IRQHandler in stm32f1xx_it.c must call
 *     MT6701_Y_Bare_EV_IRQ / MT6701_Y_Bare_ER_IRQ (bypass HAL entirely).
 *   - TIM7 IRQ (or any 1 kHz tick) calls MT6701_Y_Bare_Kick(buf).
 *   - The driver writes results into encoder_y directly (raw_angle,
 *     ENCODER_COUNT, full_rotations) via MT6701_Y_ProcessReading in util.c.
 *   - If a transaction hangs for >20 ms, MT6701_Y_Bare_NeedsRecover() returns
 *     non-zero and the main loop should call I2C1_Init() + Bare_ResetState().
 * ------------------------------------------------------------------ */
#if defined(MT6701_BAREMETAL_Y) && defined(ENCODER_Y)
void    MT6701_Y_Bare_EV_IRQ(void);
void    MT6701_Y_Bare_ER_IRQ(void);
void    MT6701_Y_Bare_Kick(uint8_t *buf);
uint8_t MT6701_Y_Bare_NeedsRecover(void);
void    MT6701_Y_Bare_ResetState(void);
#endif

/* Same shape for X on I2C2. See Y contract above. */
#if defined(MT6701_BAREMETAL_X) && defined(ENCODER_X)
void    MT6701_X_Bare_EV_IRQ(void);
void    MT6701_X_Bare_ER_IRQ(void);
void    MT6701_X_Bare_Kick(uint8_t *buf);
uint8_t MT6701_X_Bare_NeedsRecover(void);
void    MT6701_X_Bare_ResetState(void);
#endif

#endif

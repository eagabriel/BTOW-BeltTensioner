#ifndef MT6701_PWM_H
#define MT6701_PWM_H

#include "stm32f1xx_hal.h"
#include "config.h"
#include <stdint.h>

/* ---------------------------------------------------------------------
 * MT6701 PWM-mode input-capture drivers.
 *   X: PB10 via TIM2_CH3 (rising) + TIM2_CH4 (falling, indirect TI3).
 *   Y: PB6  via TIM4_CH1 (rising) + TIM4_CH2 (falling, indirect TI1).
 *
 * Contract:
 *   - Each MT6701 is pre-configured for PWM output (994.4 Hz), done once
 *     with the mt6701-programmer bench tool; the sensor comes up in PWM
 *     with no I2C init required. Wire the sensor's OUT pin directly to the
 *     capture pin above.
 *   - Angle is the native 12-bit value (0..4095) decoded from the framed
 *     PWM (16-clock start + 4096-clock data + 8-clock end, 4119 total).
 *
 * Diagnostics counters are exposed so the main loop can print them.
 * --------------------------------------------------------------------- */

#if defined(MT6701_MODE_PWM_X)
void     MT6701_X_Pwm_Init(void);            /* set up TIM2 capture on PB10 */
void     MT6701_X_Pwm_IRQ(void);             /* from TIM2_IRQHandler only */
void     MT6701_X_Pwm_Feed(void);            /* copy angle -> encoder_x (1 kHz) */
uint16_t MT6701_X_Pwm_GetRawAngle(void);     /* native 12-bit, 0..4095 */
uint32_t MT6701_X_Pwm_GetFrameCount(void);
uint32_t MT6701_X_Pwm_GetErrCount(void);
uint32_t MT6701_X_Pwm_GetSlewCount(void);    /* readings rejected as impossible jumps */
uint32_t MT6701_X_Pwm_GetIrqCount(void);     /* ~1988/s on a clean signal */
uint16_t MT6701_X_Pwm_GetLastPeriodTicks(void);  /* expect ~54320 */
uint16_t MT6701_X_Pwm_GetLastPulseTicks(void);
#endif

#if defined(MT6701_MODE_PWM_Y)
void     MT6701_Y_Pwm_Init(void);            /* set up TIM4 capture on PB6 */
void     MT6701_Y_Pwm_IRQ(void);             /* from TIM4_IRQHandler only */
void     MT6701_Y_Pwm_Feed(void);            /* copy angle -> encoder_y (1 kHz) */
uint16_t MT6701_Y_Pwm_GetRawAngle(void);     /* native 12-bit, 0..4095 */
uint32_t MT6701_Y_Pwm_GetFrameCount(void);
uint32_t MT6701_Y_Pwm_GetErrCount(void);
uint32_t MT6701_Y_Pwm_GetSlewCount(void);
uint32_t MT6701_Y_Pwm_GetIrqCount(void);
uint16_t MT6701_Y_Pwm_GetLastPeriodTicks(void);
uint16_t MT6701_Y_Pwm_GetLastPulseTicks(void);
#endif

#endif  /* MT6701_PWM_H */

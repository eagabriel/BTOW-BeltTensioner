/*
* This file is part of the hoverboard-firmware-hack project.
*
* Copyright (C) 2017-2018 Rene Hopf <renehopf@mac.com>
* Copyright (C) 2017-2018 Nico Stute <crinq@crinq.de>
* Copyright (C) 2017-2018 Niklas Fauth <niklas.fauth@kit.fail>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Define to prevent recursive inclusion
#ifndef DEFINES_H
#define DEFINES_H

#include "stm32f1xx_hal.h"
#include "config.h"
#include "rtwtypes.h" 

#define LEFT_HALL_U_PIN GPIO_PIN_5
#define LEFT_HALL_V_PIN GPIO_PIN_6
#define LEFT_HALL_W_PIN GPIO_PIN_7

#define LEFT_HALL_U_PORT GPIOB
#define LEFT_HALL_V_PORT GPIOB
#define LEFT_HALL_W_PORT GPIOB

#define RIGHT_HALL_U_PIN GPIO_PIN_10
#define RIGHT_HALL_V_PIN GPIO_PIN_11
#define RIGHT_HALL_W_PIN GPIO_PIN_12

#define RIGHT_HALL_U_PORT GPIOC
#define RIGHT_HALL_V_PORT GPIOC
#define RIGHT_HALL_W_PORT GPIOC

#define LEFT_TIM TIM8
#define LEFT_TIM_U CCR1
#define LEFT_TIM_UH_PIN GPIO_PIN_6
#define LEFT_TIM_UH_PORT GPIOC
#define LEFT_TIM_UL_PIN GPIO_PIN_7
#define LEFT_TIM_UL_PORT GPIOA
#define LEFT_TIM_V CCR2
#define LEFT_TIM_VH_PIN GPIO_PIN_7
#define LEFT_TIM_VH_PORT GPIOC
#define LEFT_TIM_VL_PIN GPIO_PIN_0
#define LEFT_TIM_VL_PORT GPIOB
#define LEFT_TIM_W CCR3
#define LEFT_TIM_WH_PIN GPIO_PIN_8
#define LEFT_TIM_WH_PORT GPIOC
#define LEFT_TIM_WL_PIN GPIO_PIN_1
#define LEFT_TIM_WL_PORT GPIOB

#define RIGHT_TIM TIM1
#define RIGHT_TIM_U CCR1
#define RIGHT_TIM_UH_PIN GPIO_PIN_8
#define RIGHT_TIM_UH_PORT GPIOA
#define RIGHT_TIM_UL_PIN GPIO_PIN_13
#define RIGHT_TIM_UL_PORT GPIOB
#define RIGHT_TIM_V CCR2
#define RIGHT_TIM_VH_PIN GPIO_PIN_9
#define RIGHT_TIM_VH_PORT GPIOA
#define RIGHT_TIM_VL_PIN GPIO_PIN_14
#define RIGHT_TIM_VL_PORT GPIOB
#define RIGHT_TIM_W CCR3
#define RIGHT_TIM_WH_PIN GPIO_PIN_10
#define RIGHT_TIM_WH_PORT GPIOA
#define RIGHT_TIM_WL_PIN GPIO_PIN_15
#define RIGHT_TIM_WL_PORT GPIOB

// #define LEFT_DC_CUR_ADC ADC1
// #define LEFT_U_CUR_ADC ADC1
// #define LEFT_V_CUR_ADC ADC1

#define LEFT_DC_CUR_PIN GPIO_PIN_0
#define LEFT_U_CUR_PIN GPIO_PIN_0
#define LEFT_V_CUR_PIN GPIO_PIN_3

#define LEFT_DC_CUR_PORT GPIOC
#define LEFT_U_CUR_PORT GPIOA
#define LEFT_V_CUR_PORT GPIOC

// #define RIGHT_DC_CUR_ADC ADC2
// #define RIGHT_U_CUR_ADC ADC2
// #define RIGHT_V_CUR_ADC ADC2

#define RIGHT_DC_CUR_PIN GPIO_PIN_1
#define RIGHT_U_CUR_PIN GPIO_PIN_4
#define RIGHT_V_CUR_PIN GPIO_PIN_5

#define RIGHT_DC_CUR_PORT GPIOC
#define RIGHT_U_CUR_PORT GPIOC
#define RIGHT_V_CUR_PORT GPIOC

// #define DCLINK_ADC ADC3
// #define DCLINK_CHANNEL
#ifdef ENCODER_Y
#define ENCODER_Y_CPR              (ENCODER_Y_PPR * 4)
#define ENCODER_Y_TIMER            TIM3
#define ENCODER_Y_CHA_PORT         GPIOB
#define ENCODER_Y_CHB_PORT         GPIOB
#define ENCODER_Y_CHA_PIN          GPIO_PIN_4
#define ENCODER_Y_CHB_PIN          GPIO_PIN_5
#endif

#ifdef ENCODER_X
#define ENCODER_X_CPR              (ENCODER_X_PPR * 4)
#define ENCODER_X_TIMER            TIM4
#define ENCODER_X_CHA_PORT         GPIOB
#define ENCODER_X_CHB_PORT         GPIOB
#define ENCODER_X_CHA_PIN          GPIO_PIN_7
#define ENCODER_X_CHB_PIN          GPIO_PIN_6
#endif

#if BOARD_VARIANT == 0
#define DCLINK_PIN GPIO_PIN_2
#define DCLINK_PORT GPIOC
#define DCLINK_ADC_CHANNEL ADC_CHANNEL_12
#elif BOARD_VARIANT == 1
#define DCLINK_PIN GPIO_PIN_1
#define DCLINK_PORT GPIOA
#define DCLINK_ADC_CHANNEL ADC_CHANNEL_1
#endif

// #define DCLINK_PULLUP 30000
// #define DCLINK_PULLDOWN 1000

#define LED_PIN GPIO_PIN_2
#define LED_PORT GPIOB

#if BOARD_VARIANT == 0
#define BUZZER_PIN GPIO_PIN_4
#define BUZZER_PORT GPIOA
#elif BOARD_VARIANT == 1
#define BUZZER_PIN GPIO_PIN_13
#define BUZZER_PORT GPIOC
#endif

// UNUSED/REDUNDANT
//#define SWITCH_PIN GPIO_PIN_1
//#define SWITCH_PORT GPIOA

#if BOARD_VARIANT == 0
#define OFF_PIN GPIO_PIN_5
#define OFF_PORT GPIOA
#elif BOARD_VARIANT == 1
#define OFF_PIN GPIO_PIN_15
#define OFF_PORT GPIOC
#endif

#if BOARD_VARIANT == 0
#define BUTTON_PIN GPIO_PIN_1
#define BUTTON_PORT GPIOA
#elif BOARD_VARIANT == 1
#define BUTTON_PIN GPIO_PIN_9
#define BUTTON_PORT GPIOB
#endif
#if defined(ANALOG_BUTTON)
  #define BUTTON_ADC_CHANNEL        ADC_CHANNEL_1
#endif

#if BOARD_VARIANT == 0
#define CHARGER_PIN GPIO_PIN_12
#define CHARGER_PORT GPIOA
#elif BOARD_VARIANT == 1
#define CHARGER_PIN GPIO_PIN_11
#define CHARGER_PORT GPIOA
#endif

#if defined(CONTROL_PPM_LEFT)
#define PPM_PIN             GPIO_PIN_3
#define PPM_PORT            GPIOA
#elif defined(CONTROL_PPM_RIGHT)
#define PPM_PIN             GPIO_PIN_11
#define PPM_PORT            GPIOB
#endif

#if defined(RC_PWM_LEFT)
#define PWM_PIN_CH1         GPIO_PIN_2
#define PWM_PORT_CH1        GPIOA
#define PWM_PIN_CH2         GPIO_PIN_3
#define PWM_PORT_CH2        GPIOA
#elif defined(RC_PWM_RIGHT)
#define PWM_PIN_CH1         GPIO_PIN_10
#define PWM_PORT_CH1        GPIOB
#define PWM_PIN_CH2         GPIO_PIN_11
#define PWM_PORT_CH2        GPIOB
#endif

#if defined(SW_PWM_LEFT) 
#define PWM_PIN_CH1         GPIO_PIN_2
#define PWM_PORT_CH1        GPIOA
#define PWM_PIN_CH2         GPIO_PIN_3
#define PWM_PORT_CH2        GPIOA
#elif defined(SW_PWM_RIGHT)
#define PWM_PIN_CH1         GPIO_PIN_10
#define PWM_PORT_CH1        GPIOB
#define PWM_PIN_CH2         GPIO_PIN_11
#define PWM_PORT_CH2        GPIOB
#endif

#if defined(HW_PWM)
#define PWM_PIN_CH2         GPIO_PIN_5
#define PWM_PORT_CH2        GPIOB
#endif

// External brake resistor PWM configuration
// Select which channel to use for the external brake resistor PWM
// Change Timer and pins if using analog input
#if defined(EXTBRK_USE_CH3)
#define EXTBRK_TIM             TIM5
#define EXTBRK_TIM_CHANNEL     TIM_CHANNEL_3
#define EXTBRK_PIN             GPIO_PIN_2
#define EXTBRK_PORT            GPIOA
#define EXT_PWM_BRK            TIM5->CCR3
#elif defined(EXTBRK_USE_CH4)
#define EXTBRK_TIM             TIM5
#define EXTBRK_TIM_CHANNEL     TIM_CHANNEL_4
#define EXTBRK_PIN             GPIO_PIN_3
#define EXTBRK_PORT            GPIOA
#define EXT_PWM_BRK            TIM5->CCR4
#endif

#if defined(ESTOP_ENABLE)
  #if defined(EXTBRK_USE_CH4)
    #error "EXTBRK_USE_CH4 conflicts with ESTOP_ENABLE (both use PA3)"
  #endif
  #define ESTOP_PIN              GPIO_PIN_3
  #define ESTOP_PORT             GPIOA
  #if defined(ESTOP_BUTTON_NO)
    #define ESTOP_ACTIVE_STATE   GPIO_PIN_RESET
    #define ESTOP_IDLE_STATE     GPIO_PIN_SET
    #define ESTOP_GPIO_PULL      GPIO_PULLUP
  #else /* ESTOP_BUTTON_NC */
    #define ESTOP_ACTIVE_STATE   GPIO_PIN_SET
    #define ESTOP_IDLE_STATE     GPIO_PIN_RESET
    #define ESTOP_GPIO_PULL      GPIO_PULLDOWN
  #endif
#endif

#if defined(HOCP)
#define TIM1_BKIN_PIN       GPIO_PIN_6
#define TIM1_BKIN_PORT      GPIOA
#define TIM8_BKIN_PIN       GPIO_PIN_12
#define TIM8_BKIN_PORT      GPIOB
#endif

#if defined(SUPPORT_BUTTONS_LEFT)
#define BUTTON1_PIN         GPIO_PIN_2
#define BUTTON1_PORT        GPIOA
#define BUTTON2_PIN         GPIO_PIN_3
#define BUTTON2_PORT        GPIOA
#elif defined(SUPPORT_BUTTONS_RIGHT)
#define BUTTON1_PIN         GPIO_PIN_10
#define BUTTON1_PORT        GPIOB
#define BUTTON2_PIN         GPIO_PIN_11
#define BUTTON2_PORT        GPIOB
#endif

#define DELAY_TIM_FREQUENCY_US 1000000

#define MILLI_R (R * 1000)
#define MILLI_PSI (PSI * 1000)
#define MILLI_V (V * 1000)

#define NO 0
#define YES 1
#define ABS(a) (((a) < 0) ? -(a) : (a))
#define LIMIT(x, lowhigh) (((x) > (lowhigh)) ? (lowhigh) : (((x) < (-lowhigh)) ? (-lowhigh) : (x)))
#define SAT(x, lowhigh) (((x) > (lowhigh)) ? (1.0f) : (((x) < (-lowhigh)) ? (-1.0f) : (0.0f)))
#define SAT2(x, low, high) (((x) > (high)) ? (1.0f) : (((x) < (low)) ? (-1.0f) : (0.0f)))
#define STEP(from, to, step) (((from) < (to)) ? (MIN((from) + (step), (to))) : (MAX((from) - (step), (to))))
#define DEG(a) ((a)*M_PI / 180.0f)
#define RAD(a) ((a)*180.0f / M_PI)
#define SIGN(a) (((a) < 0) ? (-1) : (((a) > 0) ? (1) : (0)))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define IN_RANGE(x, low, high) (((x) >= (low)) && ((x) <= (high)))
#define SCALE(value, high, max) MIN(MAX(((max) - (value)) / ((max) - (high)), 0.0f), 1.0f)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN3(a, b, c) MIN(a, MIN(b, c))
#define MAX3(a, b, c) MAX(a, MAX(b, c))
#define ARRAY_LEN(x) (uint32_t)(sizeof(x) / sizeof(*(x)))
#define MAP(x, in_min, in_max, out_min, out_max) (((((x) - (in_min)) * ((out_max) - (out_min))) / ((in_max) - (in_min))) + (out_min))

#if defined(PRINTF_FLOAT_SUPPORT) && (defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)) && defined(__GNUC__)
    asm(".global _printf_float");     // this is the magic trick for printf to support float. Warning: It will increase code considerably! Better to avoid!
#endif



typedef struct { // Structure for ADC1 and ADC2 values
  uint16_t dcr;
  uint16_t dcl;
  uint16_t rlA;
  uint16_t rlB;
  uint16_t rrB;
  uint16_t rrC;
  #if defined(ENABLE_BOARD_TEMP_SENSOR)
  uint16_t temp;
  #endif

} adc12_named_samples_t;
#ifndef ENABLE_BOARD_TEMP_SENSOR
#define ADC12_CONV_COUNT 3
#else
#define ADC12_CONV_COUNT 4
#endif  
typedef union {
  adc12_named_samples_t value;
  uint32_t raw[ADC12_CONV_COUNT];
} adc12_dma_buffer_t;

typedef struct {  // Structure for ADC3 values
  #if defined(ANALOG_BUTTON)
  uint16_t button;
#endif
  uint16_t batt1;
} adc3_named_samples_t;
#ifndef ANALOG_BUTTON
 #define ADC3_CONV_COUNT 1
#else
 #define ADC3_CONV_COUNT 2
#endif

#if defined(HW_PWM)
extern volatile boolean_T hw_pwm_ready;
void calc_hw_pwm(void);
#endif

#if defined(CONTROL_PPM_LEFT) || defined(CONTROL_PPM_RIGHT)
extern volatile boolean_T ppm_ready;
void calc_ppm(void);
#endif

#if defined(SW_PWM_LEFT) || defined(SW_PWM_RIGHT)
extern volatile boolean_T sw_pwm_ready_ch1;
extern volatile boolean_T sw_pwm_ready_ch2;
void calc_sw_pwm_ch1(void);
void calc_sw_pwm_ch2(void);
#endif

#if defined(RC_PWM_LEFT) || defined(RC_PWM_RIGHT)
extern volatile boolean_T rc_pwm_ready_ch1;
extern volatile boolean_T rc_pwm_ready_ch2;
void calc_rc_pwm_ch1(void);
void calc_rc_pwm_ch2(void);
#endif


typedef union {
  adc3_named_samples_t value;
  uint16_t raw[ADC3_CONV_COUNT]; 
} adc3_dma_buffer_t;


typedef struct {
  adc12_dma_buffer_t adc12;
  adc3_dma_buffer_t adc3;
} adc_buf_t;

typedef enum {
  NUNCHUK_CONNECTING,
  NUNCHUK_DISCONNECTED,
  NUNCHUK_RECONNECTING,
  NUNCHUK_CONNECTED
} nunchuk_state;

// Define I2C, Nunchuk, PPM, PWM functions
void I2C_Init(void);
nunchuk_state Nunchuk_Read(void);
void PPM_Init(void);
void PPM_ISR_Callback(void);
void PWM_Init(void);
void PWM_ISR_CH1_Callback(void);
void PWM_ISR_CH2_Callback(void);

// Sideboard definitions
#define LED1_SET            (0x01)
#define LED2_SET            (0x02)
#define LED3_SET            (0x04)
#define LED4_SET            (0x08)
#define LED5_SET            (0x10)
#define SENSOR1_SET         (0x01)
#define SENSOR2_SET         (0x02)
#define SENSOR_MPU          (0x04)

// RC iBUS switch definitions. Flysky FS-i6S has [SWA, SWB, SWC, SWD] = [2, 3, 3, 2] positions switch
#define SWA_SET             (0x0100)   //  0000 0001 0000 0000
#define SWB_SET             (0x0600)   //  0000 0110 0000 0000
#define SWC_SET             (0x1800)   //  0001 1000 0000 0000
#define SWD_SET             (0x2000)   //  0010 0000 0000 0000

#endif // DEFINES_H

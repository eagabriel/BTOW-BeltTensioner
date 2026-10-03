/**
  * This file is part of the hoverboard-firmware-hack project.
  *
  * Copyright (C) 2020-2021 Emanuel FERU <aerdronix@gmail.com>
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

// Includes
#include <stdio.h>
#include <stdlib.h> // for abs()
#include <string.h>
#include <math.h>   // for fmod()
#include "stm32f1xx_hal.h"
#include "defines.h"
#include "setup.h"
#include "config.h"
#include "eeprom.h"
#include "util.h"
#include "BLDC_controller.h"
#include "rtwtypes.h"
#include "comms.h"
#include "motor_cfg.h"
#include "speed_limiter.h"

/* Encoder alignment power was a fixed fixdt(1,16,4) constant in config.h
 * (ALIGNMENT_*_POWER = 3000 = 3.75 A). Redefine as runtime shims so the
 * per-axis alignment current can be configured in Amps via CFG. Peak used
 * inside the alignment state machine is 2× this value (state 2 doubles). */
#undef  ALIGNMENT_X_POWER
#undef  ALIGNMENT_Y_POWER
#define ALIGNMENT_X_POWER  ((int32_t)motor_cfg.align_i_x_x100 * A2BIT_CONV * 16 / 100)
#define ALIGNMENT_Y_POWER  ((int32_t)motor_cfg.align_i_y_x100 * A2BIT_CONV * 16 / 100)


/* =========================== Variable Definitions =========================== */

// Timebase: buzzerTimer ticks at 16 ticks per millisecond (see main.c)
#ifndef TICKS_PER_MS
#define TICKS_PER_MS 16U
#endif
#define T_MS(ms) ((uint32_t)((ms) * TICKS_PER_MS))

//------------------------------------------------------------------------
// Global variables set externally
//------------------------------------------------------------------------
extern volatile adc_buf_t adc_buffer;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
#include "mt6701.h"
#include "mt6701_pwm.h"
#include "setup.h"
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

extern int16_t batVoltage;
extern uint8_t backwardDrive;
extern uint8_t buzzerCount;             // global variable for the buzzer counts. can be 1, 2, 3, 4, 5, 6, 7...
extern volatile uint32_t buzzerTimer;   // global timer variable for buzzer timing
static volatile uint8_t dcLinkProtection = false;
static volatile boolean_T overcurrent_fault_flag = false;

#if defined(ANALOG_BUTTON)
volatile uint8_t analogButtonPressed = 0U;
static uint8_t analogButtonLatched = 0U;

#if ANALOG_BUTTON_PRESSED_MIN > 4095U
  #error "ANALOG_BUTTON_PRESSED_MIN must be <= 4095"
#endif
#if ANALOG_BUTTON_RELEASE_MAX > 4095U
  #error "ANALOG_BUTTON_RELEASE_MAX must be <= 4095"
#endif
#if ANALOG_BUTTON_RELEASE_MAX >= ANALOG_BUTTON_PRESSED_MIN
  #error "ANALOG_BUTTON_RELEASE_MAX must be less than ANALOG_BUTTON_PRESSED_MIN"
#endif

static void AnalogButton_ProcessSample(uint16_t sample) {
  if (!analogButtonLatched) {
    if (sample >= ANALOG_BUTTON_PRESSED_MIN) {
      analogButtonLatched  = 1U;
      analogButtonPressed  = 1U;
    }
  } else {
    if (sample <= ANALOG_BUTTON_RELEASE_MAX) {
      analogButtonLatched  = 0U;
      analogButtonPressed  = 0U;
    }
  }
}

void AnalogButton_Init(void) {
  analogButtonPressed = 0U;
  analogButtonLatched = 0U;
}
#endif

#if defined(ESTOP_ENABLE)
extern volatile uint32_t main_loop_counter;

volatile uint8_t estop_flag        = 0U;
volatile uint8_t estop_latch_flag  = 0U;
static uint8_t          estop_state       = 0U;
static uint8_t          estop_sample_prev = 0U;
static uint32_t         estop_change_loop = 0U;

#define ESTOP_DEBOUNCE_LOOPS ((uint32_t)(((ESTOP_DEBOUNCE_MS) + (DELAY_IN_MAIN_LOOP) - 1U) / (DELAY_IN_MAIN_LOOP)))

void estop_init(void) {
  estop_flag        = 0U;
  estop_latch_flag  = 0U;
  estop_change_loop = main_loop_counter;
  estop_sample_prev = (HAL_GPIO_ReadPin(ESTOP_PORT, ESTOP_PIN) == ESTOP_ACTIVE_STATE);
  estop_state       = estop_sample_prev;
}

void estop_update(void) {
  const uint32_t now   = main_loop_counter;
  const uint8_t sample = (HAL_GPIO_ReadPin(ESTOP_PORT, ESTOP_PIN) == ESTOP_ACTIVE_STATE);

  if (sample != estop_sample_prev) {
    estop_sample_prev = sample;
    estop_change_loop = now;
  }

  if ((now - estop_change_loop) >= ESTOP_DEBOUNCE_LOOPS && estop_state != estop_sample_prev) {
    estop_state = estop_sample_prev;
    if (estop_state) {
#if defined(ESTOP_REQUIRE_HOLD)
      estop_flag = 1U;
#else
      if (estop_latch_flag) {
        estop_latch_flag = 0U;
        estop_flag       = 0U;
      } else {
        estop_flag = 1U;
        #if defined(ESTOP_BUTTON_NO)
        estop_latch_flag = 1U;
        #endif
      }
#endif
    } else {
#if defined(ESTOP_REQUIRE_HOLD)
      estop_flag = 0U;
#else
      #if defined(ESTOP_BUTTON_NO)
        if (!estop_latch_flag) {
          estop_flag = 0U;
        }
      #else
        estop_flag = 0U;
      #endif
#endif
    }
  }
}

#else
void estop_init(void) {}
void estop_update(void) {}
#endif

#if defined(DC_LINK_WATCHDOG_ENABLE)
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

static void DcLinkWatchdog_ArmRise(void) {
  ADC_AnalogWDGConfTypeDef config;
  config.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
  config.HighThreshold = BAT_HIGH;
  config.LowThreshold  = HARD_18V_COUNTS-300;   // around 15v
  config.Channel       = DCLINK_ADC_CHANNEL;
  config.ITMode        = ENABLE;
  HAL_ADC_AnalogWDGConfig(&hadc3, &config);
}

static void DcLinkWatchdog_HandleWatchdog(void) {
  __HAL_ADC_CLEAR_FLAG(&hadc3, ADC_FLAG_AWD);
  //uint16_t sample = adc_buffer.adc3.value.batt1;
      dcLinkProtection = true;
      LEFT_TIM->BDTR &= ~TIM_BDTR_MOE;
      RIGHT_TIM->BDTR &= ~TIM_BDTR_MOE;
    }

void DcLinkWatchdog_Init(void) {
  dcLinkProtection = false;
  __HAL_ADC_CLEAR_FLAG(&hadc3, ADC_FLAG_AWD);
  DcLinkWatchdog_ArmRise();
}



void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc) {
  if ((hadc != NULL) && (hadc == &hadc3 || hadc->Instance == hadc3.Instance)) {
    DcLinkWatchdog_HandleWatchdog();
    return;
  }
  // ADC1/ADC2 analog watchdog: immediate motor disable
  // ADC1 -> right DC sensor (PC1 / ADC_CHANNEL_11)
  // ADC2 -> left  DC sensor (PC0 / ADC_CHANNEL_10)
  // Clear AWD flag and disable motors via MOE bits, then set global enable=0 so
   
  if ((hadc != NULL) && (hadc == &hadc1 || hadc->Instance == hadc1.Instance)) {
    __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_AWD);
  LEFT_TIM->BDTR &= ~TIM_BDTR_MOE;
  RIGHT_TIM->BDTR &= ~TIM_BDTR_MOE;
  overcurrent_fault_flag = true;
    return;
  }

  if ((hadc != NULL) && (hadc == &hadc2 || hadc->Instance == hadc2.Instance)) {
    __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_AWD);
  LEFT_TIM->BDTR &= ~TIM_BDTR_MOE;
  RIGHT_TIM->BDTR &= ~TIM_BDTR_MOE;
  overcurrent_fault_flag = true;
    return;
  }
}
#endif
boolean_T overcurrent_fault(void) {
  return overcurrent_fault_flag;
}
boolean_T  DLVPA(void) {
  return dcLinkProtection;
}
// Shared accessor used by the rest of the firmware regardless of button mode.
uint8_t powerButtonPressed(void) {
#if defined(ANALOG_BUTTON)
  AnalogButton_ProcessSample(adc_buffer.adc3.value.button);
  return analogButtonPressed;
#else
  return HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
#endif
}

#if defined(ENCODER_X)
SensorState encoder_x = {0}; // Initialize all members to 0/false
TIM_HandleTypeDef encoder_x_handle;
/* Set by count_x_update (potentially in TIM7 ISR) when I2C fails N times.
 * Main loop clears it after running I2C_Init recovery — see main.c. */
volatile uint8_t mt6701_x_recovery_req = 0;
/* IT-mode plumbing (see HAL_I2C_MemRxCpltCallback below). */
volatile uint8_t mt6701_x_busy  = 0;
/* Non-static so the bare-metal driver in mt6701.c can write completed bytes
 * here before invoking MT6701_X_ProcessReading. */
uint8_t          mt6701_x_rxbuf[2];
volatile uint32_t mt6701_x_busy_since = 0;
/* Diagnostic counters — same shape as the Y-side ones. */
volatile uint32_t mt6701_x_cplt_count = 0;
volatile uint32_t mt6701_x_err_count  = 0;
#endif

#if defined(ENCODER_Y)
SensorState encoder_y = {0}; // Initialize all members to 0/false
TIM_HandleTypeDef encoder_y_handle;
volatile uint8_t mt6701_y_recovery_req = 0;
volatile uint8_t mt6701_y_busy  = 0;
/* Non-static so the bare-metal driver in mt6701.c can write completed bytes
 * here before invoking MT6701_Y_ProcessReading. */
uint8_t          mt6701_y_rxbuf[2];
/* Timestamp when busy was set (buzzerTimer). Main-loop watchdog uses this to
 * detect an IT transaction that hung (no callback ever fired) and force
 * recovery. Populated by MT6701_Y_KickIT below. */
volatile uint32_t mt6701_y_busy_since = 0;
/* Debug counters — increment each time the corresponding IT callback fires.
 * Exposed in the periodic status print so we can see if the IRQ path is alive
 * even before the sensor produces valid data. */
volatile uint32_t mt6701_y_cplt_count = 0;
volatile uint32_t mt6701_y_err_count  = 0;
extern volatile uint32_t buzzerTimer;
#endif

#if defined(ENCODER_X) || defined(ENCODER_Y)
/* Direct MT6701 mechanical-speed estimator.  The generated BLDC model still
 * derives n_mot from Hall transitions; with encoder mode enabled those Hall
 * inputs are deliberately held static.  Keep this estimator outside the
 * generated code so we can first observe and validate real shaft speed before
 * replacing the legacy motor-speed limiter.
 *
 * A 32-frame moving window gives about 109 counts of travel at 50 rpm with a
 * 4096-CPR encoder, avoiding the severe 3/4-count quantisation of a 1 ms
 * derivative.  We retain the actual buzzerTimer interval between accepted PWM
 * frames, so the result remains correct when the ~994 Hz sensor frame rate and
 * the 1 kHz sampler occasionally skip/repeat relative to one another. */
#define ENC_SPEED_WINDOW       32u
#define ENC_SPEED_STALE_TICKS  (PWM_FREQ / 20u) /* 50 ms without a new frame */

typedef struct {
  int16_t  delta[ENC_SPEED_WINDOW];
  uint16_t dt_ticks[ENC_SPEED_WINDOW];
  int32_t  delta_sum;
  uint32_t dt_sum;
  int32_t  prev_count;
  uint32_t prev_tick;
  uint32_t last_frame;
  uint8_t  index;
  uint8_t  samples;
  uint8_t  seeded;
} EncoderSpeedEstimator;

static EncoderSpeedEstimator speed_est_x;
static EncoderSpeedEstimator speed_est_y;

static void encoder_speed_reset(EncoderSpeedEstimator *est, SensorState *enc)
{
  memset(est, 0, sizeof(*est));
  enc->rpm_q4 = 0;
  enc->rpm_valid = 0;
  enc->speed_last_tick = buzzerTimer;
}

static void encoder_speed_update(EncoderSpeedEstimator *est, SensorState *enc,
                                 int32_t cpr, uint32_t frame_count)
{
  uint32_t now = buzzerTimer;

  if (!enc->ali) {
    if (est->seeded || enc->rpm_valid) encoder_speed_reset(est, enc);
    return;
  }

  /* TIM7 can run slightly faster than the sensor's PWM frame rate.  Do not
   * insert a fake zero-motion sample when no new capture has arrived. */
  if (frame_count == est->last_frame) {
    if (est->seeded && (uint32_t)(now - enc->speed_last_tick) > ENC_SPEED_STALE_TICKS) {
      enc->rpm_q4 = 0;
      enc->rpm_valid = 0;
    }
    return;
  }

  if (!est->seeded) {
    est->prev_count = enc->ENCODER_COUNT;
    est->prev_tick = now;
    est->last_frame = frame_count;
    est->seeded = 1;
    enc->speed_last_tick = now;
    return;
  }

  int32_t delta = enc->ENCODER_COUNT - est->prev_count;
  if (delta > cpr / 2) delta -= cpr;
  else if (delta < -(cpr / 2)) delta += cpr;

  uint32_t dt32 = (uint32_t)(now - est->prev_tick);
  if (dt32 == 0u) dt32 = 1u;
  if (dt32 > 0xFFFFu) dt32 = 0xFFFFu;

  uint8_t idx = est->index;
  if (est->samples == ENC_SPEED_WINDOW) {
    est->delta_sum -= est->delta[idx];
    est->dt_sum -= est->dt_ticks[idx];
  } else {
    est->samples++;
  }
  est->delta[idx] = (int16_t)delta;
  est->dt_ticks[idx] = (uint16_t)dt32;
  est->delta_sum += delta;
  est->dt_sum += dt32;
  est->index = (uint8_t)((idx + 1u) % ENC_SPEED_WINDOW);

  est->prev_count = enc->ENCODER_COUNT;
  est->prev_tick = now;
  est->last_frame = frame_count;
  enc->speed_last_tick = now;

  if (est->samples == ENC_SPEED_WINDOW && est->dt_sum != 0u) {
    /* rpm_q4 = counts/tick * PWM_FREQ ticks/s * 60 s/min * 16. */
    int64_t num = (int64_t)est->delta_sum * (int64_t)PWM_FREQ * 60LL * 16LL;
    int64_t den = (int64_t)cpr * (int64_t)est->dt_sum;
    int64_t rpm_q4 = num / den;
    if (rpm_q4 > INT16_MAX) rpm_q4 = INT16_MAX;
    if (rpm_q4 < INT16_MIN) rpm_q4 = INT16_MIN;
    enc->rpm_q4 = (int16_t)rpm_q4;
    enc->rpm_valid = 1;
  }
}

/* Forward decls — bodies live further down after count_x/y_update. */
/* Non-static when the bare-metal driver is on so mt6701.c can invoke them
 * directly from the EV IRQ (same pattern as the Y side). */
#ifdef ENCODER_X
void MT6701_X_ProcessReading(void);
#endif
/* MT6701_Y_ProcessReading is NON-static when MT6701_BAREMETAL_Y is on so the
 * bare-metal driver in mt6701.c can invoke it directly from the EV IRQ. */
#ifdef ENCODER_Y
void MT6701_Y_ProcessReading(void);
#endif

/* HAL DMA callbacks — HAL_I2C_Mem_Read_DMA completion goes to MemRxCplt,
 * BERR/AF/ARLO/OVR errors go to ErrorCallback. Both weak-override.
 *
 * MT6701_BAREMETAL_Y bypasses HAL entirely for hi2c1, so these callbacks
 * never fire for Y in that mode — the bare-metal EV/ER IRQ handlers do the
 * work directly. Left in for X (hi2c2) which still uses HAL DMA. */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    #if defined(ENCODER_Y) && !defined(MT6701_BAREMETAL_Y)
    if (hi2c == &hi2c1) {
        mt6701_y_cplt_count++;
        MT6701_Y_ProcessReading();
        mt6701_y_busy = 0;
        return;
    }
    #endif
    #if defined(ENCODER_X) && !defined(MT6701_BAREMETAL_X)
    if (hi2c == &hi2c2) {
        MT6701_X_ProcessReading();
        mt6701_x_busy = 0;
        return;
    }
    #endif
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    #if defined(ENCODER_Y) && !defined(MT6701_BAREMETAL_Y)
    if (hi2c == &hi2c1) {
        mt6701_y_err_count++;
        if (encoder_y.i2c_fail_count < 255) encoder_y.i2c_fail_count++;
        mt6701_y_recovery_req = 1;
        mt6701_y_busy = 0;
        return;
    }
    #endif
    #if defined(ENCODER_X) && !defined(MT6701_BAREMETAL_X)
    if (hi2c == &hi2c2) {
        if (encoder_x.i2c_fail_count < 255) encoder_x.i2c_fail_count++;
        mt6701_x_recovery_req = 1;
        mt6701_x_busy = 0;
        return;
    }
    #endif
}
#endif  /* ENCODER_X || ENCODER_Y */


inline boolean_T encoder_alignment_faulted(void) {
#if defined(ENCODER_X) && defined(ENCODER_Y)
  return (encoder_x.align_fault != 0U) || (encoder_y.align_fault != 0U);
#elif defined(ENCODER_X)
  return (encoder_x.align_fault != 0U);
#elif defined(ENCODER_Y)
  return (encoder_y.align_fault != 0U);
#else 
return 0;
#endif
}

static inline boolean_T encoder_alignment_active(void) {
#if defined(ENCODER_X) && defined(ENCODER_Y)
  return (encoder_x.align_state != 0U) || (encoder_y.align_state != 0U);
#elif defined(ENCODER_X)
  return (encoder_x.align_state != 0U);
#elif  defined(ENCODER_Y)
  return (encoder_y.align_state != 0U);
#else 
return 0;
#endif
}


extern uint8_t buzzerFreq;              // global variable for the buzzer pitch. can be 1, 2, 3, 4, 5, 6, 7...
extern uint8_t buzzerPattern;           // global variable for the buzzer pattern. can be 1, 2, 3, 4, 5, 6, 7...

extern uint8_t enable;                  // global variable for motor enable

extern uint8_t nunchuk_data[6];
extern volatile uint32_t timeoutCntGen; // global counter for general timeout counter
extern volatile uint8_t  timeoutFlgGen; // global flag for general timeout counter
extern volatile uint32_t main_loop_counter;





//------------------------------------------------------------------------
// Global variables set here in util.c
//------------------------------------------------------------------------
// Matlab defines - from auto-code generation
//---------------
RT_MODEL rtM_Left_;                     /* Real-time model */
RT_MODEL rtM_Right_;                    /* Real-time model */
RT_MODEL *const rtM_Left  = &rtM_Left_;
RT_MODEL *const rtM_Right = &rtM_Right_;

extern P rtP_Left;                      /* Block parameters (auto storage) */
DW       rtDW_Left;                     /* Observable states */
ExtU     rtU_Left;                      /* External inputs */
ExtY     rtY_Left;                      /* External outputs */

P        rtP_Right;                     /* Block parameters (auto storage) */
DW       rtDW_Right;                    /* Observable states */
ExtU     rtU_Right;                     /* External inputs */
ExtY     rtY_Right;                     /* External outputs */
//---------------

uint8_t  inIdx      = 0;
uint8_t  inIdx_prev = 0;
#if defined(PRI_INPUT1) && defined(PRI_INPUT2) && defined(AUX_INPUT1) && defined(AUX_INPUT2)
InputStruct input1[INPUTS_NR] = { {0, 0, 0, PRI_INPUT1}, {0, 0, 0, AUX_INPUT1} };
InputStruct input2[INPUTS_NR] = { {0, 0, 0, PRI_INPUT2}, {0, 0, 0, AUX_INPUT2} };
#else
InputStruct input1[INPUTS_NR] = { {0, 0, 0, PRI_INPUT1} };
InputStruct input2[INPUTS_NR] = { {0, 0, 0, PRI_INPUT2} };
#endif
 
int16_t  speedAvg;                      // average measured speed
int16_t  speedAvgAbs;                   // average measured speed in absolute
uint8_t  timeoutFlgADC    = 0;          // Timeout Flag for ADC Protection:    0 = OK, 1 = Problem detected (line disconnected or wrong ADC data)
uint8_t  timeoutFlgSerial = 0;          // Timeout Flag for Rx Serial command: 0 = OK, 1 = Problem detected (line disconnected or wrong Rx data)

uint8_t  ctrlModReqRaw = CTRL_MOD_REQ;
uint8_t  ctrlModReq    = CTRL_MOD_REQ;  // Final control mode request 


uint16_t VirtAddVarTab[NB_OF_VAR] = {
  /* legacy input-config table */
  1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009,
  1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018,
  /* motor_cfg (see Inc/motor_cfg.h) */
  2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
  2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
  2020, 2021,
};


//------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------
static int16_t INPUT_MAX;             // [-] Input target maximum limitation
static int16_t INPUT_MIN;             // [-] Input target minimum limitation


  static uint8_t  cur_spd_valid  = 0;
  static uint8_t  inp_cal_valid  = 0;



static uint8_t  rx_buffer_R[SERIAL_BUFFER_SIZE];      // USART Rx DMA circular buffer
static uint32_t rx_buffer_R_len = ARRAY_LEN(rx_buffer_R);






/* =========================== Retargeting printf =========================== */
/* retarget the C library printf function to the USART */
  #ifdef __GNUC__
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
  #else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
  #endif
  PUTCHAR_PROTOTYPE {
      HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 1000);
    return ch;
  }
  
  #ifdef __GNUC__
    int _write(int file, char *data, int len) {
      int i;
      for (i = 0; i < len; i++) { __io_putchar( *data++ );}
      return len;
    }
  #endif

 
/* =========================== Initialization Functions =========================== */



void BLDC_Init(void) {
  BLDC_SetPwmResolution((uint16_t)LEFT_TIM->ARR);
  /* Set BLDC controller parameters */ 
  #if defined(ENCODER_X) || defined(ENCODER_Y)
  rtP_Left.b_angleMeasEna       = 1;            // Motor angle input: 0 = estimated angle, 1 = measured angle (e.g. if encoder is available)
  #else
  rtP_Left.b_angleMeasEna       = 0;            // Motor angle input: 0 = estimated angle, 1 = measured angle (e.g. if encoder is available)
  #endif
  rtP_Left.z_selPhaCurMeasABC   = 0;            // Left motor measured current phases {Blue, Yellow} = {iB, iC} -> do NOT change
  rtP_Left.z_ctrlTypSel         = CTRL_TYP_SEL;
  rtP_Left.b_diagEna            = DIAG_ENA;
  rtP_Left.i_max                = (I_MOT_MAX * A2BIT_CONV) << 4;        // fixdt(1,16,4)
  rtP_Left.n_max                = N_MOT_MAX << 4;                       // fixdt(1,16,4)
  rtP_Left.b_fieldWeakEna       = FIELD_WEAK_ENA; 
  rtP_Left.id_fieldWeakMax      = (FIELD_WEAK_MAX * A2BIT_CONV) << 4;   // fixdt(1,16,4)
  rtP_Left.a_phaAdvMax          = PHASE_ADV_MAX << 4;                   // fixdt(1,16,4)
  rtP_Left.r_fieldWeakHi        = FIELD_WEAK_HI << 4;                   // fixdt(1,16,4)
  rtP_Left.r_fieldWeakLo        = FIELD_WEAK_LO << 4;                   // fixdt(1,16,4)
  rtP_Left.n_polePairs          = N_POLE_PAIRS;                        // fixdt(1,16,4)
  rtP_Left.cf_idKi              = CFG_CF_IDKI;
  rtP_Left.cf_idKp              = CFG_CF_IDKP;
  rtP_Left.cf_iqKi              = CFG_CF_IQKI;
  rtP_Left.cf_iqKp              = CFG_CF_IQKP;
  rtP_Left.cf_currFilt          = CFG_CF_CURR_FILT;
  #ifdef ENCODER_CPR
  rtP_Left.a_cpr                = ENCODER_CPR;
  rtP_Left.a_fcpr               = FRAC_CPR;
  #else
  rtP_Left.a_cpr                = 0;
  rtP_Left.a_fcpr               = 0;
  #endif
  #ifdef FeedForward
  rtP_Left.ff_gain              = FF_GAIN; //not used anymore
  #endif
  
  rtP_Right                     = rtP_Left;     // Copy the Left motor parameters to the Right motor parameters
  rtP_Right.z_selPhaCurMeasABC  = 1;            // Right motor measured current phases {Green, Blue} = {iA, iB} -> do NOT change

  /* Pack LEFT motor data into RTM */
  rtM_Left->defaultParam        = &rtP_Left;
  rtM_Left->dwork               = &rtDW_Left;
  rtM_Left->inputs              = &rtU_Left;
  rtM_Left->outputs             = &rtY_Left;

  /* Pack RIGHT motor data into RTM */
  rtM_Right->defaultParam       = &rtP_Right;
  rtM_Right->dwork              = &rtDW_Right;
  rtM_Right->inputs             = &rtU_Right;
  rtM_Right->outputs            = &rtY_Right;

  /* Initialize BLDC controllers */
  BLDC_controller_initialize(rtM_Left);
  BLDC_controller_initialize(rtM_Right);
}
void Input_Lim_Init(void) {     // Input Limitations - ! Do NOT touch !
  if (rtP_Left.b_fieldWeakEna || rtP_Right.b_fieldWeakEna) {
    INPUT_MAX = MAX( 1000, FIELD_WEAK_HI);
    INPUT_MIN = MIN(-1000,-FIELD_WEAK_HI);
  } else {
    INPUT_MAX =  1000;
    INPUT_MIN = -1000;
  }
}

void Input_Init(void) {


    UART3_Init();
    HAL_UART_Receive_DMA(&huart3, (uint8_t *)rx_buffer_R, sizeof(rx_buffer_R));
    UART_DisableRxErrors(&huart3);

    uint16_t writeCheck, readVal;
    HAL_FLASH_Unlock();
    EE_Init();            /* EEPROM Init */
    EE_ReadVariable(VirtAddVarTab[0], &writeCheck);
    if (writeCheck == FLASH_WRITE_KEY) {
        printf("Using the configuration from EEprom\r\n");

      EE_ReadVariable(VirtAddVarTab[1] , &readVal); rtP_Left.i_max = rtP_Right.i_max = (int16_t)readVal;
      EE_ReadVariable(VirtAddVarTab[2] , &readVal); rtP_Left.n_max = rtP_Right.n_max = (int16_t)readVal;
      for (uint8_t i=0; i<INPUTS_NR; i++) {
        EE_ReadVariable(VirtAddVarTab[ 3+8*i] , &readVal); input1[i].typ = (uint8_t)readVal;
        EE_ReadVariable(VirtAddVarTab[ 4+8*i] , &readVal); input1[i].min = (int16_t)readVal;
        EE_ReadVariable(VirtAddVarTab[ 5+8*i] , &readVal); input1[i].mid = (int16_t)readVal;
        EE_ReadVariable(VirtAddVarTab[ 6+8*i] , &readVal); input1[i].max = (int16_t)readVal;
        EE_ReadVariable(VirtAddVarTab[ 7+8*i] , &readVal); input2[i].typ = (uint8_t)readVal;
        EE_ReadVariable(VirtAddVarTab[ 8+8*i] , &readVal); input2[i].min = (int16_t)readVal;
        EE_ReadVariable(VirtAddVarTab[ 9+8*i] , &readVal); input2[i].mid = (int16_t)readVal;
        EE_ReadVariable(VirtAddVarTab[10+8*i] , &readVal); input2[i].max = (int16_t)readVal;
      
        printf("Limits Input1: TYP:%i MIN:%i MID:%i MAX:%i\r\nLimits Input2: TYP:%i MIN:%i MID:%i MAX:%i\r\n",
          input1[i].typ, input1[i].min, input1[i].mid, input1[i].max,
          input2[i].typ, input2[i].min, input2[i].mid, input2[i].max);
      }
    } else {
        printf("Using the configuration from config.h\r\n");

      for (uint8_t i=0; i<INPUTS_NR; i++) {
        if (input1[i].typDef == 3) {  // If Input type defined is 3 (auto), identify the input type based on the values from config.h
          input1[i].typ = checkInputType(input1[i].min, input1[i].mid, input1[i].max);
        } else {
          input1[i].typ = input1[i].typDef;
        }
        if (input2[i].typDef == 3) {
          input2[i].typ = checkInputType(input2[i].min, input2[i].mid, input2[i].max);
        } else {
          input2[i].typ = input2[i].typDef;
        }
        printf("Limits Input1: TYP:%i MIN:%i MID:%i MAX:%i\r\nLimits Input2: TYP:%i MIN:%i MID:%i MAX:%i\r\n",
          input1[i].typ, input1[i].min, input1[i].mid, input1[i].max,
          input2[i].typ, input2[i].min, input2[i].mid, input2[i].max);
      }
    }
    HAL_FLASH_Lock();



}

/**
  * @brief  Disable Rx Errors detection interrupts on UART peripheral (since we do not want DMA to be stopped)
  *         The incorrect data will be filtered based on the START_FRAME and checksum.
  * @param  huart: UART handle.
  * @retval None
  */
#if defined(DEBUG_SERIAL_USART2) || defined(CONTROL_SERIAL_USART2) || defined(SIDEBOARD_SERIAL_USART2) || \
    defined(DEBUG_SERIAL_USART3) || defined(CONTROL_SERIAL_USART3) || defined(SIDEBOARD_SERIAL_USART3)
void UART_DisableRxErrors(UART_HandleTypeDef *huart)
{  
  CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);    /* Disable PE (Parity Error) interrupts */  
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);     /* Disable EIE (Frame error, noise error, overrun error) interrupts */
}
#endif

/* =========================== Encoder Functions =========================== */
#if defined (ENCODER_X)

static void Encoder_X_ApplyDirection(boolean_T forward_dir) {
  /* With MT6701 absolute sensor, direction inversion happens in software via
   * encoder_x.direction (consumed downstream). No timer reconfig needed. */
  encoder_x.direction = forward_dir;
}

void Encoder_X_Init(void) {
#if defined(MT6701_MODE_PWM_X)
    /* X reads its MT6701 via PWM input capture (TIM2_CH3 on PB10), not I2C.
     * The capture timer is set up once in main() by MT6701_X_Pwm_Init().
     * Do NOT call I2C_Init() — it would reconfigure PB10 as I2C2_SCL and
     * destroy the capture. Wait until the capture is producing valid frames
     * before declaring the encoder ready, so the FOC never starts on a
     * stale raw_angle=0 (or with no sensor connected, X stays uninitialized
     * and the right motor is left alone). The main-loop retry calls this
     * cheaply until frames accumulate. */
    if (MT6701_X_Pwm_GetFrameCount() < 200u) {
        encoder_x.ini = false;
        return;
    }
    encoder_x.raw_angle      = 0;
    encoder_x.i2c_fail_count = 0;
    encoder_x.cnt_offset     = 0;
    encoder_x.ali            = false;
    encoder_x.align_fault    = false;
    encoder_x.offset         = 0;
    encoder_x.direction      = 1;
    encoder_x.aligned_count  = 0;
    encoder_x.ENCODER_COUNT  = 0;
    encoder_x.full_rotations = 0;
    encoder_x.align_state    = 0;
    encoder_x.align_ini_pos  = 0;
    /* Seed ENCODER_COUNT + count_prev from the live PWM angle so the first
     * TIM7 feed produces a zero delta (no spurious full_rotations bump). */
    MT6701_X_Pwm_Feed();
    encoder_x.full_rotations = 0;
    encoder_x.ini = true;
    #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
    printf("MT6701_X: PWM capture ready, seeded cnt=%ld\r\n",
      (long)encoder_x.ENCODER_COUNT);
    #endif
    motor_cfg_apply_stored_alignment_x();   /* replay saved calibration if any */
    return;
#else
    /* SWAPPED: MT6701 sensor on I2C2 (PB10/PB11) — per user preference to
     * make X label correspond to physical board layout. */
    I2C_Init();
    #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
    static uint8_t x_first_init_logged = 0;
    if (!x_first_init_logged) {
        x_first_init_logged = 1;
        uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
        printf("I2C2 regs (X): CR1=0x%04X CR2=0x%04X CCR=0x%04X TRISE=0x%04X SR1=0x%04X SR2=0x%04X\r\n",
            (unsigned)I2C2->CR1, (unsigned)I2C2->CR2,
            (unsigned)I2C2->CCR, (unsigned)I2C2->TRISE,
            (unsigned)I2C2->SR1, (unsigned)I2C2->SR2);
        printf("I2C2 config (X): PCLK1=%luHz FREQ=%luMHz F/S=%s DUTY=%s CCR_val=%lu TRISE=%lu\r\n",
            pclk1, pclk1/1000000UL,
            (I2C2->CCR & (1u<<15)) ? "FAST" : "STD",
            (I2C2->CCR & (1u<<14)) ? "16/9" : "2/1",
            (unsigned long)(I2C2->CCR & 0x0FFF),
            (unsigned long)(I2C2->TRISE & 0x3F));
    }
    #endif

    encoder_x.raw_angle = 0;
    encoder_x.i2c_fail_count = 0;
    encoder_x.cnt_offset = 0;
    encoder_x.ali = false;
    encoder_x.align_fault = false;
    encoder_x.offset = 0;
    encoder_x.direction = 1;
    encoder_x.aligned_count = 0;
    encoder_x.ENCODER_COUNT = 0;
    encoder_x.full_rotations = 0;
    encoder_x.align_state = 0;
    encoder_x.align_ini_pos = 0;

    uint16_t test_angle = 0;
    HAL_StatusTypeDef st = MT6701_ReadAngle(&hi2c2, &test_angle);
    if (st != HAL_OK) {
        #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
        static uint32_t x_diag_ts = 0;
        if ((buzzerTimer - x_diag_ts) > 16000u) {
            x_diag_ts = buzzerTimer;
            printf("X read fail: hal=%d SR1=0x%04X SR2=0x%04X\r\n",
                (int)st, (unsigned)I2C2->SR1, (unsigned)I2C2->SR2);
        }
        #endif
        encoder_x.ini = false;
        return;
    }
    encoder_x.raw_angle = test_angle;
    encoder_x.ini = true;
#endif  /* MT6701_MODE_PWM_X */

    /* If the user saved a calibration (motor_cfg.align_stored=1), replay it now
     * so the main loop's `!encoder_x.ali` gate skips auto-align. Must run AFTER
     * the reset block above — otherwise ali/offset get wiped seconds later. */
    motor_cfg_apply_stored_alignment_x();
}

void Encoder_X_Align_Start(void) {
  if (encoder_x.align_state != 0) {
        return;
    }

  if (encoder_x.align_fault) {
    return;
  }

  #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
  printf("[ALIGN X] START align_i=%u.%02uA (raw=%d) i_mot_max=%dA\r\n",
    motor_cfg.align_i_x_x100 / 100, motor_cfg.align_i_x_x100 % 100,
    (int)ALIGNMENT_X_POWER, (int)motor_cfg.i_mot_max);
  #endif

    enable = 1;
    rtP_Left.b_diagEna = 0; // Disable diagnostics during alignment
    rtP_Right.b_diagEna = 0; // Disable diagnostics during alignment
  encoder_x.ali = false;
  encoder_x.align_state = 1; // Start alignment sequence
  encoder_x.align_timer = 0;
  encoder_x.align_start_time = buzzerTimer;
  /* encoder_x.ENCODER_COUNT is kept fresh by TIM7 IT sampler at 1 kHz. */
  encoder_x.align_ini_pos = encoder_x.ENCODER_COUNT;
  encoder_x.align_total_ini_pos = get_x_TotalCount();
    // Initialize simulation variables
  /* Target sweep: N electrical rotations during phase-1 move time, where N =
   * motor_cfg.align_turns_elec (default 2). MOVE_MS scales proportionally in
   * Encoder_X_Align so mechanical speed is constant regardless of N. */
  uint32_t turns = motor_cfg.align_turns_elec ? motor_cfg.align_turns_elec : 2u;
  uint32_t move_ms = (T_MS(1500) * turns) / 2u;
  encoder_x.count_increment_x1000 = (int32_t)((((int64_t)ENCODER_X_CPR) * (int64_t)(turns * 1000u)) / (((int64_t)N_POLE_PAIRS) * (int64_t)move_ms));
    encoder_x.align_inpTgt = 0; // Start with 0 power, will ramp up
}

// Non-blocking encoder alignment with mechanical angle simulation - call from main loop
void Encoder_X_Align(void) {
    uint32_t current_time = buzzerTimer;
    uint32_t elapsed_ticks = current_time - encoder_x.align_start_time;
    /* Encoder state kept fresh by TIM7 IT sampler — no blocking read here. */

    #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
    static uint8_t x_prev_align_state = 0;
    if (encoder_x.align_state != x_prev_align_state) {
      x_prev_align_state = encoder_x.align_state;
      if (encoder_x.align_state == 2) {
        printf("[ALIGN X] state 2: high-power hold, pos=%ld\r\n",
          (long)encoder_x.ENCODER_COUNT);
      } else if (encoder_x.align_state == 3) {
        printf("[ALIGN X] state 3: move back, dir=%d offset=%ld\r\n",
          (int)encoder_x.direction, (long)encoder_x.offset);
      }
    }
    #endif

    const uint32_t RAMP_MS = T_MS(125);
    /* MOVE_MS scales with motor_cfg.align_turns_elec (see Align_Start comment).
     * Same value is used in rotation (state 1) and move-back (state 3). */
    uint32_t turns = motor_cfg.align_turns_elec ? motor_cfg.align_turns_elec : 2u;
    const uint32_t MOVE_MS = (T_MS(1500) * turns) / 2u;

    switch (encoder_x.align_state) {
        case 1: // Rotation phase - spin to find position
            handle_x_rotation_phase(elapsed_ticks, RAMP_MS, MOVE_MS, current_time);
            break;

        case 2: // High power phase - apply extra torque
            handle_x_high_power_phase(elapsed_ticks, RAMP_MS, current_time);
            break;

        case 3: // Move back and finish
            handle_x_move_back_phase(elapsed_ticks, RAMP_MS, MOVE_MS, current_time);
            break;
            
        default:
            encoder_x.align_state = 0;
            break;
    }
}


 void handle_x_rotation_phase(uint32_t elapsed_ticks, uint32_t ramp_ms, uint32_t move_ms, uint32_t current_time) {
    // Power control: ramp up, full speed, then ramp down
    if (elapsed_ticks < ramp_ms) {
        // Ramp up to full power
        encoder_x.align_inpTgt = (ALIGNMENT_X_POWER * elapsed_ticks) / ramp_ms;
    } else if (elapsed_ticks < (move_ms-ramp_ms)) {
        // Full power rotation
        encoder_x.align_inpTgt = ALIGNMENT_X_POWER;
    } else if (elapsed_ticks < move_ms) {
        // Ramp down to stop
        uint32_t decel_ticks = elapsed_ticks - (move_ms-ramp_ms);
        encoder_x.align_inpTgt = ALIGNMENT_X_POWER - (ALIGNMENT_X_POWER * decel_ticks) / ramp_ms;
    }

    // Update emulated encoder position during movement
    if (elapsed_ticks < move_ms) {
        #ifdef STATIC_ALIGN
        /* Hold emulated position fixed: no rotating field. Rotor should snap
         * to a single electrical alignment position. */
        encoder_x.emulated_mech_count = encoder_x.align_ini_pos;
        #else
        int32_t total_increment = ((int64_t)encoder_x.count_increment_x1000 * elapsed_ticks) / 1000;
    encoder_x.emulated_mech_count = encoder_x.align_ini_pos + total_increment;
    encoder_x.emulated_mech_count = normalize_x_encoder_count(encoder_x.emulated_mech_count);
        #endif
    } else {
        // Rotation complete - snap to 0° electrical position
        int32_t counts_per_elec_cycle = ENCODER_X_CPR / N_POLE_PAIRS;
         encoder_x.offset = (ENCODER_X_CPR / N_POLE_PAIRS / 4); //90 degrees offset

      int32_t delta_from_start = encoder_x.emulated_mech_count - encoder_x.align_ini_pos;
      if (delta_from_start < 0) {
        delta_from_start += ENCODER_X_CPR;
      }
      int32_t current_elec_cycle = (delta_from_start + (counts_per_elec_cycle / 2)) / counts_per_elec_cycle;
      encoder_x.emulated_mech_count = encoder_x.align_ini_pos + (current_elec_cycle * counts_per_elec_cycle);
        encoder_x.emulated_mech_count = normalize_x_encoder_count(encoder_x.emulated_mech_count);
        encoder_x.align_state = 2;
        encoder_x.align_start_time = current_time;
    }
}

 void handle_x_high_power_phase(uint32_t elapsed_ticks, uint32_t ramp_ms, uint32_t current_time) {
    if (elapsed_ticks < ramp_ms) {
        // Ramp to double power
        encoder_x.align_inpTgt = (2*ALIGNMENT_X_POWER * elapsed_ticks) / ramp_ms;
    } else if (elapsed_ticks < T_MS(2000)) {
        // Hold at double power
        encoder_x.align_inpTgt = ALIGNMENT_X_POWER * 2;
    } else {
      int32_t seed_count;
      encoder_x.align_total_mid_pos = get_x_TotalCount();
      encoder_x.direction = (encoder_x.align_total_mid_pos >= encoder_x.align_total_ini_pos) ? 1 : 0;

      seed_count = normalize_x_encoder_count(encoder_x.emulated_mech_count - encoder_x.offset); // +90 degree offset align to Q axis
      Encoder_X_ApplyDirection(encoder_x.direction);
      /* Replace __HAL_TIM_SET_COUNTER: shift the MT6701 reading by an offset so the
       * next count_x_update() yields ENCODER_COUNT == seed_count. Must include
       * the direction inversion because count_x_update will apply it too. */
      {
        int32_t cur_scaled = ((int32_t)encoder_x.raw_angle * (int32_t)ENCODER_X_CPR) / (int32_t)MT6701_CPR;
        if (!encoder_x.direction) {
          cur_scaled = (int32_t)(ENCODER_X_CPR - 1) - cur_scaled;
        }
        encoder_x.cnt_offset = normalize_x_encoder_count(seed_count - cur_scaled);
      }
      encoder_x.ENCODER_COUNT = seed_count;
      encoder_x.count_prev = seed_count;
      encoder_x.full_rotations = 0;
        encoder_x.align_zero_pos = encoder_x.emulated_mech_count;
        encoder_x.align_start_time = current_time;
        encoder_x.align_state = 3;
    }
}

 void handle_x_move_back_phase(uint32_t elapsed_ticks, uint32_t ramp_ms, uint32_t move_ms, uint32_t current_time) {
    // Stage A: Ramp down from high power to normal power
    if (elapsed_ticks < ramp_ms) {
        uint32_t decel_ticks = ramp_ms - elapsed_ticks;
        int32_t ramp_target = ALIGNMENT_X_POWER + (ALIGNMENT_X_POWER * (int32_t)decel_ticks) / (int32_t)ramp_ms;
        encoder_x.align_inpTgt = (int16_t)ramp_target;
    }
    // Stage B: Move emulated position back toward start
    else if (elapsed_ticks < move_ms) {
        int32_t delta = encoder_x.align_zero_pos - encoder_x.align_ini_pos ;
    
    // Find shortest path (handle encoder wrap-around)
       if (delta > (ENCODER_X_CPR / 2)) delta -= ENCODER_X_CPR;
       if (delta < -(ENCODER_X_CPR / 2)) delta += ENCODER_X_CPR;
    // Interpolate position
        int32_t moved = (delta * (elapsed_ticks - ramp_ms)) / (move_ms - ramp_ms);
        encoder_x.emulated_mech_count = encoder_x.align_zero_pos - moved;
        encoder_x.emulated_mech_count = normalize_x_encoder_count(encoder_x.emulated_mech_count);

        encoder_x.align_inpTgt = ALIGNMENT_X_POWER;
    }
    // Stage C: Final ramp down to zero and finish
    else if (elapsed_ticks >= move_ms) {
        uint32_t final_ramp_time = elapsed_ticks - move_ms;
        if (final_ramp_time < ramp_ms) {
            encoder_x.align_inpTgt = ALIGNMENT_X_POWER * (ramp_ms - final_ramp_time) / ramp_ms;
        } else {
            encoder_x.align_inpTgt = 0;
            finalize_x_alignment();
        }
    }
}
/* count_x_update DELETED. Encoder sampling is now exclusively driven by TIM7 IRQ
 * → MT6701_X_KickIT() → HAL_I2C_Mem_Read_IT() → MT6701_X_ProcessReading callback.
 * See the callback definition earlier in this file for how encoder_x fields are
 * updated. Two paths (blocking + IT) caused peripheral-state races. */

int32_t get_x_TotalCount(void){
return encoder_x.full_rotations * ENCODER_X_CPR + encoder_x.count_prev;
}
  int32_t normalize_x_encoder_count(int32_t count) {
    count %= ENCODER_X_CPR;
    if (count < 0) count += ENCODER_X_CPR;
    return count;
}

/* Mirror of MT6701_Y_KickIT / MT6701_Y_ProcessReading for encoder X on I2C2. */
void MT6701_X_KickIT(void)
{
#ifdef MT6701_BAREMETAL_X
    /* Bare-metal path — see MT6701_Y_KickIT for the full rationale. */
    MT6701_X_Bare_Kick(mt6701_x_rxbuf);
#else
    /* No longer gated by mt6701_x_recovery_req — the old blocking
     * count_x_update kept firing every 5 ms regardless of recovery state and
     * that is what let the motor track smoothly. Gating for 250 ms froze the
     * encoder feed to the FOC and made the rotor cog like a stepper.
     * Fast-fail inside MT6701_StartReadIT (state != READY / SR2.BUSY) still
     * protects against poking HAL while the peripheral is inconsistent. */
    if (mt6701_x_busy) return;
    if (!encoder_x.ini) return;
    mt6701_x_busy = 1;
    mt6701_x_busy_since = buzzerTimer;
    if (MT6701_StartReadIT(&hi2c2, mt6701_x_rxbuf) != HAL_OK) {
        mt6701_x_busy = 0;
        mt6701_x_recovery_req = 1;
    }
#endif
}

void MT6701_X_ProcessReading(void)
{
    uint8_t hi = mt6701_x_rxbuf[0];
    uint8_t lo = mt6701_x_rxbuf[1];
    uint16_t raw = (uint16_t)(((uint16_t)hi << 6) | (lo >> 2));

    /* Narrow zero-byte filter — mirror of MT6701_Y_ProcessReading. */
    static uint8_t x_zero_reject_streak = 0;
    if (encoder_x.ali && hi == 0U && lo == 0U
        && encoder_x.raw_angle > 100U
        && encoder_x.raw_angle < (MT6701_CPR - 100U)) {
        if (++x_zero_reject_streak < 3U) {
            encoder_x.last_hi_byte = hi;
            encoder_x.last_lo_byte = lo;
            return;
        }
        x_zero_reject_streak = 0;
    } else {
        x_zero_reject_streak = 0;
    }

    encoder_x.raw_angle     = raw;
    encoder_x.last_hi_byte  = hi;
    encoder_x.last_lo_byte  = lo;
    encoder_x.i2c_fail_count = 0;

    int32_t scaled = ((int32_t)raw * (int32_t)ENCODER_X_CPR) / (int32_t)MT6701_CPR;
    if (!encoder_x.direction) {
        scaled = (int32_t)(ENCODER_X_CPR - 1) - scaled;
    }
    encoder_x.ENCODER_COUNT = normalize_x_encoder_count(scaled + encoder_x.cnt_offset);

    int32_t delta = encoder_x.ENCODER_COUNT - encoder_x.count_prev;
    if (abs(delta) > (ENCODER_X_CPR / 2)) {
        encoder_x.full_rotations += (delta > 0) ? -1 : 1;
    }
    encoder_x.count_prev = encoder_x.ENCODER_COUNT;
#if defined(MT6701_MODE_PWM_X)
    encoder_speed_update(&speed_est_x, &encoder_x, ENCODER_X_CPR,
                         MT6701_X_Pwm_GetFrameCount());
#endif
}

 void finalize_x_alignment(void) {
         // Calculate direction
       int32_t MIN_MOVMENT = (ENCODER_X_CPR / N_POLE_PAIRS) * 1;
        encoder_x.align_total_end_pos = get_x_TotalCount();
       int32_t movement =  abs(encoder_x.align_total_mid_pos - encoder_x.align_total_ini_pos);

       #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
       printf("[ALIGN X] finalize: ini=%ld mid=%ld end=%ld\r\n",
         (long)encoder_x.align_total_ini_pos,
         (long)encoder_x.align_total_mid_pos,
         (long)get_x_TotalCount());
       #endif

       if (abs(movement) < MIN_MOVMENT){
        encoder_x.align_fault = true;
        encoder_x.ali = false;
        encoder_x.align_state = 0;
        #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
        printf("[ALIGN X] FAULT: movement=%ld < min=%ld (motor stuck / current too low / phases swapped)\r\n",
          (long)movement, (long)MIN_MOVMENT);
        #endif
       } else {
        encoder_x.align_fault = false;
        encoder_x.ali = true;
        rtP_Right.b_diagEna = DIAG_ENA;
        encoder_x.align_state = 0;
        #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
        printf("[ALIGN X] OK: movement=%ld offset=%ld direction=%d\r\n",
          (long)movement, (long)encoder_x.offset, (int)encoder_x.direction);
        #endif
       }
}
#endif // ENCODER_X

#if defined (ENCODER_Y)

static void Encoder_Y_ApplyDirection(boolean_T forward_dir) {
  /* With MT6701 absolute sensor, direction inversion is software-only. */
  encoder_y.direction = forward_dir;
}

void Encoder_Y_Init(void) {
#if defined(MT6701_MODE_PWM_Y)
    /* Y reads its MT6701 via PWM input capture (TIM4_CH1 on PB6), not I2C.
     * The capture timer is set up once in main() by MT6701_Y_Pwm_Init().
     * Do NOT call I2C1_Init() — it would reconfigure PB6 as I2C1_SCL and
     * destroy the capture. Wait until valid frames accumulate before
     * declaring the encoder ready. See Encoder_X_Init for the full rationale. */
    if (MT6701_Y_Pwm_GetFrameCount() < 200u) {
        encoder_y.ini = false;
        return;
    }
    encoder_y.raw_angle      = 0;
    encoder_y.i2c_fail_count = 0;
    encoder_y.cnt_offset     = 0;
    encoder_y.ali            = false;
    encoder_y.align_fault    = false;
    encoder_y.offset         = 0;
    encoder_y.direction      = 1;
    encoder_y.aligned_count  = 0;
    encoder_y.ENCODER_COUNT  = 0;
    encoder_y.full_rotations = 0;
    encoder_y.align_state    = 0;
    encoder_y.align_ini_pos  = 0;
    MT6701_Y_Pwm_Feed();
    encoder_y.full_rotations = 0;
    encoder_y.ini = true;
    #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
    printf("MT6701_Y: PWM capture ready, seeded cnt=%ld\r\n",
      (long)encoder_y.ENCODER_COUNT);
    #endif
    motor_cfg_apply_stored_alignment_y();   /* replay saved calibration if any */
    return;
#else
    /* SWAPPED: MT6701 sensor on I2C1 (PB6/PB7) — per user preference to
     * make Y label correspond to physical board layout. */
    I2C1_Init();
    #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
    static uint8_t y_first_init_logged = 0;
    if (!y_first_init_logged) {
        y_first_init_logged = 1;
        printf("I2C1 init done (Y). SR2=0x%04X\r\n", (unsigned)I2C1->SR2);
    }
    #endif

    /* Reset all encoder state fields. Blocking bootstrap read follows. */
    encoder_y.raw_angle = 0;
    encoder_y.i2c_fail_count = 0;
    encoder_y.cnt_offset = 0;
    encoder_y.ali = false;
    encoder_y.align_fault = false;
    encoder_y.offset = 0;
    encoder_y.direction = 1;
    encoder_y.aligned_count = 0;
    encoder_y.ENCODER_COUNT = 0;
    encoder_y.full_rotations = 0;
    encoder_y.align_state = 0;
    encoder_y.align_ini_pos = 0;

    /* Blocking bootstrap read — verifies sensor is present. Called only
     * before TIM7 sampler is armed (ini==false gate), so no contention. */
    uint16_t test_angle = 0;
    HAL_StatusTypeDef st = MT6701_ReadAngle(&hi2c1, &test_angle);
    if (st != HAL_OK) {
        #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
        static uint32_t y_diag_ts = 0;
        if ((buzzerTimer - y_diag_ts) > 16000u) {
            y_diag_ts = buzzerTimer;
            printf("Y read fail: hal=%d SR1=0x%04X SR2=0x%04X\r\n",
                (int)st, (unsigned)I2C1->SR1, (unsigned)I2C1->SR2);
        }
        #endif
        encoder_y.ini = false;
        return;
    }
    encoder_y.raw_angle = test_angle;
    encoder_y.ini = true;
#endif  /* MT6701_MODE_PWM_Y */

    /* Same reasoning as Encoder_X_Init: restore stored alignment (if any)
     * after the reset above so `!encoder_y.ali` doesn't retrigger auto-align. */
    motor_cfg_apply_stored_alignment_y();
}




void Encoder_Y_Align_Start(void) {
  if (encoder_y.align_state != 0) {
        return;
    }

  if (encoder_y.align_fault) {
    return;
  }

  #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
  printf("[ALIGN Y] START align_i=%u.%02uA (raw=%d) i_mot_max=%dA\r\n",
    motor_cfg.align_i_y_x100 / 100, motor_cfg.align_i_y_x100 % 100,
    (int)ALIGNMENT_Y_POWER, (int)motor_cfg.i_mot_max);
  #endif

    enable = 1;
  rtP_Left.b_diagEna = 0; // Disable diagnostics during alignment
  rtP_Right.b_diagEna = 0; // Disable diagnostics during alignment
  encoder_y.ali = false;
  encoder_y.align_state = 1; // Start alignment sequence
  encoder_y.align_timer = 0;
  encoder_y.align_start_time = buzzerTimer;
  /* encoder_y.ENCODER_COUNT is kept fresh by TIM7 IT sampler at 1 kHz. */
  encoder_y.align_ini_pos = encoder_y.ENCODER_COUNT;
  encoder_y.align_total_ini_pos = get_y_TotalCount();
    // Initialize simulation variables
  /* Target sweep: N electrical rotations per motor_cfg.align_turns_elec.
   * MOVE_MS scales in Encoder_Y_Align so mechanical speed is constant. */
  uint32_t turns_y = motor_cfg.align_turns_elec ? motor_cfg.align_turns_elec : 2u;
  uint32_t move_ms_y = (T_MS(1500) * turns_y) / 2u;
  encoder_y.count_increment_x1000 = (int32_t)((((int64_t)ENCODER_Y_CPR) * (int64_t)(turns_y * 1000u)) / (((int64_t)N_POLE_PAIRS) * (int64_t)move_ms_y));
    encoder_y.align_inpTgt = 0; // Start with 0 power, will ramp up
}

// Non-blocking encoder alignment with mechanical angle simulation - call from main loop
void Encoder_Y_Align(void) {
    uint32_t current_time = buzzerTimer;
    uint32_t elapsed_ticks = current_time - encoder_y.align_start_time;
    /* Encoder state kept fresh by TIM7 IT sampler — no blocking read here. */

  #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
  static uint8_t y_prev_align_state = 0;
  if (encoder_y.align_state != y_prev_align_state) {
    y_prev_align_state = encoder_y.align_state;
    if (encoder_y.align_state == 2) {
      printf("[ALIGN Y] state 2: high-power hold, pos=%ld\r\n",
        (long)encoder_y.ENCODER_COUNT);
    } else if (encoder_y.align_state == 3) {
      printf("[ALIGN Y] state 3: move back, dir=%d offset=%ld\r\n",
        (int)encoder_y.direction, (long)encoder_y.offset);
    }
  }
  #endif
    
    const uint32_t RAMP_MS = T_MS(125);
    /* MOVE_MS scales with motor_cfg.align_turns_elec (see Align_Start comment). */
    uint32_t turns_y = motor_cfg.align_turns_elec ? motor_cfg.align_turns_elec : 2u;
    const uint32_t MOVE_MS = (T_MS(1500) * turns_y) / 2u;

    switch (encoder_y.align_state) {
        case 1: // Rotation phase - spin to find position
            handle_y_rotation_phase(elapsed_ticks, RAMP_MS, MOVE_MS, current_time);
            break;
            
        case 2: // High power phase - apply extra torque
            handle_y_high_power_phase(elapsed_ticks, RAMP_MS, current_time);
            break;
            
        case 3: // Move back and finish
            handle_y_move_back_phase(elapsed_ticks, RAMP_MS, MOVE_MS, current_time);
            break;
            
        default:
            encoder_y.align_state = 0;
            break;
    }
}

 void handle_y_rotation_phase(uint32_t elapsed_ticks, uint32_t ramp_ms, uint32_t move_ms, uint32_t current_time) {
    // Power control: ramp up, full speed, then ramp down
    if (elapsed_ticks < ramp_ms) {
        // Ramp up to full power
        encoder_y.align_inpTgt = (ALIGNMENT_Y_POWER * elapsed_ticks) / ramp_ms;
    } else if (elapsed_ticks < (move_ms-ramp_ms)) {
        // Full power rotation
        encoder_y.align_inpTgt = ALIGNMENT_Y_POWER;
    } else if (elapsed_ticks < move_ms) {
        // Ramp down to stop
        uint32_t decel_ticks = elapsed_ticks - (move_ms-ramp_ms);
        encoder_y.align_inpTgt = ALIGNMENT_Y_POWER - (ALIGNMENT_Y_POWER * decel_ticks) / ramp_ms;
    }

    // Update emulated encoder position during movement
    if (elapsed_ticks < move_ms) {
        int32_t total_increment = ((int64_t)encoder_y.count_increment_x1000 * elapsed_ticks) / 1000;
    encoder_y.emulated_mech_count = encoder_y.align_ini_pos + total_increment;
    encoder_y.emulated_mech_count = normalize_y_encoder_count(encoder_y.emulated_mech_count);
    } else {
        // Rotation complete - snap to 0° electrical position
        int32_t counts_per_elec_cycle = ENCODER_Y_CPR / N_POLE_PAIRS;
        encoder_y.offset = (ENCODER_Y_CPR / N_POLE_PAIRS/4);  // 90° electrical

      int32_t delta_from_start = encoder_y.emulated_mech_count - encoder_y.align_ini_pos;
      if (delta_from_start < 0) {
        delta_from_start += ENCODER_Y_CPR;
      }
      int32_t current_elec_cycle = (delta_from_start + (counts_per_elec_cycle / 2)) / counts_per_elec_cycle;
      encoder_y.emulated_mech_count = encoder_y.align_ini_pos + (current_elec_cycle * counts_per_elec_cycle);
        encoder_y.emulated_mech_count = normalize_y_encoder_count(encoder_y.emulated_mech_count);

        encoder_y.align_state = 2;
        encoder_y.align_start_time = current_time;
    }
}

 void handle_y_high_power_phase(uint32_t elapsed_ticks, uint32_t ramp_ms, uint32_t current_time) {
    if (elapsed_ticks < ramp_ms) {
        // Ramp to double power
        encoder_y.align_inpTgt = (2*ALIGNMENT_Y_POWER * elapsed_ticks) / ramp_ms;
    } else if (elapsed_ticks < T_MS(1000)) {
        // Hold at double power
        encoder_y.align_inpTgt = ALIGNMENT_Y_POWER * 2;
  } else {
    // Record final position and move to next phase
      int32_t seed_count;
      encoder_y.align_total_mid_pos = get_y_TotalCount();
      encoder_y.direction = (encoder_y.align_total_mid_pos >= encoder_y.align_total_ini_pos) ? 1 : 0;

      seed_count = normalize_y_encoder_count(encoder_y.emulated_mech_count - encoder_y.offset);
      Encoder_Y_ApplyDirection(encoder_y.direction);
      /* Same as X: apply direction inversion here so cnt_offset matches what
       * count_y_update will compute on next tick. */
      {
        int32_t cur_scaled = ((int32_t)encoder_y.raw_angle * (int32_t)ENCODER_Y_CPR) / (int32_t)MT6701_CPR;
        if (!encoder_y.direction) {
          cur_scaled = (int32_t)(ENCODER_Y_CPR - 1) - cur_scaled;
        }
        encoder_y.cnt_offset = normalize_y_encoder_count(seed_count - cur_scaled);
      }
      encoder_y.ENCODER_COUNT = seed_count;
      encoder_y.count_prev = seed_count;
      encoder_y.full_rotations = 0;
        encoder_y.align_zero_pos = encoder_y.emulated_mech_count;
        encoder_y.align_start_time = current_time;
        encoder_y.align_state = 3;
    }
}

 void handle_y_move_back_phase(uint32_t elapsed_ticks, uint32_t ramp_ms, uint32_t move_ms, uint32_t current_time) {
    // Stage A: Ramp down from high power to normal power
    if (elapsed_ticks < ramp_ms) {
        uint32_t decel_ticks = ramp_ms - elapsed_ticks;
        int32_t ramp_target = ALIGNMENT_Y_POWER + (ALIGNMENT_Y_POWER * (int32_t)decel_ticks) / (int32_t)ramp_ms;
        encoder_y.align_inpTgt = (int16_t)ramp_target;
    }
    // Stage B: Move emulated position back toward start
    else if (elapsed_ticks < move_ms) {
        int32_t delta = encoder_y.align_zero_pos - encoder_y.align_ini_pos ;
    
    // Find shortest path (handle encoder wrap-around)
       if (delta > (ENCODER_Y_CPR / 2)) delta -= ENCODER_Y_CPR;
       if (delta < -(ENCODER_Y_CPR / 2)) delta += ENCODER_Y_CPR;
    // Interpolate position
        int32_t moved = (delta * (elapsed_ticks - ramp_ms)) / (move_ms - ramp_ms);
        encoder_y.emulated_mech_count = encoder_y.align_zero_pos - moved;
        encoder_y.emulated_mech_count = normalize_y_encoder_count(encoder_y.emulated_mech_count);

        encoder_y.align_inpTgt = ALIGNMENT_Y_POWER;
    }
    // Stage C: Final ramp down to zero and finish
    else if (elapsed_ticks >= move_ms) {
        uint32_t final_ramp_time = elapsed_ticks - move_ms;
        if (final_ramp_time < ramp_ms) {
            encoder_y.align_inpTgt = ALIGNMENT_Y_POWER * (ramp_ms - final_ramp_time) / ramp_ms;
        } else {
            encoder_y.align_inpTgt = 0;
            finalize_y_alignment();
        }
    }
}
/* count_y_update DELETED — see count_x_update comment above. */

int32_t get_y_TotalCount(void){
return encoder_y.full_rotations * ENCODER_Y_CPR + encoder_y.count_prev;
}
  int32_t normalize_y_encoder_count(int32_t count) {
    count %= ENCODER_Y_CPR;
    if (count < 0) count += ENCODER_Y_CPR;
    return count;
}

/* IT-mode: kick a new 2-byte read. Runs from TIM7 IRQ, returns in ~5 µs.
 * Anti-reentrancy via mt6701_y_busy: HAL callbacks clear it on completion or
 * error. Bus recovery pending → don't touch I2C. */
void MT6701_Y_KickIT(void)
{
#ifdef MT6701_BAREMETAL_Y
    /* Bare-metal path: fully IT-driven state machine in mt6701.c.
     * Zero polling, zero HAL calls. TIM7 kick is < 5 us worst case. */
    MT6701_Y_Bare_Kick(mt6701_y_rxbuf);
#else
    /* No longer gated by mt6701_y_recovery_req — see MT6701_X_KickIT for the
     * full rationale. Short version: the old blocking count_y_update fired
     * every 5 ms regardless of recovery state and that is what let the FOC
     * see fresh encoder counts. Gating for 250 ms froze ENCODER_COUNT and
     * the rotor cogged like a stepper because the FOC angle stopped tracking
     * shaft position. Fast-fail inside MT6701_StartReadIT still stops us
     * from poking HAL while the peripheral is mid-recovery. */
    if (mt6701_y_busy) return;
    if (!encoder_y.ini) return;
    mt6701_y_busy = 1;
    mt6701_y_busy_since = buzzerTimer;   /* for main-loop stuck-busy watchdog */
    if (MT6701_StartReadIT(&hi2c1, mt6701_y_rxbuf) != HAL_OK) {
        /* Bus wasn't ready — request recovery and clear busy so the next tick
         * doesn't get stuck seeing busy without a callback ever firing. */
        mt6701_y_busy = 0;
        mt6701_y_recovery_req = 1;
    }
#endif
}

/* Process a completed IT read for encoder_y — mirrors the tail of the old
 * blocking count_y_update but reads from mt6701_y_rxbuf instead of hitting
 * I2C. Called from HAL_I2C_MemRxCpltCallback (HAL DMA path) OR from the
 * bare-metal EV_IRQ handler in mt6701.c (MT6701_BAREMETAL_Y). ISR context —
 * keep tight. */
void MT6701_Y_ProcessReading(void)
{
    uint8_t hi = mt6701_y_rxbuf[0];
    uint8_t lo = mt6701_y_rxbuf[1];
    uint16_t raw = (uint16_t)(((uint16_t)hi << 6) | (lo >> 2));

    /* Narrow filter against the specific silent-corruption pattern we've
     * seen in logs: bytes 0x00:0x00 arriving mid-torque while the previous
     * accepted angle was nowhere near 0. That is EMI clocking zeros onto
     * the wire past the AF/OVR/BERR detectors, not a legitimate crossing
     * of angle 0. Broader "delta > threshold" gates locked up the motor
     * when the shaft moved fast enough that even valid reads exceeded the
     * gate, so we keep this deliberately narrow.
     *
     * Escape: allow 3 consecutive zero-rejects, then accept regardless.
     * Prevents wedge if the sensor genuinely reports 0 for that long
     * (e.g. shaft actually parked at angle 0). */
    static uint8_t y_zero_reject_streak = 0;
    if (encoder_y.ali && hi == 0U && lo == 0U
        && encoder_y.raw_angle > 100U
        && encoder_y.raw_angle < (MT6701_CPR - 100U)) {
        if (++y_zero_reject_streak < 3U) {
            encoder_y.last_hi_byte = hi;
            encoder_y.last_lo_byte = lo;
            return;
        }
        y_zero_reject_streak = 0;   /* 3 consecutive zeros: accept as genuine */
    } else {
        /* Any non-zero (or near-zero-angle legitimate) read resets the
         * streak — without this, isolated rejects accumulate across the
         * whole session and the 3rd corrupted zero ever seen slips through. */
        y_zero_reject_streak = 0;
    }

    encoder_y.raw_angle     = raw;
    encoder_y.last_hi_byte  = hi;
    encoder_y.last_lo_byte  = lo;
    encoder_y.i2c_fail_count = 0;

    int32_t scaled = ((int32_t)raw * (int32_t)ENCODER_Y_CPR) / (int32_t)MT6701_CPR;
    if (!encoder_y.direction) {
        scaled = (int32_t)(ENCODER_Y_CPR - 1) - scaled;
    }
    encoder_y.ENCODER_COUNT = normalize_y_encoder_count(scaled + encoder_y.cnt_offset);

    int32_t delta = encoder_y.ENCODER_COUNT - encoder_y.count_prev;
    if (abs(delta) > (ENCODER_Y_CPR / 2)) {
        encoder_y.full_rotations += (delta > 0) ? -1 : 1;
    }
    encoder_y.count_prev = encoder_y.ENCODER_COUNT;
#if defined(MT6701_MODE_PWM_Y)
    encoder_speed_update(&speed_est_y, &encoder_y, ENCODER_Y_CPR,
                         MT6701_Y_Pwm_GetFrameCount());
#endif
}

 void finalize_y_alignment(void) {
    #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
    printf("[ALIGN Y] finalize: ini=%ld mid=%ld end=%ld\r\n",
      (long)encoder_y.align_total_ini_pos,
      (long)encoder_y.align_total_mid_pos,
      (long)get_y_TotalCount());
    #endif
    // Calculate offset
       int32_t MIN_MOVMENT = (ENCODER_Y_CPR / N_POLE_PAIRS) * 1;
        encoder_y.align_total_end_pos = get_y_TotalCount();
       int32_t movement =  abs(encoder_y.align_total_mid_pos - encoder_y.align_total_ini_pos);
       
       if (abs(movement) < MIN_MOVMENT){
        encoder_y.align_fault = true;
        encoder_y.ali = false;
        encoder_y.align_state = 0;
        #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
        printf("[ALIGN Y] FAULT: movement=%ld < min=%ld (motor stuck / current too low / phases swapped)\r\n",
          (long)movement, (long)MIN_MOVMENT);
        #endif
      } else {
        encoder_y.align_fault = false;
        encoder_y.ali = true;
        rtP_Left.b_diagEna = DIAG_ENA;
        encoder_y.align_state = 0;
        #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
        printf("[ALIGN Y] OK: movement=%ld offset=%ld direction=%d\r\n",
          (long)movement, (long)encoder_y.offset, (int)encoder_y.direction);
        #endif
       }
}

#endif // ENCODER_Y
/* =========================== General Functions =========================== */

void poweronMelody(void) {
    buzzerCount = 0;  // prevent interraction with beep counter
    for (int i = 8; i >= 0; i--) {
      buzzerFreq = (uint8_t)i;
      HAL_Delay(100);
    }
    buzzerFreq = 0;
}

void beepCount(uint8_t cnt, uint8_t freq, uint8_t pattern) {
    buzzerCount   = cnt;
    buzzerFreq    = freq;
    buzzerPattern = pattern;
}

void beepLong(uint8_t freq) {
    buzzerCount = 0;  // prevent interraction with beep counter
    buzzerFreq = freq;
    HAL_Delay(500);
    buzzerFreq = 0;
}

void beepShort(uint8_t freq) {
    buzzerCount = 0;  // prevent interraction with beep counter
    buzzerFreq = freq;
    HAL_Delay(100);
    buzzerFreq = 0;
}

void beepShortMany(uint8_t cnt, int8_t dir) {
    if (dir >= 0) {   // increasing tone
      for(uint8_t i = 2*cnt; i >= 2; i=i-2) {
        beepShort(i + 3);
      }
    } else {          // decreasing tone
      for(uint8_t i = 2; i <= 2*cnt; i=i+2) {
        beepShort(i + 3);
      }
    }
}

/* ------------------ BASEPRI helper implementation ------------------ */
/**
 * Set BASEPRI threshold to mask interrupts whose numeric priority is
 * greater or equal to 'prio'. The 'prio' value is the numeric
 * priority passed to HAL_NVIC_SetPriority (0 = highest priority).
 *
 * Returns the previous BASEPRI raw value (call basepri_restore() with it
 * to restore the previous mask).
 */
uint32_t basepri_set_threshold(uint32_t prio)
{
  uint32_t prev = __get_BASEPRI();
  /* Priority field is stored left-aligned in an 8-bit field; only the
   * top __NVIC_PRIO_BITS are implemented. Shift 'prio' into the MSBs.
   */
  const uint32_t shift = 8U - (uint32_t)__NVIC_PRIO_BITS;
  uint32_t basepri_val = (prio << shift) & 0xFFU;

  /* Write BASEPRI then ensure the write takes effect before continuing. */
  __set_BASEPRI(basepri_val);
  __DSB(); __ISB();
  return prev;
}

/**
 * Restore prior BASEPRI value returned by basepri_set_threshold().
 */
void basepri_restore(uint32_t previous_basepri)
{
  __set_BASEPRI(previous_basepri);
  __DSB(); __ISB();
}

/* Usage example:
 * uint32_t old = basepri_set_threshold(2); // allow priorities 0 and 1, block 2+
 * // critical work here (keep short)
 * basepri_restore(old);
 *
 * Notes:
 * - 'prio' is the numeric priority (0..(2^__NVIC_PRIO_BITS -1)).
 * - If you call HAL_NVIC_SetPriority(IRQn, preempt, sub) the effective
 *   numeric priority encoded into the NVIC depends on the priority
 *   grouping; simplest approach is to pick values consistent with the
 *   preempt field you use when calling HAL_NVIC_SetPriority.
 * - Avoid long blocking sections; use BASEPRI to prevent lower-priority
 *   interrupts from preempting time-critical code while still allowing
 *   higher-priority handlers to run.
 */


void calcAvgSpeed(void) {
    // Calculate measured average speed. The minus sign (-) is because motors spin in opposite directions
    speedAvg = 0;
    #if defined(MOTOR_LEFT_ENA)
      #if defined(INVERT_L_DIRECTION)
        speedAvg -= rtY_Left.n_mot;
      #else
        speedAvg += rtY_Left.n_mot;
      #endif
    #endif
    #if defined(MOTOR_RIGHT_ENA)
      #if defined(INVERT_R_DIRECTION)
        speedAvg += rtY_Right.n_mot;
      #else
        speedAvg -= rtY_Right.n_mot;
      #endif

      // Average only if both motors are enabled
      #if defined(MOTOR_LEFT_ENA)
        speedAvg /= 2;
      #endif  
    #endif

    // Handle the case when SPEED_COEFFICIENT sign is negative (which is when most significant bit is 1)
    if (SPEED_COEFFICIENT & (1 << 16)) {
      speedAvg    = -speedAvg;
    } 
    speedAvgAbs   = abs(speedAvg);
}

 /*
 * Auto-calibration of the ADC Limits
 * This function finds the Minimum, Maximum, and Middle for the ADC input
 * Procedure:
 * - press the power button for more than 5 sec and release after the beep sound
 * - move the potentiometers freely to the min and max limits repeatedly
 * - release potentiometers to the resting postion
 * - press the power button to confirm or wait for the 20 sec timeout
 * The Values will be saved to flash. Values are persistent if you flash with platformio. To erase them, make a full chip erase.
 */
void adcCalibLim(void) {
}

 /*
 * Check Input Type
 * This function identifies the input type: 0: Disabled, 1: Normal Pot, 2: Middle Resting Pot
 */
int checkInputType(int16_t min, int16_t mid, int16_t max){

  int type = 0;  
  int16_t threshold = 200;

  if ((min / threshold) == (max / threshold) || (mid / threshold) == (max / threshold) || min > max || mid > max) {
    type = 0;
    printf("ignored");                // (MIN and MAX) OR (MID and MAX) are close, disable input
  } else {
    if ((min / threshold) == (mid / threshold)){
      type = 1;
      printf("a normal pot");        // MIN and MID are close, it's a normal pot
    } else {
      type = 2;
      printf("a mid-resting pot");   // it's a mid resting pot
    }

  }

  return type;
}



/* =========================== Input Functions =========================== */

 /*
 * Calculate Input Command
 * This function realizes dead-band around 0 and scales the input between [out_min, out_max]
 */
void calcInputCmd(InputStruct *in, int16_t out_min, int16_t out_max) {
  switch (in->typ){
    case 1: // Input is a normal pot
      in->cmd = CLAMP(MAP(in->raw, in->min, in->max, 0, out_max), 0, out_max);
      break;
    case 2: // Input is a mid resting pot
      if( in->raw > in->mid - in->dband && in->raw < in->mid + in->dband ) {
        in->cmd = 0;
      } else if(in->raw > in->mid) {
        in->cmd = CLAMP(MAP(in->raw, in->mid + in->dband, in->max, 0, out_max), 0, out_max);
      } else {
        in->cmd = CLAMP(MAP(in->raw, in->mid - in->dband, in->min, 0, out_min), out_min, 0);
      }
      break;
    default: // Input is ignored
      in->cmd = 0;
      break;
  }
}

 /*
 * TWO_AXIS timeout handler: on any timeout or fault, bring motors to OPEN_MODE
 * so power spins down in a controlled way. Encoder inputs are the only source.
 */
void handleTimeout(void) {
  uint8_t idx = (inIdx < INPUTS_NR) ? inIdx : 0;

  if ((timeoutFlgADC || timeoutFlgSerial || timeoutFlgGen || DLVPA() || overcurrent_fault()) && (!encoder_alignment_active())) {
    ctrlModReq       = CFG_OPEN_MODE;
    input1[idx].cmd  = 0;
    input2[idx].cmd  = 0;
  } else {
    ctrlModReq = ctrlModReqRaw;
  }
}

void readCommand(void) {
  calcInputCmd(&input1[inIdx], INPUT_MIN, INPUT_MAX);
  calcInputCmd(&input2[inIdx], INPUT_MIN, INPUT_MAX);
  handleTimeout();
}


/*
 * Check for new data received on USART2 with DMA: refactored function from https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx
 * - this function is called for every USART IDLE line detection, in the USART interrupt handler
 */
void usart2_rx_check(void)
{




}


/*
 * Check for new data received on USART3 with DMA: refactored function from https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx
 * - this function is called for every USART IDLE line detection, in the USART interrupt handler
 */
void usart3_rx_check(void)
{
  static uint32_t old_pos;
  uint32_t pos;  
  pos = rx_buffer_R_len - __HAL_DMA_GET_COUNTER(huart3.hdmarx);         // Calculate current position in buffer

  if (pos != old_pos) {                                                 // Check change in received data
    if (pos > old_pos) {                                                // "Linear" buffer mode: check if current position is over previous one
      usart_process_debug(&rx_buffer_R[old_pos], pos - old_pos);        // Process data
    } else {                                                            // "Overflow" buffer mode
      usart_process_debug(&rx_buffer_R[old_pos], rx_buffer_R_len - old_pos);        // Process data
      usart_process_debug(&rx_buffer_R[0], pos);                           // Process data
    }
  }



  old_pos = pos;                                                        // Update old position
  if (old_pos == rx_buffer_R_len) {                                     // Check and manually update if we reached end of buffer
    old_pos = 0;
  }
}

/*
 * Process Rx debug user command input
 */
/* --- Manual torque command state (fed by UART3 ASCII parser) ---
 * Written by usart_process_debug (below) and read by the main loop in main.c
 * which overrides pwml/pwmr after readCommand() when manual_cmd_active != 0.
 * Range: -1000..+1000 (same units as pwml/pwmr).
 */
/* Per-axis manual torque overrides.  Y -> pwml (left / TIM8),  X -> pwmr
 * (right / TIM1). Each axis has its own value + active flag so X and Y can be
 * driven independently from the dashboard. */
volatile int16_t manual_cmd_y = 0, manual_cmd_x = 0;
volatile uint8_t manual_active_y = 0, manual_active_x = 0;
volatile uint32_t manual_cmd_last_tick_y = 0, manual_cmd_last_tick_x = 0;
volatile uint8_t manual_deadman_tripped = 0;
volatile uint32_t manual_deadman_trip_count = 0;

uint8_t manual_cmd_deadman_update(uint32_t now_ticks)
{
  static uint32_t last_main_loop_counter = 0;
  static uint32_t last_main_progress_tick = 0;
  extern volatile uint32_t main_loop_counter;

  if (main_loop_counter != last_main_loop_counter) {
    last_main_loop_counter = main_loop_counter;
    last_main_progress_tick = now_ticks;
  }

  const uint8_t y_expired = manual_active_y &&
    ((uint32_t)(now_ticks - manual_cmd_last_tick_y) > MANUAL_CMD_TIMEOUT_TICKS);
  const uint8_t x_expired = manual_active_x &&
    ((uint32_t)(now_ticks - manual_cmd_last_tick_x) > MANUAL_CMD_TIMEOUT_TICKS);
  const uint8_t main_stalled = (manual_active_y || manual_active_x) &&
    ((uint32_t)(now_ticks - last_main_progress_tick) > MANUAL_CMD_TIMEOUT_TICKS);

  if (y_expired || x_expired || main_stalled) {
    /* One stale axis invalidates the complete belt command. Zero both axes in
     * the ISR-visible state and in the main-loop PWM targets immediately. */
    extern volatile int pwml;
    extern volatile int pwmr;
    manual_cmd_y = 0;
    manual_cmd_x = 0;
    manual_active_y = 0;
    manual_active_x = 0;
    pwml = 0;
    pwmr = 0;
    if (!manual_deadman_tripped) manual_deadman_trip_count++;
    manual_deadman_tripped = 1;
  }

  return manual_deadman_tripped;
}

/* Dashboard telemetry request flag — set by "?TLM", cleared by main loop
 * after the TLM line is emitted. No continuous streaming: every line the
 * dashboard sees comes from an explicit poll. */
volatile uint8_t tlm_request_pending = 0;

/* Local diagnostic recorder. At 200 Hz (one sample per 5 ms main-loop pass),
 * 512 compact samples cover about 2.56 seconds without UART traffic. */
#define DIAG_SAMPLE_MAX 512U
typedef struct {
  uint16_t t_ms;
  int16_t y_cmd, y_applied, y_iq, y_rpm;
  int16_t x_cmd, x_applied, x_iq, x_rpm;
  uint16_t flags;
} DiagSample;

static DiagSample diag_samples[DIAG_SAMPLE_MAX];
static volatile uint16_t diag_count = 0;
static volatile uint8_t diag_recording = 0;
static uint32_t diag_start_ticks = 0;

void diag_capture_sample(void)
{
  if (!diag_recording || diag_count >= DIAG_SAMPLE_MAX) {
    if (diag_count >= DIAG_SAMPLE_MAX) diag_recording = 0;
    return;
  }
  DiagSample *s = &diag_samples[diag_count++];
  s->t_ms = (uint16_t)((buzzerTimer - diag_start_ticks) / 16U);
  s->y_cmd = manual_cmd_y;
  s->y_applied = speed_limiter_y.applied_cmd;
  s->y_iq = rtY_Left.iq;
  s->y_rpm = encoder_y.rpm_q4;
  s->x_cmd = manual_cmd_x;
  s->x_applied = speed_limiter_x.applied_cmd;
  s->x_iq = rtY_Right.iq;
  s->x_rpm = encoder_x.rpm_q4;
  s->flags = (speed_limiter_y.active ? 1U : 0U)
           | (speed_limiter_x.active ? 2U : 0U)
           | (encoder_y.rpm_valid ? 4U : 0U)
           | (encoder_x.rpm_valid ? 8U : 0U)
           | (speed_limiter_y.feedback_fault ? 16U : 0U)
           | (speed_limiter_x.feedback_fault ? 32U : 0U)
           | (manual_deadman_tripped ? 64U : 0U);
}

static void diag_dump(void)
{
  if (manual_active_y || manual_active_x) {
    printf("ERR DG motors active; send E first\r\n");
    return;
  }
  diag_recording = 0;
  printf("DGHZ 200 COUNT %u\r\n", (unsigned)diag_count);
  printf("DGHEADER t,Ycmd,Yapp,Yiq,Yrpm,Xcmd,Xapp,Xiq,Xrpm,flags\r\n");
  for (uint16_t i = 0; i < diag_count; i++) {
    const DiagSample *s = &diag_samples[i];
    printf("DG %u,%d,%d,%d,%d,%d,%d,%d,%d,%u\r\n",
           (unsigned)s->t_ms,
           (int)s->y_cmd, (int)s->y_applied, (int)s->y_iq, (int)s->y_rpm,
           (int)s->x_cmd, (int)s->x_applied, (int)s->x_iq, (int)s->x_rpm,
           (unsigned)s->flags);
  }
  printf("DGE\r\n");
}

#ifndef DEBUG_SERIAL_PROTOCOL
extern volatile int pwml;
extern volatile int pwmr;
extern uint8_t enable;

static void manual_cmd_process_line(char *line)
{
  /* Strip leading whitespace. */
  while (*line == ' ' || *line == '\t') line++;
  if (*line == '\0') return;

  char cmd = *line;
  if (cmd >= 'a' && cmd <= 'z') cmd -= 32;
  char axis = line[1];                     /* optional X / Y suffix */
  if (axis >= 'a' && axis <= 'z') axis -= 32;

  switch (cmd) {
    case 'T': {
      /* T<v> both | TX<v> X only | TY<v> Y only. Range -32767..+32767; at the
       * extremes the FOC Iq target saturates at I_MOT_MAX (config.h). */
      int doX, doY; char *arg;
      if      (axis == 'X') { doX = 1; doY = 0; arg = line + 2; }
      else if (axis == 'Y') { doX = 0; doY = 1; arg = line + 2; }
      else                  { doX = 1; doY = 1; arg = line + 1; }
      while (*arg == ' ' || *arg == '\t' || *arg == '=' || *arg == ':') arg++;
      int val = (int)strtol(arg, NULL, 10);
      if (val >  32767) val =  32767;
      if (val < -32767) val = -32767;
      /* Publish freshness before active flags/commands. DMA1 IRQ has higher
       * priority than USART3 and may preempt this parser between assignments. */
      uint32_t now = buzzerTimer;
      if (doY) manual_cmd_last_tick_y = now;
      if (doX) manual_cmd_last_tick_x = now;
      if (doY) { manual_cmd_y = (int16_t)val; manual_active_y = 1; }
      if (doX) { manual_cmd_x = (int16_t)val; manual_active_x = 1; }
      manual_deadman_tripped = 0;
      /* Torque commands are streamed continuously by the belt controller.
       * Do not echo each sample: the replies add avoidable UART traffic and
       * can delay/corrupt the telemetry stream under motor load. */
      break;
    }
    case 'E': {
      /* E stop both | EX / EY stop one. Zero torque, release override. */
      if (axis == 'X')      { manual_cmd_x = 0; manual_active_x = 0; printf("OK STOP X\r\n"); }
      else if (axis == 'Y') { manual_cmd_y = 0; manual_active_y = 0; printf("OK STOP Y\r\n"); }
      else { manual_cmd_x = manual_cmd_y = 0; manual_active_x = manual_active_y = 0; printf("OK STOP\r\n"); }
      manual_deadman_tripped = 0;
      break;
    }
    case 'D': {
      /* Diagnostic capture: DSTART, DSTOP, DDUMP. Capture is local so tests
       * never stream telemetry while torque commands are active. */
      char *arg = line + 1;
      while (*arg == ' ' || *arg == '\t') arg++;
      if (((arg[0]|0x20)=='s') && ((arg[1]|0x20)=='t') && ((arg[2]|0x20)=='a')) {
        diag_count = 0;
        diag_start_ticks = buzzerTimer;
        diag_recording = 1;
        printf("OK DG START 200HZ MAX=%u\r\n", (unsigned)DIAG_SAMPLE_MAX);
      } else if (((arg[0]|0x20)=='s') && ((arg[1]|0x20)=='t') && ((arg[2]|0x20)=='o')) {
        diag_recording = 0;
        printf("OK DG STOP COUNT=%u\r\n", (unsigned)diag_count);
      } else if (((arg[0]|0x20)=='d') && ((arg[1]|0x20)=='u') && ((arg[2]|0x20)=='m')) {
        diag_dump();
      } else {
        printf("ERR DG use DSTART|DSTOP|DDUMP\r\n");
      }
      break;
    }
    case 'S': {
      printf("STATUS Yact=%u YT=%d Xact=%u XT=%d pwml=%d pwmr=%d en=%u Wd=%u Wdc=%lu",
             (unsigned)manual_active_y, (int)manual_cmd_y,
             (unsigned)manual_active_x, (int)manual_cmd_x,
             (int)pwml, (int)pwmr, (unsigned)enable,
             (unsigned)manual_deadman_tripped,
             (unsigned long)manual_deadman_trip_count);
      #if defined(ENCODER_Y)
        printf(" y_ini=%u y_ali=%u y_cnt=%ld y_raw=%u",
               (unsigned)encoder_y.ini, (unsigned)encoder_y.ali,
               (long)encoder_y.ENCODER_COUNT, (unsigned)encoder_y.raw_angle);
      #endif
      #if defined(ENCODER_X)
        printf(" x_ini=%u x_ali=%u x_cnt=%ld x_raw=%u",
               (unsigned)encoder_x.ini, (unsigned)encoder_x.ali,
               (long)encoder_x.ENCODER_COUNT, (unsigned)encoder_x.raw_angle);
      #endif
      printf("\r\n");
      break;
    }
    case 'C': {
      /* CFG operations:
       *   CFG DUMP            -> emit YAML
       *   CFG SAVE            -> persist to EEPROM
       *   CFG RESET           -> restore defaults (RAM only, follow with SAVE to keep)
       *   CFG <name>=<value>  -> set one field in RAM (immediate for runtime fields)
       */
      char *arg = line + 1;
      while (*arg == ' ' || *arg == '\t') arg++;
      /* Expect "FG" to complete "CFG" (case-insensitive). */
      if (!((arg[0] == 'F' || arg[0] == 'f') && (arg[1] == 'G' || arg[1] == 'g'))) {
        printf("ERR: use 'CFG DUMP|SAVE|RESET|<name>=<value>'\r\n"); break;
      }
      arg += 2;
      while (*arg == ' ' || *arg == '\t') arg++;

      /* Case-insensitive keyword match on the first few chars. */
      #define KW(s, k, n) ((s)[0] && (((s)[0]|0x20) == ((k)[0]|0x20)) && \
                          (n<2 || ((s)[1] && (((s)[1]|0x20) == ((k)[1]|0x20)))) && \
                          (n<3 || ((s)[2] && (((s)[2]|0x20) == ((k)[2]|0x20)))) && \
                          (n<4 || ((s)[3] && (((s)[3]|0x20) == ((k)[3]|0x20)))) && \
                          (n<5 || ((s)[4] && (((s)[4]|0x20) == ((k)[4]|0x20)))))
      if (*arg == '\0' || KW(arg, "DUMP", 4)) {
        motor_cfg_dump();
      } else if (KW(arg, "SAVE_ALIGN", 5)) {  /* match on first 5 chars = "SAVE_" then check underscore */
        /* SAVE_ALIGN captures encoder_[xy] offset/direction + sets align_stored=1
         * so the next boot skips auto-align. */
        uint8_t rc = motor_cfg_capture_alignment();
        if      (rc == 0) printf("OK CFG SAVE_ALIGN (align_stored=1)\r\n");
        else if (rc == 1) printf("ERR CFG SAVE_ALIGN: some axis not aligned yet\r\n");
        else              printf("ERR CFG SAVE_ALIGN: flash write failed\r\n");
      } else if (KW(arg, "SAVE", 4)) {
        printf(motor_cfg_save() == 0 ? "OK CFG SAVE\r\n" : "ERR CFG SAVE\r\n");
      } else if (KW(arg, "RESET", 5)) {
        motor_cfg_reset_defaults();
        motor_cfg_apply_runtime();
        printf("OK CFG RESET (RAM only; CFG SAVE to persist)\r\n");
      } else {
        /* Expect NAME=VALUE. Split on '=' or ':' or space. */
        char *eq = arg;
        while (*eq && *eq != '=' && *eq != ':' && *eq != ' ' && *eq != '\t') eq++;
        if (*eq == '\0') { printf("ERR: expected NAME=VALUE\r\n"); break; }
        *eq = '\0';
        char *val = eq + 1;
        while (*val == ' ' || *val == '\t' || *val == '=' || *val == ':') val++;
        int8_t rc = motor_cfg_set(arg, val);
        if (rc == 0) {
          motor_cfg_apply_runtime();
          printf("OK CFG %s=%s\r\n", arg, val);
        } else if (rc == -1) printf("ERR CFG unknown '%s'\r\n", arg);
        else                 printf("ERR CFG out-of-range '%s'\r\n", arg);
      }
      break;
    }
    case '?': {
      /* '?CFG' → dump config, '?TLM' → one-shot dashboard telemetry.
       * Bare '?' falls through to H. */
      char *rest = line + 1;
      while (*rest == ' ' || *rest == '\t') rest++;
      if (KW(rest, "CFG", 3)) { motor_cfg_dump(); break; }
      if (KW(rest, "TLM", 3)) { tlm_request_pending = 1; break; }
      /* fall through */
    }
    case 'H': {
      printf("CMDS: T<v> both | TX<v> | TY<v> | E stop | EX | EY | S status | H help\r\n"
             "      ?TLM one-shot dashboard telemetry\r\n"
             "      DSTART | DSTOP | DDUMP diagnostic recorder\r\n"
             "      CFG DUMP | CFG SAVE | CFG RESET | CFG <name>=<value>\r\n");
      break;
    }
    default:
      printf("ERR: unknown '%c' (H for help)\r\n", *line);
      break;
  }
}
#endif /* !DEBUG_SERIAL_PROTOCOL */

void usart_process_debug(uint8_t *userCommand, uint32_t len)
{
  #ifdef DEBUG_SERIAL_PROTOCOL
    static uint8_t debug_buffer[SERIAL_BUFFER_SIZE];
    static size_t pos = 0;
    static enum {
      WAIT_START,
      IN_FRAME
    } state = WAIT_START;

    for (size_t i = 0; i < len; i++) {
      if (state == WAIT_START) {
        pos = 0;
      }
      if (userCommand[i] == '$') {
        state = IN_FRAME;
      }
      if (state != IN_FRAME) {
        continue;
      }

      debug_buffer[pos++] = userCommand[i];
      if (userCommand[i] == '\n' || userCommand[i] == '\r') {
        state = WAIT_START;
        handle_input(debug_buffer, pos);
        continue;
      }

      if (pos >= SERIAL_BUFFER_SIZE) {
        state = WAIT_START;
        pos = 0;
      }
    }
  #else
    /* Line-based ASCII parser: accumulate until CR/LF, then dispatch. */
    static char   cmd_line[32];
    static uint8_t cmd_idx = 0;

    for (uint32_t i = 0; i < len; i++) {
      char c = (char)userCommand[i];
      if (c == '\n' || c == '\r') {
        if (cmd_idx > 0) {
          cmd_line[cmd_idx] = '\0';
          manual_cmd_process_line(cmd_line);
          cmd_idx = 0;
        }
      } else if (cmd_idx < sizeof(cmd_line) - 1) {
        cmd_line[cmd_idx++] = c;
      } else {
        /* Overflow: reset silently, next char starts a fresh line. */
        cmd_idx = 0;
      }
    }
  #endif
}


/*
 * Process command Rx data
 * - if the command_in data is valid (correct START_FRAME and checksum) copy the command_in to command_out
 */

/*
 * Process Sideboard Rx data
 * - if the Sideboard_in data is valid (correct START_FRAME and checksum) copy the Sideboard_in to Sideboard_out
 */


/* =========================== Sideboard Functions =========================== */

/*
 * Sideboard LEDs Handling
 * This function manages the leds behavior connected to the sideboard
 */
void sideboardLeds(uint8_t *leds) {
}

/*
 * Sideboard Sensor Handling
 * This function manages the sideboards photo sensors.
 * In non-hoverboard variants, the sensors are used as push buttons.
 */
void sideboardSensors(uint8_t sensors) {
}



/* =========================== Poweroff Functions =========================== */

 /*
 * Save Configuration to Flash
 * This function makes sure data is not lost after power-off
 */
void saveConfig() {
    if (inp_cal_valid || cur_spd_valid) {
        printf("Saving configuration to EEprom\r\n");

      HAL_FLASH_Unlock();
      EE_WriteVariable(VirtAddVarTab[0] , (uint16_t)FLASH_WRITE_KEY);
      EE_WriteVariable(VirtAddVarTab[1] , (uint16_t)rtP_Left.i_max);
      EE_WriteVariable(VirtAddVarTab[2] , (uint16_t)rtP_Left.n_max);
      for (uint8_t i=0; i<INPUTS_NR; i++) {
        EE_WriteVariable(VirtAddVarTab[ 3+8*i] , (uint16_t)input1[i].typ);
        EE_WriteVariable(VirtAddVarTab[ 4+8*i] , (uint16_t)input1[i].min);
        EE_WriteVariable(VirtAddVarTab[ 5+8*i] , (uint16_t)input1[i].mid);
        EE_WriteVariable(VirtAddVarTab[ 6+8*i] , (uint16_t)input1[i].max);
        EE_WriteVariable(VirtAddVarTab[ 7+8*i] , (uint16_t)input2[i].typ);
        EE_WriteVariable(VirtAddVarTab[ 8+8*i] , (uint16_t)input2[i].min);
        EE_WriteVariable(VirtAddVarTab[ 9+8*i] , (uint16_t)input2[i].mid);
        EE_WriteVariable(VirtAddVarTab[10+8*i] , (uint16_t)input2[i].max);
      }
      HAL_FLASH_Lock();
    }
}


void poweroff(void) {
  enable = 0;
  printf("-- Motors disabled --\r\n");
  buzzerCount = 0;  // prevent interraction with beep counter
  buzzerPattern = 0;
  for (int i = 0; i < 8; i++) {
    buzzerFreq = (uint8_t)i;
    HAL_Delay(100);
  }
  saveConfig();
  HAL_GPIO_WritePin(OFF_PORT, OFF_PIN, GPIO_PIN_RESET);
  while(1) {}
}


void poweroffPressCheck(void) {
    if (powerButtonPressed()) {
      uint16_t cnt_press = 0;
      while (powerButtonPressed()) {
        HAL_Delay(10);
        if (cnt_press++ == 5 * 100) { beepShort(5); }
      }

      if (cnt_press > 8) {
        enable = 0;
      }

      if (cnt_press > 8 && cnt_press < 5 * 100) {         // Short press (80 ms – 5 s): power off
        printf("Powering off, button has been pressed\r\n");
        poweroff();
      }
    }
}



/* =========================== Filtering Functions =========================== */
  /* Low pass filter fixed-point 32 bits: fixdt(1,32,16)
  * Max:  32767.99998474121
  * Min: -32768
  * Res:  1.52587890625e-05
  * 
  * Inputs:       u     = int16 or int32
  * Outputs:      y     = fixdt(1,32,16)
  * Parameters:   coef  = fixdt(0,16,16) = [0,65535U]
  * 
  * Example: 
  * If coef = 0.8 (in floating point), then coef = 0.8 * 2^16 = 52429 (in fixed-point)
  * filtLowPass16(u, 52429, &y);
  * yint = (int16_t)(y >> 16); // the integer output is the fixed-point ouput shifted by 16 bits
  */
__attribute__((section(".ramfunc"), noinline))
void filtLowPass32(int32_t u, uint16_t coef, int32_t *y) {
  int64_t tmp;  
  tmp = ((int64_t)((u << 4) - (*y >> 12)) * coef) >> 4;
  tmp = CLAMP(tmp, -2147483648LL, 2147483647LL);  // Overflow protection: 2147483647LL = 2^31 - 1
  *y = (int32_t)tmp + (*y);
}
  // Old filter
  // Inputs:       u     = int16
  // Outputs:      y     = fixdt(1,32,20)
  // Parameters:   coef  = fixdt(0,16,16) = [0,65535U]
  // yint = (int16_t)(y >> 20); // the integer output is the fixed-point ouput shifted by 20 bits
  // void filtLowPass32(int16_t u, uint16_t coef, int32_t *y) {
  //   int32_t tmp;  
  //   tmp = (int16_t)(u << 4) - (*y >> 16);  
  //   tmp = CLAMP(tmp, -32768, 32767);  // Overflow protection  
  //   *y  = coef * tmp + (*y);
  // }


  /* rateLimiter16(int16_t u, int16_t rate, int16_t *y);
  * Inputs:       u     = int16
  * Outputs:      y     = fixdt(1,16,4)
  * Parameters:   rate  = fixdt(1,16,4) = [0, 32767] Do NOT make rate negative (>32767)
  */
void rateLimiter16(int16_t u, int16_t rate, int16_t *y) {
  int16_t q0;
  int16_t q1;

  q0 = (u << 4)  - *y;

  if (q0 > rate) {
    q0 = rate;
  } else {
    q1 = -rate;
    if (q0 < q1) {
      q0 = q1;
    }
  }

  *y = q0 + *y;
}
  /* mixerFcn(rtu_speed, rtu_steer, &rty_speedR, &rty_speedL); 
  * Inputs:       rtu_speed, rtu_steer                  = fixdt(1,16,4)
  * Outputs:      rty_speedR, rty_speedL                = int16_t
  * Parameters:   SPEED_COEFFICIENT, STEER_COEFFICIENT  = fixdt(0,16,14)
  */
 __attribute__((section(".ramfunc"), noinline))
void mixerFcn(int16_t rtu_speed, int16_t rtu_steer, int16_t *rty_speedR, int16_t *rty_speedL) {
    int16_t prodSpeed;
    int16_t prodSteer;
    int32_t tmp;

    prodSpeed   = (int16_t)((rtu_speed * (int16_t)SPEED_COEFFICIENT) >> 14);
    prodSteer   = (int16_t)((rtu_steer * (int16_t)STEER_COEFFICIENT) >> 14);

    tmp         = prodSpeed - prodSteer;  
    tmp         = CLAMP(tmp, -32768, 32767);  // Overflow protection
    *rty_speedR = (int16_t)(tmp >> 4);        // Convert from fixed-point to int 
    *rty_speedR = CLAMP(*rty_speedR, INPUT_MIN, INPUT_MAX);

    tmp         = prodSpeed + prodSteer;
    tmp         = CLAMP(tmp, -32768, 32767);  // Overflow protection
    *rty_speedL = (int16_t)(tmp >> 4);        // Convert from fixed-point to int
    *rty_speedL = CLAMP(*rty_speedL, INPUT_MIN, INPUT_MAX);
}

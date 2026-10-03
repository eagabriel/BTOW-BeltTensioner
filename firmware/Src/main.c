/*
* This file is part of the hoverboard-firmware-hack project.
*
* Copyright (C) 2017-2018 Rene Hopf <renehopf@mac.com>
* Copyright (C) 2017-2018 Nico Stute <crinq@crinq.de>
* Copyright (C) 2017-2018 Niklas Fauth <niklas.fauth@kit.fail>
* Copyright (C) 2019-2020 Emanuel FERU <aerdronix@gmail.com>
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

#include <stdio.h>
#include <stdlib.h> // for abs()
#include "stm32f1xx_hal.h"
#include "defines.h"
#include "setup.h"
#include "config.h"
#include "util.h"
#include "mt6701.h"
#include "mt6701_pwm.h"
#include "BLDC_controller.h"      /* BLDC's header file */
#include "rtwtypes.h"
#include "comms.h"
#include "motor_cfg.h"
#include "speed_limiter.h"



int main(void);

void SystemClock_Config(void);
void verifyClocks(void);

typedef struct {  // Structure for clock diagnostics
  uint32_t sysclk_hz;
  uint32_t hclk_hz;
  uint32_t pclk1_hz;
  uint32_t pclk2_hz;
  uint32_t tim_apb1_hz;
  uint32_t tim_apb2_hz;
} ClockDiagnostics;

volatile ClockDiagnostics g_clockDiag = {0};

//------------------------------------------------------------------------
// Global variables set externally
//------------------------------------------------------------------------
extern TIM_HandleTypeDef htim_left;
extern TIM_HandleTypeDef htim_right;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

extern volatile adc_buf_t adc_buffer;

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

volatile uint8_t uart_buf[200];

// Matlab defines - from auto-code generation
//---------------
extern P    rtP_Left;                   /* Block parameters (auto storage) */
extern P    rtP_Right;                  /* Block parameters (auto storage) */
extern ExtY rtY_Left;                   /* External outputs */
extern ExtY rtY_Right;                  /* External outputs */
extern ExtU rtU_Left;                   /* External inputs */
extern ExtU rtU_Right;                  /* External inputs */
extern DW   rtDW_Left;                  /* Internal states (diagnostic only) */
extern DW   rtDW_Right;
//---------------

extern uint8_t     inIdx;               // input index used for dual-inputs
extern uint8_t     inIdx_prev;
extern InputStruct input1[];            // input structure
extern InputStruct input2[];            // input structure

extern int16_t speedAvg;                // Average measured speed
extern int16_t speedAvgAbs;             // Average measured speed in absolute
extern volatile uint32_t timeoutCntGen; // Timeout counter for the General timeout (PPM, PWM, Nunchuk)
extern volatile uint8_t  timeoutFlgGen; // Timeout Flag for the General timeout (PPM, PWM, Nunchuk)
extern uint8_t timeoutFlgADC;           // Timeout Flag for for ADC Protection: 0 = OK, 1 = Problem detected (line disconnected or wrong ADC data)
extern uint8_t timeoutFlgSerial;        // Timeout Flag for Rx Serial command: 0 = OK, 1 = Problem detected (line disconnected or wrong Rx data)

extern volatile int pwml;               // global variable for pwm left. -1000 to 1000
extern volatile int pwmr;               // global variable for pwm right. -1000 to 1000

extern uint8_t enable;                  // global variable for motor enable

extern int16_t unf_VBUS;                // Calibrated battery voltage sample in V*100 from ISR path
int16_t batVoltage              = 400 * BAT_CELLS;
static int32_t batVoltageFixdt  = (400 * BAT_CELLS) << 16;  // Fixed-point filter starts at 4.00 V/cell in the same centivolt domain as unf_VBUS
int32_t board_temp_adcFixdt = 0;  
int16_t board_temp_adcFilt = 0;


//------------------------------------------------------------------------
// Global variables set here in main.c
//------------------------------------------------------------------------
uint8_t backwardDrive;
extern volatile uint32_t buzzerTimer;
extern uint8_t buzzerFreq;
volatile uint32_t main_loop_counter;
int16_t batVoltageCalib;         // global variable for calibrated battery voltage
int16_t board_temp_deg_c;        // global variable for calibrated temperature in degrees Celsius
int16_t left_dc_curr;            // global variable for Left DC Link current 
int16_t right_dc_curr;           // global variable for Right DC Link current
int16_t dc_curr;                 // global variable for Total DC Link current 
int16_t cmdL;                    // global variable for Left Command 
int16_t cmdR;                    // global variable for Right Command 

//------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------


static int16_t    speed;                // local variable for speed. -1000 to 1000
  static int16_t  steer;                // local variable for steering. -1000 to 1000
  static int16_t  steerRateFixdt;       // local fixed-point variable for steering rate limiter
  static int16_t  speedRateFixdt;       // local fixed-point variable for speed rate limiter
  static int32_t  steerFixdt;           // local fixed-point variable for steering low-pass filter
  static int32_t  speedFixdt;           // local fixed-point variable for speed low-pass filter

static uint32_t    buzzerTimer_prev = 0;
static uint32_t    inactivity_timeout_counter;
static uint16_t rate = RATE; // Adjustable rate to support multiple drive modes on startup



int main(void) {

  HAL_Init();
  __HAL_RCC_AFIO_CLK_ENABLE();
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  /* System interrupt init*/
  /* MemoryManagement_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
  /* BusFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
  /* UsageFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
  /* SVCall_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);
  /* DebugMonitor_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

  /* Enable the specific fault handlers in SCB so BusFault/UsageFault/MemManage
   * fire their own handlers instead of escalating to a generic HardFault.
   * Also enable trap on divide-by-zero and unaligned access — makes latent
   * bugs surface immediately as UsageFault instead of silent bad data. */
  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk
              | SCB_SHCSR_BUSFAULTENA_Msk
              | SCB_SHCSR_USGFAULTENA_Msk;
  SCB->CCR   |= SCB_CCR_DIV_0_TRP_Msk;    /* trap on div-by-zero */

  SystemClock_Config();
  verifyClocks();

  /* SystemClock_Config → HAL_SYSTICK_Config → SysTick_Config resets SysTick
   * priority to the LOWEST (15) via NVIC_SetPriority(..., (1<<__NVIC_PRIO_BITS)-1).
   * That silently defeated our earlier "SysTick=0" set. Force it back to 0 now
   * so SysTick can preempt other priority-0 IRQs like FOC/ADC — otherwise
   * uwTick freezes any time FOC is busy and every HAL_Delay / HAL_UART_Transmit
   * polling loop with a HAL_GetTick-based timeout hangs indefinitely. */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

  __HAL_RCC_DMA1_CLK_DISABLE();
  MX_GPIO_Init();
  MX_TIM_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  BLDC_Init();        // BLDC Controller Init

  HAL_GPIO_WritePin(OFF_PORT, OFF_PIN, GPIO_PIN_SET);   // Activate Latch
  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
  Input_Lim_Init();   // Input Limitations Init
  Input_Init();       // Input Init (also runs EE_Init)
  {
    /* Persistent motor configuration — overrides EEPROM legacy slots 1/2 for
     * i_max/n_max with the motor_cfg values, and applies invert/brake/etc. */
    uint8_t cfg_from_eeprom = motor_cfg_init();
    motor_cfg_apply_runtime();
    motor_cfg_apply_stored_alignment();   /* preloads encoder_[xy].ali if align_stored=1 */
    printf("motor_cfg loaded from %s%s\r\n",
      cfg_from_eeprom ? "EEPROM" : "defaults",
      motor_cfg.align_stored ? " (stored alignment restored — auto-align skipped)" : "");
  }
  estop_init();       // E-stop init (noop if disabled)

  HAL_ADC_Start(&hadc1);
  HAL_ADC_Start(&hadc2);
  HAL_ADC_Start(&hadc3);

  #if defined(ENCODER_X) || defined(ENCODER_Y)
  /* Start the MT6701 sampler ~1 kHz. The IRQ guards on encoder.ini so it does
   * nothing until Encoder_X/Y_Init succeeds later in the main loop. */
  MT6701_Sampler_Init();
  #endif

  #if defined(MT6701_MODE_PWM_X)
  /* MT6701 X in PWM output mode — PB10 free-runs on TIM2_CH3 IC. Init the
   * timer once here; sampling is autonomous from now on. See mt6701_pwm.c. */
  MT6701_X_Pwm_Init();
  #endif
  #if defined(MT6701_MODE_PWM_Y)
  /* MT6701 Y in PWM output mode — PB6 free-runs on TIM4_CH1 IC. */
  MT6701_Y_Pwm_Init();
  #endif

  poweronMelody();

  printf("\r\n=== BOOT ===\r\n");
  printf("bat_nominal=%u.%02uV  i_mot_max=%dA  i_dc_max=%dA  n_mot_max=%drpm\r\n",
    motor_cfg.bat_nominal_x100 / 100, motor_cfg.bat_nominal_x100 % 100,
    (int)motor_cfg.i_mot_max, (int)motor_cfg.i_dc_max, (int)motor_cfg.n_mot_max);
  #ifdef BRING_UP_MODE
  printf("BRING_UP_MODE active — bat/DC-link protections disabled\r\n");
  #endif
  #ifdef ENCODER_X
    #if defined(MT6701_MODE_PWM_X)
    printf("ENCODER_X: PWM input capture on PB10 (TIM2_CH3), align_i=%u.%02uA\r\n",
      motor_cfg.align_i_x_x100 / 100, motor_cfg.align_i_x_x100 % 100);
    #else
    printf("ENCODER_X: I2C1 PB6/7 @ %d Hz, align_i=%u.%02uA\r\n",
      (int)MT6701_I2C_CLOCK_HZ,
      motor_cfg.align_i_x_x100 / 100, motor_cfg.align_i_x_x100 % 100);
    #endif
  #else
  printf("ENCODER_X: DISABLED\r\n");
  #endif
  #ifdef ENCODER_Y
    #if defined(MT6701_MODE_PWM_Y)
    printf("ENCODER_Y: PWM input capture on PB6 (TIM4_CH1), align_i=%u.%02uA\r\n",
      motor_cfg.align_i_y_x100 / 100, motor_cfg.align_i_y_x100 % 100);
    #else
    printf("ENCODER_Y: I2C1 PB6/7 @ %d Hz, align_i=%u.%02uA\r\n",
      (int)MT6701_I2C_CLOCK_HZ,
      motor_cfg.align_i_y_x100 / 100, motor_cfg.align_i_y_x100 % 100);
    #endif
  #else
  printf("ENCODER_Y: DISABLED\r\n");
  #endif
  #ifdef SKIP_AUTO_ALIGN
  printf("SKIP_AUTO_ALIGN: alignment will NOT run automatically\r\n");
  #endif
  printf("=============\r\n");
  #if defined(DC_LINK_WATCHDOG_ENABLE)
  DcLinkWatchdog_Init();
  #endif
  
#if defined(ENABLE_BOARD_TEMP_SENSOR)
   board_temp_adcFixdt = adc_buffer.adc12.value.temp << 16;  // Fixed-point filter output initialized with current ADC converted to fixed-point
   board_temp_adcFilt  = adc_buffer.adc12.value.temp;
#endif


  // Loop until button is released (works for GPIO or analog button builds).
  while (powerButtonPressed()) { HAL_Delay(10); }

  
  static uint32_t enc_retry_ts = 0;
  static uint32_t heartbeat_ts = 0;
  static uint8_t  cal_done_prev = 0;

  while(1) {

    /* Throttle encoder (re-)init attempts so a missing MT6701 doesn't burn 3 ms of
     * I2C timeout on every main-loop iteration. 16 ticks/ms -> 8000 ticks = 500 ms. */
    uint8_t encoder_retry_ready = (buzzerTimer - enc_retry_ts) > 8000u;
    if (encoder_retry_ready) {
        enc_retry_ts = buzzerTimer;
    }

    /* Print once when BLDC current-offset calibration completes — signals that
     * FOC current sensing is now trusted and alignment can begin. */
    if (!cal_done_prev && BLDC_CurrentOffsetCalDone()) {
      cal_done_prev = 1;
      printf("[BLDC] current offsets calibrated — alignment can start\r\n");
    }

       #ifdef ENCODER_X
      if (!estop_active()) {
        if (!encoder_x.ini){
          if (encoder_retry_ready) {
            Encoder_X_Init();
          }
        }
        #ifndef SKIP_AUTO_ALIGN
        else if (!encoder_x.ali && BLDC_CurrentOffsetCalDone()) { // Start alignment only after current-offset calibration is complete
          // Run non-blocking encoder alignment
          if (encoder_x.align_state == 0) {
            Encoder_X_Align_Start(); // Start alignment if not already running
          }
          Encoder_X_Align(); // Process alignment state machine
        }
        #endif
      }
    #endif
    #ifdef ENCODER_Y
      if (!estop_active()) {
        if (!encoder_y.ini){
          if (encoder_retry_ready) {
            Encoder_Y_Init();
          }
        }
        #ifndef SKIP_AUTO_ALIGN
        else if (!encoder_y.ali && BLDC_CurrentOffsetCalDone()
              #ifdef ENCODER_X
                && encoder_x.ali        /* wait for X to finish — sequential
                                         * alignment avoids DC-bus/current-sense
                                         * cross-talk that skewed one axis's
                                         * offset when both swept together. */
              #endif
                ) {
          // Run non-blocking encoder alignment
          if (encoder_y.align_state == 0) {
            Encoder_Y_Align_Start(); // Start alignment if not already running
          }
          Encoder_Y_Align(); // Process alignment state machine
        }
        #endif
      }
    #endif
        

    if (buzzerTimer - buzzerTimer_prev > 16*DELAY_IN_MAIN_LOOP) {   // 1 ms = 16 ticks buzzerTimer
    /* MT6701 sampling now runs in TIM7 IRQ at MT6701_SAMPLE_RATE_HZ (~1 kHz).
     * Here we only service deferred I2C bus recovery that the ISR flagged.
     * Recovery is DEBOUNCED to at most once per 20 ms and its log is rate-limited
     * to once per second — motor EMI during alignment can otherwise trigger a
     * flurry of recoveries that fill the TX serial and back up main-loop time. */
    #if defined(ENCODER_X) || defined(ENCODER_Y)
    extern I2C_HandleTypeDef hi2c1;
    extern I2C_HandleTypeDef hi2c2;
    /* buzzerTimer ticks at 16 kHz — see PWM_FREQ. 16 ticks = 1 ms.
     * Debounce was originally 320 (20 ms) sized for HAL_I2C_Init which took
     * that long. FastRecover finishes in <200 µs, so gate recovery only long
     * enough to avoid re-entry from the same IRQ storm — 1 ms is plenty and
     * lets main loop retry every iteration when the bus is bad. Faster
     * recovery cadence directly reduces the encoder-freshness gap the FOC
     * sees during error bursts. */
    #define MT6701_RECOV_DEBOUNCE_TICKS 16u
    #define MT6701_RECOV_PRINT_TICKS   16000u
    #endif
    #if defined(ENCODER_X)
    #if defined(MT6701_BAREMETAL_X)
    /* Bare-metal X recovery. Fast path: I2C2_FastRecover is atomic and takes
     * ~1 µs typical, so we skip the NVIC dance the old HAL_I2C_Init path
     * needed. PE=0 inside the recover blocks IRQ generation for the reset
     * cycle; Kick re-enables ITEVTEN when it starts the next transaction. */
    static uint32_t recov_x_ts       = 0;
    static uint32_t recov_x_print_ts = 0;
    if (MT6701_X_Bare_NeedsRecover() && (buzzerTimer - recov_x_ts) > MT6701_RECOV_DEBOUNCE_TICKS) {
      recov_x_ts = buzzerTimer;
      #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
      if ((buzzerTimer - recov_x_print_ts) > MT6701_RECOV_PRINT_TICKS) {
        recov_x_print_ts = buzzerTimer;
        printf("I2C2 fast-recover X (SR2=0x%04X)\r\n", (unsigned)I2C2->SR2);
      }
      #endif
      I2C2_FastRecover();
      MT6701_X_Bare_ResetState();
    }
    #else
    extern volatile uint8_t mt6701_x_recovery_req;
    extern volatile uint8_t mt6701_x_busy;
    static uint32_t recov_x_ts       = 0;
    static uint32_t recov_x_print_ts = 0;
    if (mt6701_x_recovery_req && (buzzerTimer - recov_x_ts) > MT6701_RECOV_DEBOUNCE_TICKS) {
      mt6701_x_recovery_req = 0;
      recov_x_ts = buzzerTimer;
      /* Silence sampler + I2C IRQs during peripheral reset so no callback fires
       * mid-init and leaves state inconsistent. I2C1_Init re-enables them. */
      HAL_NVIC_DisableIRQ(TIM7_IRQn);
      HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
      HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
      #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
      if ((buzzerTimer - recov_x_print_ts) > MT6701_RECOV_PRINT_TICKS) {
        recov_x_print_ts = buzzerTimer;
        printf("I2C2 recover X (SR2=0x%04X errCode=0x%08lX)\r\n",
          (unsigned)I2C2->SR2, (unsigned long)hi2c2.ErrorCode);
      }
      #endif
      I2C_Init();
      mt6701_x_busy = 0;    /* IT transaction (if any) was killed by DeInit path */
      HAL_NVIC_EnableIRQ(TIM7_IRQn);
    }
    #endif  /* MT6701_BAREMETAL_X */
    #endif  /* ENCODER_X */
    #if defined(ENCODER_Y)
    #if defined(MT6701_BAREMETAL_Y)
    /* Bare-metal driver owns Y I2C. Only trigger a full peripheral re-init
     * when its internal watchdog signals a hard bus lockup (state stuck
     * >20 ms) — the state machine itself handles normal AF/OVR/BERR errors
     * inline in the ER IRQ (STOP + back to IDLE). No mt6701_y_busy watchdog,
     * no error-callback debounce — those existed to fight HAL_I2C_Mem_Read_DMA
     * failure modes that don't apply to the bare-metal path. */
    static uint32_t recov_y_ts       = 0;
    static uint32_t recov_y_print_ts = 0;
    if (MT6701_Y_Bare_NeedsRecover() && (buzzerTimer - recov_y_ts) > MT6701_RECOV_DEBOUNCE_TICKS) {
      recov_y_ts = buzzerTimer;
      #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
      if ((buzzerTimer - recov_y_print_ts) > MT6701_RECOV_PRINT_TICKS) {
        recov_y_print_ts = buzzerTimer;
        printf("I2C1 fast-recover Y (SR2=0x%04X)\r\n", (unsigned)I2C1->SR2);
      }
      #endif
      I2C1_FastRecover();
      MT6701_Y_Bare_ResetState();
    }
    #else
    extern volatile uint8_t mt6701_y_recovery_req;
    extern volatile uint8_t mt6701_y_busy;
    static uint32_t recov_y_ts       = 0;
    static uint32_t recov_y_print_ts = 0;
    /* Watchdog: if busy stayed 1 for >100 ms (1600 ticks) without either the
     * Rx-complete or Error callback firing, an IT transaction got stuck
     * (peripheral event never happened). Force recovery so we don't lose the
     * sampler forever. Timestamp is refreshed by the callbacks (set to
     * buzzerTimer any time busy transitions 0→1). */
    extern volatile uint32_t mt6701_y_busy_since;
    if (mt6701_y_busy && (buzzerTimer - mt6701_y_busy_since) > 1600u) {
      mt6701_y_recovery_req = 1;
    }
    if (mt6701_y_recovery_req && (buzzerTimer - recov_y_ts) > MT6701_RECOV_DEBOUNCE_TICKS) {
      mt6701_y_recovery_req = 0;
      recov_y_ts = buzzerTimer;
      HAL_NVIC_DisableIRQ(TIM7_IRQn);
      HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
      HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
      #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
      if ((buzzerTimer - recov_y_print_ts) > MT6701_RECOV_PRINT_TICKS) {
        recov_y_print_ts = buzzerTimer;
        printf("I2C1 recover Y (SR2=0x%04X errCode=0x%08lX)\r\n",
          (unsigned)I2C1->SR2, (unsigned long)hi2c1.ErrorCode);
      }
      #endif
      I2C1_Init();   /* re-enables I2C1_EV_IRQn / I2C1_ER_IRQn internally */
      mt6701_y_busy = 0;
      HAL_NVIC_EnableIRQ(TIM7_IRQn);
    }
    #endif  /* MT6701_BAREMETAL_Y */
    #endif  /* ENCODER_Y */

    /* Heartbeat: toggle LED every 500 ms to prove main loop is alive.
     * LED is independent of buzzer — both can run in parallel. */
    if ((buzzerTimer - heartbeat_ts) > 8000u) {
        heartbeat_ts = buzzerTimer;
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
    }

    estop_update();                       // E-stop update
    readCommand();                        // Read Command: input1[inIdx].cmd, input2[inIdx].cmd
    calcAvgSpeed();                       // Calculate average measured speed: speedAvg, speedAvgAbs
      // ####### MOTOR ENABLING: Only if the initial input is very small (for SAFETY) #######
        if (enable == 0 && !rtY_Left.z_errCode && !rtY_Right.z_errCode && !estop_active() &&
          ABS(input1[inIdx].cmd) < 50 && ABS(input2[inIdx].cmd) < 50){
        beepShort(6);                     // make 2 beeps indicating the motor enable
        beepShort(4); HAL_Delay(100);
        steerFixdt = speedFixdt = 0;      // reset filters
        enable = 1;                       // enable motors
        printf("-- Motors enabled --\r\n");
      }
     
      // ####### LOW-PASS FILTER #######
      rateLimiter16(input1[inIdx].cmd, rate, &steerRateFixdt);
      rateLimiter16(input2[inIdx].cmd, rate, &speedRateFixdt);
      filtLowPass32(steerRateFixdt >> 4, FILTER, &steerFixdt);
      filtLowPass32(speedRateFixdt >> 4 , FILTER, &speedFixdt);
      steer = (int16_t)(steerFixdt >> 16);  // convert fixed-point to integer
      speed = (int16_t)(speedFixdt >> 16);  // convert fixed-point to integer

        // ####### MIXER #######
        mixerFcn(speed << 4, steer << 4, &cmdR, &cmdL);   // This function implements the equations above


      // ####### SET OUTPUTS (if the target change is less than +/- 100) #######
      #ifdef INVERT_R_DIRECTION
        pwmr = cmdR;
      #else
        pwmr = -cmdR;
      #endif
      #ifdef INVERT_L_DIRECTION
        pwml = -cmdL;
      #else
        pwml = cmdL;
      #endif

    /* Manual serial-command torque override, per axis.
     * Y -> pwml (left/TIM8), X -> pwmr (right/TIM1). Set by the T/TX/TY
     * commands in usart_process_debug over UART3.
     *
     * T is dimensionless in ±32767 and passed straight through: the
     * Simulink-generated FOC (BLDC_controller_step) already maps r_inpTgt
     * proportionally onto ±rtP_*.i_max internally. Any extra scaling here
     * would double-scale (regression 49a1c0e — reverted; showed as 1 A
     * when i_mot_max=5 A because pwml*i_max/32767 got re-mapped by i_max
     * again in the FOC).
     *
     * invert_x/y flip the sign so a positive T spins the physical axis
     * in the desired direction. */
    extern volatile int16_t manual_cmd_y, manual_cmd_x;
    extern volatile uint8_t manual_active_y, manual_active_x;
    if (manual_active_y) pwml = motor_cfg.invert_y ? -manual_cmd_y : manual_cmd_y;
    if (manual_active_x) pwmr = motor_cfg.invert_x ? -manual_cmd_x : manual_cmd_x;

    /* Diagnostic recorder samples locally; no UART traffic during a test. */
    diag_capture_sample();




    // ####### SIDEBOARDS HANDLING #######

    
#if defined(ENABLE_BOARD_TEMP_SENSOR)
    // ####### CALC BOARD TEMPERATURE #######
    filtLowPass32(adc_buffer.adc12.value.temp, TEMP_FILT_COEF, &board_temp_adcFixdt);
    board_temp_adcFilt  = (int16_t)(board_temp_adcFixdt >> 16);  // convert fixed-point to integer
    board_temp_deg_c    = (TEMP_CAL_HIGH_DEG_C - TEMP_CAL_LOW_DEG_C) * (board_temp_adcFilt - TEMP_CAL_LOW_ADC) / (TEMP_CAL_HIGH_ADC - TEMP_CAL_LOW_ADC) + TEMP_CAL_LOW_DEG_C;
#endif
    // ####### CALC DC LINK CURRENT #######
    left_dc_curr  = -(rtU_Left.i_DCLink * 100) / A2BIT_CONV;   // Left DC Link Current * 100  
    right_dc_curr = -(rtU_Right.i_DCLink * 100) / A2BIT_CONV;  // Right DC Link Current * 100
    dc_curr       = left_dc_curr + right_dc_curr;            // Total DC Link Current * 100
    //###### BATTERY VOLTAGE FILTERING #######
    filtLowPass32(unf_VBUS, BAT_FILT_COEF, &batVoltageFixdt);
    batVoltage = (int16_t)( batVoltageFixdt >> 16);  // convert fixed-point to integer
    batVoltageCalib = batVoltage;
    // ####### DEBUG SERIAL OUT #######
      /* ---- MT6701 encoder event prints (once per state transition) ----
       * Runs every main-loop iteration for low latency on state changes. */
      #if defined(ENCODER_X)
      {
        static uint8_t prev_x_ini = 0, prev_x_ali = 0;
        if (encoder_x.ini != prev_x_ini) {
          printf("MT6701_X: %s\r\n", encoder_x.ini ? "DETECTED" : "LOST");
          prev_x_ini = encoder_x.ini;
        }
        if (encoder_x.ali != prev_x_ali) {
          printf("MT6701_X: %s\r\n", encoder_x.ali ? "ALIGNED" : "unaligned");
          prev_x_ali = encoder_x.ali;
        }
      }
      #endif
      #if defined(ENCODER_Y)
      {
        static uint8_t prev_y_ini = 0, prev_y_ali = 0;
        if (encoder_y.ini != prev_y_ini) {
          printf("MT6701_Y: %s\r\n", encoder_y.ini ? "DETECTED" : "LOST");
          prev_y_ini = encoder_y.ini;
        }
        if (encoder_y.ali != prev_y_ali) {
          printf("MT6701_Y: %s\r\n", encoder_y.ali ? "ALIGNED" : "unaligned");
          prev_y_ali = encoder_y.ali;
        }
      }
      #endif

      /* One-shot dashboard TLM emit — util.c's ?TLM command sets this flag,
       * we clear it after emitting. Checked every main-loop iteration so
       * latency ≤ DELAY_IN_MAIN_LOOP (5 ms). No periodic push anymore. */
      extern volatile uint8_t tlm_request_pending;
      if (tlm_request_pending) {
        tlm_request_pending = 0;
        int32_t vcv = batVoltageCalib;
        #define PR_CA(name, raw) do{ \
            int32_t _ca=(int32_t)(raw)*100/A2BIT_CONV; \
            int32_t _a=_ca<0?-_ca:_ca; \
            printf(" " name "=%s%ld.%02ld", _ca<0?"-":"", (long)(_a/100), (long)(_a%100)); \
          }while(0)
        #define PR_Q4(name, raw) do{ \
            int32_t _q=(int32_t)(raw); int32_t _a=_q<0?-_q:_q; \
            printf(" " name "=%s%ld.%02ld", _q<0?"-":"", \
                   (long)(_a/16), (long)((_a%16)*100/16)); \
          }while(0)
        extern volatile uint8_t manual_deadman_tripped;
        extern volatile uint32_t manual_deadman_trip_count;
        printf("TLM V=%ld.%02ld Imax=%d Kt=%u.%03u Wd=%u Wdc=%lu",
               (long)(vcv / 100), (long)(vcv % 100),
               (int)motor_cfg.i_mot_max,
               (unsigned)(motor_cfg.motor_kt_x1000 / 1000u),
               (unsigned)(motor_cfg.motor_kt_x1000 % 1000u),
               (unsigned)manual_deadman_tripped,
               (unsigned long)manual_deadman_trip_count);
        #if defined(ENCODER_Y)
        {
          extern int16_t curL_DC, curL_phaA, curL_phaB;
          extern volatile int16_t manual_cmd_y;
          int32_t ya = curL_phaA, yb = curL_phaB, yc = -(ya + yb);
          PR_CA("YiDC", curL_DC); PR_CA("YiA", ya); PR_CA("YiB", yb); PR_CA("YiC", yc);
          printf(" YT=%d Ypos=%u", (int)manual_cmd_y, (unsigned)encoder_y.raw_angle);
          PR_Q4("Yrpm", encoder_y.rpm_q4);
          printf(" Yrv=%u Yn=%d Yvqcap=%d Yfrm=%lu Yerr=%lu Yslw=%lu",
                 (unsigned)encoder_y.rpm_valid, (int)rtY_Left.n_mot,
                 (int)rtDW_Left.Integrator_DSTATE,
                 (unsigned long)MT6701_Y_Pwm_GetFrameCount(),
                 (unsigned long)MT6701_Y_Pwm_GetErrCount(),
                 (unsigned long)MT6701_Y_Pwm_GetSlewCount());
          printf(" Ycmd=%d Ycap=%d Yint=%d Ylim=%u Ysf=%u",
                 (int)speed_limiter_y.applied_cmd,
                 (int)speed_limiter_y.cap_magnitude,
                 (int)speed_limiter_y.integral_cmd,
                 (unsigned)speed_limiter_y.active,
                 (unsigned)speed_limiter_y.feedback_fault);
        }
        #endif
        #if defined(ENCODER_X)
        {
          extern int16_t curR_DC, curR_phaB, curR_phaC;
          extern volatile int16_t manual_cmd_x;
          int32_t xb = curR_phaB, xc = curR_phaC, xa = -(xb + xc);
          PR_CA("XiDC", curR_DC); PR_CA("XiA", xa); PR_CA("XiB", xb); PR_CA("XiC", xc);
          printf(" XT=%d Xpos=%u", (int)manual_cmd_x, (unsigned)encoder_x.raw_angle);
          PR_Q4("Xrpm", encoder_x.rpm_q4);
          printf(" Xrv=%u Xn=%d Xvqcap=%d Xfrm=%lu Xerr=%lu Xslw=%lu",
                 (unsigned)encoder_x.rpm_valid, (int)rtY_Right.n_mot,
                 (int)rtDW_Right.Integrator_DSTATE,
                 (unsigned long)MT6701_X_Pwm_GetFrameCount(),
                 (unsigned long)MT6701_X_Pwm_GetErrCount(),
                 (unsigned long)MT6701_X_Pwm_GetSlewCount());
          printf(" Xcmd=%d Xcap=%d Xint=%d Xlim=%u Xsf=%u",
                 (int)speed_limiter_x.applied_cmd,
                 (int)speed_limiter_x.cap_magnitude,
                 (int)speed_limiter_x.integral_cmd,
                 (unsigned)speed_limiter_x.active,
                 (unsigned)speed_limiter_x.feedback_fault);
        }
        #endif
        printf("\r\n");
        #undef PR_CA
        #undef PR_Q4
      }

    // ####### FEEDBACK SERIAL OUT #######

    // ####### POWEROFF BY POWER-BUTTON #######
    poweroffPressCheck();

    // ####### BEEP AND EMERGENCY POWEROFF #######
#if defined(ENABLE_BOARD_TEMP_SENSOR)
    if (TEMP_POWEROFF_ENABLE && board_temp_deg_c >= TEMP_POWEROFF && speedAvgAbs < 20){  // poweroff before mainboard burns OR low bat 3
      #if defined(DEBUG_SERIAL_USART2) || defined(DEBUG_SERIAL_USART3)
        printf("Powering off, temperature is too high\r\n");
      #endif
      poweroff();
    } else
#endif

    if ( BAT_DEAD_ENABLE && (batVoltage < BAT_DEAD || batVoltage < HARD_18V_COUNTS || DLVPA()) && speedAvgAbs < 20){
        printf("Powering off, battery voltage is too low\r\n");
      poweroff();
    } else if (rtY_Left.z_errCode || rtY_Right.z_errCode) {                                           // 1 beep (low pitch): Motor error, disable motors
      enable = 0;
      beepCount(1, 24, 1);
    } else if (timeoutFlgADC) {                                                                       // 2 beeps (low pitch): ADC timeout
      beepCount(2, 24, 1);
    } else if (timeoutFlgSerial) {                                                                    // 3 beeps (low pitch): Serial timeout
      beepCount(3, 24, 1);
    } else if (timeoutFlgGen) {                                                                       // 4 beeps (low pitch): General timeout (PPM, PWM, Nunchuk)
      beepCount(4, 24, 1);
#if defined(ENABLE_BOARD_TEMP_SENSOR)
    } else if (TEMP_WARNING_ENABLE && board_temp_deg_c >= TEMP_WARNING) {                             // 5 beeps (low pitch): Mainboard temperature warning
      beepCount(5, 24, 1);
#endif
    } else if (BAT_LVL1_ENABLE && batVoltage < BAT_LVL1) {                                            // 1 beep fast (medium pitch): Low bat 1
      beepCount(0, 10, 6);
    } else if (BAT_LVL2_ENABLE && batVoltage < BAT_LVL2) {                                            // 1 beep slow (medium pitch): Low bat 2
      beepCount(0, 10, 30);
    } else if (BEEPS_BACKWARD && (cmdR < -50 || cmdL < -50) && speedAvg < 0) {                        // 1 beep fast (high pitch): Backward spinning motors
      beepCount(0, 5, 1);
      backwardDrive = 1;
    } else if (DLVPA()) {
      beepCount(5, 10, 1);  // 1 beep very slow (high pitch): DLVPA active
      enable = 0;
    } 
    else {  // do not beep
      beepCount(0, 0, 0);
      backwardDrive = 0;
    }


    inactivity_timeout_counter++;

    // ####### INACTIVITY TIMEOUT #######
    if (abs(cmdL) > 50 || abs(cmdR) > 50) {
      inactivity_timeout_counter = 0;
    }


    /* INACTIVITY_TIMEOUT == 0 means feature disabled (avoid spurious poweroff because
     * threshold would be 0 and the counter trivially exceeds it on the first tick). */
    if (INACTIVITY_TIMEOUT > 0 &&
        inactivity_timeout_counter > (INACTIVITY_TIMEOUT * 60 * 1000) / (DELAY_IN_MAIN_LOOP + 1)) {  // rest of main loop needs maybe 1ms
        printf("Powering off, wheels were inactive for too long\r\n");
      poweroff();
    }


    // HAL_GPIO_TogglePin(LED_PORT, LED_PIN);                 // This is to measure the main() loop duration with an oscilloscope connected to LED_PIN
    // Update states
    inIdx_prev = inIdx;
    buzzerTimer_prev = buzzerTimer;
    main_loop_counter++;
    }
  }
}

// ===========================================================
/** System Clock Configuration
*/
void SystemClock_Config(void) {

#if defined(GD32F103Rx)
  /* Direct clock tree setup for GD32F103: target 108 MHz from HSI/2 with PLL x27. */
  /* Ensure HSI is ready */
  SET_BIT(RCC->CR, RCC_CR_HSION);
  while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U) {
    /* wait */
  }

  /* Disable PLL before reconfiguration */
  CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
  while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0U) {
    /* wait for PLL to stop */
  }

  /* Configure Flash wait states and enable prefetch for >72 MHz operation */
  MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE,
             FLASH_ACR_LATENCY_2 | FLASH_ACR_PRFTBE);

  /* AHB = SYSCLK, APB2 = AHB, APB1 = AHB/2 (PCLK1 = 54 MHz) */
  MODIFY_REG(RCC->CFGR,
             RCC_CFGR_HPRE | RCC_CFGR_PPRE2 | RCC_CFGR_PPRE1,
             RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PPRE1_DIV2);

  /* Use HSI/2 as PLL source and set multiplier to x27 (bits 21:18 = 0x0A, bit 27 = 1) */
  MODIFY_REG(RCC->CFGR,
             RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL | 0x08000000U,
             RCC_PLLSOURCE_HSI_DIV2 | ((uint32_t)0x0A << 18) | 0x08000000U);

  /* Configure ADC prescaler to keep ADC clock within specification (18 MHz) */
  MODIFY_REG(RCC->CFGR, RCC_CFGR_ADCPRE, RCC_CFGR_ADCPRE_DIV6);

  /* Enable PLL and wait until locked */
  SET_BIT(RCC->CR, RCC_CR_PLLON);
  while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0U) {
    /* wait */
  }

  /* Switch system clock to PLL */
  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
    /* wait */
  }

#else
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI; //0x00000002U
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON; //(0x1UL << (0U))
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON; //0x00000002U
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI_DIV2; //0x00000000U
  RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL16; //(0x7UL << (19U))
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType           = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2; //0x00000002U, 0x00000001U, 0x00000004U, 0x00000008U
  RCC_ClkInitStruct.SYSCLKSource        = RCC_SYSCLKSOURCE_PLLCLK; //0x00000002U
  RCC_ClkInitStruct.AHBCLKDivider       = RCC_SYSCLK_DIV1; //0x00000000U
  /* APB1 = HCLK/4 = 27 MHz. Original was HCLK/2 = 54 MHz which is above the
   * F103 I2C peripheral hard limit of 50 MHz for the FREQ field and above the
   * 36 MHz max spec'd for Fast Mode. /4 brings I2C Fast Mode (400kHz) fully
   * into spec. TIM1/TIM8 (motor PWM) are on APB2 and unaffected. */
  RCC_ClkInitStruct.APB1CLKDivider      = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider      = RCC_HCLK_DIV1; //0x00000000U

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  PeriphClkInit.PeriphClockSelection    = RCC_PERIPHCLK_ADC; //0x00000002U
  // PeriphClkInit.AdcClockSelection    = RCC_ADCPCLK2_DIV8;  // 8 MHz
  PeriphClkInit.AdcClockSelection       = RCC_ADCPCLK2_DIV4;  // 16 MHz
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
#endif

  SystemCoreClockUpdate();

  /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000U);

  /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK); //0x00000004U

  /* SysTick_IRQn interrupt configuration */
}


void verifyClocks(void) {
  uint32_t sysclk = SystemCoreClock;  // SystemCoreClockUpdate already reflects the GD32 108 MHz setup
  uint32_t hclk   = HAL_RCC_GetHCLKFreq();
  uint32_t pclk1  = HAL_RCC_GetPCLK1Freq();
  uint32_t pclk2  = HAL_RCC_GetPCLK2Freq();
  uint32_t tim_apb2 = pclk2;
  if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1) {
    tim_apb2 *= 2U;
  }
  uint32_t tim_apb1 = pclk1;
  if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1) {
    tim_apb1 *= 2U;
  }

  g_clockDiag.sysclk_hz   = sysclk;
  g_clockDiag.hclk_hz     = hclk;
  g_clockDiag.pclk1_hz    = pclk1;
  g_clockDiag.pclk2_hz    = pclk2;
  g_clockDiag.tim_apb1_hz = tim_apb1;
  g_clockDiag.tim_apb2_hz = tim_apb2;

  printf("Clock tree: SYSCLK=%lu Hz HCLK=%lu Hz PCLK1=%lu Hz PCLK2=%lu Hz TIM(APB1)=%lu Hz TIM(APB2)=%lu Hz\r\n",
         sysclk, hclk, pclk1, pclk2, tim_apb1, tim_apb2);
}

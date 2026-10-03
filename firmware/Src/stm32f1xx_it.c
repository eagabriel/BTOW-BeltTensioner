/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include "stm32f1xx_it.h"
#include "defines.h"
#include "config.h"
#include "util.h"
#include "mt6701.h"
#include "mt6701_pwm.h"

extern DMA_HandleTypeDef hdma_i2c2_rx;
extern DMA_HandleTypeDef hdma_i2c2_tx;
extern I2C_HandleTypeDef hi2c2;

extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;

/* USER CODE BEGIN 0 */
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
#if defined(DC_LINK_WATCHDOG_ENABLE)
extern ADC_HandleTypeDef hadc3;
#endif
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/


/******************************************************************************/
/*            Cortex-M3 Processor Interruption and Exception Handlers         */
/******************************************************************************/

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void) {
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/* ============================ FAULT DIAGNOSTICS ============================
 * If the CPU takes a HardFault/BusFault/UsageFault/MemManage the default
 * behavior was while(1) — LED off, main loop dead, motor stuck at last PWM
 * (looks exactly like the "trava" symptom).
 *
 * Now we:
 *   1) Cut motor PWM immediately (TIM1/TIM8 BDTR MOE=0) — safety.
 *   2) Blink LED_PIN at a distinctive rate per fault type — visible signal.
 *   3) Save the fault registers into a known RAM slot for post-mortem via
 *      debugger (fault_info[]). If you attach OpenOCD/ST-LINK you can dump
 *      that array to see what fired and where.
 *
 * Distinguish faults by LED pattern:
 *   Fast strobe ~10 Hz  → HardFault (generic)
 *   Faster      ~20 Hz  → BusFault
 *   Very fast   ~50 Hz  → UsageFault
 *   Slow-fast   burst   → MemManage
 * "LED off, not blinking, motor stuck" alone (no fast blink) => main loop
 * hang WITHOUT fault (spinning in an infinite loop somewhere in C code).
 */
volatile uint32_t fault_info[8] __attribute__((used));

static inline void fault_capture_and_secure(uint32_t which)
{
  __disable_irq();
  /* Cut motor PWM: clears BDTR.MOE so all six phases stop switching. */
  LEFT_TIM->BDTR  &= ~TIM_BDTR_MOE;
  RIGHT_TIM->BDTR &= ~TIM_BDTR_MOE;
  /* Snapshot the ARM SCB fault registers. Read via debugger:
   *   fault_info[0] = fault-type tag (1=HF, 2=BF, 3=UF, 4=MM)
   *   fault_info[1] = CFSR (Configurable Fault Status Register)
   *   fault_info[2] = HFSR (HardFault Status)
   *   fault_info[3] = MMFAR (MemManage Fault Address)
   *   fault_info[4] = BFAR  (Bus Fault Address)
   *   fault_info[5] = SHCSR (System Handler Control/State)
   *   fault_info[6] = reserved
   *   fault_info[7] = reserved */
  fault_info[0] = which;
  fault_info[1] = SCB->CFSR;
  fault_info[2] = SCB->HFSR;
  fault_info[3] = SCB->MMFAR;
  fault_info[4] = SCB->BFAR;
  fault_info[5] = SCB->SHCSR;
}

static inline void fault_led_blink(uint32_t on_ticks, uint32_t off_ticks)
{
  for (;;) {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    for (volatile uint32_t i = 0; i < on_ticks; i++) { __NOP(); }
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    for (volatile uint32_t i = 0; i < off_ticks; i++) { __NOP(); }
  }
}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void) {
  fault_capture_and_secure(1);
  fault_led_blink(500000u, 500000u);    /* ~10 Hz strobe */
}

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void) {
  fault_capture_and_secure(4);
  /* Burst: 3 quick blinks, pause, repeat */
  for (;;) {
    for (int n = 0; n < 3; n++) {
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
      for (volatile uint32_t i = 0; i < 200000u; i++) { __NOP(); }
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
      for (volatile uint32_t i = 0; i < 200000u; i++) { __NOP(); }
    }
    for (volatile uint32_t i = 0; i < 3000000u; i++) { __NOP(); }
  }
}

/**
* @brief This function handles Prefetch fault, memory access fault.
*/
void BusFault_Handler(void) {
  fault_capture_and_secure(2);
  fault_led_blink(250000u, 250000u);    /* ~20 Hz strobe */
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void) {
  fault_capture_and_secure(3);
  fault_led_blink(100000u, 100000u);    /* ~50 Hz strobe */
}

/**
* @brief This function handles System service call via SWI instruction.
*/
void SVC_Handler(void) {
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
* @brief This function handles Debug monitor.
*/
void DebugMon_Handler(void) {
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
* @brief This function handles Pendable request for system service.
*/
void PendSV_Handler(void) {
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/


void SysTick_Handler(void) {
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

#if defined(ENCODER_X) || defined(ENCODER_Y)
/* MT6701 IT-mode I2C handlers. The CPU is free during byte-level events
 * except for ~2 µs per event servicing. HAL runs the STM32F1 EV6_3 sequence
 * (POS/ACK/STOP for 1/2-byte reads, ERRATUM I2C 2.10.7) automatically. */
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

/* Raw hit counters — increment BEFORE HAL runs, so if NVIC routes the IRQ
 * to our handler these grow even if HAL then decides to do nothing. If they
 * stay at 0 while a transaction is in progress, the NVIC line isn't firing. */
volatile uint32_t i2c1_ev_irq_hits = 0;
volatile uint32_t i2c1_er_irq_hits = 0;
volatile uint32_t i2c2_ev_irq_hits = 0;
volatile uint32_t i2c2_er_irq_hits = 0;

void I2C1_EV_IRQHandler(void) {
  i2c1_ev_irq_hits++;
#if defined(MT6701_BAREMETAL_Y) && defined(ENCODER_Y)
  MT6701_Y_Bare_EV_IRQ();                    /* bare-metal state machine, no HAL */
#else
  HAL_I2C_EV_IRQHandler(&hi2c1);
#endif
}
void I2C1_ER_IRQHandler(void) {
  i2c1_er_irq_hits++;
#if defined(MT6701_BAREMETAL_Y) && defined(ENCODER_Y)
  MT6701_Y_Bare_ER_IRQ();                    /* bare-metal error path */
#else
  HAL_I2C_ER_IRQHandler(&hi2c1);
#endif
}
void I2C2_EV_IRQHandler(void) {
  i2c2_ev_irq_hits++;
#if defined(MT6701_BAREMETAL_X) && defined(ENCODER_X)
  MT6701_X_Bare_EV_IRQ();                    /* bare-metal state machine, no HAL */
#else
  HAL_I2C_EV_IRQHandler(&hi2c2);
#endif
}
void I2C2_ER_IRQHandler(void) {
  i2c2_er_irq_hits++;
#if defined(MT6701_BAREMETAL_X) && defined(ENCODER_X)
  MT6701_X_Bare_ER_IRQ();                    /* bare-metal error path */
#else
  HAL_I2C_ER_IRQHandler(&hi2c2);
#endif
}

/* DMA handlers for I2C1 DMA channels. */
extern DMA_HandleTypeDef hdma_i2c1_tx;
extern DMA_HandleTypeDef hdma_i2c1_rx;
void DMA1_Channel6_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_i2c1_tx); }
void DMA1_Channel7_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_i2c1_rx); }

#endif






/**
  * @brief This function handles DMA1 channel2 global interrupt.
  */
void DMA1_Channel2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_IRQn 0 */

  /* USER CODE END DMA1_Channel2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
  /* USER CODE BEGIN DMA1_Channel2_IRQn 1 */

  /* USER CODE END DMA1_Channel2_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel3 global interrupt.
  */
void DMA1_Channel3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel3_IRQn 0 */

  /* USER CODE END DMA1_Channel3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
  /* USER CODE BEGIN DMA1_Channel3_IRQn 1 */

  /* USER CODE END DMA1_Channel3_IRQn 1 */
}


#if defined(MT6701_MODE_PWM_X)
/* TIM2 IRQ — MT6701 X PWM input capture on PB10 (CH3 rising, CH4 falling).
 * Handler lives in Src/mt6701_pwm.c. Prio 4 in NVIC, below FOC. */
void TIM2_IRQHandler(void)
{
    MT6701_X_Pwm_IRQ();
}
#endif

#if defined(MT6701_MODE_PWM_Y)
/* TIM4 IRQ — MT6701 Y PWM input capture on PB6 (CH1 rising, CH2 falling).
 * Handler lives in Src/mt6701_pwm.c. Prio 4 in NVIC, below FOC. */
void TIM4_IRQHandler(void)
{
    MT6701_Y_Pwm_IRQ();
}
#endif

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART2_IRQn 1 */
  if(RESET != __HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_IDLE)) {  // Check for IDLE line interrupt  
      __HAL_UART_CLEAR_IDLEFLAG(&huart3);                         // Clear IDLE line flag (otherwise it will continue to enter interrupt)
      usart3_rx_check();                                          // Check for data to process
  }
  /* USER CODE END USART2_IRQn 1 */
}

#if defined(ENCODER_X) || defined(ENCODER_Y)
/*
 * TIM7 update IRQ — dedicated MT6701 sampler.
 * Fires at MT6701_SAMPLE_RATE_HZ (see config.h). Priority 4 in NVIC, below
 * FOC/SysTick (0) and USART (3). Gated on encoder.ini so the reads only
 * happen once the sensor init succeeded, and on !align_state so the ISR
 * doesn't race the alignment state machine (which drives count_?_update
 * from the main loop and writes direction/cnt_offset non-atomically).
 * I2C bus recovery is deferred to the main loop via flags in util.c —
 * calling I2C_Init/HAL_Delay from here would hang (SysTick can preempt but
 * bit-bang recovery in ISR context is still poor citizenship).
 */
extern TIM_HandleTypeDef htim7;

void TIM7_IRQHandler(void)
{
  if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE) != RESET) {
    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
    /* IT-mode sampler: fire a non-blocking read and return in ~5 µs. Actual
     * byte transfer + processing happens in I2C EV/ER IRQs (priority 2).
     * MT6701_?_KickIT handles busy-flag, recovery-pending, and !ini guards.
     * Runs unconditionally — alignment code now reads encoder_?.raw_angle /
     * .ENCODER_COUNT directly from the cache instead of triggering its own
     * blocking read, so there's no path conflict. */
    #ifdef ENCODER_X
      #if defined(MT6701_MODE_PWM_X)
      /* PWM mode: the angle is captured autonomously by TIM2 into
       * s_raw_angle. Here we just copy it into encoder_x and run the same
       * scaling the I2C path used. Gated on ini so the FOC doesn't see a
       * stale zero before Encoder_X_Init has seeded count_prev. */
      if (encoder_x.ini) MT6701_X_Pwm_Feed();
      #else
      MT6701_X_KickIT();
      #endif
    #endif
    #ifdef ENCODER_Y
      #if defined(MT6701_MODE_PWM_Y)
      if (encoder_y.ini) MT6701_Y_Pwm_Feed();
      #else
      MT6701_Y_KickIT();
      #endif
    #endif
  }
}
#endif

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

#if defined(DC_LINK_WATCHDOG_ENABLE)
void ADC3_IRQHandler(void)
{
  /* Forward analog watchdog events into the HAL so the DC-link handler can run. */
  HAL_ADC_IRQHandler(&hadc3);
}
#endif



/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

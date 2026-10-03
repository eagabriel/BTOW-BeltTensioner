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

/*
tim1 master, enable -> trgo
tim8, gated slave mode, trgo by tim1 trgo. overflow -> trgo
adc1,adc2 triggered by tim8 trgo
adc 1,2 dual mode

ADC1             ADC2
R_Blau PC4 CH14  R_Gelb PC5 CH15
L_Grün PA0 CH01  L_Blau PC3 CH13
R_DC PC1 CH11    L_DC PC0 CH10
BAT   PC2 CH12   L_TX PA2 CH02
BAT   PC2 CH12   L_RX PA3 CH03

pb10 usart3 dma1 channel2/3
*/

#include "defines.h"
#include "config.h"
#include "setup.h"
#include "util.h"

TIM_HandleTypeDef htim_right;
TIM_HandleTypeDef htim_left;
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;
volatile adc_buf_t adc_buffer;



/* USART3 init function */
void UART3_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  
  huart3.Instance = USART3;
  huart3.Init.BaudRate = USART3_BAUD;
  huart3.Init.WordLength = USART3_WORDLENGTH;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart3);
}

#if defined(DEBUG_SERIAL_USART2) || defined(CONTROL_SERIAL_USART2) || defined(FEEDBACK_SERIAL_USART2) || defined(SIDEBOARD_SERIAL_USART2) || \
    defined(DEBUG_SERIAL_USART3) || defined(CONTROL_SERIAL_USART3) || defined(FEEDBACK_SERIAL_USART3) || defined(SIDEBOARD_SERIAL_USART3)
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Channel6;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart2_rx);
    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Channel7;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart2_tx);
    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */
	__HAL_UART_ENABLE_IT (uartHandle, UART_IT_IDLE);  // Enable the USART IDLE line detection interrupt
  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART3_PARTIAL();   /* TX=PC10, RX=PC11 (PB10/PB11 stay free for I2C2) */
    /**USART3 GPIO Configuration (partial remap)
    PC10     ------> USART3_TX
    PC11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART3 DMA Init */
    /* USART3_RX Init */
    hdma_usart3_rx.Instance = DMA1_Channel3;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart3_rx);
    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart3_rx);

    /* USART3_TX Init */
    hdma_usart3_tx.Instance = DMA1_Channel2;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart3_tx);
    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart3_tx);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */
	__HAL_UART_ENABLE_IT (uartHandle, UART_IT_IDLE);  // Enable the USART IDLE line detection interrupt
  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();
  
    /**USART3 GPIO Configuration (partial remap)
    PC10     ------> USART3_TX
    PC11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
} 
#endif

DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;
DMA_HandleTypeDef hdma_i2c2_rx;
DMA_HandleTypeDef hdma_i2c2_tx;

/* STM32F1 / GD32F103 I2C init following ST errata sheet AN2824 §3.1.14.
 *
 * The peripheral gets stuck in BUSY state if SDA/SCL are seen LOW at the
 * moment PE (peripheral enable) transitions to 1 — even briefly, due to
 * mode-switch transients. Fix requires:
 *   1) PE = 0 (disable) before touching anything
 *   2) GPIO as output OD with pull-up, drive both HIGH, verify SDA = HIGH
 *   3) If SDA stuck LOW, bit-bang 9 SCL pulses + STOP to unblock a hung slave
 *   4) Switch to AF_OD
 *   5) SWRST cycle (with PE still = 0)
 *   6) Configure + PE = 1 via HAL_I2C_Init
 *
 * This helper does steps 2..5. Steps 1 and 6 are the caller's job.
 */
static void I2C_PrepPinsAndReset(I2C_TypeDef *inst,
                                 GPIO_TypeDef *port,
                                 uint16_t scl_pin, uint16_t sda_pin)
{
    GPIO_InitTypeDef gpio = {0};

    /* Step 2: GPIO OD output with pull-up, both HIGH */
    gpio.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio.Pull  = GPIO_PULLUP;                  /* weak internal pull-up backup */
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pin   = scl_pin | sda_pin;
    HAL_GPIO_Init(port, &gpio);
    HAL_GPIO_WritePin(port, scl_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(port, sda_pin, GPIO_PIN_SET);
    HAL_Delay(1);

    /* Step 3: if SDA stuck LOW, bit-bang recovery */
    if (HAL_GPIO_ReadPin(port, sda_pin) == GPIO_PIN_RESET) {
        for (int i = 0; i < 9; i++) {
            HAL_GPIO_WritePin(port, scl_pin, GPIO_PIN_RESET);
            HAL_Delay(1);
            HAL_GPIO_WritePin(port, scl_pin, GPIO_PIN_SET);
            HAL_Delay(1);
        }
        /* Manual STOP: SDA low → SDA high while SCL high */
        HAL_GPIO_WritePin(port, sda_pin, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(port, sda_pin, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    /* Step 4: switch to AF_OD (peripheral now controls the lines) */
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_NOPULL;                   /* ignored by F1 in AF mode */
    HAL_GPIO_Init(port, &gpio);

    /* Step 5: SWRST cycle while PE = 0 (write CR1 directly, no read/modify)
     * This clears BUSY and any spurious start state captured in the FSM. */
    inst->CR1 = (1u << 15);                    /* SWRST = 1, PE = 0 */
    inst->CR1 = 0;                              /* SWRST = 0, PE = 0 */
}

/* Saved register state after HAL_I2C_Init succeeds. Used by I2C?_FastRecover
 * to reconfigure the peripheral in ~1 µs instead of the ~20 ms that
 * HAL_I2C_Init + I2C_PrepPinsAndReset cost (they use HAL_Delay(1) at multiple
 * points, and TIM7 is disabled around the whole thing so the encoder freezes
 * for that full window). */
static uint32_t i2c1_saved_cr2   = 0;
static uint32_t i2c1_saved_ccr   = 0;
static uint32_t i2c1_saved_trise = 0;
static uint32_t i2c1_saved_oar1  = 0;
static uint32_t i2c2_saved_cr2   = 0;
static uint32_t i2c2_saved_ccr   = 0;
static uint32_t i2c2_saved_trise = 0;
static uint32_t i2c2_saved_oar1  = 0;

/* CPU-loop bit-bang to unstick a slave that is holding SDA low.
 * Configures the pins as GPIO Output-OD, drives both high, and if SDA is
 * still low, toggles SCL 9 times followed by a manual STOP, then restores
 * the original pin CNF (AF_OD for I2C). Delays are ~1 µs per NOP loop at
 * 108 MHz — total ~120 µs when unstick is needed, ~5 µs when SDA is already
 * high. Sub-microsecond compared to the 20 ms that HAL_Delay-based bit-bang
 * in I2C_PrepPinsAndReset costs. */
static inline void bit_bang_unstick(GPIO_TypeDef *port,
                                    uint32_t scl_pin,
                                    uint32_t sda_pin)
{
    const uint32_t scl_bit = 1u << scl_pin;
    const uint32_t sda_bit = 1u << sda_pin;

    __IO uint32_t *scl_cr = (scl_pin < 8u) ? &port->CRL : &port->CRH;
    __IO uint32_t *sda_cr = (sda_pin < 8u) ? &port->CRL : &port->CRH;
    const uint32_t scl_sh = (scl_pin & 7u) * 4u;
    const uint32_t sda_sh = (sda_pin & 7u) * 4u;

    /* Save current pin CNF+MODE (4 bits each) so we can restore AF_OD after. */
    const uint32_t scl_saved = (*scl_cr >> scl_sh) & 0xFu;
    const uint32_t sda_saved = (*sda_cr >> sda_sh) & 0xFu;

    /* Configure as Output Open-Drain 50 MHz (CNF=01, MODE=11 = 0x7). */
    *scl_cr = (*scl_cr & ~(0xFu << scl_sh)) | (0x7u << scl_sh);
    *sda_cr = (*sda_cr & ~(0xFu << sda_sh)) | (0x7u << sda_sh);

    /* Both HIGH. Small settle. */
    port->BSRR = scl_bit | sda_bit;
    for (volatile uint32_t d = 0; d < 500u; d++) { __NOP(); }

    /* If SDA is stuck LOW, bit-bang 9 clocks + manual STOP. */
    if ((port->IDR & sda_bit) == 0U) {
        for (int i = 0; i < 9; i++) {
            port->BRR  = scl_bit;
            for (volatile uint32_t d = 0; d < 200u; d++) { __NOP(); }
            port->BSRR = scl_bit;
            for (volatile uint32_t d = 0; d < 200u; d++) { __NOP(); }
        }
        /* Manual STOP: SDA low -> high while SCL high. */
        port->BRR  = sda_bit;
        for (volatile uint32_t d = 0; d < 200u; d++) { __NOP(); }
        port->BSRR = sda_bit;
        for (volatile uint32_t d = 0; d < 200u; d++) { __NOP(); }
    }

    /* Restore original CNF+MODE (peripheral takes control again). */
    *scl_cr = (*scl_cr & ~(0xFu << scl_sh)) | (scl_saved << scl_sh);
    *sda_cr = (*sda_cr & ~(0xFu << sda_sh)) | (sda_saved << sda_sh);
}

static inline void I2C_FastRecoverImpl(I2C_TypeDef *inst,
                                       uint32_t saved_cr2,
                                       uint32_t saved_ccr,
                                       uint32_t saved_trise,
                                       uint32_t saved_oar1,
                                       GPIO_TypeDef *port,
                                       uint32_t scl_pin,
                                       uint32_t sda_pin)
{
    /* Disabling PE stops IRQ generation and releases the pins so bit-bang
     * can drive them without contention. No NVIC dance needed. */
    inst->CR1 = 0;

    /* Physical bus unstick if a slave is holding SDA low. Cheap when the
     * bus is fine (~5 µs); a full bit-bang when it's needed (~120 µs). */
    bit_bang_unstick(port, scl_pin, sda_pin);

    /* SWRST cycle clears BUSY, all SR1 flags, and internal FSM. */
    inst->CR1 = I2C_CR1_SWRST;
    for (volatile uint32_t d = 0; d < 100u; d++) { __NOP(); }
    inst->CR1 = 0;

    /* Restore configuration bits captured after HAL_I2C_Init completed. */
    inst->CR2   = saved_cr2;
    inst->CCR   = saved_ccr;
    inst->TRISE = saved_trise;
    inst->OAR1  = saved_oar1;
    inst->CR1   = I2C_CR1_PE;

    /* If BUSY is STILL latched (F1 errata), a manual START -> STOP forces
     * the peripheral to observe both edges and clear the flag. Tight
     * bounded timeouts so the worst case stays under 100 µs. */
    if ((inst->SR2 & I2C_SR2_BUSY) != 0U) {
        uint32_t to = 1000u;
        inst->CR1 |= I2C_CR1_START;
        while (to-- && !(inst->SR1 & I2C_SR1_SB)) { }
        inst->CR1 |= I2C_CR1_STOP;
        to = 1000u;
        while (to-- && (inst->SR2 & I2C_SR2_BUSY)) { }
    }
}

void I2C1_FastRecover(void)
{
    /* I2C1: PB6=SCL, PB7=SDA. */
    I2C_FastRecoverImpl(I2C1, i2c1_saved_cr2, i2c1_saved_ccr,
                        i2c1_saved_trise, i2c1_saved_oar1,
                        GPIOB, 6u, 7u);
}

void I2C2_FastRecover(void)
{
    /* I2C2: PB10=SCL, PB11=SDA. */
    I2C_FastRecoverImpl(I2C2, i2c2_saved_cr2, i2c2_saved_ccr,
                        i2c2_saved_trise, i2c2_saved_oar1,
                        GPIOB, 10u, 11u);
}

void I2C1_Init(void)
{
  /* I2C1 GPIO Configuration: PB6=SCL, PB7=SDA — MT6701 sensor (X / motor R) */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_I2C1_CLK_ENABLE();

  /* Step 1: peripheral must be disabled before touching anything */
  I2C1->CR1 = 0;

  /* Steps 2..5: pin prep + SWRST cycle following ST errata sequence */
  I2C_PrepPinsAndReset(I2C1, GPIOB, GPIO_PIN_6, GPIO_PIN_7);

  /* Step 6: configure and enable peripheral via HAL */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = MT6701_I2C_CLOCK_HZ;  /* see config.h */
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c1);

  /* F1 errata 2.14.7 workaround: BUSY can stay latched at 1 even after
   * SWRST + reinit when a prior transaction was aborted mid-flight. The
   * bit-bang in I2C_PrepPinsAndReset only helps if SDA was stuck low —
   * an internally-latched BUSY needs the peripheral to see a full
   * START->STOP cycle to clear. Force one via CR1 with bounded polls.
   * Bounded because if BUSY is truly stuck (bad HW), the outer main-loop
   * retry at 20 ms cadence handles it — we just don't want to hang here. */
  {
    volatile uint32_t to = 100000u;
    if ((I2C1->SR2 & I2C_SR2_BUSY) != 0U) {
      I2C1->CR1 |= I2C_CR1_START;
      while (to-- && !(I2C1->SR1 & I2C_SR1_SB)) { /* wait SB or bail */ }
      I2C1->CR1 |= I2C_CR1_STOP;
      to = 100000u;
      while (to-- && (I2C1->SR2 & I2C_SR2_BUSY)) { /* wait BUSY clear */ }
    }
  }

  /* Snapshot the fully-configured registers so I2C1_FastRecover can restore
   * the peripheral in ~1 µs without going through HAL_I2C_Init again. */
  i2c1_saved_cr2   = I2C1->CR2;
  i2c1_saved_ccr   = I2C1->CCR;
  i2c1_saved_trise = I2C1->TRISE;
  i2c1_saved_oar1  = I2C1->OAR1;

  /* DMA setup for I2C1 — DMA1_Ch6 (TX) + DMA1_Ch7 (RX). */
  __HAL_RCC_DMA1_CLK_ENABLE();

  hdma_i2c1_tx.Instance                 = DMA1_Channel6;
  hdma_i2c1_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_i2c1_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_i2c1_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_i2c1_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_i2c1_tx.Init.Mode                = DMA_NORMAL;
  hdma_i2c1_tx.Init.Priority            = DMA_PRIORITY_MEDIUM;
  HAL_DMA_Init(&hdma_i2c1_tx);
  __HAL_LINKDMA(&hi2c1, hdmatx, hdma_i2c1_tx);

  hdma_i2c1_rx.Instance                 = DMA1_Channel7;
  hdma_i2c1_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_i2c1_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_i2c1_rx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_i2c1_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_i2c1_rx.Init.Mode                = DMA_NORMAL;
  hdma_i2c1_rx.Init.Priority            = DMA_PRIORITY_MEDIUM;
  HAL_DMA_Init(&hdma_i2c1_rx);
  __HAL_LINKDMA(&hi2c1, hdmarx, hdma_i2c1_rx);

  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  HAL_NVIC_SetPriority(I2C1_ER_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

void I2C_Init(void)
{
  /* Initialise I2C2 GPIO pins
  *  I2C2 GPIO Configuration
  *  PB10     ------> I2C2_SCL
  *  PB11     ------> I2C2_SDA  (MT6701 sensor — Y / motor L)
  */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_I2C2_CLK_ENABLE();

  /* Step 1: peripheral must be disabled before touching anything */
  I2C2->CR1 = 0;

  /* Steps 2..5: pin prep + SWRST cycle following ST errata sequence */
  I2C_PrepPinsAndReset(I2C2, GPIOB, GPIO_PIN_10, GPIO_PIN_11);

  /* Step 6: configure and enable peripheral via HAL */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = MT6701_I2C_CLOCK_HZ;  /* see config.h */
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c2);

  /* F1 errata 2.14.7 workaround — same reasoning as I2C1_Init above. */
  {
    volatile uint32_t to = 100000u;
    if ((I2C2->SR2 & I2C_SR2_BUSY) != 0U) {
      I2C2->CR1 |= I2C_CR1_START;
      while (to-- && !(I2C2->SR1 & I2C_SR1_SB)) { /* wait SB or bail */ }
      I2C2->CR1 |= I2C_CR1_STOP;
      to = 100000u;
      while (to-- && (I2C2->SR2 & I2C_SR2_BUSY)) { /* wait BUSY clear */ }
    }
  }

  /* Snapshot for I2C2_FastRecover — see comment in I2C1_Init. */
  i2c2_saved_cr2   = I2C2->CR2;
  i2c2_saved_ccr   = I2C2->CCR;
  i2c2_saved_trise = I2C2->TRISE;
  i2c2_saved_oar1  = I2C2->OAR1;

  /* Peripheral DMA init*/
/*  __HAL_RCC_DMA1_CLK_ENABLE();
*/  
  /* DMA1_Channel4_IRQn interrupt configuration */
/*  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 1, 4);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
*/
  /* DMA1_Channel5_IRQn interrupt configuration */
/*  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 1, 3);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
*/

/*  hdma_i2c2_rx.Instance = DMA1_Channel5;
  hdma_i2c2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_i2c2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_i2c2_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_i2c2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_i2c2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_i2c2_rx.Init.Mode = DMA_NORMAL;
  hdma_i2c2_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
  HAL_DMA_Init(&hdma_i2c2_rx);

  __HAL_LINKDMA(&hi2c2,hdmarx,hdma_i2c2_rx);
*/

/*  hdma_i2c2_tx.Instance = DMA1_Channel4;
  hdma_i2c2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_i2c2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_i2c2_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_i2c2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_i2c2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_i2c2_tx.Init.Mode = DMA_NORMAL;
  hdma_i2c2_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
  HAL_DMA_Init(&hdma_i2c2_tx);

  __HAL_LINKDMA(&hi2c2,hdmatx,hdma_i2c2_tx);
*/
  /* Peripheral interrupt init */
/*  HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
  HAL_NVIC_SetPriority(I2C2_ER_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
*/
  /* Event + Error IRQs for IT-mode transactions — matches I2C1_Init. */
  HAL_NVIC_SetPriority(I2C2_EV_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
  HAL_NVIC_SetPriority(I2C2_ER_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
}

#if defined(ENCODER_X) || defined(ENCODER_Y)
/*
 * TIM7 basic timer at MT6701_SAMPLE_RATE_HZ (1 kHz by default).
 * IRQ handler (in stm32f1xx_it.c) calls count_x_update() / count_y_update()
 * so the FOC ISR sees a fresh position every ~1 ms instead of ~5 ms from the
 * old main-loop cadence. NVIC priority 4 keeps this below FOC (0) and USART (3);
 * SysTick (0) can still preempt so HAL_GetTick works inside the ISR body.
 */
TIM_HandleTypeDef htim7;

void MT6701_Sampler_Init(void)
{
  __HAL_RCC_TIM7_CLK_ENABLE();

  htim7.Instance               = TIM7;
  /* Timer clock on APB1 timer domain equals SystemCoreClock (72 MHz STM32,
   * 108 MHz GD32) as long as APB1 prescaler > 1 — which is our case (/2). */
  htim7.Init.Prescaler         = (SystemCoreClock / 1000000U) - 1U;   /* 1 MHz counter */
  htim7.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim7.Init.Period            = (1000000U / MT6701_SAMPLE_RATE_HZ) - 1U;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim7);

  HAL_NVIC_SetPriority(TIM7_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(TIM7_IRQn);

  HAL_TIM_Base_Start_IT(&htim7);
}
#endif

void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = LEFT_HALL_U_PIN;
  HAL_GPIO_Init(LEFT_HALL_U_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_HALL_V_PIN;
  HAL_GPIO_Init(LEFT_HALL_V_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_HALL_W_PIN;
  HAL_GPIO_Init(LEFT_HALL_W_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_HALL_U_PIN;
  HAL_GPIO_Init(RIGHT_HALL_U_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_HALL_V_PIN;
  HAL_GPIO_Init(RIGHT_HALL_V_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_HALL_W_PIN;
  HAL_GPIO_Init(RIGHT_HALL_W_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Pin = CHARGER_PIN;
  HAL_GPIO_Init(CHARGER_PORT, &GPIO_InitStruct);

  

  GPIO_InitStruct.Pull = GPIO_NOPULL;

#if defined(ANALOG_BUTTON)
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
#else
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
#endif
  GPIO_InitStruct.Pin = BUTTON_PIN;
  HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);

#if defined(ESTOP_ENABLE)
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = ESTOP_GPIO_PULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin   = ESTOP_PIN;
  HAL_GPIO_Init(ESTOP_PORT, &GPIO_InitStruct);
#endif


  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

  GPIO_InitStruct.Pin = LED_PIN;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BUZZER_PIN;
  HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = OFF_PIN;
  HAL_GPIO_Init(OFF_PORT, &GPIO_InitStruct);

#if defined(HOCP)
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Pin   = TIM1_BKIN_PIN;
  HAL_GPIO_Init(TIM1_BKIN_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin   = TIM8_BKIN_PIN;
  HAL_GPIO_Init(TIM8_BKIN_PORT, &GPIO_InitStruct);
#endif


  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;

  GPIO_InitStruct.Pin = LEFT_DC_CUR_PIN;
  HAL_GPIO_Init(LEFT_DC_CUR_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_U_CUR_PIN;
  HAL_GPIO_Init(LEFT_U_CUR_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_V_CUR_PIN;
  HAL_GPIO_Init(LEFT_V_CUR_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_DC_CUR_PIN;
  HAL_GPIO_Init(RIGHT_DC_CUR_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_U_CUR_PIN;
  HAL_GPIO_Init(RIGHT_U_CUR_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_V_CUR_PIN;
  HAL_GPIO_Init(RIGHT_V_CUR_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DCLINK_PIN;
  HAL_GPIO_Init(DCLINK_PORT, &GPIO_InitStruct);

  //Analog in
  #if !defined(ESTOP_ENABLE)
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  #endif
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;

  GPIO_InitStruct.Pin = LEFT_TIM_UH_PIN;
  HAL_GPIO_Init(LEFT_TIM_UH_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_TIM_VH_PIN;
  HAL_GPIO_Init(LEFT_TIM_VH_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_TIM_WH_PIN;
  HAL_GPIO_Init(LEFT_TIM_WH_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_TIM_UL_PIN;
  HAL_GPIO_Init(LEFT_TIM_UL_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_TIM_VL_PIN;
  HAL_GPIO_Init(LEFT_TIM_VL_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_TIM_WL_PIN;
  HAL_GPIO_Init(LEFT_TIM_WL_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_TIM_UH_PIN;
  HAL_GPIO_Init(RIGHT_TIM_UH_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_TIM_VH_PIN;
  HAL_GPIO_Init(RIGHT_TIM_VH_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_TIM_WH_PIN;
  HAL_GPIO_Init(RIGHT_TIM_WH_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_TIM_UL_PIN;
  HAL_GPIO_Init(RIGHT_TIM_UL_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_TIM_VL_PIN;
  HAL_GPIO_Init(RIGHT_TIM_VL_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_TIM_WL_PIN;
  HAL_GPIO_Init(RIGHT_TIM_WL_PORT, &GPIO_InitStruct);
}

void MX_TIM_Init(void) {
  __HAL_RCC_TIM1_CLK_ENABLE();
  __HAL_RCC_TIM8_CLK_ENABLE();
SET_BIT(DBGMCU->CR,DBGMCU_CR_DBG_TIM1_STOP);
SET_BIT(DBGMCU->CR,DBGMCU_CR_DBG_TIM8_STOP);
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
  TIM_SlaveConfigTypeDef sTimConfig;

  /* TIM1/TIM8 reside on APB2; derive their clock dynamically from the clock tree. */
  uint32_t timClockHz = HAL_RCC_GetPCLK2Freq();
  if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1) {
    timClockHz *= 2U; // Timer clock doubles when APB prescaler is not 1
  }
  uint32_t pwmPeriodCounts = timClockHz / 2U / PWM_FREQ;
  if (pwmPeriodCounts == 0U) {
    pwmPeriodCounts = 1U; // Prevent zero period in case of misconfiguration
  }

  htim_right.Instance               = RIGHT_TIM;
  htim_right.Init.Prescaler         = 0;
  htim_right.Init.CounterMode       = TIM_COUNTERMODE_CENTERALIGNED3;
  htim_right.Init.Period            = pwmPeriodCounts;
  htim_right.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim_right.Init.RepetitionCounter = 0;
  htim_right.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_PWM_Init(&htim_right);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim_right, &sMasterConfig);

  sConfigOC.OCMode       = TIM_OCMODE_PWM1;
  sConfigOC.Pulse        = 0;
  sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_LOW;
  sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_SET;
  HAL_TIM_PWM_ConfigChannel(&htim_right, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_ConfigChannel(&htim_right, &sConfigOC, TIM_CHANNEL_2);
  HAL_TIM_PWM_ConfigChannel(&htim_right, &sConfigOC, TIM_CHANNEL_3);

  sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_ENABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_ENABLE;
  sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime         = DEAD_TIME;
#if defined(HOCP)
  sBreakDeadTimeConfig.BreakState       = TIM_BREAK_ENABLE;
#else
  sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
#endif
  sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_LOW;
  sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
  HAL_TIMEx_ConfigBreakDeadTime(&htim_right, &sBreakDeadTimeConfig);

  htim_left.Instance               = LEFT_TIM;
  htim_left.Init.Prescaler         = 0;
  htim_left.Init.CounterMode       = TIM_COUNTERMODE_CENTERALIGNED3;
  htim_left.Init.Period            = pwmPeriodCounts;
  htim_left.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim_left.Init.RepetitionCounter = 0;
  htim_left.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_PWM_Init(&htim_left);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_ENABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim_left, &sMasterConfig);

  sTimConfig.InputTrigger = TIM_TS_ITR0;
  sTimConfig.SlaveMode    = TIM_SLAVEMODE_GATED;
  HAL_TIM_SlaveConfigSynchronization(&htim_left, &sTimConfig);

  // Start counting >0 to effectively offset timers by the time it takes for one ADC conversion to complete.
  // This method allows that the Phase currents ADC measurements are properly aligned with LOW-FET ON region for both motors
  LEFT_TIM->CNT 		     = ADC_TOTAL_CONV_TIME;

  sConfigOC.OCMode       = TIM_OCMODE_PWM1;
  sConfigOC.Pulse        = 0;
  sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_LOW;
  sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_SET;
  HAL_TIM_PWM_ConfigChannel(&htim_left, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_ConfigChannel(&htim_left, &sConfigOC, TIM_CHANNEL_2);
  HAL_TIM_PWM_ConfigChannel(&htim_left, &sConfigOC, TIM_CHANNEL_3);

  sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_ENABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_ENABLE;
  sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime         = DEAD_TIME;
#if defined(HOCP)
  sBreakDeadTimeConfig.BreakState       = TIM_BREAK_ENABLE;
#else
  sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
#endif
  sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_LOW;
  sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
  HAL_TIMEx_ConfigBreakDeadTime(&htim_left, &sBreakDeadTimeConfig);

  LEFT_TIM->BDTR &= ~TIM_BDTR_MOE;
  RIGHT_TIM->BDTR &= ~TIM_BDTR_MOE;

  HAL_TIM_PWM_Start(&htim_left, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim_left, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim_left, TIM_CHANNEL_3);
  HAL_TIMEx_PWMN_Start(&htim_left, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim_left, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(&htim_left, TIM_CHANNEL_3);  

  HAL_TIM_PWM_Start(&htim_right, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim_right, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim_right, TIM_CHANNEL_3);
  HAL_TIMEx_PWMN_Start(&htim_right, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim_right, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(&htim_right, TIM_CHANNEL_3);

  htim_left.Instance->RCR = 1;

  /* Optional: External brake resistor PWM on CH3 or CH4
   * Enable this by defining EXTBRK_EN and selecting channel by
   * defining EXTBRK_USE_CH3 or EXTBRK_USE_CH4 in `Inc/config.h`.
   */
#if defined(EXTBRK_EN) && (defined(EXTBRK_USE_CH3) || defined(EXTBRK_USE_CH4))
  {
    TIM_HandleTypeDef htim_brk = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_TIM5_CLK_ENABLE();

    gpio.Pin = EXTBRK_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(EXTBRK_PORT, &gpio);

    
    htim_brk.Instance = TIM5;
    htim_brk.Init.Prescaler = 0;
    htim_brk.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_brk.Init.Period = pwmPeriodCounts;
    htim_brk.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim_brk.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim_brk);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

#if defined(EXTBRK_USE_CH3)
    HAL_TIM_PWM_ConfigChannel(&htim_brk, &sConfigOC, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim_brk, TIM_CHANNEL_3);
#elif defined(EXTBRK_USE_CH4)
    HAL_TIM_PWM_ConfigChannel(&htim_brk, &sConfigOC, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim_brk, TIM_CHANNEL_4);
#endif
  }
#endif

  __HAL_TIM_ENABLE(&htim_right);
}

void MX_ADC1_Init(void) {
  ADC_MultiModeTypeDef multimode;
  ADC_ChannelConfTypeDef sConfig;
  ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
  __HAL_RCC_ADC1_CLK_ENABLE();

  hadc1.Instance                   = ADC1;
  hadc1.Init.ScanConvMode          = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T8_TRGO;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = ADC12_CONV_COUNT;

  HAL_ADC_Init(&hadc1);
  /**Enable or disable the remapping of ADC1_ETRGREG:
    * ADC1 External Event regular conversion is connected to TIM8 TRG0
    */
  __HAL_AFIO_REMAP_ADC1_ETRGREG_ENABLE();
  /** Configure the ADC multi-mode
  */
 multimode.Mode = ADC_DUALMODE_REGSIMULT;
  HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode);

  /** Configure Analog WatchDog 1
  */
  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
  /* Right DC link current (ADC1) watchdog: use compile-time counts from config.h */
  AnalogWDGConfig.HighThreshold = DCR_HIGH_COUNTS;
  AnalogWDGConfig.LowThreshold = DCR_LOW_COUNTS;
  /* Right DC sensor is on PC1 -> ADC_CHANNEL_11 */
  AnalogWDGConfig.Channel = ADC_CHANNEL_11;
  AnalogWDGConfig.ITMode = ENABLE;
  HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig);

  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.Channel = ADC_CHANNEL_11;  // pc1 left cur  ->  right
  sConfig.Rank    = 1;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  // sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
  sConfig.Channel = ADC_CHANNEL_0;  // pa0 right a   ->  left
  sConfig.Rank    = 2;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  sConfig.Channel = ADC_CHANNEL_14;  // pc4 left b   -> right
  sConfig.Rank    = 3;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

#if defined(ENABLE_BOARD_TEMP_SENSOR)
  // temperature requires at least 17.1us sampling time
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;  // internal temp

  sConfig.Rank    = 4;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
#endif

  hadc1.Instance->CR2 |= ADC_CR2_DMA;
#if defined(ENABLE_BOARD_TEMP_SENSOR)
  hadc1.Instance->CR2 |= ADC_CR2_TSVREFE;
#endif

  __HAL_ADC_ENABLE(&hadc1);

  /* ADC1/2 analog watchdogs: make sure their IRQ runs at highest priority (group 0)
   * so critical DC link and phase-current watchdogs are serviced immediately.
   */
  /* ADC watchdog IRQ at priority 0 — must beat FOC (below) so overcurrent
   * cuts PWM before FOC continues. Kept at 0. */
  HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ADC1_2_IRQn);

  __HAL_RCC_DMA1_CLK_ENABLE();

  DMA1_Channel1->CCR   = 0;
  DMA1_Channel1->CNDTR = ADC12_CONV_COUNT;
  DMA1_Channel1->CPAR  = (uint32_t) & (ADC1->DR);
  DMA1_Channel1->CMAR  = (uint32_t)&adc_buffer.adc12.raw[0];
  DMA1_Channel1->CCR   = DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_1 | DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TCIE;
  DMA1_Channel1->CCR |= DMA_CCR_EN;

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

/* ADC2 init function */
void MX_ADC2_Init(void) {
  ADC_ChannelConfTypeDef sConfig;
  ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
  __HAL_RCC_ADC2_CLK_ENABLE();

  // HAL_ADC_DeInit(&hadc2);
  // hadc2.Instance->CR2 = 0;
  /**Common config
    */
  hadc2.Instance                   = ADC2;
  hadc2.Init.ScanConvMode          = ADC_SCAN_ENABLE;
  hadc2.Init.ContinuousConvMode    = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion       = ADC12_CONV_COUNT;
  HAL_ADC_Init(&hadc2);

   AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
  /* Left DC link current (ADC2) watchdog: use compile-time counts from config.h */
  AnalogWDGConfig.HighThreshold = DCL_HIGH_COUNTS;
  AnalogWDGConfig.LowThreshold = DCL_LOW_COUNTS;
  /* Left DC sensor is on PC0 -> ADC_CHANNEL_10 */
  AnalogWDGConfig.Channel = ADC_CHANNEL_10;
  AnalogWDGConfig.ITMode = ENABLE;
  HAL_ADC_AnalogWDGConfig(&hadc2, &AnalogWDGConfig);


  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.Channel = ADC_CHANNEL_10;  // pc0 right cur   -> left
  sConfig.Rank    = 1;
  HAL_ADC_ConfigChannel(&hadc2, &sConfig);

  // sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
  sConfig.Channel = ADC_CHANNEL_13;  // pc3 right b   -> left
  sConfig.Rank    = 2;
  HAL_ADC_ConfigChannel(&hadc2, &sConfig);

  sConfig.Channel = ADC_CHANNEL_15;  // pc5 left c   -> right
  sConfig.Rank    = 3;
  HAL_ADC_ConfigChannel(&hadc2, &sConfig);

  hadc2.Instance->CR2 |= ADC_CR2_DMA; //Transfer enable DMA
  __HAL_ADC_ENABLE(&hadc2); // Enable ADC2
}

void MX_ADC3_Init(void) {
  ADC_ChannelConfTypeDef sConfig = {0};

  __HAL_RCC_ADC3_CLK_ENABLE();

  hadc3.Instance                   = ADC3;
  hadc3.Init.ScanConvMode          = ADC_SCAN_ENABLE;
  hadc3.Init.ContinuousConvMode    = ENABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion       = ADC3_CONV_COUNT;
  HAL_ADC_Init(&hadc3);

 

  #if defined(ANALOG_BUTTON)
  sConfig.Channel = BUTTON_ADC_CHANNEL;
  sConfig.Rank = 1;  //Power Button
  sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
  HAL_ADC_ConfigChannel(&hadc3, &sConfig);
  #endif

  /* Battery is sampled immediately after the optional power button. PA2/PA3
   * are intentionally excluded: when unused they float and can couple PWM
   * noise into the ADC sample-and-hold before the high-impedance battery
   * divider is measured. Use the longest sample time so that divider settles. */
  sConfig.Channel = DCLINK_ADC_CHANNEL;
  #if defined(ANALOG_BUTTON)
  sConfig.Rank = 2;
  #else
  sConfig.Rank = 1;
  #endif
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  HAL_ADC_ConfigChannel(&hadc3, &sConfig);

 __HAL_ADC_ENABLE(&hadc3);  // Enable ADC3

  #if defined(DC_LINK_WATCHDOG_ENABLE)
  HAL_NVIC_SetPriority(ADC3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ADC3_IRQn);
  #endif
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////   DMA ZONE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  hadc3.Instance->CR2 |= ADC_CR2_DMA; //transfer enable DMA

  __HAL_RCC_DMA2_CLK_ENABLE();

  DMA2_Channel5->CCR  = 0;                                           // Disable channel & clear previous configuration
  DMA2_Channel5->CNDTR = ADC3_CONV_COUNT;                                          // Set number of data items to transfer (N)

  DMA2_Channel5->CPAR  = (uint32_t)&(ADC3->DR);                      // Peripheral address: ADC3 data register (source)
  DMA2_Channel5->CMAR  = (uint32_t)&adc_buffer.adc3.raw[0];          // Memory address: destination buffer (target)
  /* Configure DMA channel:
   * - Peripheral size = half-word (16-bit)
   * - Memory   size = half-word (16-bit)
   * - Memory increment enabled so subsequent samples write to successive buffer entries
   * - Circular mode enabled so DMA restarts automatically after finishing transfers
   */
  DMA2_Channel5->CCR   = DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0 | DMA_CCR_MINC | DMA_CCR_CIRC;
  DMA2_Channel5->CCR  |= DMA_CCR_EN;                                  // Enable channel to start transfers
 
  #if defined(ANALOG_BUTTON)
  AnalogButton_Init();
#endif


}

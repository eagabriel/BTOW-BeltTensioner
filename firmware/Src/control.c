
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "defines.h"
#include "setup.h"
#include "config.h"
#include "util.h"
#include "rtwtypes.h"

#define NUNCHUK_I2C_ADDRESS 0xA4

TIM_HandleTypeDef TimHandle;
TIM_HandleTypeDef TimHandle2;
uint8_t  ppm_count = 0;
uint8_t  pwm_count = 0;
uint32_t timeoutCntGen = TIMEOUT;
uint8_t  timeoutFlgGen = 0;
uint8_t  nunchuk_data[6] = {0};

uint8_t i2cBuffer[2];
nunchuk_state nunchukState = NUNCHUK_CONNECTING;

extern I2C_HandleTypeDef hi2c2;
extern DMA_HandleTypeDef hdma_i2c2_rx;
extern DMA_HandleTypeDef hdma_i2c2_tx;









uint8_t Nunchuk_tx(uint8_t i2cBuffer[], uint8_t i2cBufferLength) {
  if(HAL_I2C_Master_Transmit(&hi2c2,NUNCHUK_I2C_ADDRESS,(uint8_t*)i2cBuffer, i2cBufferLength, 100) == HAL_OK) {
    return true;
  }
  return false;
}

uint8_t Nunchuk_rx(uint8_t i2cBuffer[], uint8_t i2cBufferLength) {
  if(HAL_I2C_Master_Receive(&hi2c2,NUNCHUK_I2C_ADDRESS,(uint8_t*)i2cBuffer, i2cBufferLength, 100) == HAL_OK) {
    return true;
  }
  return false;
}

uint8_t Nunchuk_Init(void) {
  //-- START -- init WiiNunchuk
  i2cBuffer[0] = 0xF0;
  i2cBuffer[1] = 0x55;

  if(Nunchuk_tx(i2cBuffer, 2) == false) {
    return false;
  }
  HAL_Delay(10);

  i2cBuffer[0] = 0xFB;
  i2cBuffer[1] = 0x00;

  if(Nunchuk_tx(i2cBuffer, 2) == false) {
    return false;
  }
  HAL_Delay(10);

  return true;
}

uint8_t Nunchuk_Connect() {
  /* Initialise / re-initialise I2C peripheral */
  I2C_Init();
  
  /* Initialise / re-initialise nunchuk */
  if(Nunchuk_Init() == true) {
    nunchukState = NUNCHUK_CONNECTED;
    return true;
  } else {
    return false;
  }
}

nunchuk_state Nunchuk_Read(void) {
  static uint8_t delay_counter = 0;
  uint16_t checksum = 0;
  uint8_t success = true;
  uint8_t i = 0;

  switch(nunchukState) {
    case NUNCHUK_DISCONNECTED:
      success = false;
      /* Delay a bit before reconnecting */
      if(delay_counter++ > 100) {
        success = Nunchuk_Connect();
        delay_counter = 0;
      }
      break;
      
    case NUNCHUK_CONNECTING:
    case NUNCHUK_RECONNECTING:
        /* Try to reconnect once, if fails again fall back to disconnected state */
        success = Nunchuk_Connect();
        if(!success) {
          nunchukState = NUNCHUK_DISCONNECTED;
        }
      break;

    case NUNCHUK_CONNECTED:
      /* Send read address of 0x00 to the Nunchuk */
      i2cBuffer[0] = 0x00;
      if(!Nunchuk_tx(i2cBuffer, 1)) {
        success = false;
      }
      HAL_Delay(3);

      /* Clear the receive data buffer */
      for(i = 0; i<6; i++) {
        nunchuk_data[i] = 0;
      }

      /* Read back 6 bytes from the Nunchuk */
      if(!Nunchuk_rx(nunchuk_data, 6)) {
        success = false;
      }
      HAL_Delay(3);

      /* Checksum the receive buffer to ensure it is not in an error condition, i.e. all 0x00 or 0xFF */
      for(i = 0; i<6; i++) {
        checksum += nunchuk_data[i];
      }
      if(checksum == 0 || checksum == 0x5FA) {
        success = false;
      }

      /* Comms failure or timeout counter reached timeout limit */
      if(success == false || timeoutCntGen > 3) {
        /* Clear the receive data buffer */
        for(i = 0; i<6; i++) {
          nunchuk_data[i] = 0;
        }
        /* Brings motors to safe stop */
        /* Expected values from nunchuk for stopped (mid) position */
        nunchuk_data[0] = 127;
        nunchuk_data[1] = 128;
        timeoutFlgGen = 1;
        nunchukState = NUNCHUK_RECONNECTING;
      }
      break;
  }
  /* Reset the timeout flag and counter if successful communication */
  if(success == true) {
    timeoutCntGen = 0;
    timeoutFlgGen = 0;
  }
  return nunchukState;
  //setScopeChannel(0, (int)nunchuk_data[0]);
  //setScopeChannel(1, (int)nunchuk_data[1]);
  //setScopeChannel(2, (int)nunchuk_data[5] & 1);
  //setScopeChannel(3, ((int)nunchuk_data[5] >> 1) & 1);
}

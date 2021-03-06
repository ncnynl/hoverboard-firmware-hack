
#include "stm32f1xx_hal.h"
#include "defines.h"
#include "setup.h"
#include "config.h"

TIM_HandleTypeDef TimHandle;
uint8_t ppm_count = 0;
uint32_t timeout = 100;
uint8_t nunchuck_data[6] = {0};

uint8_t ai2cBuffer[6];

extern I2C_HandleTypeDef hi2c2;
DMA_HandleTypeDef hdma_i2c2_rx;
DMA_HandleTypeDef hdma_i2c2_tx;

#ifdef CONTROL_PPM
uint16_t ppm_captured_value[PPM_NUM_CHANNELS+1] = {0};

void PPM_ISR_Callback() {
  // Dummy loop with 16 bit count wrap around
  uint16_t rc_delay = TIM2->CNT;
  TIM2->CNT = 0;

  if (rc_delay > 3000) {
    ppm_count = 0;
  }
  else if (ppm_count < PPM_NUM_CHANNELS){
    timeout = 0;
    ppm_captured_value[ppm_count] = CLAMP(rc_delay, 1000, 2000) - 1000;
    ppm_count++;
  }

}

void PPM_Init() {
  GPIO_InitTypeDef GPIO_InitStruct;
  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  __HAL_RCC_TIM2_CLK_ENABLE();
  TimHandle.Instance = TIM2;
  TimHandle.Init.Period = UINT16_MAX;
  TimHandle.Init.Prescaler = (SystemCoreClock/DELAY_TIM_FREQUENCY_US)-1;;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  HAL_TIM_Base_Init(&TimHandle);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  HAL_TIM_Base_Start(&TimHandle);
}
#endif

void Nunchuck_Init() {
    //-- START -- init WiiNunchuck
  ai2cBuffer[0] = 0xF0;
  ai2cBuffer[1] = 0x55;
  //Originale
  ai2cBuffer[0] = 0x40;
  ai2cBuffer[1] = 0x00;
  //HAL_I2C_Master_Transmit_DMA(&hi2c2, 0xA4, (uint8_t*)ai2cBuffer, 2);
  //while(wii_JOYdati.I2CTxDone ==0);
  //wii_JOYdati.I2CTxDone = 0;
  HAL_I2C_Master_Transmit(&hi2c2,0xA4,(uint8_t*)ai2cBuffer, 2, 100);
  HAL_Delay(10);
  //wii_JOYdati.done = 0;
}

void Nunchuck_Read() {
  ai2cBuffer[0] = 0x00;
  HAL_I2C_Master_Transmit(&hi2c2,0xA4,(uint8_t*)ai2cBuffer, 1, 100);
  HAL_Delay(2);
  HAL_I2C_Master_Receive(&hi2c2,0xA4,(uint8_t*)ai2cBuffer, 6, 100);
  for (int i = 0; i < 6; i++) {
     nunchuck_data[i] = (ai2cBuffer[i] ^ 0x17) + 0x17;
  }
  //setScopeChannel(0, (int)nunchuck_data[0]);
  //setScopeChannel(1, (int)nunchuck_data[1]);
  //setScopeChannel(2, (int)nunchuck_data[5] & 1);
  //setScopeChannel(3, ((int)nunchuck_data[5] >> 1) & 1);
}

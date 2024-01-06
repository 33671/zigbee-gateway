#include "utils.h"
#include "tim.h"
#include "stm32f1xx_hal_rtc.h"
#include "stdlib.h"
#include "string.h"
#include <stdio.h>
#include "gpio.h"
#include "usart.h"
uint8_t rxbuffer_uart5[RxBuf_SIZE_uart5];
uint32_t rx_posi;
extern DMA_HandleTypeDef hdma_usart2_rx;
uint8_t RxBuf[512];
bool is_uart5_idle = false;
bool uart_idle_data_prepared;
uint16_t newPos_uart5 = 0;
uint16_t oldPos_uart5 = 0;
uint16_t format_text_start_pos = 0;
uint8_t MainBuf_uart5[MainBuf_SIZE_uart5];
uint16_t newPos = 0;
uint16_t oldPos = 0;
uint8_t MainBuf[MainBuf_SIZE];
uint8_t RxBuf[RxBuf_SIZE];


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance == USART2)
  {
    uart_idle_data_prepared = false;
    oldPos = newPos;  // Update the last position before copying new data
    /* If the data in large and it is about to exceed the buffer size, we have to route it to the start of the buffer
     * This is to maintain the circular buffer
     * The old data in the main buffer will be overlapped
     */
    if (oldPos+Size > MainBuf_SIZE)  // If the current position + new data size is greater than the main buffer
    {
      oldPos = 0;  // point to the start of the buffer
      memcpy ((uint8_t *)MainBuf, (uint8_t *)RxBuf, Size);  // copy the data to start
      newPos = Size;  // update the position
    }
    else
    {
      memcpy ((uint8_t *)MainBuf+oldPos, RxBuf, Size);
      newPos = Size+oldPos;
    }
    uart_idle_data_prepared = true;
    /* start the DMA again */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *) RxBuf, RxBuf_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
  }
  else if (huart->Instance == UART5)
  {
    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_8);
    is_uart5_idle = false;
    oldPos_uart5 = newPos_uart5;  // Update the last position before copying new data
    /* If the data in large and it is about to exceed the buffer size, we have to route it to the start of the buffer
     * This is to maintain the circular buffer
     * The old data in the main buffer will be overlapped
     */
		if (oldPos_uart5+Size > MainBuf_SIZE_uart5 && rxbuffer_uart5[0] != '[')
		{
			oldPos_uart5 = 0;
			memset(MainBuf_uart5,0,MainBuf_SIZE_uart5);
			newPos_uart5 = 0;
		}
    else if (rxbuffer_uart5[0] == '[')  // If the current position + new data size is greater than the main buffer
    {
      oldPos_uart5 = 0;  // point to the start of the buffer
			memset(MainBuf_uart5,0,MainBuf_SIZE_uart5);
      memcpy ((uint8_t *)MainBuf_uart5, (uint8_t *)rxbuffer_uart5, Size);  // copy the data to start
      newPos_uart5 = Size;  // update the position
    }
    else if (oldPos_uart5 != 0)
    {
      memcpy ((uint8_t *)MainBuf_uart5+oldPos_uart5, rxbuffer_uart5, Size);
      newPos_uart5 = Size+oldPos_uart5;
    }
    if (rxbuffer_uart5[Size - 1] == '\n' || rxbuffer_uart5[Size - 1] == ']')
    {
      is_uart5_idle = true;
    }
    else {
      is_uart5_idle = false;
    }
    HAL_UARTEx_ReceiveToIdle_IT(&huart5,rxbuffer_uart5,255);
  }
}
void led_signal()
{
  HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_8);
  HAL_Delay(200);
  HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_8);
  HAL_Delay(200);
}

int utils_num = 1;
RTC_TimeTypeDef time_after(RTC_TimeTypeDef *currentTime,uint32_t secondsToAdd) {
  // 将时间转换为秒数
  uint32_t totalSeconds = currentTime->Hours * 3600 + currentTime->Minutes * 60 + currentTime->Seconds;
  // 增加秒数
  totalSeconds += secondsToAdd;
  totalSeconds = totalSeconds % (3600 * 24);
  // 计算新的时分秒
  RTC_TimeTypeDef newTime = {0};
  newTime.Hours = totalSeconds / 3600;
  newTime.Minutes = (totalSeconds % 3600) / 60;
  newTime.Seconds = totalSeconds % 60;

  return newTime;
}
uint32_t RTC_ReadTimeCounter(RTC_HandleTypeDef *hrtc)
{
  uint16_t high1 = 0U, high2 = 0U, low = 0U;
  uint32_t timecounter = 0U;

  high1 = READ_REG(hrtc->Instance->CNTH & RTC_CNTH_RTC_CNT);
  low   = READ_REG(hrtc->Instance->CNTL & RTC_CNTL_RTC_CNT);
  high2 = READ_REG(hrtc->Instance->CNTH & RTC_CNTH_RTC_CNT);

  if (high1 != high2)
  {
    /* In this case the counter roll over during reading of CNTL and CNTH registers,
       read again CNTL register then return the counter value */
    timecounter = (((uint32_t) high2 << 16U) | READ_REG(hrtc->Instance->CNTL & RTC_CNTL_RTC_CNT));
  }
  else
  {
    /* No counter roll over during reading of CNTL and CNTH registers, counter
       value is equal to first value of CNTL and CNTH */
    timecounter = (((uint32_t) high1 << 16U) | low);
  }

  return timecounter;
}
/**
 * @brief  This function provides a delay (in microseconds)
 * @param  microseconds: delay in microseconds
 */
void delay_us(uint16_t us)
{
  __HAL_TIM_SET_COUNTER(&htim1,0);  // set the counter value a 0
  while (__HAL_TIM_GET_COUNTER(&htim1) < us);  // wait for the counter to reach the us input in the parameter
}
void delay_ms(uint32_t ms)
{
  HAL_Delay(ms);
}
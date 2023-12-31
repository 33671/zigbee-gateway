#include "utils.h"
#include "tim.h"
#include "stm32f1xx_hal_rtc.h"
#include "stdlib.h"
#include "string.h"
#include <stdio.h>
#include "gpio.h"
#include "usart.h"
uint8_t rxbuffer_uart5[RxBuf_SIZE_uart5];
uint16_t last_size_uart5 = 0;
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
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == UART5)
  {
    if(__HAL_UART_GET_FLAG(huart,UART_FLAG_ORE) != RESET)
    {
      __HAL_UART_CLEAR_OREFLAG(huart);
      HAL_UARTEx_ReceiveToIdle_IT(&huart5, (uint8_t *)&rxbuffer_uart5,RxBuf_SIZE_uart5);
    }
  }
}
#define is_start(rxbuffer_uart5) (rxbuffer_uart5[0] == '[' || rxbuffer_uart5[0] == 0xFD)
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  //__disable_irq();
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
    HAL_StatusTypeDef return_state = HAL_UARTEx_ReceiveToIdle_IT(&huart5,rxbuffer_uart5,RxBuf_SIZE_uart5);
    is_uart5_idle = false;
    oldPos_uart5 = newPos_uart5;  // Update the last position before copying new data
    /* If the data in large and it is about to exceed the buffer size, we have to route it to the start of the buffer
     * This is to maintain the circular buffer
     * The old data in the main buffer will be overlapped
     */
    if (oldPos_uart5+Size > MainBuf_SIZE_uart5 && is_start(rxbuffer_uart5) == false)
    {
      oldPos_uart5 = 0;
      memset(MainBuf_uart5,0,MainBuf_SIZE_uart5);
      newPos_uart5 = 0;
    }
    else if (is_start(rxbuffer_uart5))  // If the current position + new data size is greater than the main buffer
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
    if (rxbuffer_uart5[Size - 1] == '\n' || rxbuffer_uart5[Size - 1] == ']' || rxbuffer_uart5[Size - 3] == ']')
    {
      last_size_uart5 = Size;
      is_uart5_idle = true;
    }
    else {
      is_uart5_idle = false;
    }
    //__enable_irq();
    if(return_state != HAL_OK)
    {
      HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_8);
      do
      {
        //清除ORE错误
//        __HAL_UART_CLEAR_FEFLAG(&huart5);
//        __HAL_UART_CLEAR_PEFLAG(&huart5);
//        __HAL_UART_CLEAR_IDLEFLAG(&huart5);
        __HAL_UART_CLEAR_OREFLAG(&huart5);//清楚ORE标志位
        huart5.RxState= HAL_UART_STATE_READY;
        huart5.Lock = HAL_UNLOCKED;
//        //解除忙状态（由ORE导致，清零ORE位）
//        if(return_state == HAL_BUSY)
//        {
//
//        }
      }
      while(HAL_UARTEx_ReceiveToIdle_IT(&huart5, (uint8_t *)&rxbuffer_uart5,RxBuf_SIZE_uart5)!=HAL_OK);//重新开始接收
    }
  }
  //__enable_irq();
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
  * @brief  Enters the RTC Initialization mode.
  * @param  hrtc   pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval HAL status
  */
static HAL_StatusTypeDef RTC_EnterInitMode(RTC_HandleTypeDef *hrtc)
{
  uint32_t tickstart = 0U;

  tickstart = HAL_GetTick();
  /* Wait till RTC is in INIT state and if Time out is reached exit */
  while ((hrtc->Instance->CRL & RTC_CRL_RTOFF) == (uint32_t)RESET)
  {
    if ((HAL_GetTick() - tickstart) >  RTC_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Disable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);


  return HAL_OK;
}
/**
  * @brief  Exit the RTC Initialization mode.
  * @param  hrtc   pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval HAL status
  */
static HAL_StatusTypeDef RTC_ExitInitMode(RTC_HandleTypeDef *hrtc)
{
  uint32_t tickstart = 0U;

  /* Disable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);

  tickstart = HAL_GetTick();
  /* Wait till RTC is in INIT state and if Time out is reached exit */
  while ((hrtc->Instance->CRL & RTC_CRL_RTOFF) == (uint32_t)RESET)
  {
    if ((HAL_GetTick() - tickstart) >  RTC_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  return HAL_OK;
}

/**
  * @brief  Write the time counter in RTC_CNT registers.
  * @param  hrtc   pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @param  TimeCounter: Counter to write in RTC_CNT registers
  * @retval HAL status
  */
HAL_StatusTypeDef RTC_WriteTimeCounter(RTC_HandleTypeDef *hrtc, uint32_t TimeCounter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Set Initialization mode */
  if (RTC_EnterInitMode(hrtc) != HAL_OK)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Set RTC COUNTER MSB word */
    WRITE_REG(hrtc->Instance->CNTH, (TimeCounter >> 16U));
    /* Set RTC COUNTER LSB word */
    WRITE_REG(hrtc->Instance->CNTL, (TimeCounter & RTC_CNTL_RTC_CNT));

    /* Wait for synchro */
    if (RTC_ExitInitMode(hrtc) != HAL_OK)
    {
      status = HAL_ERROR;
    }
  }

  return status;
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
#include "utils.h"
//#include "stm32f1xx_hal.h"
#include "tim.h"
#include "stm32f1xx_hal_rtc.h"
//#define FLASH_KEY1               ((uint32_t)0x45670123)
//#define FLASH_KEY2               ((uint32_t)0xCDEF89AB)
/**
* @brief  Unlocks the FLASH control register access
* @param  None
* @retval None
*/
void FLASH_Unlock(void)
{
    if ((FLASH->CR & FLASH_CR_LOCK) != RESET) {
        /* Authorize the FLASH Registers access */
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

/**
* @brief  Locks the FLASH control register access
* @param  None
* @retval None
*/
void FLASH_Lock(void)
{
    /* Set the LOCK Bit to lock the FLASH Registers access */
    FLASH->CR |= FLASH_CR_LOCK;
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
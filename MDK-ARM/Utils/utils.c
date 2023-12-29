#include "utils.h"
#include "stm32f1xx_hal.h"
#include "tim.h"
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
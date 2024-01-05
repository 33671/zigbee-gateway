#include "rtc.h"
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
RTC_TimeTypeDef time_after(RTC_TimeTypeDef *currentTime,uint32_t secondsToAdd);
void delayMicroseconds(volatile uint32_t microseconds);
void delay_us(uint16_t us);
void delay_ms(uint32_t ms);
void led_signal();
extern int utils_num;
uint32_t RTC_ReadTimeCounter(RTC_HandleTypeDef *hrtc);
#include "rtc.h"
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
RTC_TimeTypeDef time_after(RTC_TimeTypeDef *currentTime,uint32_t secondsToAdd);
void delayMicroseconds(volatile uint32_t microseconds);
void delay_us(uint16_t us);
void delay_ms(uint32_t ms);
void led_signal();
uint32_t RTC_ReadTimeCounter(RTC_HandleTypeDef *hrtc);
#define RxBuf_SIZE   512
#define RxBuf_SIZE_uart5   255
#define MainBuf_SIZE 2048
#define MainBuf_SIZE_uart5 512

#ifndef ZIGBEE
#define ZIGBEE
typedef struct ZigbeeDevice
{
	u8 id;
	bool is_online;
	uint32_t ask_time;
	uint32_t last_response_time;
} ZigbeeDevice;
#endif
#ifndef __lcd12864_H_
#define __lcd12864_H_

#include "gpio.h"
#include "utils.h"
//#include "sys.h"

#define WRITE_CMD	0xF8//写命令  
#define WRITE_DAT	0xFA//写数据

//接口(SID: PE1  SCLK: PE0) 
#define SID GPIOC,GPIO_PIN_9
#define SCLK GPIOB,GPIO_PIN_1
void print_hex(uint8_t *rxBuffer);
void show_time(uint8_t x,uint8_t y,RTC_TimeTypeDef* sTime);
void show_date(uint8_t x,uint8_t y,RTC_DateTypeDef* sDate);
void lcd_GPIO_init(void);
void Lcd_Init(void);

void SendByte(u8 Dbyte);
void LCD_Clear(void);
void LCD_Display_Words(uint8_t x,uint8_t y,uint8_t*str);
void LCD_Display_Picture(uint8_t *img);

void Lcd_WriteData(u8 Dat );
void Lcd_WriteCmd(u8 Cmd );
static uint8_t RTC_Bcd2ToByte(uint8_t Value)
{
  uint32_t tmp = 0U;
  tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10U;
  return (tmp + (Value & (uint8_t)0x0F));
}
#endif

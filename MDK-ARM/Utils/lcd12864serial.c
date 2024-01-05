#include "lcd12864serial.h"
#include "gpio.h"
#include <stdio.h>
#include "stm32f1xx_hal_rtc.h"
void btox(uint8_t *xp,uint8_t *bb, int n)
{
  const char xx[]= "0123456789ABCDEF";
  while (--n >= 0) xp[n] = xx[(bb[n>>1] >> ((1 - (n&1)) << 2)) & 0xF];
}

void print_hex(uint8_t *rxBuffer)
{
  uint8_t hexstr[17];
  btox(hexstr, rxBuffer, 8 * 2);
  hexstr[16] = 0;
  LCD_Display_Words(1,0,hexstr);
}
/* 字符显示RAM地址    4行8列 */
u8 LCD_addr[4][8]= {
  {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87},  		//第一行
  {0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97},		//第二行
  {0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F},		//第三行
  {0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F}		//第四行
};
/*!
*  @brief      LCD显示时间
 *  @since      v1.0
 *  @param  sTime   时间
 *  @author
 */
void show_time(uint8_t x,uint8_t y,RTC_TimeTypeDef* sTime)
{
  char current_time[] = "23:59:59";
  sprintf(current_time,"%02d:%02d:%02d",sTime->Hours,sTime->Minutes,sTime->Seconds);
  LCD_Display_Words(x,y,(uint8_t *)current_time);
}
/*!
*  @brief      LCD显示日期
 *  @since      v1.0
 *  @param  sTime   时间
 *  @author
 */
void show_date(uint8_t x,uint8_t y,RTC_DateTypeDef* sDate)
{
  char current_time[] = "2023-59-59";
  sprintf(current_time,"20%02d/%02d/%02d",sDate->Year,RTC_Bcd2ToByte(sDate->Month),sDate->Date);
  LCD_Display_Words(x,y,(uint8_t *)current_time);
}
/*!
*  @brief      LCD串行发送一个字节
 *  @since      v1.0
 *  @param  byte   写入字节
 *  @author     Z小旋
 */
void SendByte(u8 byte)
{
  u8 i;
  for(i = 0; i < 8; i++)
  {
    if((byte << i) & 0x80)  //0x80(1000 0000)  只会保留最高位
    {
      HAL_GPIO_WritePin(SID,1);           // 引脚输出高电平，代表发送1
    }
    else
    {
      HAL_GPIO_WritePin(SID,0);         // 引脚输出低电平，代表发送0
    }
    HAL_GPIO_WritePin(SCLK,0);   //时钟线置低  允许SID变化
    delay_us(10); //延时使数据写入
    HAL_GPIO_WritePin(SCLK,1);    //拉高时钟，让从机读SID
  }
}


/*!
 *  @brief      LCD写指令
 *  @since      v1.0
 *  @param  Cmd   要写入的指令
 *  @author     Z小旋
 */
void Lcd_WriteCmd(u8 Cmd )
{
  delay_ms(1);     //由于我们没有写LCD正忙的检测，所以直接延时1ms，使每次写入数据或指令间隔大于1ms 便可不用写忙状态检测
  SendByte(WRITE_CMD);            //11111,RW(0),RS(0),0
  SendByte(0xf0&Cmd);      //高四位
  SendByte(Cmd<<4);   //低四位(先执行<<)
}

/*!
 *  @brief      LCD写数据
 *  @since      v1.0
 *  @param  Dat   要写入的数据
 *  @author     Z小旋
 */
void Lcd_WriteData(u8 Dat )
{
  delay_ms(1);
  SendByte(WRITE_DAT);            //11111,RW(0),RS(1),0
  SendByte(0xf0&Dat);      //高四位
  SendByte(Dat<<4);   //低四位(先执行<<)
}
/*!
 *  @brief      LCD初始化
 *  @since      v1.0
 *  @param  None
 *  @author     Z小旋
 */
void Lcd_Init(void)
{
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,1);
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,1);
  led_signal();
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,0);
  delay_ms(50);   	//等待液晶自检（延时>40ms）
  //led_signal();
  Lcd_WriteCmd(0x30);        //功能设定:选择基本指令集

  delay_ms(1);//延时>100us
  Lcd_WriteCmd(0x30);        //功能设定:选择8bit数据流
  delay_ms(1);	//延时>37us
  Lcd_WriteCmd(0x0c);        //开显示
  delay_ms(1);	//延时>100us
  Lcd_WriteCmd(0x01);        //清除显示，并且设定地址指针为00H
  delay_ms(30);	//延时>10ms
  Lcd_WriteCmd(0x06);        //进入设定点，初始化完成
}


/*!
 *  @brief      显示字符或汉字
 *  @since      v1.0
 *  @param  x: row(0~3)
 *  @param  y: line(0~7)
 *  @param 	str: 要显示的字符或汉字
 *  @author     Z小旋
 */
void LCD_Display_Words(uint8_t x,uint8_t y,uint8_t*str)
{
  Lcd_WriteCmd(LCD_addr[x][y]); //写初始光标位置
  while(*str>0)
  {
    Lcd_WriteData(*str);    //写数据
    str++;
  }
}
/*!
 *  @brief      显示图片
 *  @since      v1.0
 *  @param  *pic   图片地址
 *  @author
 */
void LCD_Display_Picture(uint8_t *img)
{
  uint8_t x,y,i;
  Lcd_WriteCmd(0x34);		//切换到扩充指令
  Lcd_WriteCmd(0x34);		//关闭图形显示
  for(i = 0; i < 1; i++)   //上下屏写入
  {
    for(y=0; y<32; y++) //垂直Y写32次
    {
      for(x=0; x<8; x++) //横向X写8次
      {
        Lcd_WriteCmd(0x80 + y);		//行地址
        Lcd_WriteCmd(0x80 + x+i);		//列地址
        Lcd_WriteData(*img ++);		//写高位字节数据 D15－D8
        Lcd_WriteData(*img ++);		//写低位字节数据 D7－D0
      }
    }
  }
  Lcd_WriteCmd(0x36);//打开图形显示
  Lcd_WriteCmd(0x30);        //切换回基本指令
}
/*!
 *  @brief      清屏函数
 *  @since      v1.0
 *  @param  None
 *  @author     Z小旋
 */
void LCD_Clear(void)
{
  Lcd_WriteCmd(0x01);			//清屏指令
  delay_ms(2);				//延时以待液晶稳定【至少1.6ms】
}


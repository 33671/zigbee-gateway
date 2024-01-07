#include "drf1605h.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "usart.h"
#include "lcd12864serial.h"
bool areArrayEqual(uint8_t* arr1, uint8_t* arr2,int N)
{
  // Linearly compare elements
  for (int i = 0; i < N; i++)
    if (arr1[i] != arr2[i])
      return false;
  // If all elements were same.
  return true;
}
bool is_router_network_joined()
{
  uint8_t rx_buffer[8];
  uint8_t no_network_joined[2] =  {0xFF, 0xFE};
  uint8_t read_panid[] = {0xFC,0x00,0x91,0x03,0xA3,0xB3,0xE6};
  HAL_UART_Transmit(ZIGBEE_UART,read_panid,7,0xFFFF);
  HAL_StatusTypeDef status1 = HAL_UART_Receive(ZIGBEE_UART,rx_buffer,2,1000);
  if (status1 == HAL_OK) {
    return !areArrayEqual(rx_buffer,no_network_joined,2);
  }
  else {
    return false;
  }
}
void transparent_send(uint8_t* something,uint8_t size)
{
  HAL_StatusTypeDef return_state = HAL_UART_Transmit_IT(ZIGBEE_UART,something,size);
  if(return_state != HAL_OK)
  {
		__HAL_UART_ENABLE_IT(ZIGBEE_UART, UART_IT_ERR);
  }
}
//max data packet size: 256
void send_to_coordinator_p2p(uint8_t* something,uint8_t size)
{
	uint8_t send_to_addr[256] = {0xFD,size,0x00,0x00};
  memcpy(send_to_addr+4,something,size);
  HAL_StatusTypeDef return_state = HAL_UART_Transmit_IT(ZIGBEE_UART,send_to_addr,size+4);
  if(return_state != HAL_OK)
  {
		__HAL_UART_ENABLE_IT(ZIGBEE_UART, UART_IT_ERR);
//    __HAL_UART_CLEAR_FEFLAG(&huart5);
//    __HAL_UART_CLEAR_PEFLAG(&huart5);
//    __HAL_UART_CLEAR_IDLEFLAG(&huart5);
//    __HAL_UART_CLEAR_OREFLAG(ZIGBEE_UART);//清楚ORE标志位
//    huart5.RxState= HAL_UART_STATE_READY;
//    huart5.Lock = HAL_UNLOCKED;
    //HAL_UART_Transmit_IT(ZIGBEE_UART,something,size);//重新开始接收
  }
}
bool transparent_receive(uint8_t* something,uint8_t size,uint32_t timeout)
{
  HAL_StatusTypeDef is_ok = HAL_UART_Receive(ZIGBEE_UART,something,size,2000);
  //LCD_Fill(10,10,128,22,BLUE);
  return (is_ok == HAL_OK);
}
bool read_short_addr(uint8_t* addr,bool* network_joined)
{
  uint8_t rx_buffer[8];
  uint8_t no_network_joined[2] =  {0xFF, 0xFE};
  uint8_t read_short_addr[] = {0xFC,0x00, 0x91,0x04,0xC4,0xD4,0x29};
  HAL_UART_Transmit(ZIGBEE_UART,read_short_addr,7,0xFFFF);
  HAL_StatusTypeDef status1 = HAL_UART_Receive(ZIGBEE_UART,rx_buffer,2,1000);
  if (status1 == HAL_OK) {
    //print_hex(rx_buffer);
    *network_joined = !areArrayEqual(rx_buffer,no_network_joined,2);
    addr[0] = rx_buffer[0];
    addr[1] = rx_buffer[1];
    return true;
  }
  else {
    return false;
  }
}
bool set_channel()
{
  uint8_t rx_buffer[8];
  uint8_t set_channel[] = {0xFC,0x01,0x91,0x0C,0x0D,0x1A,0xC1};
  uint8_t expected_response[5] = {0x00,0x20,0x00,0x00,0x0D};
  HAL_UART_Transmit(ZIGBEE_UART,set_channel,7,0xFFFF);
  HAL_StatusTypeDef status1 = HAL_UART_Receive(ZIGBEE_UART,rx_buffer,5,1000);
  if (status1 == HAL_OK) {
    //print_hex(rx_buffer);
    return areArrayEqual(rx_buffer,expected_response,5);
  }
  else {
    return false;
  }
}
bool set_panid()
{
  uint8_t rx_buffer[8];
  uint8_t set_panid[] = {0xFC,0x02,0x91,0x01,0x13,0x14,0xB7};
  uint8_t expected_response[2] = {0x13,0x14};
  HAL_UART_Transmit(ZIGBEE_UART,set_panid,7,0xFFFF);
  HAL_StatusTypeDef status1 = HAL_UART_Receive(ZIGBEE_UART,rx_buffer,2,1000);
  if (status1 == HAL_OK) {
    //print_hex(rx_buffer);
    return areArrayEqual(rx_buffer,expected_response,2);
  }
  else {
    return false;
  }
}
bool set_as_router()
{
  uint8_t rx_buffer[8];
  uint8_t set_as_router[] = {0xFC, 0x00,0x91,0x0A,0xBA,0xDA,0x2B};
  uint8_t expected_response[8] = {0x52,0x6F,0x75,0x74,0x65,0x3B,0x00,0x19};
  HAL_UART_Transmit(ZIGBEE_UART,set_as_router,7,0xFFFF);
  HAL_StatusTypeDef status1 = HAL_UART_Receive(ZIGBEE_UART,rx_buffer,8,1000);
  if (status1 == HAL_OK) {
    //print_hex(rx_buffer);
    return areArrayEqual(rx_buffer,expected_response,8);
  }
  else {
    return false;
  }
}
bool set_as_coordinator()
{
  uint8_t rx_buffer[8];
  uint8_t set_coord[] = {0xFC,0x00,0x91, 0x09, 0xA9, 0xC9, 0x08};
  uint8_t expected_response[8] = {0x43,0x6F,0x6F,0x72,0x64,0x3B,0x00,0x19};
  HAL_UART_Transmit(ZIGBEE_UART,set_coord,7,0xFFFF);
  HAL_StatusTypeDef status1 = HAL_UART_Receive(ZIGBEE_UART,rx_buffer,8,1000);
  if (status1 == HAL_OK) {
    //print_hex(rx_buffer);
    return areArrayEqual(rx_buffer,expected_response,8);
  }
  else {
    return false;
  }
}
bool check_connection_ok()
{
  uint8_t rx_buffer[8];
  unsigned char at_check_baud[] = {0xFC,0x00,0x91,0x07,0x97,0xA7,0xD2};
  uint8_t expected_response[5] = {0x01,0x02,0x03,0x04,0x05};
  int count = 0;
  while (1)
  {
    HAL_UART_Transmit(ZIGBEE_UART,at_check_baud,7,0xFFFF);
    HAL_StatusTypeDef status1 = HAL_UART_Receive(ZIGBEE_UART,rx_buffer,8,1000);

    if (status1 == HAL_OK) {
      return areArrayEqual(rx_buffer,expected_response,5);
    }
    else {
      if (count > 10) {
        return false;
      }
      HAL_Delay(200);
      count++;
      continue;
    }
  }
  return false;
}
#include "sender.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "drf1605h.h"
#include "usart.h"
ZigbeeDevice devices[TOTAL_DEVICE];
void return_devices()
{

  u8 i = 0;
  char part[] = "id:1z,addr:0000,online:true,last_res:00000000\n";
  u8 len = strlen(part);
  char devices_description[len * 9];

  for (i = 0; i<TOTAL_DEVICE; i++)
  {
    u8 is_online = devices[i].is_online;
    u8 id = devices[i].id;
    u32 last_res = devices[i].last_response_time;
    u16 addr = devices[i].addr;
    sprintf((char *)part,"id:%dz,addr:%02x%02x,online:%d,last_res:%x\n",id,(u8)(addr>>8),(u8)(addr),is_online,last_res);
    //memcpy((devices_description+len*i),part,len);
    HAL_UART_Transmit(&huart2,(u8 *)part,strlen(part),0xFFFF);
  }
  HAL_UART_Transmit(&huart2,(u8 *)"end\n",4,0xFFFF);
}
void parse_command(uint8_t * data,u8 len)
{
  char* posi = strstr((char *)data,"devices");
  if (posi)
  {
    return_devices();
  }
  else if(strstr((char *)data,"send"))
  {
    int id = 0xFF;
    char* dataposi = strstr((char *)data,"data") + 5;
    sscanf((const char*)(data+5), "id:%dz,data:%*", &id);
    send_to_p2p(id - 1,(u8 *)(dataposi),strlen(dataposi));
  }
  else if(strstr((char *)data,"set-time"))
  {
    u32 time = 0;
    char* dataposi = strstr((char *)data,"time") + 4;
    if(sscanf((const char*)(dataposi), " %d", &time))
    {
      RTC_WriteTimeCounter(&hrtc,time);
    }
  }
  else {
    transparent_send((u8 *)data,len);
  }
}
u8 get_online_count()
{
  filter_offline();
  u8 i = 0,count = 0 ;
  for (i = 0; i<TOTAL_DEVICE; i++)
  {
    if (devices[i].is_online)
    {
      count++;
    }
  }
  return count;
}

void filter_offline() //把超时的设备设为离线
{
  u8 i = 0;
  u32 now = RTC_ReadTimeCounter(&hrtc);
  for (i = 0; i<TOTAL_DEVICE; i++)
  {
    if (devices[i].last_response_time == 0)
    {
      devices[i].is_online = false;
    }
    else if (now - devices[i].last_response_time > 60)
    {
      devices[i].is_online = false;
    }
    else {
      devices[i].is_online = true;
    }
  }
}
u8 parse_data_from_device(uint8_t * data,u16 len)
{
  int id = 0xFF;
  int short_addr = 0xFFFF;
  int status = 0;
  u16 p2d_end = 0;
  bool is_p2p = (data[0] == 0xFD);
  u8* start_ptr = data;
  if (is_p2p)
  {
    start_ptr += 4;
  }
  sscanf((const char*)start_ptr, "[id:%dz,addr:%d,data:%*,status:%d]", &id, &short_addr, &status );
  if (is_p2p)
  {
    short_addr = (data[len - 1]) | ((u16)data[len - 2] << 8);
  }
  if (id < 10 && status < 2)
  {
    id = id - 1;
    if (id < 0) return 0xFF;
    devices[id].last_response_time = RTC_ReadTimeCounter(&hrtc);
    devices[id].is_online = (status == 1);
    if (short_addr <= 0xFF00)
    {
      devices[id].addr = short_addr;
    }
  }
  return id;
}
bool send_to_p2p(u8 id,uint8_t* data,u8 len)
{
  if (id > 10) return false;
  u16 short_addr = devices[id].addr;
  if (short_addr > 0xFF00) return false;
  uint8_t send_to_addr[256] = {0xFD,len,(short_addr >> 8),short_addr};
  memcpy(send_to_addr+4,data,len);
  HAL_StatusTypeDef return_state = HAL_UART_Transmit_IT(ZIGBEE_UART,send_to_addr,len+4);
  return true;
}
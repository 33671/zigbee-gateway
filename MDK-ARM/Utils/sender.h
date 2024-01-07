#include "main.h"
#include "utils.h"
void parse_command(uint8_t * data,u8 len);
#define TOTAL_DEVICE 9
u8 parse_data_from_device(uint8_t * data,u16 len);
u8 get_online_count();
void filter_offline();
bool send_to_p2p(u8 id,uint8_t* data,u8 len);
#ifndef ZIGBEE
#define ZIGBEE
typedef struct ZigbeeDevice
{
  u8 id;
  bool is_online;
  uint16_t addr;
  bool is_pending;
  uint32_t ask_time;
  uint32_t last_response_time;
} ZigbeeDevice;
#endif
extern ZigbeeDevice devices[TOTAL_DEVICE];
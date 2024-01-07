#include "usart.h"
#define ZIGBEE_UART &huart5

bool is_router_network_joined();
bool set_as_coordinator();
bool set_as_router();
bool check_connection_ok();
void transparent_send(uint8_t* something,uint8_t size);
bool transparent_receive(uint8_t* something,uint8_t size,uint32_t timeout);
bool set_panid();
bool set_channel();
bool read_short_addr(uint8_t* addr,bool* network_joined);
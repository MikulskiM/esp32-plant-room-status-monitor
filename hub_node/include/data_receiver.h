#ifndef DATA_RECEIVER_H
#define DATA_RECEIVER_H

#include <esp_now.h>
#include <esp_wifi.h>

#include "utils.h"
#include "types.h"
#include "ring_buffer.h"
#include "node_registry.h"

// MAC address of sensor_node (5C:01:3B:73:7C:80)
#define SENSOR_MAC    {0x5C, 0x01, 0x3B, 0x73, 0x7C, 0x80}

extern RingBuffer ring_buffer;
extern NodeRegistry node_registry;

void initEspNow(uint8_t channel);
void onDataReceive(const uint8_t* mac, const uint8_t* incoming_data, int len);

#endif
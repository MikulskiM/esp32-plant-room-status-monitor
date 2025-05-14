#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

#define MAX_SOIL_SENSORS 5

typedef struct {
  float humidity;
  float temperature;
  float pressure;
  int soil_moisture_mapped[MAX_SOIL_SENSORS];
} RawSensorData;

typedef struct {
  RawSensorData* data;
  uint8_t peer_addr[6];
  int attempt;
} ResendContext;

#endif
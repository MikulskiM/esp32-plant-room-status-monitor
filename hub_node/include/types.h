#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

typedef struct {
  float humidity;
  float temperature;
  float pressure;
  int soil_moisture_mapped;
  time_t timestamp;
} SensorData;

#endif
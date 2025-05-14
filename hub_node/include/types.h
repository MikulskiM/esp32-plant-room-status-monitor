#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

#define SOIL_MOISTURE_NOT_PRESENT -999
#define MAX_SOIL_SENSORS 5

typedef struct {
  float humidity;
  float temperature;
  float pressure;
  int soil_moisture_mapped[MAX_SOIL_SENSORS];
  time_t timestamp;
} SensorData;

typedef struct {
  float humidity;
  float temperature;
  float pressure;
  int soil_moisture_mapped[MAX_SOIL_SENSORS];
} RawSensorData;

#endif
#ifndef UTILS_h
#define UTILS_H

#include <Arduino.h>
#include "types.h"

void initTime();
String formatTimeStamp(time_t timestamp);
String getCurrentTimestamp();
void printSensorData(SensorData& data);
String macToStr(const uint8_t* mac);

#endif
#ifndef UTILS_h
#define UTILS_H

#include <Arduino.h>

void initTime();
String formatTimeStamp(time_t timestamp);
String getCurrentTimestamp();

#endif
#include "utils.h"

void initTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "CET-1CEST,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
  tzset();
  Serial.print("Getting real time");
  while (time(nullptr) < 100000) {  // wait for valid time
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTime ready");
}

String formatTimeStamp(time_t timestamp) {
    // [09.05.2025 - 10:58]
    char timestamp_str[21];

    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);

    strftime(timestamp_str, sizeof(timestamp_str), "%d.%m.%Y - %H:%M", &timeinfo);

    return String(timestamp_str);
}

String getCurrentTimestamp() {
  time_t now = time(nullptr);
  return formatTimeStamp(now);
}

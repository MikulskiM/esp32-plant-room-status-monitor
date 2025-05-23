#include "utils.h"

void initTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "CET-1CEST,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
  tzset();

  Serial.print("Getting real time");

  int attempts = 0;
  const int maxAttempts = 20;

  while (time(nullptr) < 100000 && attempts < maxAttempts) {
    Serial.print(".");
    delay(500);
    attempts++;
  }

  if (time(nullptr) < 100000) {
    Serial.println("\nFailed to sync time via NTP.");
  } else {
    Serial.println("\nTime synced successfully.");
  }
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

void printSensorData(SensorData& data) {
    Serial.print("[");
    Serial.print(formatTimeStamp(data.timestamp));
    Serial.println("]");
    Serial.printf("  temp: %.2f °C\n", data.temperature);
    Serial.printf("  hum : %.2f %%\n", data.humidity);
    Serial.printf("  pres: %.2f hPa\n", data.pressure);
    Serial.print("  soil: ");
    for (int i = 0; i < MAX_SOIL_SENSORS; i++) {
      Serial.printf("[%d]: %d%%", i, data.soil_moisture_mapped[i]);
      if (i < MAX_SOIL_SENSORS - 1) Serial.print(" | ");
    }
    Serial.print(" %%\n");
}

String macToStr(const uint8_t* mac) {
  char buf[18];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

typedef struct {
  float humidity;
  float temperature;
  float pressure;
  int soil_moisture_mapped;
} SensorData;

void onDataReceive(const uint8_t* mac, const uint8_t* incoming_data, int len) {
  if (len == sizeof(SensorData)) {
    Serial.printf("Received data with SensorData size: %d bytes\n", len);

    SensorData* data = (SensorData*)incoming_data;

    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    Serial.printf("ESP-NOW Data received from %s:\n", mac_str);
    Serial.printf("\tTemp:\t%.2f°C\n", data->temperature);
    Serial.printf("\tHum:\t%.2f%%\n", data->humidity);
    Serial.printf("\tPress:\t%.2f hPa\n", data->pressure);
    Serial.printf("\tSoil:\t%d%%\n", data->soil_moisture_mapped);
  }
  else {
    Serial.printf("Received data with weird size: %d bytes\n", len);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  // ESP-NOW

  // check MAC address
  Serial.print("Hub node MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW esp_now_init() FAIL");
    return;
  }
  
  esp_now_register_recv_cb(onDataReceive);
  Serial.println("ESP-NOW receiver ready");
}

void loop() {
  // put your main code here, to run repeatedly:
}

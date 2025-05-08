#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_timer.h>
#include <ESPmDNS.h>

#include "esp_wifi.h"
#include "secrets.h"  // put your wifi name nad password

#define SERIAL_BAUD_RATE  115200
#define WIFI_CHANNEL      6

// MAC address of sensor_node (5C:01:3B:73:7C:80)
#define SENSOR_MAC    {0x5C, 0x01, 0x3B, 0x73, 0x7C, 0x80}
#define MAC_ADDR_LEN  6

typedef struct {
  float humidity;
  float temperature;
  float pressure;
  int soil_moisture_mapped;
} SensorData;

void onDataReceive(const uint8_t* mac, const uint8_t* incoming_data, int len) {
  if (len == sizeof(SensorData)) {
    SensorData* data = (SensorData*)incoming_data;

    // Serial log
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

void initEspNow() {
  WiFi.mode(WIFI_STA);  // ESP-NOW
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW esp_now_init() FAIL");
    return;
  }

  // Add sensor_node as peer
  esp_now_peer_info_t sensor_peer = {
    .peer_addr = SENSOR_MAC,
    .channel = 0,
    .encrypt = false
  };

  if (!esp_now_is_peer_exist(sensor_peer.peer_addr)) {
    if (esp_now_add_peer(&sensor_peer) != ESP_OK) {
      Serial.println("Failed to add sensor_node peer");
    } else {
      Serial.println("Sensor node peer added");
    }
  }

  esp_now_register_recv_cb(onDataReceive);
  Serial.println("ESP-NOW receiver ready");
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  // check MAC address
  Serial.print("Hub node MAC Address: ");
  Serial.println(WiFi.macAddress());

  initEspNow();

  Serial.println("Starting hub node...");
}

void loop() {
  // put your main code here, to run repeatedly:
}

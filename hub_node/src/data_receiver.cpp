#include "data_receiver.h"

SensorData latest_sensor_data;
bool has_sensor_data = false;

void initEspNow(uint8_t channel) {
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
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
      Serial.printf("Added peer MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
        sensor_peer.peer_addr[0], sensor_peer.peer_addr[1], sensor_peer.peer_addr[2],
        sensor_peer.peer_addr[3], sensor_peer.peer_addr[4], sensor_peer.peer_addr[5]);
    }
  }

  esp_now_register_recv_cb(onDataReceive);
  Serial.println("ESP-NOW receiver ready");
}

void onDataReceive(const uint8_t* mac, const uint8_t* incoming_data, int len) {
  if (len == sizeof(SensorData)) {
    SensorData* data = (SensorData*)incoming_data;

    latest_sensor_data = *data;
    has_sensor_data = true;

    String timestamp = getCurrentTimestamp();

    Serial.printf("[%s] ESP-NOW Data received from %02X:%02X:%02X:%02X:%02X:%02X:\n",
      timestamp.c_str(), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.printf("\tTemp:\t%.2f°C\n", data->temperature);
    Serial.printf("\tHum:\t%.2f%%\n", data->humidity);
    Serial.printf("\tPress:\t%.2f hPa\n", data->pressure);
    Serial.printf("\tSoil:\t%d%%\n", data->soil_moisture_mapped);
  } else {
    Serial.printf("Received data with weird size: %d bytes\n", len);
  }
}

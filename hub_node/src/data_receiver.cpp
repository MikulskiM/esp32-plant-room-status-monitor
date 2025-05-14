#include "data_receiver.h"

SensorData latest_sensor_data;
bool has_sensor_data = false;
RingBuffer ring_buffer;

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
  if (len != sizeof(RawSensorData)) {
    Serial.printf("Received data with weird size: %d bytes\n", len);
    Serial.printf("Expected data size: %d bytes\n", sizeof(RawSensorData));
    return;
  }

  SensorData data;
  RawSensorData* raw_data = (RawSensorData*)incoming_data;

  data.humidity = raw_data->humidity;
  data.temperature = raw_data->temperature;
  data.pressure = raw_data->pressure;
  data.timestamp = time(nullptr);
  Serial.printf("Raw timestamp: %ld\n", data.timestamp);

  for (int i = 0; i < MAX_SOIL_SENSORS; i++) {
    data.soil_moisture_mapped[i] = raw_data->soil_moisture_mapped[i];
  }

  Serial.printf("ESP-NOW Data received from %02X:%02X:%02X:%02X:%02X:%02X:\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  printSensorData(data);

  ring_buffer.addData(data);
}

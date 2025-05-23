#include <Arduino.h>
#include <esp_wifi.h>
#include <time.h>

#include "secrets.h"  // put your wifi name nad password
#include "types.h"
#include "utils.h"
#include "data_receiver.h"
#include "my_server.h"
#include "config.h"

#define SERIAL_BAUD_RATE  115200

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  setupWebServer(); // WiFi + HTTP server

  initTime();

  uint8_t primaryChan;
  wifi_second_chan_t secondChan;

  esp_wifi_get_channel(&primaryChan, &secondChan);
  Serial.printf("Connected WiFi channel: %d\n", primaryChan);

  // Start ESP-NOW after Wi-Fi is fully up and we know the channel, so it's more stable
  initEspNow(primaryChan);

  #if TEST_MODE
  uint8_t fake_mac[6] = {0xAA, 0xBB, 0xCC, 0x00, 0x00, 0x01};
  node_registry.registerNode(fake_mac);
  node_registry.addTestDataForAllNodes();
  #endif
}

void loop() {
  handleWebServer();
}

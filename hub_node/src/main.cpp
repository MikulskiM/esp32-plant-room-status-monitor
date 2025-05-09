#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_timer.h>
#include <ESPmDNS.h>
#include <time.h>
#include <lwip/sockets.h>

#include "esp_wifi.h"
#include "secrets.h"  // put your wifi name nad password

#define SERIAL_BAUD_RATE  115200

// MAC address of sensor_node (5C:01:3B:73:7C:80)
#define SENSOR_MAC    {0x5C, 0x01, 0x3B, 0x73, 0x7C, 0x80}

typedef struct {
  float humidity;
  float temperature;
  float pressure;
  int soil_moisture_mapped;
} SensorData;

SensorData latest_sensor_data;
bool has_sensor_data = false;

WebServer server(80);  // HTTP server on port 80

void initTime() {
  configTime(3600, 0, "pool.ntp.org", "time.nist.gov"); // 3600 = GMT+1
  Serial.print("Getting real time");
  while (time(nullptr) < 100000) {  // wait for valid time
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTime ready");
}

String getCurrentTimestamp() {
  // [09.05.2025 - 10:58]
  char timestamp[21];

  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  strftime(timestamp, sizeof(timestamp), "%d.%m.%Y - %H:%M", &timeinfo);

  return String(timestamp);
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

void handleRoot() {
  if (!has_sensor_data) {
    server.send(200, "text/plain", "No sensor data received");
    Serial.println("has_sensor_data: false");
    return;
  }

  Serial.println("has_sensor_data: true");

  String html = "<html><head><title>Plant Status</title></head><body>";
  html += "<meta charset='UTF-8'>";
  html += "<h1>Latest Sensor Data</h1>";
  html += "<ul>";
  html += "<li><b>Temperature:</b> " + String(latest_sensor_data.temperature, 2) + " °C</li>";
  html += "<li><b>Humidity:</b> " + String(latest_sensor_data.humidity, 2) + " %</li>";
  html += "<li><b>Pressure:</b> " + String(latest_sensor_data.pressure, 2) + " hPa</li>";
  html += "<li><b>Soil Moisture:</b> " + String(latest_sensor_data.soil_moisture_mapped) + " %</li>";
  html += "<li><b>Received:</b> " + getCurrentTimestamp() + "</li>";
  html += "</ul></body></html>";

  server.send(200, "text/html", html);
  Serial.println("Sent HTML response to client");
}

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

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  // check MAC address
  Serial.print("Hub node MAC Address: ");
  Serial.println(WiFi.macAddress());

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  initTime();

  uint8_t primaryChan;
  wifi_second_chan_t secondChan;

  esp_wifi_get_channel(&primaryChan, &secondChan);
  Serial.printf("Connected WiFi channel: %d\n", primaryChan);

  // Start ESP-NOW after Wi-Fi is fully up and we know the channel, so it's more stable
  initEspNow(primaryChan);

  // HTTP setup
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

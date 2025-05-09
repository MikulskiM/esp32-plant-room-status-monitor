#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <esp_timer.h>
#include <esp_now.h>
#include <WiFi.h>

#include "esp_wifi.h"

#define SERIAL_BAUD_RATE  115200
#define WIFI_CHANNEL      11
#define BROADCAST_MAC     {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
#define HUB_MAC           {0x5C, 0x01, 0x3B, 0x72, 0x6E, 0x34}

#define SOIL_MOISTURE_PIN     34
#define SOIL_MOISTURE_DRY_ADC 2600  // 100% dryness on my desk
#define SOIL_MOISTURE_WET_ADC 900   // 100% wet? probably will never reach that
                                    // 950-1000 is correctly watered

#define SENSOR_READ_INTERVAL_S    2 // seconds between sensor measurements
#define ESP_NOW_SEND_INTERVAL_MIN 1 // minutes between esp-now sends data to the hub node

#define BMP280_I2C_ADDR   0x77

#define MAX_SEND_RETRIES 5
#define RETRY_DELAY_MS   100

typedef struct {
  float humidity;
  float temperature;
  float pressure;
  int soil_moisture_mapped;
} SensorData;

Adafruit_AHTX0 aht_sensor;
Adafruit_BMP280 bmp_sensor;

esp_timer_handle_t sensor_timer;
esp_timer_handle_t esp_now_timer;

volatile bool lastSendSuccess = false;

void readSensors(void* arg) {
  sensors_event_t humidity;
  sensors_event_t temperature;

  aht_sensor.getEvent(&humidity, &temperature);
  float pressure = bmp_sensor.readPressure() / 100.0F;

  int soil_moisture_ADC = analogRead(SOIL_MOISTURE_PIN);
  int soil_moisture_mapped = map(soil_moisture_ADC, SOIL_MOISTURE_DRY_ADC, SOIL_MOISTURE_WET_ADC, 0, 100);

  Serial.printf("[AHT20+BMP280] Temp: %.2f°C Hum: %.2f%% | Pressure: %.2f hPa | ",
    temperature.temperature, humidity.relative_humidity, pressure);
  Serial.printf("Soil moisture: %d%%\n", soil_moisture_mapped);

  SensorData* data = (SensorData*)arg;
  data->humidity = humidity.relative_humidity;
  data->pressure = pressure;
  data->temperature = temperature.temperature;
  data->soil_moisture_mapped = soil_moisture_mapped;
}

void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.print("ESP-NOW send status: ");
    if (status == ESP_NOW_SEND_SUCCESS) {
      Serial.print("SUCCESS\n");
      lastSendSuccess = true;
    }
    else {
      Serial.print("FAIL\n");
      lastSendSuccess = false;
    }
}

void initEspNow() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  uint8_t ch; wifi_second_chan_t sc;
  esp_wifi_get_channel(&ch, &sc);
  Serial.printf("Sensor node running on channel: %d\n", ch);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW esp_now_init() FAIL");
    return;
  }

  esp_now_peer_info_t hub_peer = {
    .peer_addr = HUB_MAC,
    .channel = 0,
    .encrypt = false
  };

  if (!esp_now_is_peer_exist(hub_peer.peer_addr)) {
    if (esp_now_add_peer(&hub_peer) != ESP_OK) {
      Serial.println("ESP-NOW esp_now_add_peer() FAIL");
    } else {
      Serial.printf("Added peer MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
        hub_peer.peer_addr[0], hub_peer.peer_addr[1], hub_peer.peer_addr[2],
        hub_peer.peer_addr[3], hub_peer.peer_addr[4], hub_peer.peer_addr[5]);
    }
  }

  esp_now_register_send_cb(onDataSent);
  Serial.println("ESP-NOW transmitter ready");
}

void sendSensorData(void* arg) {
  SensorData* data = (SensorData*)arg;
  uint8_t hub_mac[] = HUB_MAC;

  Serial.println("ESP-NOW Sending sensor data:");
  Serial.printf("\tTemp:\t%.2f°C\n", data->temperature);
  Serial.printf("\tHum:\t%.2f%%\n", data->humidity);
  Serial.printf("\tPress:\t%.2f hPa\n", data->pressure);
  Serial.printf("\tSoil:\t%d%%\n", data->soil_moisture_mapped);

  for (int attempt = 1; attempt <= MAX_SEND_RETRIES; ++attempt) {
    Serial.printf("ESP-NOW Sending sensor data (attempt %d)...\n", attempt);
    lastSendSuccess = false;  // reset before sending

    esp_err_t result = esp_now_send(hub_mac, (uint8_t*)data, sizeof(SensorData));
    if (result == ESP_OK) {
      Serial.println("\tesp_now_send() queued.");
    } else {
      Serial.printf("\tesp_now_send() QUEUE FAIL! Error: %d\n", result);
      delay(RETRY_DELAY_MS);
      continue;  // try again
    }

    delay(50);  // wait for status callback

    if (lastSendSuccess) {
      Serial.println("Send confirmed by callback.");
      break;
    } else {
      Serial.println("Send failed, retrying...");
      delay(RETRY_DELAY_MS);
    }
  }
}


void setup() {
  SensorData* sensor_data = new SensorData();

  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  // check MAC address
  Serial.print("Sensor node MAC Address: ");
  Serial.println(WiFi.macAddress());

  initEspNow();

  Serial.println("Starting sensor node...");

  if(!aht_sensor.begin()) {
    Serial.println("ERROR: couldn't start the AHT20 sensor");
  }

  if(!bmp_sensor.begin(0x77)) {
    Serial.println("ERROR: couldn't start the BMP280 sensor");
  }

  esp_timer_create_args_t sensor_timer_args = {
    .callback = &readSensors,
    .arg = sensor_data,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "sensor_timer"
  };

  esp_timer_create(&sensor_timer_args, &sensor_timer);
  esp_timer_start_periodic(sensor_timer, SENSOR_READ_INTERVAL_S * 1000 * 1000);
  Serial.println("Sensor timer started...");

  esp_timer_create_args_t esp_now_timer_args = {
    .callback = &sendSensorData,
    .arg = sensor_data,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "esp_now_timer"
  };

  esp_timer_create(&esp_now_timer_args, &esp_now_timer);
  esp_timer_start_periodic(esp_now_timer, 60 * ESP_NOW_SEND_INTERVAL_MIN * 1000 * 1000);
  Serial.println("ESP-NOW timer started...");
}

void loop() {
  // put your main code here, to run repeatedly:
}

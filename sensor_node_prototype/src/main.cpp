#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <esp_timer.h>
#include <esp_now.h>
#include <WiFi.h>

#define SOIL_MOISTURE_PIN 34

#define SECONDS_DELAY 2 // seconds between sensor measurements
#define MINUTES_DELAY 1 // minutes between esp-now sends data to the main node

#define DRY 2600  // 100% dryness on my desk
#define WET 900   // 100% wet? probably will never reach that
                  // 950-1000 is correctly watered

#define MASTER_MAC {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF} // main node mac

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

void readSensors(void* arg) {
  sensors_event_t humidity;
  sensors_event_t temperature;

  aht_sensor.getEvent(&humidity, &temperature);
  float pressure = bmp_sensor.readPressure() / 100.0F;

  int soil_moisture_ADC = analogRead(SOIL_MOISTURE_PIN);
  int soil_moisture_mapped = map(soil_moisture_ADC, DRY, WET, 0, 100);

  Serial.printf("[AHT20+BMP280] Temp: %.2f°C Hum: %.2f%% | Pressure: %.2f hPa | ",
    temperature.temperature, humidity.relative_humidity, pressure);
  Serial.printf("Mapped soil moisture data: %d\%\n", soil_moisture_mapped);

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
    }
    else {
      Serial.print("FAIL\n");
    }
}

void initEspNow() {
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW esp_now_init() FAIL");
    return;
  }

  esp_now_peer_info_t peerInfo = {
    .peer_addr = MASTER_MAC,
    .channel = 0,
    .encrypt = false
  };

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ESP-NOW esp_now_add_peer() FAIL");
    return;
  }

  esp_now_register_send_cb(onDataSent);
}

void sendSensorData(void* arg) {
  SensorData* data = (SensorData*)arg;
  uint8_t MASTER[] = MASTER_MAC;

  Serial.println("ESP-NOW Sending sensor data:");
  Serial.printf("\tTemp:\t%.2f°C\n", data->temperature);
  Serial.printf("\tHum:\t%.2f%%\n", data->humidity);
  Serial.printf("\tPress:\t%.2f hPa\n", data->pressure);
  Serial.printf("\tSoil:\t%d%%\n", data->soil_moisture_mapped);

  esp_err_t result = esp_now_send(MASTER, (uint8_t*)data, sizeof(SensorData));

  if (result == ESP_OK) {
    Serial.println("\tesp_now_send() queued successfully.");
  } else {
    Serial.printf("\tesp_now_send() FAILED! Error code: %d\n", result);
  }
}

void setup() {
  SensorData* sensor_data = new SensorData();

  Serial.begin(115200);
  delay(1000);

  initEspNow();

  Serial.println("Starting...");

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
  esp_timer_start_periodic(sensor_timer, SECONDS_DELAY * 1000 * 1000);
  Serial.println("Sensor timer started...");

  esp_timer_create_args_t esp_now_timer_args = {
    .callback = &sendSensorData,
    .arg = sensor_data,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "esp_now_timer"
  };

  esp_timer_create(&esp_now_timer_args, &esp_now_timer);
  esp_timer_start_periodic(esp_now_timer, 60 * MINUTES_DELAY * 1000 * 1000);
  Serial.println("ESP-NOW timer started...");
}

void loop() {
  // put your main code here, to run repeatedly:
}

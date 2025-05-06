#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <esp_timer.h>

Adafruit_AHTX0 aht_sensor;
Adafruit_BMP280 bmp_sensor;

esp_timer_handle_t sensor_timer;

void readSensors(void* arg) {
  sensors_event_t humidity;
  sensors_event_t temperature;

  aht_sensor.getEvent(&humidity, &temperature);
  float pressure = bmp_sensor.readPressure() / 100.0F;

  Serial.printf("[AHT20+BMP280] Temp: %.2f°C Hum: %.2f%% | Pressure: %.2f hPa\n",
    temperature.temperature, humidity.relative_humidity, pressure);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting...");

  if(!aht_sensor.begin()) {
    Serial.println("ERROR: couldn't start the AHT20 sensor");
  }

  if(!bmp_sensor.begin(0x77)) {
    Serial.println("ERROR: couldn't start the BMP280 sensor");
  }

  esp_timer_create_args_t timer_args = {
    .callback = &readSensors,
    .arg = nullptr,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "sensor_timer"
  };

  esp_timer_create(&timer_args, &sensor_timer);
  esp_timer_start_periodic(sensor_timer, 2 * 1000 * 1000);  // 2 seconds

  Serial.println("Timer started...");
}

void loop() {
  // put your main code here, to run repeatedly:
}

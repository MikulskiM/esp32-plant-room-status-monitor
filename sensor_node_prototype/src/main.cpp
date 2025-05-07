#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <esp_timer.h>

#define SOIL_MOISTURE_PIN 34

#define DRY 2600  // 100% dryness on my desk
#define WET 900   // 100% wet? probably will never reach that
                  // 950-1000 is correctly watered

Adafruit_AHTX0 aht_sensor;
Adafruit_BMP280 bmp_sensor;

esp_timer_handle_t sensor_timer;

void readSensors(void* arg) {
  sensors_event_t humidity;
  sensors_event_t temperature;

  aht_sensor.getEvent(&humidity, &temperature);
  float pressure = bmp_sensor.readPressure() / 100.0F;

  int soil_moisture_ADC = analogRead(SOIL_MOISTURE_PIN);
  int soil_moisture_mapped = map(soil_moisture_ADC, DRY, WET, 0, 100);

  Serial.printf("[AHT20+BMP280] Temp: %.2f°C Hum: %.2f%% | Pressure: %.2f hPa | ",
    temperature.temperature, humidity.relative_humidity, pressure);
  Serial.printf("Mapped soil moisture data: %d\% | ", soil_moisture_mapped);
  Serial.printf("Raw soil moisture data: %d\n", soil_moisture_ADC);
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

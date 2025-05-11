#include "my_server.h"

WebServer server(80);  // HTTP server on port 80

void setupWebServer() {
  // Wi-Fi setup
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

  // HTTP server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
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

void handleWebServer() {
  server.handleClient();
}

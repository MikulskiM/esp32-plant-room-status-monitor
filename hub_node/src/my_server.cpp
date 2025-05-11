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
  SensorData dataList[BUFFER_SIZE];
  int count = ring_buffer.getAll(dataList);

  String html = "<html><head><meta charset='UTF-8'><title>Plant Status</title></head><body>";
  html += "<h1>Recent Sensor Data (" + String(count) + " entries)</h1>";

  if (count == 0) {
    html += "<p><i>No sensor data available yet.</i></p>";
  } else {
    html += "<ul>";
    for (int i = 0; i < count; i++) {
      html += "<li><b>[" + formatTimeStamp(dataList[i].timestamp) + "]</b> ";
      html += "Temp: " + String(dataList[i].temperature, 2) + " °C, ";
      html += "Hum: " + String(dataList[i].humidity, 2) + " %, ";
      html += "Pres: " + String(dataList[i].pressure, 2) + " hPa, ";
      html += "Soil: " + String(dataList[i].soil_moisture_mapped) + " %</li>";
    }
    html += "</ul>";
  }

  html += "</body></html>";

  server.send(200, "text/html", html);
  Serial.println("Sent HTML response to client");
}

void handleWebServer() {
  server.handleClient();
}

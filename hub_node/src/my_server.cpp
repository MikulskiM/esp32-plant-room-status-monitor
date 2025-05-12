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

  String labels = "[";
  String tempData = "[";
  String humData = "[";
  String pressData = "[";
  String soilData = "[";

  for (int i = 0; i < count; i++) {
    String label = "\"" + formatTimeStamp(dataList[i].timestamp) + "\"";
    labels += label + (i < count - 1 ? "," : "");
    tempData += String(dataList[i].temperature, 2) + (i < count - 1 ? "," : "");
    humData += String(dataList[i].humidity, 2) + (i < count - 1 ? "," : "");
    pressData += String(dataList[i].pressure, 2) + (i < count - 1 ? "," : "");
    soilData += String(dataList[i].soil_moisture_mapped) + (i < count - 1 ? "," : "");
  }

  labels += "]";
  tempData += "]";
  humData += "]";
  pressData += "]";
  soilData += "]";

  String html = "<html><head><meta charset='UTF-8'><title>Plant Status</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script></head><body>";
  html += "<h1>Sensor Data Charts</h1>";

  html += "<canvas id='tempChart'></canvas>";
  html += "<canvas id='humChart'></canvas>";
  html += "<canvas id='pressChart'></canvas>";
  html += "<canvas id='soilChart'></canvas>";

  html += "<script>";
  html += "const labels = " + labels + ";";
  html += "const tempData = {label: 'Temperature (°C)', data: " + tempData + ", borderColor: 'red', fill: false};";
  html += "const humData = {label: 'Humidity (%)', data: " + humData + ", borderColor: 'blue', fill: false};";
  html += "const pressData = {label: 'Pressure (hPa)', data: " + pressData + ", borderColor: 'orange', fill: false};";
  html += "const soilData = {label: 'Soil Moisture (%)', data: " + soilData + ", borderColor: 'green', fill: false};";

  html += R"rawliteral(
  function createChart(ctxId, data) {
    new Chart(document.getElementById(ctxId), {
      type: 'line',
      data: {
        labels: labels,
        datasets: [data]
      },
      options: {
        responsive: true,
        scales: {
          x: { display: true, title: { display: true, text: 'Time' }},
          y: { display: true, beginAtZero: true }
        }
      }
    });
  }
  createChart('tempChart', tempData);
  createChart('humChart', humData);
  createChart('soilChart', soilData);
  createChart('soilChart', soilData);
  </script></body></html>)rawliteral";

  server.send(200, "text/html", html);
  Serial.println("Sent HTML response to client");
}

void handleWebServer() {
  server.handleClient();
}

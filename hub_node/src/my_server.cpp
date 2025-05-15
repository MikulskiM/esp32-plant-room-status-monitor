#include "my_server.h"

WebServer server(80);  // HTTP server on port 80
extern NodeRegistry node_registry;

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
  String labels = "[";
  String tempLines, humLines, pressLines, soilCharts;

  int nodeCount = node_registry.getNodeCount();
  for (int i = 0; i < nodeCount; i++) {
    NodeEntry* node = node_registry.getNodeByIndex(i);
    SensorData dataList[BUFFER_SIZE];
    int count = node->ring_buffer.getAll(dataList);
    std::sort(dataList, dataList + count, [](const SensorData& a, const SensorData& b) {
      return a.timestamp < b.timestamp;
    });

    String tempData = "[";
    String humData = "[";
    String pressData = "[";
    String soilLines[MAX_SOIL_SENSORS] = {"[", "[", "[", "[", "["};

    for (int j = 0; j < count; j++) {
      if (i == 0) {
        String label = "\"" + formatTimeStamp(dataList[j].timestamp) + "\"";
        labels += label + (j < count - 1 ? "," : "");
      }

      tempData += String(dataList[j].temperature, 2) + (j < count - 1 ? "," : "");
      humData += String(dataList[j].humidity, 2) + (j < count - 1 ? "," : "");
      pressData += String(dataList[j].pressure, 2) + (j < count - 1 ? "," : "");

      for (int k = 0; k < MAX_SOIL_SENSORS; k++) {
        int val = dataList[j].soil_moisture_mapped[k];
        soilLines[k] += (val != SOIL_MOISTURE_NOT_PRESENT) ? String(val) : "null";
        if (j < count - 1) soilLines[k] += ",";
      }
    }

    tempData += "]";
    humData += "]";
    pressData += "]";
    for (int k = 0; k < MAX_SOIL_SENSORS; k++) soilLines[k] += "]";

    tempLines += "{label: '" + node->name + "', data: " + tempData + ", borderColor: getColor(" + String(i) + "), fill: false},";
    humLines += "{label: '" + node->name + "', data: " + humData + ", borderColor: getColor(" + String(i) + "), fill: false},";
    pressLines += "{label: '" + node->name + "', data: " + pressData + ", borderColor: getColor(" + String(i) + "), fill: false},";

    // Soil chart per node
    soilCharts += "<canvas id='soilChart" + String(i) + "'></canvas>";
    soilCharts += "<script>\nconst soilDatasets" + String(i) + " = [";
    for (int k = 0; k < MAX_SOIL_SENSORS; k++) {
      if (soilLines[k].indexOf("null") != soilLines[k].length() - 1 || soilLines[k].indexOf("null") == -1) {
        soilCharts += "{label: '" + node->name + " – Soil " + String(k+1) + "', data: " + soilLines[k] + ", borderColor: getColor(" + String(k) + "), fill: false},";
      }
    }
    soilCharts += "];\ncreateChart('soilChart" + String(i) + "', soilDatasets" + String(i) + ");\n</script>";
  }
  labels += "]";

  String html = "<html><head><meta charset='UTF-8'><title>Plant Status</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script></head><body>";
  html += "<h1>Sensor Data Charts</h1>";
  html += "<canvas id='tempChart'></canvas>";
  html += "<canvas id='humChart'></canvas>";
  html += "<canvas id='pressChart'></canvas>";
  html += soilCharts;

  html += "<script>\n";
  html += "function getColor(index) { const colors = ['red','blue','green','orange','purple','brown','black','pink','gray','teal']; return colors[index % colors.length]; }\n";
  html += "function createChart(id, datasets) {\nnew Chart(document.getElementById(id), { type: 'line', data: { labels: " + labels + ", datasets: datasets }, options: { responsive: true, scales: { x: { display: true, title: { display: true, text: 'Time' } }, y: { beginAtZero: true } } } }); }\n";
  html += "createChart('tempChart', [" + tempLines + "]);\n";
  html += "createChart('humChart', [" + humLines + "]);\n";
  html += "createChart('pressChart', [" + pressLines + "]);\n";
  html += "</script></body></html>";

  server.send(200, "text/html", html);
  Serial.println("Sent HTML response to client");
}

void handleWebServer() {
  server.handleClient();
}

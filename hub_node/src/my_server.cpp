#include "my_server.h"
#include <vector>
#include <algorithm>

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

String getColor(int index) {
  const char* colors[] = {
    "#e6194b", "#3cb44b", "#4363d8", "#f58231", "#911eb4",
    "#46f0f0", "#f032e6", "#bcf60c", "#fabebe", "#008080"
  };
  int colorCount = sizeof(colors) / sizeof(colors[0]);
  return String(colors[index % colorCount]);
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <title>Plant Room Status Monitor</title>
      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
      <style>
        body { font-family: Arial; margin: 40px; }
        canvas { margin-bottom: 60px; }
      </style>
    </head>
    <body>
      <h1>Plant Room Status Monitor</h1>
      <div id="charts"></div>

      <script>
  )rawliteral";

  for (int i = 0; i < node_registry.getNodeCount(); i++) {
    NodeEntry* node = node_registry.getNodeByIndex(i);
    if (!node) continue;

    SensorData data[BUFFER_SIZE];
    int size = node->ring_buffer.getAll(data);

    html += "const node_" + String(i) + " = {\n";
    html += "  label: '" + node->name + "',\n";
    html += "  temperature: [],\n  humidity: [],\n  pressure: [],\n";

    for (int j = 0; j < MAX_SOIL_SENSORS; j++) {
      html += "  soil_" + String(j) + ": [],\n";
    }

    html += "  timestamps: []\n};\n";

    for (int k = 0; k < size; k++) {
      time_t ts = data[k].timestamp;
      html += "node_" + String(i) + ".timestamps.push('" + formatTimeStamp(ts) + "');\n";
      html += "node_" + String(i) + ".temperature.push(" + String(data[k].temperature, 2) + ");\n";
      html += "node_" + String(i) + ".humidity.push(" + String((int)round(data[k].humidity)) + ");\n";
      html += "node_" + String(i) + ".pressure.push(" + String(data[k].pressure, 2) + ");\n";

      for (int j = 0; j < MAX_SOIL_SENSORS; j++) {
        int moisture = data[k].soil_moisture_mapped[j];
        if (moisture != SOIL_MOISTURE_NOT_PRESENT) {
          html += "node_" + String(i) + ".soil_" + String(j) + ".push(" + String(moisture) + ");\n";
        } else {
          html += "node_" + String(i) + ".soil_" + String(j) + ".push(null);\n";
        }
      }
    }
  }

  html += R"rawliteral(
    function drawChart(containerId, label, datasets, labels, unit, precision) {
      const canvas = document.createElement('canvas');
      document.getElementById('charts').appendChild(canvas);

      new Chart(canvas, {
        type: 'line',
        data: {
          labels: labels,
          datasets: datasets
        },
        options: {
          responsive: true,
          plugins: {
            title: {
              display: true,
              text: label
            },
            tooltip: {
              callbacks: {
                label: function(context) {
                  let value = context.raw;
                  if (value == null) return 'Brak danych';
                  return context.dataset.label + ': ' + value.toFixed(precision) + ' ' + unit;
                }
              }
            }
          },
          scales: {
            x: { display: true, title: { display: true, text: 'Time' } },
            y: {
              display: true,
              ticks: {
                callback: function(value) {
                  return value.toFixed(precision) + ' ' + unit;
                }
              }
            }
          }
        }
      });
    }

    const allNodes = [)rawliteral";

  for (int i = 0; i < node_registry.getNodeCount(); i++) {
    html += "node_" + String(i);
    if (i < node_registry.getNodeCount() - 1) html += ", ";
  }
  html += "];\n";

  html += R"rawliteral(
    drawChart("charts", "Temperature", allNodes.map(n => ({
      label: n.label,
      data: n.temperature,
      tension: 0.2
    })), allNodes[0].timestamps, "°C", 2);

    drawChart("charts", "Humidity", allNodes.map(n => ({
      label: n.label,
      data: n.humidity,
      tension: 0.2
    })), allNodes[0].timestamps, "%", 0);

    drawChart("charts", "Pressure", allNodes.map(n => ({
      label: n.label,
      data: n.pressure,
      tension: 0.2
    })), allNodes[0].timestamps, "hPa", 2);
  )rawliteral";

  for (int i = 0; i < node_registry.getNodeCount(); i++) {
    html += "drawChart('charts', 'Soil Moisture - ' + node_" + String(i) + ".label, [\n";
    for (int j = 0; j < MAX_SOIL_SENSORS; j++) {
      html += "{ label: 'Sensor " + String(j) + "', data: node_" + String(i) + ".soil_" + String(j) +
              ", tension: 0.2, fill: false, borderColor: '" + getColor(j) + "', backgroundColor: '" + getColor(j) + "' }";
      if (j < MAX_SOIL_SENSORS - 1) html += ",\n";
    }
    html += "], node_" + String(i) + ".timestamps, '%', 0);\n";
  }

  html += R"rawliteral(
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
  Serial.println("Sent HTML response to client");
}

void handleWebServer() {
  server.handleClient();
}

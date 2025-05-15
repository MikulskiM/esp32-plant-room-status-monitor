#include "node_registry.h"
#include "utils.h"

const char* default_names[] = {
    "Office Marek", "Living Room", "Bedroom", "Office Wifey"
};

NodeRegistry::NodeRegistry() {
    for (int i = 0; i < MAX_NODES; i++) {
        nodes[i] = nullptr;
    }
    node_count = 0;
}

NodeEntry* NodeRegistry::registerNode(const uint8_t* mac) {
    String mac_str = macToStr(mac);
    NodeEntry* existing_node = findNodeByMac(mac_str);

    if (existing_node) {
        Serial.printf("Found existing node: %s\n", existing_node->name);
        return existing_node;
    }

    if (node_count >= MAX_NODES) {
        Serial.printf("Reached max numbers of nodes - node count: %d\n", node_count);
        return nullptr;
    }

    NodeEntry* new_node = new NodeEntry();
    new_node->mac_str = mac_str;

    // set name
    int name_count = sizeof(default_names) / sizeof(default_names[0]);
    if (node_count < name_count) {
        new_node->name = default_names[node_count];
    }
    else {
        new_node->name = "Room " + String(node_count + 1);
    }

    nodes[node_count] = new_node;
    node_count++;
    Serial.println("Registered new node: " + new_node->name + " (" + new_node->mac_str + ")");

    return new_node;
}

NodeEntry* NodeRegistry::findNodeByMac(String mac_str) {
    for (int i = 0; i < node_count; i++) {
        if ((nodes[i] != nullptr) && (nodes[i]->mac_str == mac_str)) {
            return nodes[i];
        }
    }
    return nullptr;
}

int NodeRegistry::getNodeCount() {
    return node_count;
}

NodeEntry* NodeRegistry::getNodeByIndex(int index) {
    if (index < 0 || index >= node_count) {
        Serial.println("ERROR: getNodeByIndex(int index) INDEX OUT OF RANGE");
        return nullptr;
    }
    return nodes[index];
}

bool NodeRegistry::addSensorData(const uint8_t* mac, const SensorData& data) {
    NodeEntry* node = registerNode(mac);
    if (node == nullptr) {
        return false;
    }

    node->ring_buffer.addData(const_cast<SensorData&>(data));
    return true;
}

void NodeRegistry::addTestDataForAllNodes() {
  time_t now = time(nullptr);

  for (int n = 0; n < node_count; n++) {
    NodeEntry* node = nodes[n];
    if (!node) continue;

    for (int i = 0; i < 50; i++) {
      SensorData sample;
      sample.temperature = 20.0 + random(-50, 50) / 10.0; // 15.0 - 25.0 °C
      sample.humidity = 30.0 + random(-100, 100) / 10.0;  // 20.0 - 40.0 %
      sample.pressure = 984.0 + random(-50, 50) / 10.0;   // 979.0 - 989.0 hPa

      for (int j = 0; j < MAX_SOIL_SENSORS; j++) {
        if (j == 3) {
          sample.soil_moisture_mapped[j] = SOIL_MOISTURE_NOT_PRESENT;
        } else {
          sample.soil_moisture_mapped[j] = 75 + random(-15, 21);  // 60 - 95%
        }
      }

      sample.timestamp = now - (50 - i) * 60;  // 1 min apart
      node->ring_buffer.addData(sample);
    }

    Serial.println("Test data added for node: " + node->name);
  }
}

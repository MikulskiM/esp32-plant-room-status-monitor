#ifndef NODE_REGISTRY_H
#define NODE_REGISTRY_H

#include <Arduino.h>
#include "types.h"
#include "ring_buffer.h"

#define MAX_NODES 5

typedef struct {
  String mac_str;
  String name;
  RingBuffer ring_buffer;
} NodeEntry;

class NodeRegistry {
    NodeEntry* nodes[MAX_NODES];
    int node_count;

public:
    NodeRegistry();
    NodeEntry* registerNode(const uint8_t* mac);
    NodeEntry* findNodeByMac(String mac_str);
    int getNodeCount();
    NodeEntry* getNodeByIndex(int index);
    bool addSensorData(const uint8_t* mac, const SensorData& data);
    void addTestDataForAllNodes();
};

#endif
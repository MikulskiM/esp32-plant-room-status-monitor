#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "types.h"

#define BUFFER_SIZE 100

class RingBuffer {
public:
    RingBuffer();

    void addData(SensorData& newData);
    bool isFull();
    int getSize();
    int getAll(SensorData* outArray);
private:
    SensorData buffer[BUFFER_SIZE];
    int head;
    int count;
};

#endif
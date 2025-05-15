#include "ring_buffer.h"
#include <algorithm>

RingBuffer::RingBuffer() : head(0), count(0) {}

void RingBuffer::addData(SensorData& newData) {
    Serial.println("Saving data in ring buffer:");
    buffer[head] = newData;

    head++;
    head = head % BUFFER_SIZE;

    if (count < BUFFER_SIZE) {
        count++;
    }
    Serial.println("Saving successful\n");
}

bool RingBuffer::isFull() {
    if (count > BUFFER_SIZE) {
        Serial.println("ERROR: ring buffer counter > BUFFER SIZE !!!");
        return true;
    }

    return (count == BUFFER_SIZE);
}

int RingBuffer::getSize() {
    return count;
}

int RingBuffer::getAll(SensorData* outArray) {
  Serial.print("Getting all data from the ring buffer: ");
  int actualCount = count;

  if (count < BUFFER_SIZE) {
    for (int i = 0; i < count; i++) {
      outArray[i] = buffer[i];
    }
  } else {
    int out_counter = 0;
    for (int i = head; i < BUFFER_SIZE; i++) {
      outArray[out_counter++] = buffer[i];
    }
    for (int i = 0; i < head; i++) {
      outArray[out_counter++] = buffer[i];
    }
  }

  // Sort chronologicaly - timestamp
  std::sort(outArray, outArray + actualCount, [](const SensorData& a, const SensorData& b) {
    return a.timestamp < b.timestamp;
  });

  Serial.println("Successful");
  return actualCount;
}

#include "ring_buffer.h"

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

void RingBuffer::addTestData() {
  time_t now = time(nullptr);

  for (int i = 0; i < 50; i++) {
    SensorData sample;
    sample.temperature = 20.0 + random(-50, 50) / 10.0; // 15.0 - 25.0 °C
    sample.humidity = 30.0 + random(-100, 100) / 10.0;  // 20.0 - 40.0 %
    sample.pressure = 984.0 + random(-50, 50) / 10.0;   // 979.0 - 989.0 hPa
    sample.soil_moisture_mapped = 90 + random(0, 11);   // 90 - 100 %
    sample.timestamp = now - (50 - i) * 60;             // every 1 minute back

    addData(sample);
  }

  Serial.println("Added 50 test entries to the ring buffer.");
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
    if (count < BUFFER_SIZE) {
        // no wrapping
        for (int i = 0; i < count; i++) {
            outArray[i] = buffer[i];
        }
    }
    else {
        int out_counter = 0;
        // start copying from head
        for (int i = head; i < BUFFER_SIZE; i++) {
            outArray[out_counter] = buffer[i];
            out_counter++;
        }

        for (int i = 0; i < head; i++) {
            outArray[out_counter] = buffer[i];
            out_counter++;
        }
    }
    Serial.println("Successful");
    return count;
}

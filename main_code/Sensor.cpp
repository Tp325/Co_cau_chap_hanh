#include "Sensor.h"

Sensor::Sensor(SensorCfg *distance_sensor) {
  this->sensor_config = distance_sensor;
}

void Sensor::begin() {
  Serial2.begin(sensor_config->baud, SERIAL_8N1, S_RX, S_TX);
}

int Sensor::getSensorValue(uint8_t reTries) {
  node.begin(sensor_config->slaveID, Serial2);
  
  for (uint8_t t = 0; t < reTries; t++) {
    uint8_t result = node.readHoldingRegisters(sensor_config->regAddr, 1);
    
    if (result == node.ku8MBSuccess) {
      uint16_t value = node.getResponseBuffer(0);
      node.clearResponseBuffer();
      return value;
    } 
    // Delay
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
  return 0; // Trả về 0 nếu lỗi quá số lần thử
}

float Sensor::get_distance() {
  int raw = getSensorValue(2); // Thử 2 lần
  // Chia 10 để ra cm
  return (float)raw / 10.0; 
}
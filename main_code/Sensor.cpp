#include "Sensor.h"
#include <ModbusMaster.h>
Sensor::Sensor(SensorCfg *distance_sensor) {
  this->sensor_config = distance_sensor;
}
void Sensor::begin() {
  Serial2.begin(sensor_config->baud, SERIAL_8N1, S_RX, S_TX);
}
int Sensor::getSensorValue(uint8_t reTries) {
  node.begin(sensor_config->slaveID, Serial2);
  for (uint8_t t = 0; t < reTries; t++) {
    if (node.readHoldingRegisters(sensor_config->regAddr, 1) == node.ku8MBSuccess) {
      uint16_t value = node.getResponseBuffer(0);
      node.clearResponseBuffer();
      return value;
    } else {
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
  return 0;
}

float Sensor::get_distance() {
  return getSensorValue(3);
}
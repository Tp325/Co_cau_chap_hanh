#ifndef SENSOR_H
#define SENSOR_H
#include "config.h"
#include <Arduino.h>
#include <ModbusMaster.h>

class Sensor {
private:
  ModbusMaster node;
  SensorCfg *sensor_config;
  
  int getSensorValue(uint8_t reTries);
  
public:
  Sensor(SensorCfg *distance_sensor);
  void begin();
  
  // Trả về khoảng cách CM
  float get_distance();
};
#endif
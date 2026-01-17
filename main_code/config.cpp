#include "config.h"

// Cấu hình Modbus
SensorCfg sensors = { 0x50, 0x34, 115200 };

// Cấu hình LED
LED led = { 27, 26, 25, 0, 0, 0 };

// Cấu hình Motor
MOTOR motor = { 
  21, 22, 23, 18, 19, 
  0, 0, 0, 0, 0, 0    
};

SENSOR_STATE sensor = { 0, 0 }; // Khởi tạo biến sensor
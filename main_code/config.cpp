#include "config.h"

SensorCfg sensors = { 0x50, 0x34, 115200, "distance" };

LED led = { 27, 26, 25,
            0, 0, 0 };
BUZZEL buzzel = { 33,
                  0 };
MOTOR motor = { 21, 22, 23, 18, 19,
                0, 0, 0, 0, 0, 0, 0 };
SENSOR sensor = { 0, 0 };
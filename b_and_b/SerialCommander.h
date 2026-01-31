#ifndef SERIAL_COMMANDER_H
#define SERIAL_COMMANDER_H

#include <Arduino.h>
#include "PID.h" // Import PID để gọi hàm reset()

class SerialCommander {
private:
    float _minVal;
    float _maxVal;
    String _inputBuffer; 
    
    volatile float* _targetSetpoint; 
    
    PID* _pidPos;

public:
    SerialCommander(volatile float* setpointVar, PID* pidInstance, float minV, float maxV);

    void begin(unsigned long baudRate);
    void update();
    void printHelp();
};

#endif
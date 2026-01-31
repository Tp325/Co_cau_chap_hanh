#include "SerialCommander.h"

SerialCommander::SerialCommander(volatile float* setpointVar, PID* pidInstance, float minV, float maxV) {
    _targetSetpoint = setpointVar;
    _pidPos = pidInstance;
    _minVal = minV;
    _maxVal = maxV;
    _inputBuffer = "";
}

void SerialCommander::begin(unsigned long baudRate) {
    Serial.begin(baudRate);
    while (!Serial) { delay(10); }
    printHelp();
}

void SerialCommander::printHelp() {
    Serial.println(F("\n=========================================="));
    Serial.println(F("   BALL & BEAM COMMANDER - READY   "));
    Serial.println(F("=========================================="));
    Serial.printf("Current Setpoint: %.2f cm\n", *_targetSetpoint);
    Serial.printf("Valid Range:      [%.1f - %.1f] cm\n", _minVal, _maxVal);
    Serial.println(F(">>> Type a number and press ENTER to update."));
    Serial.println(F("==========================================\n"));
}

void SerialCommander::update() {

    while (Serial.available() > 0) {
        char inChar = (char)Serial.read();

        if (inChar == '\n' || inChar == '\r') {
            if (_inputBuffer.length() > 0) {
                float newVal = _inputBuffer.toFloat();

                if (newVal >= _minVal && newVal <= _maxVal) {
                    
                    // --- UPDATE SUCCESFUL ---
                    *_targetSetpoint = newVal; 
                    
                    // Reset PID
                    if (_pidPos != nullptr) {
                        _pidPos->reset();
                    }

                    Serial.print("\033[32m[OK] New Setpoint Updated: ");
                    Serial.print(*_targetSetpoint);
                    Serial.println(" cm\033[0m");
                } 
                else {
                    // --- error ---
                    Serial.print("\033[31m[ERROR] Out of Range! Keep: ");
                    Serial.print(*_targetSetpoint);
                    Serial.printf(" (Must be %.1f - %.1f)\033[0m\n", _minVal, _maxVal);
                }
                _inputBuffer = "";
            }
        } 
        else {
            _inputBuffer += inChar;
        }
    }
}
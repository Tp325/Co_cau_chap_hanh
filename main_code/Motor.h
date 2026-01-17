#ifndef MOTOR_H
#define MOTOR_H
#include "config.h"
#include <ESP32Encoder.h>
#include <Arduino.h>

class Motor {
private:
  unsigned long time_counter;
  long xung_truoc;
  MOTOR *motor;
  ESP32Encoder encoder;
  int pwm_pin;
  
public:
  Motor(MOTOR *motor);
  void begin(uint16_t PWM_frequency, uint8_t rotation_direction);
  
  void motor_enalble();     // Bật Enable
  void soft_power_on();     // Khởi động mềm
  void process();           // PWM
  
  // State
  void switch_rotation(int rotation);      // 1: Thuận, 0: Nghịch
  void switch_duty_cycle(int duty_cycle);  // 0 - 1023
  
  // RPM
  int get_speed(); 
};

#endif
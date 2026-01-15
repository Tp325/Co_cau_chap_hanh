#ifndef MOTOR_H
#define MOTOR_H
#include "config.h"
#include <ESP32Encoder.h>
#include <Arduino.h>

class Motor {  /// độ phân giải 10 bit
private:
  unsigned long time_counter;
  long xung_truoc;
  MOTOR *motor;
  ESP32Encoder encoder;
  int pwm_pin;
public:
  Motor(MOTOR *motor);
  void begin(uint16_t PWM_frequency, uint8_t rotation_direction);
  int get_speed();
  void power_off_motor();
  void soft_power_off();
  void motor_enalble();
  void soft_power_on();
  void switch_rotation(int rotation);
  void switch_duty_cycle(int duty_cycle);
  void switch_frequency(int frequency);
  void process();
};

#endif
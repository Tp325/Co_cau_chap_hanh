#include "Motor.h"
Motor::Motor(MOTOR *motor) {
  this->motor = motor;
}
void Motor::begin(uint16_t PWM_frequency, uint8_t rotation_direction) {
  pinMode(motor->enable_motor_pin, OUTPUT);
  pinMode(motor->PWM_1_pin, OUTPUT);
  pinMode(motor->PWM_2_pin, OUTPUT);
  motor->rotation_direction = 1;
  this->pwm_pin = motor->rotation_direction == 1 ? motor->PWM_2_pin : motor->PWM_1_pin;
  if (pwm_pin == motor->PWM_2_pin)
    digitalWrite(motor->PWM_1_pin, 0);
  else
    digitalWrite(motor->PWM_2_pin, 0);
  motor->frequency = PWM_frequency;
  motor->rotation_direction = rotation_direction;
  motor->time_cycle = 1.0 / (PWM_frequency * 1.0);
  encoder.attachHalfQuad(motor->encoder_a_pin, motor->encoder_b_pin);
  encoder.setCount(0);
  ledcAttach(pwm_pin, motor->frequency, 10);
}

int Motor::get_speed() {
  if (millis() - time_counter >= 100) {
    long delta_xung = encoder.getCount() - xung_truoc;
    xung_truoc = encoder.getCount();
    time_counter = millis();
    return ((delta_xung / 1152.0) * (1000.0 / (millis() - time_counter)) * 60.0);
  } else return motor->speed;
}

void Motor::power_off_motor() {
  motor->is_ready = 0;
  motor->is_running = 0;
  motor->last_motor_state = 0;
  motor->duty_cycle = 0;
  ledcWrite(pwm_pin, motor->duty_cycle);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  digitalWrite(motor->enable_motor_pin, LOW);
}
void Motor::motor_enalble() {
  motor->is_ready = 1;
  digitalWrite(motor->enable_motor_pin, HIGH);
}
void Motor::soft_power_off() {
  motor->is_ready = 1;
  motor->is_running = 0;
  motor->last_motor_state = 0;
  for (int i = 1023; i >= 0; i--) {
    ledcWrite(pwm_pin, i);
    motor->duty_cycle = i;
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
void Motor::soft_power_on() {
  motor->is_ready = 1;
  motor->is_running = 1;
  motor->last_motor_state = 1;
  for (int i = 0; i <= 1023; i++) {
    ledcWrite(pwm_pin, i);
    motor->duty_cycle = i;
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
void Motor::switch_rotation(int rotation) {
  ledcWrite(pwm_pin, 0);
  motor->rotation_direction = rotation;
  this->pwm_pin = motor->rotation_direction == 1 ? motor->PWM_2_pin : motor->PWM_1_pin;
  if (pwm_pin == motor->PWM_2_pin)
    digitalWrite(motor->PWM_1_pin, 0);
  else
    digitalWrite(motor->PWM_2_pin, 0);
  ledcAttach(pwm_pin, motor->frequency, 10);
}
void Motor::switch_duty_cycle(int duty_cycle) {
  motor->duty_cycle = duty_cycle;
}
void Motor::switch_frequency(int frequency) {
  motor->frequency = frequency;
  ledcAttach(pwm_pin, motor->frequency, 10);
}
void Motor::process() {
  if (motor->is_ready == 1) {
    if (motor->duty_cycle > 10)
      motor->is_running = 1;
    ledcWrite(pwm_pin, motor->duty_cycle);
  } else {
    power_off_motor();
  }
}

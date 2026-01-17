#include "Motor.h"

Motor::Motor(MOTOR *motor) {
  this->motor = motor;
  this->xung_truoc = 0;
  this->time_counter = 0;
}

void Motor::begin(uint16_t PWM_frequency, uint8_t rotation_direction) {
  pinMode(motor->enable_motor_pin, OUTPUT);
  pinMode(motor->PWM_1_pin, OUTPUT);
  pinMode(motor->PWM_2_pin, OUTPUT);
  
  // Cài đặt hướng mặc định
  motor->rotation_direction = rotation_direction;
  
  // Logic PWM
  // rotation = 1 -> PWM_2 (Thuận)
  // rotation = 0 -> PWM_1 (Nghịch)
  if (motor->rotation_direction == 1) {
      this->pwm_pin = motor->PWM_2_pin;
      digitalWrite(motor->PWM_1_pin, 0);
  } else {
      this->pwm_pin = motor->PWM_1_pin;
      digitalWrite(motor->PWM_2_pin, 0);
  }

  motor->frequency = PWM_frequency;
  
  // Setup Encoder
  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encoder.attachHalfQuad(motor->encoder_a_pin, motor->encoder_b_pin);
  encoder.setCount(0);
  
  // Setup PWM:ĐỘ PHÂN GIẢI 10 BIT
  ledcAttach(this->pwm_pin, motor->frequency, 10);
}

void Motor::motor_enalble() {
  motor->is_ready = 1;
  digitalWrite(motor->enable_motor_pin, HIGH);
}

void Motor::switch_rotation(int rotation) {
  // Nếu đổi chiều thì reset chân cũ về 0 và attach chân mới
  if (motor->rotation_direction != rotation) {
      ledcWrite(this->pwm_pin, 0); // Tắt chân hiện tại
      ledcDetach(this->pwm_pin);   // Gỡ PWM khỏi chân cũ
      
      motor->rotation_direction = rotation;
      
      if (motor->rotation_direction == 1) {
          this->pwm_pin = motor->PWM_2_pin;
          digitalWrite(motor->PWM_1_pin, 0);
      } else {
          this->pwm_pin = motor->PWM_1_pin;
          digitalWrite(motor->PWM_2_pin, 0);
      }
      // Attach PWM vào chân mới
      ledcAttach(this->pwm_pin, motor->frequency, 10);
  }
}

void Motor::switch_duty_cycle(int duty_cycle) {
  motor->duty_cycle = duty_cycle;
}

void Motor::process() {
  if (motor->is_ready == 1) {
    ledcWrite(this->pwm_pin, motor->duty_cycle);
  } else {
    ledcWrite(this->pwm_pin, 0);
  }
}

void Motor::soft_power_on() {
    motor->is_ready = 1;
    // Hàm này chỉ để bật cờ sẵn sàng, 
    // logic tăng tốc từ từ nên để PID lo
}

int Motor::get_speed() {
  // Tính RPM: (Xung / 2304) * (60000ms / delta_time)
  // thông số từ NSX: 1152 * 2  = 2304
  if (millis() - time_counter >= 20) { // Cập nhật mỗi 20ms     ***************** chỉnh cái này với ở apptasks.cpp *****************
    long current_count = encoder.getCount();
    long delta_xung = current_count - xung_truoc;
    xung_truoc = current_count;
    
    long dt = millis() - time_counter;
    time_counter = millis();
    
    // Tính RPM
    double rpm = ((double)delta_xung / 2304.0) * (60000.0 / dt);
    return (int)rpm;
  } 
  return motor->speed; // Trả về giá trị cũ nếu chưa đủ thời gian
}
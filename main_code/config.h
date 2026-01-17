#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

// --- PIN MAPPING ---
#define S_RX 16
#define S_TX 17

// --- CẤU TRÚC DỮ LIỆU ---
struct LED {
  uint8_t led_red_pin;
  uint8_t led_yellow_pin;
  uint8_t led_blue_pin;
  bool led_red_state = 0;
  bool led_yellow_state = 0;
  bool led_blue_state = 0;
};

struct MOTOR {
  uint8_t enable_motor_pin;
  uint8_t PWM_1_pin;
  uint8_t PWM_2_pin;
  uint8_t encoder_a_pin;
  uint8_t encoder_b_pin;

  bool is_ready = 0;
  double is_running = 0;
  uint32_t frequency = 0;       
  bool rotation_direction = 0;  
  double speed = 0;             
  int duty_cycle = 0;          
};

struct SensorCfg {
  uint8_t slaveID;   
  uint16_t regAddr;  
  uint32_t baud;     
};

// --- PHẦN BỊ THIẾU CẦN THÊM VÀO ---
struct SENSOR_STATE {
  double is_running = 0;  // Trạng thái hoạt động
  double distance = 0;    // Khoảng cách (cm)
};

// --- KHAI BÁO EXTERN ---
extern SensorCfg sensors;
extern LED led;
extern MOTOR motor;
extern SENSOR_STATE sensor; // Thêm dòng này để AppTasks nhìn thấy biến 'sensor'

#endif
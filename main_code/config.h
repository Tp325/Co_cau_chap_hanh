#ifndef config_h
#define config_h
#include <Arduino.h>

// #define led_red_pin 27
// #define led_yellow_pin 26
// #define led_blue_pin 25
// #define buzzel_pin 33

#define emergency_button 32

// #define enable_motor_pin 21
// #define PWM_1_pin = 22
// #define PWM_2_pin = 23
// #define encoder_a_pin 18
// #define encoder_b_pin 19

#define S_RX 16
#define S_TX 17

struct LED {
  uint8_t led_red_pin;
  uint8_t led_yellow_pin;
  uint8_t led_blue_pin;
  bool led_red_state = 0;     // trạng thái led đỏ
  bool led_yellow_state = 0;  // trạng thái led vàng
  bool led_blue_state = 0;    // trạng thái led xanh
};

struct BUZZEL {
  uint8_t buzzel_pin;
  bool buzzel_state = 0;  // trạng thái còi
};

struct MOTOR {
  uint8_t enable_motor_pin;
  uint8_t PWM_1_pin;
  uint8_t PWM_2_pin;
  uint8_t encoder_a_pin;
  uint8_t encoder_b_pin;

  bool is_ready = 0;            // động cơ đã sẵn sàng chưa
  double last_motor_state = 0;  // giá trị trạng thái cũ của động cơ
  double is_running = 0;        // động cơ có đang chạy hay không
  double time_cycle = 0;        // chu kỳ
  uint32_t frequency = 0;       // tần số khiển
  bool rotation_direction = 0;  // chiều quay động cơ 1 thuận 0 nghịch
  double speed = 0;             // vận tốc
  int dulty_cycle = 0;          // dulty cycle
};

struct SENSOR {
  double is_running = 0;  // cảm biến có đang chạy hay không
  double distance = 0;    // khoảng cách hiện tại
};
struct SensorCfg {
  uint8_t slaveID;   // ID cảm biến
  uint16_t regAddr;  // thanh ghi
  uint32_t baud;     // baudrate
  const char* name;  // tên cảm biến
};


#endif
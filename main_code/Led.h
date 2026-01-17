#ifndef LED_H
#define LED_H
#include "config.h"
#include <Arduino.h>

#define led_red 1
#define led_yellow 2
#define led_blue 3

class LIGHT {
private:
  uint8_t led_red_pin;
  uint8_t led_yellow_pin;
  uint8_t led_blue_pin;
  LED *buff_led;
public:
  LIGHT(LED *led);
  void begin();
  void process();
  void set_led(int light_number, int state);
};
#endif
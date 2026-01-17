#include "Led.h"

LIGHT::LIGHT(LED *led) {
  led_yellow_pin = led->led_yellow_pin;
  led_red_pin = led->led_red_pin;
  led_blue_pin = led->led_blue_pin;
  this->buff_led = led;
}

void LIGHT::begin() {
  pinMode(led_red_pin, OUTPUT);
  pinMode(led_yellow_pin, OUTPUT);
  pinMode(led_blue_pin, OUTPUT);
}

void LIGHT::process() {
  digitalWrite(led_red_pin, buff_led->led_red_state);
  digitalWrite(led_yellow_pin, buff_led->led_yellow_state);
  digitalWrite(led_blue_pin, buff_led->led_blue_state);
}

void LIGHT::set_led(int light_number, int state) {
  switch (light_number) {
    case led_red:
     buff_led->led_red_state = state;
      break;
    case led_yellow:
     buff_led->led_yellow_state = state;
      break;
    case led_blue:
     buff_led->led_blue_state = state;
      break;
  }
}
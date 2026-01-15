#include "Communication.h"
#include "config.h"
#include "Led.h"
#include "Motor.h"
#include "Sensor.h"

Communication communication;
LIGHT light(&led);
Motor engine(&motor);
Sensor distance(&sensors);

void setup() {
  Serial.begin(9600);
  communication.begin();
  light.begin();
  engine.begin(5000, 1);
  engine.motor_enalble();
  distance.begin();
  light.set_led(led_red, 1);
  light.set_led(led_yellow, 1);
  light.set_led(led_blue, 1);
  delay(3000);
  xTaskCreatePinnedToCore(vtaskDebug, "taskDebug", 4096, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(vtaskLED, "taskLED", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskSensor, "taskSensor", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskMotor, "taskMotor", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskSendToServer, "taskSendToServer", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskReceiveFromServer, "taskReceiveFromServer", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskAnalize, "taskAnalize", 4096, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(vtaskProcess, "taskProcess", 4096, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(vtaskBlocking, "taskBlocking", 4096, NULL, 5, NULL, 0);
  delay(2000);
  vTaskDelete(NULL);
}

void loop() {
  vTaskDelete(NULL);
}
void vtaskDebug(void *pvParameters) {
  while (1) {
    Serial.printf("khoang_cach(mm):%.2lf,Toc_do(mm):%.2lf", sensor.distance, motor.speed);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}
void vtaskMotor(void *pvParameters) {
  engine.soft_power_on();
  while (1) {
    engine.process();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
void vtaskLED(void *pvParameters) {
  while (1) {
    light.process();
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
void vtaskSensor(void *pvParameters) {
  sensor.is_running = 1;
  while (1) {
    motor.speed = engine.get_speed();
    sensor.distance = distance.get_distance();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
void vtaskReceiveFromServer(void *pvParameters) {
  while (1) {
    communication.receiveFromServer();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
void vtaskSendToServer(void *pvParameters) {
  while (1) {
    communication.sendToServer();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
void vtaskAnalize(void *pvParameters) {
  while (1) {

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
void vtaskProcess(void *pvParameters) {
  while (1) {
    communication.process();
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void vtaskBlocking(void *pvParameters) {
  while (1) {
    communication.blocking();
    if (communication.haveToReset == 1)
      ESP.restart();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}
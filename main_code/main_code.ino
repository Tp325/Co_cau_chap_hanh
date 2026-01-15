#include "Communication.h"
#include "config.h"

Communication communication;
void setup() {
  Serial.begin(9600);
  communication.begin();
  delay(3000);
  xTaskCreatePinnedToCore(vtaskLED, "taskLED", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskSensor, "taskSensor", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskSendToServer, "taskSendToServer", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskReceiveFromServer, "taskReceiveFromServer", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskAnalize, "taskAnalize", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vtaskProcess, "taskProcess", 4096, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(vtaskBlocking, "taskBlocking", 4096, NULL, 5, NULL, 0);
  delay(2000);
  vTaskDelete(NULL);
}

void loop() {
  vTaskDelete(NULL);
}

void vtaskSensor(void *pvParameters) {
  while (1) {

    vTaskDelay(500 / portTICK_PERIOD_MS);
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

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}
void vtaskProcess(void *pvParameters) {
  while (1) {
    communication.process();
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void vtaskLED(void *pvParameters) {
  while (1) {
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
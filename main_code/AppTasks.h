#ifndef APP_TASKS_H
#define APP_TASKS_H

#include <Arduino.h>
#include <freertos/semphr.h>

#include "config.h"
#include "Motor.h"
#include "Sensor.h"
#include "Communication.h"
#include "Led.h"

#include "PID_Logic.h"

// --- KHAI BÁO EXTERN ---

extern Motor engine;
extern Sensor distance;
extern LIGHT light;
extern Communication communication;

extern SemaphoreHandle_t xSysMutex;
extern PID_PARAM pid_pos;   // PID Vị trí
extern PID_PARAM pid_speed; // PID Tốc độ

// Biến chia sẻ giữa Task Sensor và Motor
extern double sys_target_rpm;

// --- PROTOTYPES CÁC TASK ---
void vtaskSensor(void *pvParameters);
void vtaskMotor(void *pvParameters);
void vtaskDebug(void *pvParameters);

// (Wifi/MQTT)
void vtaskLED(void *pvParameters);
void vtaskReceiveFromServer(void *pvParameters);
void vtaskSendToServer(void *pvParameters);
void vtaskAnalize(void *pvParameters);
void vtaskProcess(void *pvParameters);
void vtaskBlocking(void *pvParameters);

#endif
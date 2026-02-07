#include "Config.h"
#include "PID.h"
#include "Motor.h"
#include "Sensors.h"
#include "SerialCommander.h"

// setpoint mặc định
float SETPOINT_X = 17.0;    // cm

volatile bool newDataAvailable = false; 

// --- MODULES ---
Motor motor;
LaserSensor laser;
AngleSensor angleSensor;

// PID Configuration
PID pidPos(KP_X, KI_X, KD_X, -MAX_TILT, MAX_TILT, 0.01);
PID pidAngle(KP_TH, KI_TH, KD_TH, -255, 255, 0.01);

// --- KHỞI TẠO COMMANDER ---
// SETPOINT_X: 5.0cm -> 45.0cm
SerialCommander commander(&SETPOINT_X, &pidPos, 5.0, 45.0);

// --- State ---
enum SystemState { STATE_IDLE, STATE_RUNNING };
volatile SystemState currentState = STATE_IDLE;

// Task
void TaskSensor(void *pvParameters);
void TaskControl(void *pvParameters);

void setup() {
    // Init serial
    commander.begin(115200);

    // Init IO
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);

    // Init Modules
    motor.init();
    laser.init();
    angleSensor.init();

    // Init Tasks
    // Core 1: sensors
    xTaskCreatePinnedToCore(TaskSensor, "Sensor", 4096, NULL, 1, NULL, 1);
    
    // Core 0: PID & Motor
    xTaskCreatePinnedToCore(TaskControl, "Control", 4096, NULL, 5, NULL, 0);

    // Notice of availability
    Serial.println(">>> BALL & BEAM SYSTEM READY <<<");
    Serial.println(">>> Type a number (e.g., 25) to change Setpoint <<<");
}

void loop() {
    commander.update();

    // Debug (200ms)
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 200) {
        Serial.printf("State:%d | Set:%.1f | Dist:%.2f | Ang:%.2f\n", 
                      currentState, SETPOINT_X, laser.getDistance(), angleSensor.getAngle());
        lastPrint = millis();
    }
}

// --- LOGIC ---
void stopSystem() {
    currentState = STATE_IDLE;
    motor.stop();
    digitalWrite(PIN_BUZZER, HIGH); delay(100); digitalWrite(PIN_BUZZER, LOW);
}

void startSystem() {
    angleSensor.reset(); 
    pidPos.reset();
    pidAngle.reset();
    currentState = STATE_RUNNING;

    // Start engine
    digitalWrite(PIN_BUZZER, HIGH); delay(50); digitalWrite(PIN_BUZZER, LOW);
    delay(50);
    digitalWrite(PIN_BUZZER, HIGH); delay(50); digitalWrite(PIN_BUZZER, LOW);
}

void handleButton() {
    static unsigned long pressStart = 0;
    static bool isPressed = false;
    int btnState = digitalRead(PIN_BUTTON);

    if (btnState == LOW && !isPressed) {
        isPressed = true;
        pressStart = millis();
    } else if (btnState == HIGH && isPressed) {
        unsigned long duration = millis() - pressStart;
        isPressed = false;

        if (duration > LONG_PRESS_MS) {
            if (currentState == STATE_IDLE) {
                motor.toggleDirection();
                digitalWrite(PIN_BUZZER, HIGH); delay(500); digitalWrite(PIN_BUZZER, LOW);
            }
        } else if (duration > 50) {
            if (currentState == STATE_IDLE) startSystem();
            else stopSystem();
        }
    }
}

// --- TASK CONTROL ---
void TaskControl(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10); 

    float targetAngle = 0.0; 
    const float MAX_SLEW_RATE = 0.4; 

    for (;;) {
        handleButton();
        float currentDist = laser.getDistance(); 

        if (currentState == STATE_RUNNING) {
            float currentAngle = angleSensor.getAngle();
            if (abs(currentAngle) > MAX_TILT + 5.0) stopSystem();
            else {
                float error = abs(currentDist - SETPOINT_X);
                
                // ==========================================
                // 1. FUNNEL
                // ==========================================
                float dynamicMaxTilt = 0;

                if (error > 15.0) {
                    dynamicMaxTilt = 10.0;
                }
                if (error > 12.0) {
                    dynamicMaxTilt = 5.0;
                }
                else if (error > 5.0) {
                    dynamicMaxTilt = 3.0;
                }   
                else if (error > 3.0) {
                    dynamicMaxTilt = 0.8;
                } 
                else {
                    dynamicMaxTilt = 0.7;  // tuyệt đối không đụng cái này 
                }

                // ==========================================
                // 2. COMPUTE PID
                // ==========================================
                if (newDataAvailable) {
                    newDataAvailable = false; 
                    
                    if (error < 0.3) {
                        targetAngle = 0;
                        pidPos.reset(); 
                    }
                    else {
                        float rawTarget = pidPos.compute(SETPOINT_X, currentDist);

                        // Clamp
                        if (rawTarget > dynamicMaxTilt) rawTarget = dynamicMaxTilt;
                        if (rawTarget < -dynamicMaxTilt) rawTarget = -dynamicMaxTilt;

                        // Slew Rate
                        float diff = rawTarget - targetAngle;
                        if (diff > MAX_SLEW_RATE) targetAngle += MAX_SLEW_RATE;
                        else if (diff < -MAX_SLEW_RATE) targetAngle -= MAX_SLEW_RATE;
                        else targetAngle = rawTarget;
                    }
                }

                // ==========================================
                // 3. DRIVE MOTOR
                // ==========================================
                float finalTarget = targetAngle + 0.0; 
                float rawPWM = pidAngle.compute(finalTarget, currentAngle);

                int drivePWM = 0;
                if (rawPWM > 0) drivePWM = (int)rawPWM + PWM_MIN;
                else if (rawPWM < 0) drivePWM = (int)rawPWM - PWM_MIN;

                if (drivePWM > MAX_PWM) drivePWM = MAX_PWM;
                if (drivePWM < -MAX_PWM) drivePWM = -MAX_PWM;

                motor.drive(drivePWM);
            }
        } 
        else { 
            motor.stop();
            targetAngle = 0;
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// --- TASK SENSOR ---
void TaskSensor(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 20Hz

    for (;;) {
        laser.read(); 
        newDataAvailable = true; 
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
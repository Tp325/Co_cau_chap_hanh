/* FILE: BallAndBeam.ino */
#include "Config.h"
#include "PID.h"
#include "Motor.h"
#include "Sensors.h"

// --- KHỞI TẠO ---
Motor motor;
LaserSensor laser;
AngleSensor angleSensor;

// PID Vòng Ngoài: Input=Cm, Output=Độ
PID pidPos(KP_X, KI_X, KD_X, -MAX_TILT, MAX_TILT, 0.01);
// PID Vòng Trong: Input=Độ, Output=PWM
PID pidAngle(KP_TH, KI_TH, KD_TH, -255, 255, 0.01);

// --- TRẠNG THÁI ---
enum SystemState { STATE_IDLE, STATE_RUNNING };
volatile SystemState currentState = STATE_IDLE;

// Task FreeRTOS
void TaskSensor(void *pvParameters);
void TaskControl(void *pvParameters);

void setup() {
    Serial.begin(115200);

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
    // Task Sensor chạy Core 1 (20Hz)
    xTaskCreatePinnedToCore(TaskSensor, "Sensor", 4096, NULL, 1, NULL, 1);
    // Task Control chạy Core 0 (100Hz - Realtime)
    xTaskCreatePinnedToCore(TaskControl, "Control", 4096, NULL, 5, NULL, 0);

    Serial.println(">>> BALL & BEAM PRO - OOP VERSION <<<");
}

void loop() {
    // Chỉ in Debug
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 200) {
        Serial.print("Stt:"); Serial.print(currentState);
        Serial.print(",Dir:"); Serial.print(motor.getDirection());
        Serial.print(",Dst:"); Serial.print(laser.getDistance());
        Serial.print(",Ang:"); Serial.println(angleSensor.getAngle());
        lastPrint = millis();
    }
}

// --- LOGIC HỆ THỐNG ---

void stopSystem() {
    currentState = STATE_IDLE;
    motor.stop();
    digitalWrite(PIN_BUZZER, HIGH); delay(100); digitalWrite(PIN_BUZZER, LOW);
}

void startSystem() {
    angleSensor.reset(); // Reset góc về 0
    pidPos.reset();
    pidAngle.reset();
    currentState = STATE_RUNNING;
    // Bíp kép báo chạy
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
            // Nhấn giữ: Đảo chiều (Chỉ khi IDLE)
            if (currentState == STATE_IDLE) {
                motor.toggleDirection();
                digitalWrite(PIN_BUZZER, HIGH); delay(500); digitalWrite(PIN_BUZZER, LOW);
            }
        } else if (duration > 50) {
            // Nhấn ngắn: Start/Stop
            if (currentState == STATE_IDLE) startSystem();
            else stopSystem();
        }
    }
}

// --- FREERTOS TASKS ---

void TaskControl(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 100Hz

    for (;;) {
        handleButton();
        float currentDist = laser.getDistance();

        // Safety: Mất bóng -> Dừng
        if (currentState == STATE_RUNNING && currentDist > SAFETY_DIST) {
            stopSystem();
        }

        if (currentState == STATE_IDLE) {
            motor.stop();
            static int blink = 0;
            digitalWrite(PIN_LED_R, (blink++ % 20 < 10)); // Nháy Đỏ
            digitalWrite(PIN_LED_G, LOW);
        } 
        else if (currentState == STATE_RUNNING) {
            digitalWrite(PIN_LED_R, LOW);
            digitalWrite(PIN_LED_G, HIGH);

            float currentAngle = angleSensor.getAngle();

            // Safety: Nghiêng quá mức -> Dừng
            if (abs(currentAngle) > MAX_TILT) {
                stopSystem();
            } else {
                // Cascaded PID
                float targetAngle = pidPos.compute(SETPOINT_X, currentDist);
                float motorPWM = pidAngle.compute(targetAngle, currentAngle);
                motor.drive((int)motorPWM);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void TaskSensor(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 20Hz

    for (;;) {
        laser.read();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
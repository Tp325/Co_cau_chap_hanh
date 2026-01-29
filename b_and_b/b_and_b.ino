/* FILE: BallAndBeam.ino */
#include "Config.h"
#include "PID.h"
#include "Motor.h"
#include "Sensors.h"

volatile bool newDataAvailable = false; // Cờ báo có dữ liệu mới

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

    // Biến lưu góc mục tiêu (Giữ nguyên giá trị cũ cho đến khi có tính toán mới)
    float targetAngle = 0.0;

    for (;;) {
        handleButton();
        float currentDist = laser.getDistance(); // Lấy giá trị hiện tại (có thể là cũ)

        // --- Safety ---
        if (currentState == STATE_RUNNING && currentDist > SAFETY_DIST) {
            stopSystem();
        }

        if (currentState == STATE_RUNNING) {
            float currentAngle = angleSensor.getAngle();

            // Safety góc nghiêng
            if (abs(currentAngle) > MAX_TILT + 5.0) stopSystem();
            else {
                // ====================================================
                // 1. VÒNG NGOÀI (PID VỊ TRÍ) - CHỈ CHẠY KHI CÓ SỐ MỚI
                // ====================================================
                if (newDataAvailable) {
                    newDataAvailable = false; // Xóa cờ
                    
                    // DEADBAND (VÙNG BÌNH YÊN)
                    if (abs(currentDist - SETPOINT_X) < 0.8) {
                        targetAngle = 0; // Về phẳng
                        pidPos.reset();
                    } 
                    else {
                        // Tính góc cần nghiêng mới
                        // Lưu ý: PID Pos bây giờ chạy với chu kỳ thực tế là 50ms theo Sensor
                        targetAngle = pidPos.compute(SETPOINT_X, currentDist);
                    }
                }

                // ====================================================
                // 2. VÒNG TRONG (PID GÓC) - CHẠY LIÊN TỤC (100Hz)
                // ====================================================
                // Motor luôn cần được cập nhật liên tục để giữ cái targetAngle kia
                float rawPWM = pidAngle.compute(targetAngle, currentAngle);

                // BÙ MA SÁT
                int drivePWM = 0;
                if (rawPWM > 0) drivePWM = (int)rawPWM + PWM_MIN;
                else if (rawPWM < 0) drivePWM = (int)rawPWM - PWM_MIN;

                // Kẹp dòng
                if (drivePWM > MAX_PWM) drivePWM = MAX_PWM;
                if (drivePWM < -MAX_PWM) drivePWM = -MAX_PWM;

                // Deadband tác động trực tiếp lên motor nếu đã vào đích
                if (abs(currentDist - SETPOINT_X) < 0.8) {
                     motor.drive(0);
                } else {
                     motor.drive(drivePWM);
                }
            }
        } 
        else { 
            // IDLE STATE
            motor.stop();
            // ... (Led code cũ) ...
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
// --- DÁN CÁI NÀY VÀO CUỐI CÙNG FILE BallAndBeam.ino ---

void TaskSensor(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 20Hz

    for (;;) {
        laser.read(); // Đọc Modbus
        
        // Bật cờ báo hiệu cho TaskControl biết
        newDataAvailable = true; 
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
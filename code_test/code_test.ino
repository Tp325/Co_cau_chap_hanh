// LOGIC: Distance > 20 -> Rotate CW (Positive RPM).
// Distance < 20 -> Rotate CCW (Negative RPM).

#include <Arduino.h>
#include <ESP32Encoder.h>
#include <freertos/semphr.h>
#include <ModbusMaster.h>

#define PWM_PIN_1 22       
#define PWM_PIN_2 23       
#define ENABLE_PIN 21      
#define ENCODER_A 18
#define ENCODER_B 19
#define SENSOR_RX 16
#define SENSOR_TX 17

#define SENSOR_ID 0x50     
#define SENSOR_REG 0x34    

const double PULSE_PER_REV = 2304.0; 
const uint32_t PWM_FREQ = 20000;
const uint8_t PWM_RES = 8;
const long LOOP_TIME_MS = 50; 
const double SETPOINT_POS = 20.0; // Vị trí cân bằng 20cm

// --- 2. STRUCTS ---
struct PID_PARAM {
    double Kp, Ki, Kd;
    double integral_err = 0;
    double last_err = 0;
};

struct SYSTEM_STATE {
    // Motor State
    double target_rpm = 0;    
    double current_rpm = 0;   
    int pwm_value = 0;        
    
    // Sensor State
    double distance_cm = 0.0;
    bool system_ready = false; 
};

// --- BIẾN TOÀN CỤC ---
ESP32Encoder encoder;
ModbusMaster node;
SemaphoreHandle_t xSysMutex;
SYSTEM_STATE sys;

PID_PARAM pid_speed; 
PID_PARAM pid_pos;   

// --- PROTOTYPES ---
void vTaskPID_Motor(void *pvParameters);
void vTaskSensor_Modbus(void *pvParameters); // Task đọc cảm biến 
void vTaskConsole(void *pvParameters);
double computePID_Speed(double target, double current, double dt);
double computePID_Pos(double setpoint, double input, double dt);



void setup() {
    Serial.begin(115200);
    
    // Setup Modbus 
    Serial2.begin(115200, SERIAL_8N1, SENSOR_RX, SENSOR_TX);
    node.begin(SENSOR_ID, Serial2);

    ESP32Encoder::useInternalWeakPullResistors = puType::up;
    encoder.attachHalfQuad(ENCODER_A, ENCODER_B);
    encoder.setCount(0);

    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, HIGH);
    ledcAttach(PWM_PIN_1, PWM_FREQ, PWM_RES);
    ledcAttach(PWM_PIN_2, PWM_FREQ, PWM_RES);

    xSysMutex = xSemaphoreCreateMutex();

    // --- TUNING (CHỈNH PID Ở ĐÂY)************************************************************ ---
    
    // 1. PID TỐC ĐỘ (Inner Loop)
    pid_speed.Kp = 4.0;
    pid_speed.Ki = 15.0; 
    pid_speed.Kd = 0.2;

    // 2. PID VỊ TRÍ 
    // Quy tắc: Dist > 20 -> Cần Thuận (+) -> Error = Input - Setpoint
    pid_pos.Kp = 4.0;   // Kp=4: Lệch 1cm chạy 4 RPM
    pid_pos.Ki = 0.1;   // Ki nhỏ để tránh trôi
    pid_pos.Kd = 8.0;   // Kd cao để hãm khi bóng lăn nhanh

    // Tasks
    xTaskCreatePinnedToCore(vTaskPID_Motor, "MotorTask", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(vTaskSensor_Modbus, "SensorTask", 4096, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(vTaskConsole, "DebugTask", 2048, NULL, 1, NULL, 0);

    Serial.println("System Restored: Separate Sensor Task");
}

void loop() { vTaskDelay(1000); }

// ================= TASK: SENSOR & BALANCE LOGIC =================
void vTaskSensor_Modbus(void *pvParameters) {
    while (1) {
        // 1. Đọc Modbus 
        uint8_t result = node.readHoldingRegisters(SENSOR_REG, 1);
        
        double current_dist = 0;
        bool read_success = false;

        if (result == node.ku8MBSuccess) {
            uint16_t raw = node.getResponseBuffer(0);
            node.clearResponseBuffer();
            current_dist = raw / 10.0;
            read_success = true;
        } else {
            // Đọc lỗi
            read_success = false;
        }

        // 2. Tính toán Cân bằng
        double req_rpm = 0;

        if (read_success && current_dist > 2.0 && current_dist < 50.0) {
            // Tính toán PID Vị trí
            // Khoảng cách > 20 -> Input > Setpoint -> Error Dương -> RPM Dương 
            // Đúng yêu cầu: "Hơn 20 quay thuận"
            req_rpm = computePID_Pos(SETPOINT_POS, current_dist, 0.2); // dt ~ 200ms

            // Giới hạn tốc độ an toàn (Max 60 RPM)
            if (req_rpm > 60) req_rpm = 60;
            if (req_rpm < -60) req_rpm = -60;

        } else {
            // Lỗi cảm biến hoặc bóng ra ngoài -> Dừng
            req_rpm = 0;
            // Reset PID tích lũy để tránh vọt khi bóng quay lại
            pid_pos.integral_err = 0;
        }

        // 3. Cập nhật dữ liệu cho Task Motor
        if (xSemaphoreTake(xSysMutex, 10) == pdTRUE) {
            sys.distance_cm = current_dist;
            sys.system_ready = read_success;
            sys.target_rpm = req_rpm; // Gửi lệnh tốc độ xuống
            xSemaphoreGive(xSysMutex);
        }

        // Delay 200ms ( này tần số đọc cảm biến thay 200 để đổi tần số đọc)
        vTaskDelay(200 / portTICK_PERIOD_MS); 
    }
}

// ================= TASK: MOTOR PID =================
void vTaskPID_Motor(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(LOOP_TIME_MS);
    long last_enc = 0;

    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Sensing
        long curr_enc = encoder.getCount();
        long delta = curr_enc - last_enc;
        last_enc = curr_enc;
        double speed = ((double)delta / PULSE_PER_REV) * (60.0 / (LOOP_TIME_MS / 1000.0));

        // Get Command
        double target = 0;
        bool active = false;
        if (xSemaphoreTake(xSysMutex, 5) == pdTRUE) {
            target = sys.target_rpm;
            active = sys.system_ready;
            sys.current_rpm = speed;
            xSemaphoreGive(xSysMutex);
        }

        // Compute PID Speed
        int pwm_out = 0;
        // Nếu cảm biến OK thì chạy, không thì dừng
        // Nếu muốn test motor mà không cần cảm biến, sửa 'active' thành 'true'
        if (active) { 
            double res = computePID_Speed(target, speed, LOOP_TIME_MS / 1000.0);
            
            if (res > 255) res = 255;
            if (res < -255) res = -255;
            if (target == 0 && abs(res) < 15) res = 0; // Deadband

            pwm_out = (int)res;
        } else {
            pid_speed.integral_err = 0;
            pwm_out = 0;
        }

        // Actuation ( logic đảo chiều)
        if (pwm_out >= 0) {
            ledcWrite(PWM_PIN_1, 0);
            ledcWrite(PWM_PIN_2, abs(pwm_out));
        } else {
            ledcWrite(PWM_PIN_1, abs(pwm_out));
            ledcWrite(PWM_PIN_2, 0);
        }
        sys.pwm_value = pwm_out;
    }
}

// ================= PID ALGORITHMS =================

// PID 1: VỊ TRÍ (Khoảng cách -> RPM)
double computePID_Pos(double setpoint, double input, double dt) {
    // Logic: Input (30) > Setpoint (20) => Error (+10) => Output (+) => Quay thuận => Kéo bóng về
    double error = input - setpoint; 

    double P = pid_pos.Kp * error;
    
    pid_pos.integral_err += error * dt;
    // Kẹp I vòng ngoài nhỏ thôi
    if (pid_pos.integral_err > 20) pid_pos.integral_err = 20;
    else if (pid_pos.integral_err < -20) pid_pos.integral_err = -20;
    double I = pid_pos.Ki * pid_pos.integral_err;

    double D = pid_pos.Kd * ((error - pid_pos.last_err) / dt);
    pid_pos.last_err = error;

    return P + I + D;
}

// PID 2: TỐC ĐỘ (RPM -> PWM)
double computePID_Speed(double target, double current, double dt) {
    double error = target - current;
    double P = pid_speed.Kp * error;
    
    pid_speed.integral_err += error * dt;
    double max_I = 255.0 / pid_speed.Ki;
    if (pid_speed.integral_err > max_I) pid_speed.integral_err = max_I;
    else if (pid_speed.integral_err < -max_I) pid_speed.integral_err = -max_I;
    double I = pid_speed.Ki * pid_speed.integral_err;

    double D = pid_speed.Kd * ((error - pid_speed.last_err) / dt);
    pid_speed.last_err = error;

    return P + I + D;
}

// ================= CONSOLE =================
void vTaskConsole(void *pvParameters) {
    while(1) {
        if (xSemaphoreTake(xSysMutex, 10) == pdTRUE) {
            // Format: Dist, Setpoint, RPM_Command, RPM_Real
            Serial.print("Dist:"); Serial.print(sys.distance_cm);
            Serial.print(",Set:20.0");
            Serial.print(",RPM_Cmd:"); Serial.print(sys.target_rpm);
            Serial.print(",RPM_Act:"); Serial.println(sys.current_rpm);
            xSemaphoreGive(xSysMutex);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
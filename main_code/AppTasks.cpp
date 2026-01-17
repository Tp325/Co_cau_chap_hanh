#include "AppTasks.h"

// Biến cục bộ chỉ dùng để truyền tin giữa Sensor và Motor
double sys_target_rpm = 0; 

// ================= TASK SENSOR =================
// Đọc khoảng cách -> Compute tốc độ cần chạy
void vtaskSensor(void *pvParameters) {
  sensor.is_running = 1; 
  const double SETPOINT = 20.0; // ***************** kiếm tao chứ gì *****************

  while (1) {
    // 1. Đọc cảm biến
    float dist = distance.get_distance();
    
    // Lưu vào struct toàn cục (Có bảo vệ Mutex rồi)
    if(xSemaphoreTake(xSysMutex, 10) == pdTRUE) {
       sensor.distance = dist;
       xSemaphoreGive(xSysMutex);
    }

    double target_speed = 0;
    
    // 2. Chỉ chạy khi bóng nằm trong thanh (2cm - 50cm)
    // Nếu sensor trả về 0 hoặc >50 thì dừng
    if (dist > 2.0 && dist < 50.0) {
        // Gọi thuật toán PID Vị trí (Từ PID_Logic.cpp)
        target_speed = computePID_Pos(pid_pos, SETPOINT, dist, 0.05); 
        
        // Giới hạn tốc độ (-60 đến 60 RPM)
        if (target_speed > 60) target_speed = 60;
        if (target_speed < -60) target_speed = -60;
    } else {
        // Bóng rơi -> Dừng motor & Reset PID tích lũy
        target_speed = 0;
        pid_pos.integral_err = 0; 
    }

    // 3. Gửi lệnh xuống Task Motor
    if(xSemaphoreTake(xSysMutex, 10) == pdTRUE) {
       sys_target_rpm = target_speed;
       xSemaphoreGive(xSysMutex);
    }

    vTaskDelay(50 / portTICK_PERIOD_MS); // Chu kỳ 50ms           ***************** chỉnh tần số đọc sensor *****************
  }
}

// ================= TASK MOTOR =================
// Điều khiển PWM bám theo cái tốc độ đang taget
void vtaskMotor(void *pvParameters) {
  engine.soft_power_on();
  
  while (1) {
    // 1. Đọc tốc độ thực
    double current_speed = engine.get_speed();
    
    // Cập nhật struct motor toàn cục
    if(xSemaphoreTake(xSysMutex, 5) == pdTRUE) {
        motor.speed = current_speed;
        xSemaphoreGive(xSysMutex);
    }

    // 2. Lấy lệnh Target từ Task Sensor
    double target = 0;
    if(xSemaphoreTake(xSysMutex, 5) == pdTRUE) {
        target = sys_target_rpm;
        xSemaphoreGive(xSysMutex);
    }

    // 3. Tính toán PID Tốc độ (Inner Loop)
    // PID tính ra giá trị 8-bit (-255 đến 255)
    double pid_out_8bit = computePID_Speed(pid_speed, target, current_speed, 0.02);

    if (pid_out_8bit > 255) pid_out_8bit = 255;
    if (pid_out_8bit < -255) pid_out_8bit = -255;

    // 4. Mapping sang Motor Driver 10-bit      không biết làm vầy để chi mà thoi đang chạy đừng có đụng vô
    int duty_10bit = abs(pid_out_8bit) * 4; 
    if (duty_10bit > 1023) duty_10bit = 1023;

    // 5. Điều khiển chiều quay (Logic đảo chiều đã test ổn định đừng đụng luôn)
    if (pid_out_8bit >= 0) {
        engine.switch_rotation(1); // Quay thuận
    } else {
        engine.switch_rotation(0); // Quay nghịch
    }

    // 6. Nạp PWM và chơi
    engine.switch_duty_cycle(duty_10bit);
    engine.process();

    vTaskDelay(20 / portTICK_PERIOD_MS); // Chu kỳ 20ms     ***************** chỉnh tần số cập nhật vận tốc *****************
                                        //  nhớ chỉnh luôn cái dòng if (millis() - time_counter >= 20) { ... } trong motor.cpp theo không thoi không chạy
  }
}

// ================= TASK DEBUG =================
void vtaskDebug(void *pvParameters) {
  while (1) {
    // In ra Serial để vẽ đồ thị hoặc kiểm tra
    Serial.printf("Dist:%.2f, RPM_Act:%.2f, RPM_Cmd:%.2f\n", 
                  sensor.distance, motor.speed, sys_target_rpm);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

// ================= CÁC TASK HỆ THỐNG =================
void vtaskLED(void *pvParameters) {
  while (1) {
    light.process();
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
  while (1) { vTaskDelay(500 / portTICK_PERIOD_MS); }
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
    if (communication.haveToReset == 1) ESP.restart();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}
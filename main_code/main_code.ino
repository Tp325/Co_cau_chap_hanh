/* 
code chạy khi bóng nằm trong khoảng 2 đến 50cm out range thì tắt động cơ


sensor chia 10 ra cm làm tròn đến số thập phân thứ nhất,
PWM 10bit,
encoder 2304 xung/vòng,
đọc sensor 20HZ,      chỉnh trong tab sensor.cpp 
cập nhật giá trị vận tốc 50HZ     trong tab apptaks.cpp và motor.cpp


*/


#include "AppTasks.h" 



Communication communication;
LIGHT light(&led);
Motor engine(&motor);
Sensor distance(&sensors);



SemaphoreHandle_t xSysMutex;
PID_PARAM pid_pos;
PID_PARAM pid_speed;



void setup() {
  Serial.begin(115200);

  // 1. Init Modules      "mấy thư viện giao tiếp t chưa đọc mẹ gì hết"
  communication.begin();
  light.begin();
  
  engine.begin(20000, 1); //      Freq 20kHz, Default Dir 1 (chiều quay)
  engine.motor_enalble();
  
  distance.begin();
  
  // Test đèn báo hiệu khởi động
  light.set_led(led_red, 1);
  delay(500);
  light.set_led(led_red, 0);

  // 2. Init RTOS System
  xSysMutex = xSemaphoreCreateMutex();
  
  // 3. PID Tuning      *****************(Chỉnh thông số tại đây)*****************
  // Vòng ngoài: Vị trí (này thông số không tải nha cu)
  pid_pos.Kp = 4.0;   pid_pos.Ki = 0.1;   pid_pos.Kd = 8.0;   
  
  // Vòng trong: Tốc độ
  pid_speed.Kp = 4.0; pid_speed.Ki = 15.0; pid_speed.Kd = 0.2; 

  delay(1000); // Chờ hệ thống ổn định

  // 4. Create Tasks
  // Task Debug & System
  xTaskCreatePinnedToCore(vtaskDebug, "Debug", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(vtaskLED, "LED", 2048, NULL, 1, NULL, 1);
  
  // Nhóm Task Điều khiển 
  // (Priority CAO NHẤT = 10)
  // thay đổi vị trí cân bằng trong apptasks.cpp (setpoint)     ***************** Thay đổi setpoint *****************
  xTaskCreatePinnedToCore(vtaskSensor, "Sensor", 4096, NULL, 10, NULL, 1); 
  xTaskCreatePinnedToCore(vtaskMotor, "Motor", 4096, NULL, 10, NULL, 1);
  
  // Nhóm Task Mạng (Priority Thấp = 2)
  xTaskCreatePinnedToCore(vtaskSendToServer, "MqttSend", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(vtaskReceiveFromServer, "MqttRecv", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(vtaskAnalize, "Analize", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(vtaskProcess, "Process", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(vtaskBlocking, "Blocking", 4096, NULL, 2, NULL, 0);
  
  vTaskDelete(NULL);
}

void loop() {
  vTaskDelete(NULL);
}
#include <ESP32Encoder.h>
#include <ModbusMaster.h>

#define led_red_pin 27
#define led_yellow_pin 26
#define led_blue_pin 25
#define buzzel_pin 33

#define emergency_button 32

#define enable_motor_pin 21
#define PWM_1_pin = 22
#define PWM_2_pin = 23
#define encoder_a_pin 18
#define encoder_b_pin 19

#define S_RX 16
#define S_TX 17


struct LED {
  bool led_red_state = 0;     // trạng thái led đỏ
  bool led_yellow_state = 0;  // trạng thái led vàng
  bool led_blue_state = 0;    // trạng thái led xanh
};

struct BUZZEL {
  bool buzzel_state = 0;  // trạng thái còi
};

struct MOTOR {
  bool is_ready = 0;               // động cơ đã sẵn sàng chưa
  double last_motor_state = 0;     // giá trị trạng thái cũ của động cơ
  double is_running = 0;           // động cơ có đang chạy hay không
  bool time_cycle = 0;             // chu kỳ
  uint32_t frequency = 0;          // tần số khiển
  uint8_t rotation_direction = 0;  // chiều quay động cơ 1 thuận 0 nghịch
  double speed = 0;                // vận tốc
};

struct SENSOR {
  double is_running = 0;  // cảm biến có đang chạy hay không
  double distance = 0;    // khoảng cách hiện tại
};
struct SensorCfg {
  uint8_t slaveID;   // ID cảm bienes
  uint16_t regAddr;  // thanh ghi
  uint32_t baud;     // baudrate
  const char* name;  // tên cảm biến
};
SensorCfg sensors[] = {
  { 0x50, 0x34, 115200, "distance" }
};

ModbusMaster node;
ESP32Encoder encoder;
LED led;
BUZZEL buzzel;
MOTOR motor;
SENSOR sensor;

void setup() {
  Serial2.begin(115200, SERIAL_8N1, S_RX, S_TX);
  Serial.begin(9600);

  pinMode(led_red_pin, OUTPUT);
  pinMode(led_yellow_pin, OUTPUT);
  pinMode(led_blue_pin, OUTPUT);
  pinMode(buzzel_pin, OUTPUT);

  pinMode(emergency_button, INPUT);

  pinMode(enable_motor_pin, OUTPUT);
  pinMode(PWM_1_pin, OUTPUT);
  pinMode(PWM_2_pin, OUTPUT);
  // pinMode(encoder_a_pin, INPUT);
  // pinMode(encoder_b_pin, INPUT);

  digitalWrite(led_red_pin, led.led_red_state);
  digitalWrite(led_yellow_pin, led.led_yellow_state);
  digitalWrite(led_blue_pin, led.led_blue_state);
  digitalWrite(buzzel_pin, buzzel.buzzel_state);

  encoder.attachHalfQuad(encoder_a_pin, encoder_b_pin);
  encoder.setCount(0);

  motor.rotation_direction = 1;
  motor.is_ready = 1;
  motor.frequency = 40000;


  xTaskCreatePinnedToCore(vTaskMotor, "TaskMotor", 4096, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(vTaskButton, "TaskButton", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vTaskLED, "TaskLED", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vTaskBuzzel, "TaskBuzzel", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vTaskEncoder, "TaskEncoder", 2048, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(vTaskSensor, "TaskSensor", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(vTaskDebug, "TaskDebug", 2048, NULL, 5, NULL, 1);
  delay(1000);
  vTaskDelete(NULL);
}

void loop() {
}

void vTaskDebug(void* pvParameters) {
  while (1) {
    // Serial.printf("Van toc: %.2f vong/phut, Khoang cach: %.2f cm \n", motor.speed, sensor.distance);
    Serial.print("Van_toc(RPM):");
    Serial.print(motor.speed);
    Serial.print(",");
    Serial.print("Khoang_cach(cm):");
    Serial.println(sensor.distance);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void vTaskSensor(void* pvParameters) {
  sensor.is_running = 0;
  while (1) {
    sensor.distance = getSensorValue(sensors[0].slaveID, sensors[0].regAddr, 3) / 10.0;
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void vTaskEncoder(void* pvParameters) {
  unsigned long time_counter = millis();
  long xung_truoc = 0;
  while (1) {
    if (millis() - time_counter >= 100) {
      long delta_xung = encoder.getCount() - xung_truoc;
      xung_truoc = encoder.getCount();
      motor.speed = (delta_xung / 1152.0) * (1000.0 / (millis() - time_counter)) * 60.0;
      time_counter = millis();
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void vTaskBuzzel(void* pvParameters) {
  while (1) {
    digitalWrite(buzzel_pin, buzzel.buzzel_state);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
void vTaskLED(void* pvParameters) {
  while (1) {
    digitalWrite(led_red_pin, led.led_red_state);
    digitalWrite(led_yellow_pin, led.led_yellow_state);
    digitalWrite(led_blue_pin, led.led_blue_state);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void vTaskButton(void* pvParameters) {
  uint32_t time_counter = millis();
  while (1) {
    if (digitalRead(emergency_button) == 0 && millis() - time_counter >= 250) {
      motor.is_ready = !motor.is_ready;
      motor.is_running = !motor.is_running;
      if (motor.is_ready == 0) {
        buzzel.buzzel_state = 1;
        led.led_red_state = 1;
        led.led_yellow_state = 0;
        led.led_blue_state = 0;
      } else {
        buzzel.buzzel_state = 0;
        led.led_red_state = 0;
        led.led_yellow_state = 0;
        led.led_blue_state = 1;
      }
      time_counter = millis();
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void vTaskMotor(void* pvParameters) {
  int pwm_pin;
  if (motor.rotation_direction == 1) {
    digitalWrite(PWM_1_pin, 0);
    pwm_pin = PWM_2_pin;
  } else {
    digitalWrite(PWM_2_pin, 0);
    pwm_pin = PWM_1_pin;
  }
  ledcAttach(pwm_pin, motor.frequency, 8);
  if (motor.is_ready == 1) {
    led.led_red_state = 0;
    led.led_yellow_state = 1;
    led.led_blue_state = 1;
    digitalWrite(enable_motor_pin, HIGH);
    motor.last_motor_state = 1;
    motor.is_running = 1;
    for (int i = 0; i <= 255; i++) {
      ledcWrite(pwm_pin, i);
      delay(20 / portTICK_PERIOD_MS);
    }
  } else {
    digitalWrite(enable_motor_pin, LOW);
  }

  while (1) {
    // cấp điện động cơ
    if (motor.is_ready == 1) {
      digitalWrite(enable_motor_pin, HIGH);
      //Khởi động mềm
      if (motor.is_running == 1 && motor.last_motor_state == 0) {
        led.led_yellow_state = 1;
        motor.last_motor_state = 1;
        for (int i = 0; i <= 255; i++) {
          ledcWrite(pwm_pin, i);
          vTaskDelay(20 / portTICK_PERIOD_MS);
        }
      }
      // tắt dần mềm
      else if (motor.is_running == 0 && motor.last_motor_state == 1) {
        led.led_yellow_state = 0;
        motor.last_motor_state = 0;
        for (int i = 255; i >= 0; i--) {
          ledcWrite(pwm_pin, i);
          vTaskDelay(20 / portTICK_PERIOD_MS);
        }
      }
      // duy trì trạng thái bật
      else if (motor.is_running == 1 && motor.last_motor_state == 1) {
        led.led_yellow_state = 1;
        ledcWrite(pwm_pin, 255);
      }
      // duy trì trạng thái tắt
      else if (motor.is_running == 0 && motor.last_motor_state == 0) {
        led.led_yellow_state = 0;
        ledcWrite(pwm_pin, 0);
      }
    }
    // ngắt điện động cơ
    else {
      motor.last_motor_state = 0;
      motor.is_running = 0;
      ledcWrite(pwm_pin, 0);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      digitalWrite(enable_motor_pin, LOW);
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

int getSensorValue(uint8_t slaveID, uint16_t reg, uint8_t reTries) {
  node.begin(slaveID, Serial2);
  for (uint8_t t = 0; t < reTries; t++) {
    if (node.readHoldingRegisters(reg, 1) == node.ku8MBSuccess) {
      uint16_t value = node.getResponseBuffer(0);
      node.clearResponseBuffer();
      // Serial.println(value);
      return value;
    } else {
      // Serial.print("Retry: ");
      // Serial.println(t + 1);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
  return 0;
}

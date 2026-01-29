/* FILE: Config.h */
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- HARDWARE PINOUT (SƠ ĐỒ CHÂN) ---
#define PIN_PWM1    22  // Động cơ
#define PIN_PWM2    23
#define PIN_ENABLE  21
#define PIN_ENC_A   18  // Encoder
#define PIN_ENC_B   19
#define PIN_RX      16  // Modbus Laser
#define PIN_TX      17
#define PIN_BUTTON  32  // Nút nhấn (Start/Stop/Reverse)
#define PIN_BUZZER  33  // Còi
#define PIN_LED_R   27  // Led Đỏ (Stop)
#define PIN_LED_G   26  // Led Xanh (Run)

// --- MODBUS CONFIG ---
#define MODBUS_ID   0x50
#define MODBUS_REG  0x34

// --- PHYSICS (VẬT LÝ & CƠ KHÍ) ---
const float GEAR_RATIO     = 1.2;
const float ENCODER_PPR    = 1152.0;
// Tổng xung cho 1 vòng thanh Beam
const float PULSES_PER_REV = ENCODER_PPR * 4.0 * GEAR_RATIO;
// Hệ số chuyển đổi Xung -> Độ
const float PULSE_TO_DEG   = PULSES_PER_REV / 360.0;

// Kích thước thanh & Điểm cân bằng
const float BEAM_LENGTH = 40.0; // Thanh dài 40cm
const float SETPOINT_X  = 20.0; // Điểm giữa là 20cm

// --- SAFETY (AN TOÀN) ---
const float SAFETY_DIST = 45.0; // > 45cm là mất bóng -> STOP
const float MAX_TILT    = 8.0;  // Chỉ nghiêng +/- 8 độ (Chiến thuật nhẹ nhàng)
const int   MAX_PWM     = 180;  // Giới hạn tốc độ motor
const int   PWM_MIN     = 0;    // Ma sát thấp -> Không cần bù
const int   LONG_PRESS_MS = 2000; // Giữ 2s để đảo chiều

// --- CALIBRATION ---
// Chỉnh số này nếu Angle=0 mà thanh vẫn bị lệch cơ khí
// Dương (+) là nâng đầu motor, Âm (-) là hạ xuống
const float ANGLE_OFFSET = 0.0; 

// --- PID TUNING (TINH CHỈNH) ---
// Vòng Ngoài (Position): Tăng D để phanh sớm
const float KP_X = 2.0;
const float KI_X = 0.01;
const float KD_X = 2.5;

// Vòng Trong (Angle): Giữ cứng vững
const float KP_TH = 15.0;
const float KI_TH = 0.5;
const float KD_TH = 10.0;

// Filter LPF (Lọc nhiễu Laser)
const float LPF_ALPHA = 0.15;

#endif
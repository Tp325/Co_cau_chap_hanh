/* FILE: Config.h */
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- HARDWARE PINOUT (GIỮ NGUYÊN) ---
#define PIN_PWM1    22  
#define PIN_PWM2    23
#define PIN_ENABLE  21
#define PIN_ENC_A   18  
#define PIN_ENC_B   19
#define PIN_RX      16  
#define PIN_TX      17
#define PIN_BUTTON  32  
#define PIN_BUZZER  33  
#define PIN_LED_R   27  
#define PIN_LED_G   26  

// --- MODBUS CONFIG ---
#define MODBUS_ID   0x50
#define MODBUS_REG  0x34

// --- PHYSICS (QUAN TRỌNG: CHECK GEAR RATIO) ---
// BẠN CẦN LÀM BÀI TEST 90 ĐỘ ĐỂ CHẮC CHẮN SỐ 1.2 NÀY ĐÚNG
const float GEAR_RATIO     = 1.2; 
const float ENCODER_PPR    = 1152.0;
const float PULSES_PER_REV = ENCODER_PPR * 4.0 * GEAR_RATIO;
const float PULSE_TO_DEG   = PULSES_PER_REV / 360.0;

// Kích thước & Setpoint
const float BEAM_LENGTH = 50.0; 
const float SETPOINT_X  = 17.0;

// --- SAFETY ---
const float SAFETY_DIST = 38.0; 
const float MAX_TILT    = 4.0; // Giữ nguyên để có không gian xử lý

// [THAY ĐỔI 1] Tăng full công suất để phanh cho ăn
const int   MAX_PWM     = 255;  // Tăng từ 180 lên 255 (Max lực)
const int   PWM_MIN     = 40;    
const int   LONG_PRESS_MS = 2000;

const float ANGLE_OFFSET = 0.0; 

// --- PID TUNING (ĐIỀU TRỊ DAO ĐỘNG) ---

// Vòng Ngoài (Vị trí): GIẢM GA, GIỮ PHANH
const float KP_X = 2.5;
const float KI_X = 0.0;  // Giữ nhỏ
const float KD_X = 25.0;  

// Vòng Trong (Góc): TĂNG ĐỘ CỨNG
// Motor cần phản ứng nhanh hơn để kịp cứu bóng
const float KP_TH = 15.0; // Tăng từ 15 lên 25 (Cứng hơn)
const float KI_TH = 0.05;
const float KD_TH = 15.0;

// Filter LPF
const float LPF_ALPHA = 0.2; 

#endif
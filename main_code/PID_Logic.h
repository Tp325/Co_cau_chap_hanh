#ifndef PID_LOGIC_H
#define PID_LOGIC_H

#include <Arduino.h>

// Cấu trúc tham số PID
struct PID_PARAM {
    double Kp, Ki, Kd;
    double integral_err = 0;
    double last_err = 0;
};

// Khai báo prototype các hàm tính toán
double computePID_Pos(PID_PARAM &pid, double setpoint, double input, double dt);
double computePID_Speed(PID_PARAM &pid, double target, double current, double dt);

#endif
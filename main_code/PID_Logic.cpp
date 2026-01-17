#include "PID_Logic.h"

// PID Vị trí (Khoảng cách sang RPM)
double computePID_Pos(PID_PARAM &pid, double setpoint, double input, double dt) {
    // setpoint - input (Để tạo phản hồi âm)
    double error = setpoint - input;                  // không chắc nhưng mà thấy nó ngược thì đảo cái này
    
    double P = pid.Kp * error;
    
    pid.integral_err += error * dt;
    // Anti-windup
    if (pid.integral_err > 20) pid.integral_err = 20;
    else if (pid.integral_err < -20) pid.integral_err = -20;
    
    double I = pid.Ki * pid.integral_err;

    double D = pid.Kd * ((error - pid.last_err) / dt);
    pid.last_err = error;

    return P + I + D;
}

// PID Tốc độ (RPM sang PWM)
double computePID_Speed(PID_PARAM &pid, double target, double current, double dt) {
    double error = target - current;
    double P = pid.Kp * error;
    
    pid.integral_err += error * dt;
    // Anti-windup cho hệ 8-bit
    double max_I = 255.0 / pid.Ki;
    if (pid.integral_err > max_I) pid.integral_err = max_I;
    else if (pid.integral_err < -max_I) pid.integral_err = -max_I;
    
    double I = pid.Ki * pid.integral_err;

    double D = pid.Kd * ((error - pid.last_err) / dt);
    pid.last_err = error;

    return P + I + D;
}
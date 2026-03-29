% =========================================================
% SYSTEM SIMULATION: BALL AND BEAM
% Author: NhatDang47 & Tp325
% =========================================================
clear; clc; close all;

% --- 1. THÔNG SỐ ---
% Tỷ lệ chuyển đổi đơn vị
deg2rad = pi/180;
rad2deg = 180/pi;

% Vòng ngoài (Position - cm)
KP_X = 4.0;
KI_X = 0.0;
KD_X = 4.0;
DEADBAND_X = 0.2;

% Vòng trong (Angle - deg)
KP_TH = 2.4;
KI_TH = 0.05;
KD_TH = 0.6;

% Vật lý
K_SYS = 588.6; % cm/s^2
MAX_TILT = 20.0; % độ
SETPOINT_X = 25.0; % cm

% Thông số động cơ (Ước lượng cho Gear Motor 12V)
R = 5.0; L = 0.01; Kt = 1.5; Ke = 0.5; J = 0.05; b = 0.1;
m_ball = 0.05; % kg
g = 9.81;

% --- 2. CẤU HÌNH THỜI GIAN ---
dt_sim = 0.0001; % Thế giới thực (10kHz)
dt_inner = 0.01; % Vòng lặp Góc 100Hz (10ms)
dt_outer = 0.05; % Vòng lặp Vị trí ~20Hz (50ms)

t_end = 4.0; 
t = 0:dt_sim:t_end;
N = length(t);

% --- 3. KHỞI TẠO TRẠNG THÁI ---
x = zeros(1, N); v = zeros(1, N); % Bóng
theta = zeros(1, N); omega = zeros(1, N); curr = zeros(1, N); % Motor

% Biến PID
e_x_prev = 0; E_x_sum = 0;
e_th_prev = 0; E_th_sum = 0;
targetAngle = 0; 
V_cmd = 0;

% Vị trí ban đầu
x(1) = 5.0; % Bóng bắt đầu ở vạch 5cm

% --- 4. VÒNG LẶP MÔ PHỎNG (DISCRETE + ANALOG) ---
for k = 1:(N-1)
    
    % --- VÒNG NGOÀI (20Hz) ---
    if mod(k-1, round(dt_outer/dt_sim)) == 0
        ex = SETPOINT_X - x(k);
        
        % Tính PD Vị trí
        E_x_sum = E_x_sum + ex * dt_outer;
        edot_x = (ex - e_x_prev) / dt_outer;
        e_x_prev = ex;
        
        a_req = KP_X * ex + KI_X * E_x_sum + KD_X * edot_x;
        
        % Tuyến tính hóa hồi tiếp (ArcSin)
        ratio = a_req / K_SYS;
        ratio = max(min(ratio, 0.99), -0.99);
        targetAngle = asin(ratio) * rad2deg;
        targetAngle = max(min(targetAngle, MAX_TILT), -MAX_TILT);
    end

    % --- VÒNG TRONG (100Hz) ---
    if mod(k-1, round(dt_inner/dt_sim)) == 0
        e_th = targetAngle - theta(k);
        
        E_th_sum = E_th_sum + e_th * dt_inner;
        edot_th = (e_th - e_th_prev) / dt_inner;
        e_th_prev = e_th;
        
        % Lệnh điện áp (Ánh xạ PWM sang Voltage)
        V_cmd = KP_TH * e_th + KI_TH * E_th_sum + KD_TH * edot_th;
        V_cmd = max(min(V_cmd, 12.0), -12.0);
    end

    % --- VẬT LÝ HỆ THỐNG (10kHz) ---
    % 1. Động lực học Motor & Nhiễu tải
    % Nhiễu do trọng lượng quả bóng đè lên thanh
    dist_torque = (m_ball * g * (x(k)/100) * cos(theta(k)*deg2rad)) * 0.1; 

    di = (V_cmd - R*curr(k) - Ke*omega(k)*deg2rad) / L;
    curr(k+1) = curr(k) + di * dt_sim;
    
    dw = (Kt*curr(k+1) - b*omega(k)*deg2rad - dist_torque) / J;
    omega(k+1) = omega(k) + (dw * rad2deg) * dt_sim;
    theta(k+1) = theta(k) + omega(k+1) * dt_sim;
    
    % 2. Động lực học quả bóng lăn
    a_ball = K_SYS * sin(theta(k+1) * deg2rad); 
    v(k+1) = v(k) + a_ball * dt_sim;
    x(k+1) = x(k) + v(k+1) * dt_sim;
end

% --- 5. HIỂN THỊ KẾT QUẢ ---
figure('Name', 'Ball and Beam Cascade Control Simulation');

subplot(2,1,1);
plot(t, x, 'b', 'LineWidth', 2); hold on;
yline(SETPOINT_X, 'r--', 'Setpoint');
title('Vị trí quả bóng (Outer Loop Response)');
ylabel('Vị trí (cm)'); grid on;

subplot(2,1,2);
plot(t, theta, 'g', 'LineWidth', 1.5); hold on;
plot(t, repmat(targetAngle, 1, length(t)), 'k--');
title('Góc nghiêng thanh Beam (Inner Loop Response)');
ylabel('Góc (Độ)'); xlabel('Thời gian (s)'); grid on;
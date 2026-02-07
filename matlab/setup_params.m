% === FILE: setup_params.m ===
clear; clc;

% 1. Thông số Vật Lý (Physics)
g = 9.81;                   % Gia tốc trọng trường (m/s^2)
m = 0.05;                   % Khối lượng bóng (kg) - ví dụ 50g
r = 0.015;                  % Bán kính bóng (m) - ví dụ 1.5cm
J = (2/5) * m * r^2;        % Mô-men quán tính (Bóng đặc)

% Hằng số hệ thống (System Constant)
% Phương trình động lực học: (m + J/r^2) * x_ddot = m * g * sin(theta)
% => x_ddot = K_system * theta (với theta nhỏ)
K_system = (m * g) / (m + J/r^2); 

% 2. Giới hạn (Constraints)
MAX_TILT_DEG = 20.0;        % Góc nghiêng tối đa (độ)
BEAM_LENGTH = 0.5;          % Chiều dài thanh dầm (m)

% 3. Bộ PID (Lấy từ code ESP32 của ông)
Kp = 4.5;
Ki = 0.01;
Kd = 60.0;

% 4. Sample Time (Giả lập tốc độ vòng lặp ESP32)
Ts = 0.01; % 10ms (giống xFrequency trong TaskControl)
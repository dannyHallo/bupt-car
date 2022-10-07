#pragma once
// paraments change frequently

const int serial_btr = 115200;

const float aim_speed = 0.5;

// angle pid
const float angle_kp = 1;
const float angle_ki = 0.01;
const float angle_kd = 0.1;

// speed pid
const float speed_kp = 1e5;
const float speed_ki = 1e4;
const float speed_kd = 1e4;
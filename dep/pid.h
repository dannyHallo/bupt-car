#pragma once

#include "math.h"

/// @brief this class aims to give user a higher lever control over the basic pid algorithm
class pid {
public:
  pid(float kp, float ki, float kd) {
    Kp = kp;
    Ki = ki;
    Kd = kd;
  }

  float update(float error, float t = 1) {
    P         = error;
    I         = I + error;
    D         = error - previous_error;
    PID_value = ((Kp * P) + (Ki * I) + (Kd * D)) * t;

    previous_error = error;
    return PID_value;
  }

  void reset() {
    I              = 0;
    previous_error = 0;
  }

private:
  float Kp = 0.9f, Ki = 0.1, Kd = 0.1;
  float P = 0, I = 0, D = 0, PID_value = 0;
  float previous_error = 0;
};
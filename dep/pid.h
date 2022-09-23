#pragma once

float Kp = 0.4f, Ki = 0.2f, Kd = 0;
float P = 0, I = 0, D = 0, PID_value = 0, error = 0;
float previous_error = 0, previous_I = 0;

int getPID(float error) {
  if (previous_error > 0 && error < 0 || previous_error < 0 && error > 0) {
    I = 0;
  }

  P         = error;
  I         = I + error;
  D         = error - previous_error;
  PID_value = (Kp * P) + (Ki * I) + (Kd * D);
  clamp(PID_value, -64.0f, 64.0f);

  previous_error = error;
  return customRound(PID_value);
}
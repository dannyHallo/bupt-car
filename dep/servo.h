#pragma once

#include "math.h"
#include "pinouts.h"

const float cAngleLimit    = 42.0f; // Max: 90 degrees
const float cBias          = 2.0f;
const int cServoResolution = 16; // Max: 16 bit

void servoWriteAngle(float angle);

// init pinout, pwm
void initServo() {
  ledcSetup(0, 50, cServoResolution); // Channel 0, 50 Hz, 16 bit resolution
  ledcAttachPin(PINOUT_SERVO, 0);     // Attach servo pin to channel 0
  servoWriteAngle(0);
}

void servoWriteAngle(float angle) {
  angle += cBias;

  clamp(angle, cBias - cAngleLimit, cBias + cAngleLimit);

  // Steering -> Servo angle mapping
  if (angle < 0) {
    map(angle, -42.0f, 0.0f, -47.0f, 0.0f);
  } else {
    map(angle, 0.0f, 42.0f, 0.0f, 80.0f);
  }
  float t = map(angle, -90.0f, 90.0f, 0.5f, 2.5f);
  ledcWrite(0, (t / 20.0f) * ((1 << cServoResolution) - 1));
}

void servoWritePixel(int trackMidPoint) {
  servoWriteAngle(map(float(trackMidPoint), 0.0f, 128.0f, -cAngleLimit, cAngleLimit));
}
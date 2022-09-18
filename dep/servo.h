#pragma once

#include "pinouts.h"
#include "math.h"

const float cAngleLimit = 45.0f; // Max: 90 degrees
const float cBias = 5.0f;
const int cServoResolution = 16; // Max: 16 bit

void servoWriteAngle(float angle);

void pinoutAndPwmChannelInitServo() {
    ledcSetup(0,50,cServoResolution); // Channel 0, 50 Hz, 16 bit resolution
    ledcAttachPin(PINOUT_SERVO,0);     // Attach servo pin to channel 0
    servoWriteAngle(0);
}

void servoWriteAngle(float angle) {
    angle += cBias;

    clamp(angle,cBias-cAngleLimit,cBias+cAngleLimit);

    float t = map(angle,-90.0f,90.0f,0.5f,2.5f);

    ledcWrite(0,(t/20.0f)*((1<<cServoResolution)-1));
}

void servoLoop(int trackMidPoint) {
    servoWriteAngle(map(float(softmax(trackMidPoint)),0.0f,128.0f,-cAngleLimit,cAngleLimit));
}
#pragma once

#include "pinouts.h"
#include "math.h"

const float cAngleLimit = 45.0f; // Max: 90 degrees
const float cBias = 5.0f;
const int cResolution = 16; // Max: 16 bit

void pinoutAndPwmChannelInit()
{
    ledcSetup(0, 50, cResolution);  // Channel 0, 50 Hz, 16 bit resolution
    ledcAttachPin(PINOUT_SERVO, 0); // Attach servo pin to channel 0
}

void servoToAngle(float angle)
{
    angle += cBias;

    clamp(angle, cBias - cAngleLimit, cBias + cAngleLimit);

    float t = map(angle, -90.0f, 90.0f, 0.5f, 2.5f);

    ledcWrite(0, (t / 20.0f) * ((1 << cResolution) - 1));
}

void servoLoop()
{
    // // increase the LED brightness
    // for (float angle = -cAngleLimit; angle <= cAngleLimit; angle += 0.2f)
    // {
    //     servoToAngle(angle);
    //     delay(10);
    // }

    // // increase the LED brightness
    // for (float angle = cAngleLimit; angle >= -cAngleLimit; angle -= 0.2f)
    // {
    //     servoToAngle(angle);
    //     delay(10);
    // }

    servoToAngle(-cAngleLimit);
    delay(1000);

    servoToAngle(0);
    delay(2000);

    servoToAngle(cAngleLimit);
    delay(3000);
}
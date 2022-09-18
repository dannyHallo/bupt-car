#pragma once

#include "allCommands.h"
#include "motor.h"
#include "servo.h"
#include "math.h"

void parseCommands(int command)
{
    if (command == -1)
        return;

    // Turning commands
    if (command >= BIAS_TURNING_START)
    {
        servoWriteAngle(map(float(command - BIAS_TURNING_START), 0.0f, 180.0f, -cAngleLimit, cAngleLimit));
    }
}
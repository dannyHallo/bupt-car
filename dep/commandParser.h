#pragma once

#include "allCommands.h"
#include "math.h"
#include "motor.h"
#include "servo.h"

const float cServoSensitivity = 2.2f;
const float cDefaultThrottle  = 30000.0f;

bool boost = false;

// the commands are sent from the remote side, by an Android application developed by us.
void parseCommands(int command) {
  if (command == -1)
    return;

  Serial.print("Command: ");
  Serial.println(command);

  switch (command) {
  case COMMAND_BOOST:
    boost = true;
    break;
  case COMMAND_BOOST_CANCEL:
    boost = false;
    break;
  }

  if (command >= BIAS_POWER_LEVEL_START && command <= BIAS_POWER_LEVEL_END) {
    int commandParsed = command - BIAS_POWER_LEVEL_START;

    // Motor start
    if (commandParsed % 2 == 0) {
      float throttle = boost ? cDefaultThrottle * 2 : cDefaultThrottle;

      switch (commandParsed) {
      case COMMAND_POWER_LEVEL_0_ACTIVE:
        motorControl(false, false, throttle, throttle);
        break;
      case COMMAND_POWER_LEVEL_1_ACTIVE:
        motorBrake();
        break;
      case COMMAND_POWER_LEVEL_2_ACTIVE:
        motorControl(true, true, throttle, throttle);
        break;
      }
    }
    // Motor stop
    else {
      motorIdle();
    }

    return;
  }

  // the steering commands are linear values, but with a bias, so we should cancel the bias to
  // retrieve for the original angle
  if (command >= BIAS_TURNING_START) {
    int commandParsed = command - BIAS_TURNING_START; // 0 -> 180
    commandParsed -= 90;                              // -90 -> 90
    commandParsed = customRound(float(commandParsed) * cServoSensitivity);
    clamp(commandParsed, -90, 90);

    servoWriteAngle(map(float(commandParsed), -90.0f, 90.0f, -cAngleLimit, cAngleLimit));
    return;
  }
}
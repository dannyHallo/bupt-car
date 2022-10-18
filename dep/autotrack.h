#pragma once

#include "../args.h"
#include "bluetooth.h"
#include "boardLed.h"
#include "ccd.h"
#include "color.h"
#include "commandParser.h"
#include "data.h"
#include "motor.h"
#include "oled.h"
#include "pid.h"
#include "pinouts.h"
#include "servo.h"

int location = 0;
pid angelPID(angle_kp, angle_ki, angle_kd);
bt_data data;

bool autoTrack(explosureRecord& bestRecord, int bestExplosureTime, int substractedExplosureTime,
               bool initStarting) {
  // motor_on pin is a debug pin, as mentioned in the main loop
  bool motorEnable = digitalRead(PINOUT_MOTOR_ON) ? true : false;
  // motorAimSpeed is read from the car configuration file, thus we can change its value freely
  // later this function
  float motorAimSpeed = motorEnable ? aim_speed : 0;

  int trackMidPixel = 0;
  int trackStatus   = 0;

  // read the raw values from ccd
  if (initStarting) {
    // we will clear all the previous explosure values and do explosuring another time, this is time
    // consuming but accurate in vaule readings, for we can fine tune the exactly explosuring time
    processCCD(trackMidPixel, trackStatus, bestExplosureTime, true, false);
  } else {
    // the ccd explosuring value is not cleared, and will be reused. if we want to use this value,
    // we have to adjust the explosuring time due to the excecution time in the last loop
    processCCD(trackMidPixel, trackStatus, substractedExplosureTime, false, false);
  }

  // switch for all the status to print onto the oled screen
  switch (trackStatus) {
    // if the car is in normal tracking state
  case STATUS_NORMAL:
    boardLedOff();
    oledPrint("TRACKING", 1);
    break;
    // if the car cannot see the track...
  case STATUS_NO_TRACK:
    boardLedOff();
    oledPrint("!!!NOTRACK", 1);
    break;
    // if a platform is detected
  case STATUS_PLATFORM:
    boardLedOn();
    oledPrint("!!!PLATFORM", 1);
    break;
  }

  // this time we will actually control how the car moves
  switch (trackStatus) {
    // during the normal tracking status, the car will steer its wheel according to the mid pixel of
    // the ccd sensor, with the help of a fine-tuned pid controller. the car will move forward
  case STATUS_NORMAL:
    servoWritePixel(angelPID.update(trackMidPixel - 64) + 64);
    motorForward(motorAimSpeed);
    break;
    // when the platform is first detected:
  case STATUS_PLATFORM:
    // the car will steer its wheel back to the center, since the prev mid pixel data is not always
    // a good value to go. the car will stop
    servoWritePixel(64);
    motorBrake();

    // print the location out to oled screen
    display.clearDisplay();
    oledPrint(++location, "Location", 1);
    oledFlush();

    delay(1000);

    // capture the color of the goods
    colorSensorOn();
    oledCountdown("Capturing", 600, 1);
    color = getRGB();
    colorSensorOff();

    // pack the data to data class
    data.set_cargo(location, color);

    // send out the data(debugging puporse)
    btSend("Platform Reached");
    Serial.println("Platform Reached");

    // the full circle has been tracked, the car will communicate to the pc, also by bluetooth
    if (location % platform_num == 0) {
      Serial.println("New Circle");
      data.set_count(location / platform_num);
      btSend(data.encode()); // the location / color data will be encoded to a custom format
    }

    delay(2000);

    // detection ended, move forward, and return to normal tracking mode after a valid track has
    // been appeared
    motorForward(motorAimSpeed / 3);
    processCCD(trackMidPixel, trackStatus, bestExplosureTime, true, false);
    for (;;) {
      processCCD(trackMidPixel, trackStatus, bestExplosureTime, true, false);
      oledFlush();
      if (trackStatus == STATUS_NORMAL)
        return true;
    }

    // this special case is designed for error handling, but it is mostly useless in practice
  default:
    angelPID.reset();
    servoWritePixel(64);
    motorBrake();
    // the car will eventually enter this endless blinking loop and refuse to move any how, so we
    // can know there is something wrong
    for (;;) {
      flipBoardLed();
      delay(1000);
    }
    break;
  }

  return false;
}
#pragma once

#include "../args.h"
#include "bluetooth.h"
#include "boardLed.h"
#include "ccd.h"
#include "color.h"
#include "commandParser.h"
#include "motor.h"
#include "oled.h"
#include "pid.h"
#include "pinouts.h"
#include "servo.h"
#include "data.h"


int location = 0;
pid angelPID(angle_kp,angle_ki,angle_kd);
bt_data data;

bool autoTrack(explosureRecord& bestRecord,int bestExplosureTime,int substractedExplosureTime,
    bool initStarting) {
    bool motorEnable = digitalRead(PINOUT_MOTOR_ON) ? true : false;
    float motorAimSpeed = motorEnable ? aim_speed : 0;

    int trackMidPixel = 0;
    int trackStatus = 0;

    if (initStarting) {
        processCCD(trackMidPixel,trackStatus,bestExplosureTime,true,false);
    } else {
        processCCD(trackMidPixel,trackStatus,substractedExplosureTime,false,false);
    }

    // Print status only
    switch (trackStatus) {
    case STATUS_NORMAL:
        boardLedOff();
        oledPrint("TRACKING",1);
        break;
    case STATUS_NO_TRACK:
        boardLedOff();
        oledPrint("!!!NOTRACK",1);
        break;
    case STATUS_PLATFORM:
        boardLedOn();
        oledPrint("!!!PLATFORM",1);
        break;
    }

    switch (trackStatus) {
    case STATUS_NORMAL:
        servoWritePixel(angelPID.update(trackMidPixel-64)+64);
        motorForward(motorAimSpeed);
        break;

    case STATUS_PLATFORM:
        servoWritePixel(64);
        motorBrake();

        display.clearDisplay();
        oledPrint(++location,"Location",1);
        oledFlush();

        delay(1000);
        colorSensorOn();
        oledCountdown("Capturing",600,1);
        color = getRGB();
        colorSensorOff();

        data.set_cargo(location,color);

        btSend("Platform Reached");
        Serial.println("Platform Reached");

        if (location%platform_num==0) {
            Serial.println("New Circle");
            data.set_count(location/platform_num);
            btSend(data.encode());
        }

        delay(2000);

        motorForward(motorAimSpeed/3);
        processCCD(trackMidPixel,trackStatus,bestExplosureTime,true,false);
        for (;;) {
            processCCD(trackMidPixel,trackStatus,bestExplosureTime,true,false);
            oledFlush();
            if (trackStatus==STATUS_NORMAL)
                return true;
        }

        // TODO: error handling
    default:
        angelPID.reset();
        servoWritePixel(64);
        motorBrake();
        for (;;) {
            flipBoardLed();
            delay(1000);
        }
        break;
    }

    return false;
}
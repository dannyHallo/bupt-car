#include "args.h"
#include "dep/bluetooth.h"
#include "dep/boardLed.h"
#include "dep/ccd.h"
#include "dep/color.h"
#include "dep/commandParser.h"
#include "dep/motor.h"
#include "dep/oled.h"
#include "dep/pid.h"
#include "dep/pinouts.h"
#include "dep/servo.h"
#include "dep/data.h"
#include "dep/autotrack.h"

TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

int command = -1;

void setup() {
    Serial.begin(serial_btr);
    Serial.println("-- startup --");

    pinoutInitBoardLed();
    initOled();
    initColor();
    initCCD();
    initServo();
    initMotor();
    initBluetooth();
    pinMode(PINOUT_MOTOR_ON,INPUT_PULLDOWN);

    oledCountdown("Booting",200,1);
    assignTasks();
}

void assignTasks() {
    xTaskCreatePinnedToCore(Task1,        // Task function
        "Task1",      // Task name
        2000,         // Stack size
        NULL,         // Parameter
        1,            // Priority
        &Task1Handle, // Task handle to keep track of created task
        0             // Core ID: 0:
    );

    xTaskCreatePinnedToCore(Task2,        // Task function
        "Task2",      // Task name
        2000,         // Stack size
        NULL,         // Parameter
        1,            // Priority
        &Task2Handle, // Task handle to keep track of created task
        1             // Core ID: 0:
    );
}



// this loop is intentionally left blank
void loop() { delay(1000); }

void Task1(void* pvParameters) {
    for (;;) {
        //   command = btRecieve();
        //   delay(20);
        delay(1000);
    }
}

void prepareCCD(bool& cameraIsBlocked,bool& recordAvailable,explosureRecord& bestRecord) {

    // while (true) {
    //     btSend("Hello!");
    //     Serial.println("Hello Sent!");
    //     delay(1000);
    // }

    display.clearDisplay();
    cameraIsBlocked = false;

    getBestExplosureTime(bestRecord,cameraIsBlocked,true);
    recordAvailable = bestRecord.isValid;

    Serial.println("----------------------------------------");
    if (cameraIsBlocked) {
        Serial.println("bluetooth mode activated");
    } else {
        Serial.println("tracking mode activated");
    }

    if (recordAvailable) {
        Serial.print("best explosure time: ");
        Serial.print(bestRecord.explosureTime);
        Serial.print(" with parting avg: ");
        Serial.println(bestRecord.avgVal);
    } else {
        Serial.println("no available record");
    }

    Serial.println("----------------------------------------");

    oledPrint(bestRecord.explosureTime,"expl",0);
    oledPrint(bestRecord.avgVal,"parting avg",1);
    oledFlush();
    delay(2000);
    display.clearDisplay();
}

int loopTime = 0;
int prevTime = 0;

int getTime() {
    loopTime = millis();
    int prevTimeMs = (prevTime==0) ? -1 : loopTime-prevTime;
    prevTime = loopTime;

    return prevTimeMs;
}

void Task2(void* pvParameters) {
    colorSensorOn();
    setupBlankColor();

    //   for (;;) {
    //     btSend(123);
    //     btSend("hello!");
    //     flipBoardLed();
    //     delay(1000);
    //   }

    bool cameraIsBlocked,recordAvailable,returnFromPlatform;
    explosureRecord bestRecord;
    prepareCCD(cameraIsBlocked,recordAvailable,bestRecord);

    if (cameraIsBlocked) {
        oledPrint("BT MODE",1);
        oledFlush();

        for (;;) {
            parseCommands(command);
        }
    } else {
        oledPrint("TRACK MODE",1);
        oledFlush();
        delay(1000);

        for (;;) {
            display.clearDisplay();

            int prevTimeMs = getTime();
            bool noTimeRecord = (prevTimeMs==-1);
            int substractedExplosureTime = 0;
            if (!noTimeRecord) {
                substractedExplosureTime -= prevTimeMs;
                if (substractedExplosureTime<0)
                    substractedExplosureTime = 0;

                // Frame length, proper explosure time
                oledPrint("std",bestRecord.explosureTime,"frm",prevTimeMs,0);
            }

            returnFromPlatform = autoTrack(bestRecord,bestRecord.explosureTime,substractedExplosureTime,
                noTimeRecord||returnFromPlatform);
            oledFlush();
        }
    }
}
// #define BT_ON

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
#include "args.h"

TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

int command = -1;

void printColorToRow(int row = 0) {
    switch (getColor()) {
    case COLOR_RED:
        oledPrint("RED",row);
        break;
    case COLOR_YELLOW:
        oledPrint("YELLOW",row);
        break;
    case COLOR_PINK:
        oledPrint("PINK",row);
        break;
    case COLOR_WHITE:
        oledPrint("WHITE",row);
        break;
    case COLOR_BLACK:
        oledPrint("BLACK",row);
        break;
    case COLOR_GREEN:
        oledPrint("GREEN",row);
        break;
    case COLOR_DARK_BLUE:
        oledPrint("DARK_BLUE",row);
        break;
    case COLOR_BLUE:
        oledPrint("BLUE",row);
        break;
    }
}

pid angelPID(angle_kp,angle_ki,angle_kd);

void setup() {
    Serial.begin(serial_btr);

    pinoutInitBoardLed();
    initColor();
    initCCD();
    initServo();
    initMotor();
    initBluetooth();
    initOled();

    assignTasks();

    pinMode(PINOUT_MOTOR_ON,INPUT_PULLDOWN);
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

void autoTrack(int bestExplosureTime,float minThrehold) {
    bool motorEnable = digitalRead(PINOUT_MOTOR_ON) ? true : false;

    int trackMidPixel = 0;
    float usingThrehold = 0;
    int trackStatus = 0;

    bool isPlatform = false;

    processCCD(trackMidPixel,usingThrehold,trackStatus,bestExplosureTime,minThrehold,true);

    switch (trackStatus) {
    case STATUS_NORMAL:
        // Show status
        boardLedOff();

        // Get val
        servoWritePixel(angelPID.update(trackMidPixel-64)+64);
        break;

    case STATUS_HIGH_DL:
    case STATUS_PLATFORM:
        boardLedOn();
        motorIdle();
        vTaskDelay(200);
        printColorToRow();
        vTaskDelay(2000);
        motorForward(aim_speed/2);
        vTaskDelay(500);

        break;
    case STATUS_NO_TRACK:
        boardLedOn();

        if (trackStatus==STATUS_HIGH_DL)
            oledPrint("!highratio",1);
        if (trackStatus==STATUS_NO_TRACK)
            oledPrint("!notrack",1);

        servoWritePixel(127);
        break;
    }

    if (motorEnable) {
        motorForward(aim_speed);
    } else {
        motorIdle();
    }
}

// this loop is intentionally left blank
void loop() { delay(1000); }

void Task1(void* pvParameters) {
    for (;;) {
        if (Serial.available()) {
            btSend(Serial.read());
        }

        command = btRecieve();
        vTaskDelay(20);
    }
}


void mainLoop1() {
    display.clearDisplay();

    bool cameraIsBlocked = false;
    bool bestAvailable = false;

    explosureRecord bestRecord;
    getBestExplosureTime(bestRecord,cameraIsBlocked,bestAvailable,true);

    Serial.println("----------------------------------------");
    if (cameraIsBlocked) {
        Serial.print("camera blocked, ");
        Serial.println("bluetooth mode activated");
    } else {
        Serial.print("camera available, ");
        Serial.println("tracking mode activated");
    }

    if (bestAvailable) {
        Serial.print("Best explosure time: ");
        Serial.print(bestRecord.explosureTime);
        Serial.print(" with minimum ratio: ");
        Serial.println(bestRecord.threhold);
    } else {
        Serial.println("No available record");
    }

    Serial.println("----------------------------------------");

    oledPrint(bestRecord.explosureTime,"expl",0);
    oledPrint(bestRecord.threhold,"thre",1);
    oledFlush();
    delay(2500);
    display.clearDisplay();

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

            printColorToRow(0);
            autoTrack(bestRecord.explosureTime,bestRecord.threhold);
            oledFlush();
        }
    }
}

void colorLoop() { getColor(); }

void Task2(void* pvParameters) {
    // Greeting from core 1
    display.clearDisplay();
    oledPrint("Hello!",0);
    oledFlush();

    Serial.println("Hello!");

    for (;;) {
        // colorLoop();
        mainLoop1();
    }
}
#include "args.h"
#include "dep/autotrack.h"
#include "dep/bluetooth.h"
#include "dep/boardLed.h"
#include "dep/ccd.h"
#include "dep/color.h"
#include "dep/commandParser.h"
#include "dep/data.h"
#include "dep/motor.h"
#include "dep/oled.h"
#include "dep/pid.h"
#include "dep/pinouts.h"
#include "dep/servo.h"

TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

int command = -1;

// overall setup
void setup() {
  // start serial communication
  Serial.begin(serial_btr);
  Serial.println("-- startup --");

  // peripherals initialization
  pinoutInitBoardLed();
  initOled();
  initColor();
  initCCD();
  initServo();
  initMotor();
  initBluetooth();

  pinMode(PINOUT_MOTOR_ON, INPUT_PULLDOWN); // debug pin, detatch this pin will disable the motor
  oledCountdown("Booting", 200, 1);         // oled testing function
  assignTasks();                            // assign tasks for two cores
}

// assign tasks for two cores
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

void prepareCCD(bool& cameraIsBlocked, bool& recordAvailable, explosureRecord& bestRecord) {
  display.clearDisplay();
  cameraIsBlocked = false;

  getBestExplosureTime(bestRecord, cameraIsBlocked, true);
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

  oledPrint(bestRecord.explosureTime, "expl", 0);
  oledPrint(bestRecord.avgVal, "parting avg", 1);
  oledFlush();
  delay(2000);
  display.clearDisplay();
}

int loopTime = 0;
int prevTime = 0;

int getTime() {
  loopTime       = millis();
  int prevTimeMs = (prevTime == 0) ? -1 : loopTime - prevTime;
  prevTime       = loopTime;

  return prevTimeMs;
}

bool connected = false;

void Task2(void* pvParameters) {
  // turn the color sensor on and setup the blank color, since the initial lighting status may vary,
  // we need to calculate it every time we start
  colorSensorOn();
  setupBlankColor();

  // get status back from the ccd initialzation function
  bool cameraIsBlocked, recordAvailable, returnFromPlatform;
  explosureRecord bestRecord;
  prepareCCD(cameraIsBlocked, recordAvailable, bestRecord);

  // entering bt control mode, if the camera is blocked
  if (cameraIsBlocked) {
    oledPrint("BT MODE", 1);
    oledFlush();

    for (;;) {
      if (!connected) {
        if (btConnected()) {
          connected = true;
          oledPrintAndFlush("CONNECTED!", 1);
        }
      }
      if (connected) {
        command = btRecieve();
        parseCommands(command);
        delay(20);
      }
    }
  }
  // normal tracking mode
  else {
    // oled indecator
    oledPrint("TRACK MODE", 1);
    oledFlush();
    delay(1000);

    // a closed loop for tracking purpose
    for (;;) {
      display.clearDisplay();

      // the ccd's explosuring time will be a major drawback of excecution time, therefore we use
      // the excecution time of other functions to compensate the ccd explosure time
      int prevTimeMs               = getTime();
      bool noTimeRecord            = (prevTimeMs == -1);
      int substractedExplosureTime = 0;
      if (!noTimeRecord) {
        substractedExplosureTime -= prevTimeMs;
        if (substractedExplosureTime < 0)
          substractedExplosureTime = 0;

        // Frame length, proper explosure time
        oledPrint("std", bestRecord.explosureTime, "frm", prevTimeMs, 0);
      }

      // if there's no time record (prev time is not setuped), or the car is just returning to
      // tracking state from the platform detection state, we will tell the ccd sensor to clear all
      // the explosuring values and start as new
      returnFromPlatform = autoTrack(bestRecord, bestRecord.explosureTime, substractedExplosureTime,
                                     noTimeRecord || returnFromPlatform);
      oledFlush();
    }
  }
}
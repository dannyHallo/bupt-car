// #define BT_ON

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

TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

int command  = -1;
int location = 0;

pid angelPID(angle_kp, angle_ki, angle_kd);

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
  pinMode(PINOUT_MOTOR_ON, INPUT_PULLDOWN);

  oledCountdown("Booting", 200, 1);
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

bool autoTrack(explosureRecord& bestRecord, int bestExplosureTime, int substractedExplosureTime,
               bool initStarting) {
  bool motorEnable    = digitalRead(PINOUT_MOTOR_ON) ? true : false;
  float motorAimSpeed = motorEnable ? aim_speed : 0;

  int trackMidPixel = 0;
  int trackStatus   = 0;

  if (initStarting) {
    processCCD(trackMidPixel, trackStatus, bestExplosureTime, true, false);
  } else {
    processCCD(trackMidPixel, trackStatus, substractedExplosureTime, false, false);
  }

  // Print status only
  switch (trackStatus) {
  case STATUS_NORMAL:
    boardLedOff();
    oledPrint("TRACKING", 1);
    break;
  case STATUS_NO_TRACK:
    boardLedOff();
    oledPrint("!!!NOTRACK", 1);
    break;
  case STATUS_PLATFORM:
    boardLedOn();
    oledPrint("!!!PLATFORM", 1);
    break;
  }

  switch (trackStatus) {
  case STATUS_NORMAL:
    servoWritePixel(angelPID.update(trackMidPixel - 64) + 64);
    motorForward(motorAimSpeed);
    break;

  case STATUS_PLATFORM:
    servoWritePixel(64);
    motorBrake();

    display.clearDisplay();
    oledPrint(++location, "Location", 1);
    oledFlush();

    delay(1000);
    colorSensorOn();
    oledCountdown("Capturing", 600, 1);
    getRGB();
    colorSensorOff();
    delay(2000);

    motorForward(motorAimSpeed / 3);
    processCCD(trackMidPixel, trackStatus, bestExplosureTime, true, false);
    for (;;) {
      processCCD(trackMidPixel, trackStatus, bestExplosureTime, true, false);
      oledFlush();
      if (trackStatus == STATUS_NORMAL)
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

void Task2(void* pvParameters) {
  colorSensorOn();
  setupBlankColor();

  //   for (;;) {
  //     btSend(123);
  //     btSend("hello!");
  //     flipBoardLed();
  //     delay(1000);
  //   }

  bool cameraIsBlocked, recordAvailable, returnFromPlatform;
  explosureRecord bestRecord;
  prepareCCD(cameraIsBlocked, recordAvailable, bestRecord);

  if (cameraIsBlocked) {
    oledPrint("BT MODE", 1);
    oledFlush();

    for (;;) {
      parseCommands(command);
    }
  } else {
    oledPrint("TRACK MODE", 1);
    oledFlush();
    delay(1000);

    for (;;) {
      display.clearDisplay();

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

      returnFromPlatform = autoTrack(bestRecord, bestRecord.explosureTime, substractedExplosureTime,
                                     noTimeRecord || returnFromPlatform);
      oledFlush();
    }
  }
}
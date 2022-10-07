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

int command = -1;

int location = 0;

pid angelPID(angle_kp, angle_ki, angle_kd);

void setup() {
  Serial.begin(serial_btr);

  pinoutInitBoardLed();
  initColor();
  initCCD();
  initServo();
  initMotor();
  initBluetooth();
  initOled();
  pinMode(PINOUT_MOTOR_ON, INPUT_PULLDOWN);

  display.clearDisplay();
  oledPrint("Booting...", 0);
  oledFlush();
  Serial.println("Booting...");

  assignTasks();
}

void assignTasks() {
  xTaskCreatePinnedToCore(Task1,        // Task function
                          "Task1",      // Task name
                          65535,        // Stack size
                          NULL,         // Parameter
                          1,            // Priority
                          &Task1Handle, // Task handle to keep track of created task
                          0             // Core ID: 0:
  );

  xTaskCreatePinnedToCore(Task2,        // Task function
                          "Task2",      // Task name
                          65535,        // Stack size
                          NULL,         // Parameter
                          1,            // Priority
                          &Task2Handle, // Task handle to keep track of created task
                          1             // Core ID: 0:
  );
}

void autoTrack(int explosureTime, int originalExplosureTime, float threhold) {
  bool motorEnable = digitalRead(PINOUT_MOTOR_ON) ? true : false;

  int trackMidPixel = 0;
  int trackStatus   = 0;

  processCCD(trackMidPixel, trackStatus, explosureTime, threhold, true);

  // Print status only
  switch (trackStatus) {
  case STATUS_NORMAL:
    boardLedOff();
    oledPrint("tracking", 1);
    break;
  case STATUS_NO_TRACK:
    boardLedOff();
    oledPrint("notrack", 1);
    break;
  case STATUS_PLATFORM:
    boardLedOn();
    oledPrint("platform", 1);
    break;
  }

  switch (trackStatus) {
  case STATUS_NORMAL: {
    servoWritePixel(angelPID.update(trackMidPixel - 64) + 64);

    float motorAimSpeed = motorEnable ? aim_speed : 0;
    motorForward(motorAimSpeed);
    break;
  }
  case STATUS_PLATFORM: {
    servoWritePixel(64);
    motorBrake();
    delay(2000);

    // printColorToRow(0);
    float motorAimSpeed = motorEnable ? aim_speed : 0;
    motorForward(motorAimSpeed);

    while (trackStatus == STATUS_PLATFORM)
      processCCD(trackMidPixel, trackStatus, originalExplosureTime, threhold, true);

    break;
  }
  default:
    motorIdle();
    break;
  }

  //   case STATUS_PLATFORM:
  //     boardLedOn();
  //     motorIdle();
  //     vTaskDelay(200);
  //     printColorToRow();
  //     oledPrint(++location, "Location", 1);
  //     vTaskDelay(2000);
  //     if (motorEnable) {
  //       motorForward(aim_speed / 2);
  //     } else {
  //       motorIdle();
  //     }
  //     vTaskDelay(500);

  //     break;
  //   case STATUS_NO_TRACK:
  //     boardLedOn();

  //     servoWritePixel(127);
  //     break;
  //   }
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

void prepareCCD(bool& cameraIsBlocked, bool& bestAvailable, explosureRecord& bestRecord) {
  display.clearDisplay();
  cameraIsBlocked = false;
  bestAvailable   = false;

  getBestExplosureTime(bestRecord, cameraIsBlocked, bestAvailable, false);

  Serial.println("----------------------------------------");
  if (cameraIsBlocked) {
    Serial.println("bluetooth mode activated");
  } else {
    Serial.println("tracking mode activated");
  }

  if (bestAvailable) {
    Serial.print("best explosure time: ");
    Serial.print(bestRecord.explosureTime);
    Serial.print(" with minimum ratio: ");
    Serial.println(bestRecord.threhold);
  } else {
    Serial.println("no available record");
  }

  Serial.println("----------------------------------------");

  oledPrint(bestRecord.explosureTime, "expl", 0);
  oledPrint(bestRecord.threhold, "thre", 1);
  oledFlush();
  delay(2000);
  display.clearDisplay();
}

int loopTime = 0;
int prevTime = 0;

int getTime() {
  loopTime       = millis();
  int prevTimeMs = loopTime - prevTime;
  prevTime       = loopTime;
  return prevTimeMs;
}

void Task2(void* pvParameters) {
  for (;;) {
    bool cameraIsBlocked, bestAvailable;
    explosureRecord bestRecord;
    prepareCCD(cameraIsBlocked, bestAvailable, bestRecord);

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

        int prevTimeMs = getTime();
        oledPrint(prevTimeMs, "time");

        int thisTimeExplosureTime = bestRecord.explosureTime - prevTimeMs;
        if (thisTimeExplosureTime < 0)
          thisTimeExplosureTime = 0;

        autoTrack(thisTimeExplosureTime, bestRecord.explosureTime, bestRecord.threhold + 0.2f);
        oledFlush();
      }
    }
  }
}
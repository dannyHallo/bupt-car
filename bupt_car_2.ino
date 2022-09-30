// #define BT_ON

#include "dep/bluetooth.h"
#include "dep/boardLed.h"
#include "dep/ccd.h"
#include "dep/commandParser.h"
#include "dep/motor.h"
#include "dep/oled.h"
#include "dep/pid.h"
#include "dep/pinouts.h"
#include "dep/servo.h"

TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

int command           = -1;
int lastValidMidPixel = -1;

void setup() {
  Serial.begin(115200);

  pinoutInitBoardLed();
  initCCD();
  initServo();
  initMotor();
  pinoutInitAndOpenBTSerialBluetooth();
  pinoutInitAndI2cConfigOled();

  assignTasks();

  pinMode(PINOUT_MOTOR_ON, INPUT_PULLDOWN);
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

void autoTrack(int bestExplosureTime, float bestRatio) {
  bool motorEnable = false;
  if (digitalRead(PINOUT_MOTOR_ON)) {
    motorEnable = true;
  }

  int trackMidPixel = 0;
  float darkRatio   = 0;
  int trackStatus   = 0;
  processCCD(trackMidPixel, darkRatio, trackStatus, bestExplosureTime, bestRatio, true);

  oledPrint(darkRatio, "RTO(%)", 2);

  switch (trackStatus) {
  case STATUS_NORMAL:
    // boardLedOff();

    lastValidMidPixel = getPID(trackMidPixel - 64);
    lastValidMidPixel += 64;
    servoWritePixel(lastValidMidPixel);
    // oledPrint(lastValidMidPixel, "out pix", 0);
    oledPrint("tracking", 1);
    break;

  case STATUS_HIGH_DL:
    // boardLedOn();
    oledPrint("high ratio", 1);
    servoWritePixel(127);
    break;

  case STATUS_NO_TRACK:
    // boardLedOn();
    oledPrint("no track", 1);
    servoWritePixel(127);
    break;
  }

  if (motorEnable) {
    int currentPower   = 0;
    float currentSpeed = 0;

    motorForward(0.2f, currentPower, currentSpeed);
    Serial.print("power: ");
    Serial.print(currentPower);
    Serial.print(" ");
    Serial.print("speed: ");
    Serial.println(currentSpeed);

  } else {
    motorIdle();
  }
}

// This loop is automatically assigned to Core 1, so block it manually
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

void Task2(void* pvParameters) {
  for (;;) {
    display.clearDisplay();

    int bestExplosureTime = 0;
    float bestRatio       = 0;
    bool cameraIsBlocked  = false;
    getBestExplosureTime(bestExplosureTime, bestRatio, cameraIsBlocked, true);

    Serial.println("----------------------------------------");
    if (cameraIsBlocked) {
      Serial.print("Best explosure time: ");
      Serial.print(bestExplosureTime);
      Serial.print(" with minimum ratio: ");
      Serial.println(bestRatio);
      Serial.println("Camera is blocked");
      Serial.println("Bluetooth mode activated");
    } else {
      Serial.print("Best explosure time: ");
      Serial.print(bestExplosureTime);
      Serial.print(" with minimum ratio: ");
      Serial.println(bestRatio);
      Serial.println("Tracking mode activated");
    }
    Serial.println("----------------------------------------");

    oledPrint(bestExplosureTime, "EPL(s)", 0);
    oledPrint(bestRatio, "RTO(%)", 1);
    oledFlush();
    delay(3000);
    display.clearDisplay();

    if (cameraIsBlocked) {
      oledPrint("CAM BLOCKED", 0);
      oledFlush();
      delay(1000);

      display.clearDisplay();
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

        autoTrack(bestExplosureTime, bestRatio);
        // float currentSpeed = getSpeed();
        // oledPrint(currentSpeed, "Speed", 0);
        // Serial.println(currentSpeed);

        oledFlush();
      }
    }
  }
}
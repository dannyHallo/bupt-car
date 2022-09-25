#include "dep/bluetooth.h"
#include "dep/boardLed.h"
#include "dep/ccd.h"
#include "dep/commandParser.h"
#include "dep/motor.h"
#include "dep/oled.h"
// #include "dep/naviLine.h"
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
  pinoutInitCCD();
  pinoutAndPwmChannelInitServo();
  pinoutAndPwmChannelInitMotor();
  pinoutInitAndOpenBTSerialBluetooth();
  pinoutInitAndI2cConfigOled();

  assignTasks();

  //   navi.initNaviLine();

  //   Serial.printf("Clocck cycle: %lld\n", clockCycle);
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
  int trackMidPixel = 0;
  float darkRatio   = 0;
  bool isNormal     = false;
  processCCD(trackMidPixel, darkRatio, isNormal, bestExplosureTime, bestRatio);

  display.clearDisplay();
  oledPrint(darkRatio, "RTO(%)", 2);

  if (isNormal) {
    boardLedOff();

    lastValidMidPixel = getPID(trackMidPixel - 64);
    lastValidMidPixel += 64;
    servoWritePixel(lastValidMidPixel);
    oledPrint(lastValidMidPixel, "out pix", 0);
    oledPrint("tracking", 1);

    // motorForward();
  } else {
    boardLedOn();

    // Reverse
    if (lastValidMidPixel < 64)
      servoWritePixel(0);
    else
      servoWritePixel(127);

    oledPrint("target lost", 1);

    // motorForward();
    // motorBackward();
  }

  oledFlush();
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
    delay(2000);
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
      for (;;) {
        autoTrack(bestExplosureTime, bestRatio);
      }
    }
  }
}
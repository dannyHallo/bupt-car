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

int command = -1;
// naviLine navi     = naviLine();

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

// This loop is automatically assigned to Core 1, so block it manually
void loop() {
  // Serial.printf("Clocck cycle: %lld",clockCycle);
  delay(1000);
  // navi.printBinarizedPixels();
  // navi.printLTBrightness();
  // navi.printLinearPixels();
}

void Task1(void* pvParameters) {
  for (;;) {
    if (Serial.available()) {
      btSend(Serial.read());
    }

    command = btRecieve();
    vTaskDelay(20);
  }
}

int lastValidMidPixel = -1;

void Task2(void* pvParameters) {
  for (;;) {
    display.clearDisplay();

    int trackMidPixel = 0;
    float darkRatio   = 0;
    bool isNormal     = false;
    processCCD(trackMidPixel, darkRatio, isNormal);

    oledPrint(darkRatio, "d/l", 2);

    if (isNormal) {
      boardLedOff();
      lastValidMidPixel = getPID(trackMidPixel - 64);
      lastValidMidPixel += 64;
      servoWritePixel(lastValidMidPixel);
      oledPrint(lastValidMidPixel, "out pix", 0);

      oledPrint("isNormal: true", 1);

      motorForward();
    } else {
      boardLedOn();

      // Reverse
      if (lastValidMidPixel < 64)
        servoWritePixel(127);
      else
        servoWritePixel(0);

      oledPrint("isNormal: false", 1);

      motorBackward();
    }

    oledFlush();
  }
}
#include "dep/pinouts.h"
#include "dep/boardLed.h"
#include "dep/ccd.h"
#include "dep/servo.h"
#include "dep/motor.h"
#include "dep/bluetooth.h"
#include "dep/commandParser.h"
#include "dep/naviLine.h"

TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

int trackMidPoint = -1;
int command = -1;
naviLine navi = naviLine();

void setup() {
    Serial.begin(115200);

    pinoutInitBoardLed();
    pinoutInitCCD();
    pinoutAndPwmChannelInitServo();
    pinoutAndPwmChannelInitMotor();
    pinoutInitAndOpenBTSerialBluetooth();

    assignTasks();

    navi.initNaviLine();

    Serial.printf("Clocck cycle: %lld\n",clockCycle);
}

void assignTasks() {
    xTaskCreatePinnedToCore(
        Task1,        // Task function
        "Task1",      // Task name
        2000,         // Stack size
        NULL,         // Parameter
        1,            // Priority
        &Task1Handle, // Task handle to keep track of created task
        0             // Core ID: 0:
    );

    xTaskCreatePinnedToCore(
        Task2,        // Task function
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
    //Serial.printf("Clocck cycle: %lld",clockCycle);
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

void Task2(void* pvParameters) {
    for (;;) {
        //Serial.printf("Clocck cycle: %lld\n",clockCycle);
        // motorLoop();

        trackMidPoint = processCCD();
        // Serial.println(trackMidPoint);

        if (trackMidPoint!=-1) {
            servoLoop(trackMidPoint);
        }

        // int direction = navi.getMidLine();

        // servoWriteAngle(scaleAngle(direction,0,45,1000));
        // Serial.printf("Direction: %d\n",direction);

        // vTaskDelay(5);
    }
}
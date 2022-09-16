#include <Arduino.h>
#line 1 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
#include "dep/pinouts.h"
#include "dep/boardLed.h"
#include "dep/ccd.h"

TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

#line 8 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
void setup();
#line 18 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
void assignTasks();
#line 42 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
void loop();
#line 44 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
void Task1(void *pvParameters);
#line 57 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
void Task2(void *pvParameters);
#line 8 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
void setup()
{
    Serial.begin(115200);

    pinoutInitBoardLed();
    pinoutInitCCD();

    assignTasks();
}

void assignTasks()
{
    xTaskCreatePinnedToCore(
        Task1,        // Task function
        "Task1",      // Task name
        1000,         // Stack size
        NULL,         // Parameter
        1,            // Priority
        &Task1Handle, // Task handle to keep track of created task
        0             // Core ID: 0:
    );

    xTaskCreatePinnedToCore(
        Task2,        // Task function
        "Task2",      // Task name
        1000,         // Stack size
        NULL,         // Parameter
        1,            // Priority
        &Task2Handle, // Task handle to keep track of created task
        1             // Core ID: 0:
    );
}

// This loop is automatically assigned to Core 1, so block it manually
void loop() { delay(1000); }

void Task1(void *pvParameters)
{
    for (;;)
    {
        // delay(1000);
        // delay(10);

        processCCD();

        digitalWrite(PINOUT_BOARD_LED_PIN, !digitalRead(PINOUT_BOARD_LED_PIN));
    }
}

void Task2(void *pvParameters)
{
    for (;;)
    {
        delay(1000);
    }
}


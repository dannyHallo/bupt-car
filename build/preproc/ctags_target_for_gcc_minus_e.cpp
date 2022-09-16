# 1 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
# 2 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino" 2
# 3 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino" 2
# 4 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino" 2

TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

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
        Task1, // Task function
        "Task1", // Task name
        1000, // Stack size
        
# 24 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino" 3 4
       __null
# 24 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
           , // Parameter
        1, // Priority
        &Task1Handle, // Task handle to keep track of created task
        0 // Core ID: 0:
    );

    xTaskCreatePinnedToCore(
        Task2, // Task function
        "Task2", // Task name
        1000, // Stack size
        
# 34 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino" 3 4
       __null
# 34 "c:\\Users\\Administrator\\Desktop\\bupt_car_2\\bupt_car_2.ino"
           , // Parameter
        1, // Priority
        &Task2Handle, // Task handle to keep track of created task
        1 // Core ID: 0:
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

        digitalWrite(2, !digitalRead(2));
    }
}

void Task2(void *pvParameters)
{
    for (;;)
    {
        delay(1000);
    }
}

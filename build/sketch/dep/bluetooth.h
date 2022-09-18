#pragma once

#include "pinouts.h"
#include "../lib/arduino-esp32/libraries/BluetoothSerial/src/BluetoothSerial.h"

BluetoothSerial serialBT;

void pinoutInitAndOpenBTSerialBluetooth()
{
    serialBT.begin("ESP32Test");
    Serial.println("Bluetooth configured, now you can pair it!");
}

void checkBTInput()
{
    if (Serial.available())
    {
        serialBT.write(Serial.read());
    }
}
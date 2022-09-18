#pragma once

#include "pinouts.h"
#include "../lib/arduino-esp32/libraries/BluetoothSerial/src/BluetoothSerial.h"

BluetoothSerial serialBT;

void pinoutInitAndOpenBTSerialBluetooth()
{
    serialBT.begin("ESP32Test");
    Serial.println("Bluetooth configured, now you can pair it!");
}

void btSend(int message)
{
    serialBT.write(message);
}

int btRecieve()
{
    if (serialBT.available())
    {
        int message = serialBT.read();

        digitalWrite(PINOUT_BOARD_LED_PIN, !digitalRead(PINOUT_BOARD_LED_PIN));
        // Serial.println(message); // print on serial monitor
        return message;
    }
    return -1;
}
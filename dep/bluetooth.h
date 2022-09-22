#pragma once

#include "boardLed.h"
#include "pinouts.h"

#include "../lib/arduino-esp32/libraries/BluetoothSerial/src/BluetoothSerial.h"

BluetoothSerial serialBT;

void pinoutInitAndOpenBTSerialBluetooth() {
  serialBT.begin("ESP32Test");
  Serial.println("Bluetooth configured, now you can pair it!");
}

void btSend(int message) { serialBT.write(message); }

int btRecieve() {
  if (serialBT.available()) {
    int message = serialBT.read();

    flipBoardLed();
    return message;
  }
  return -1;
}
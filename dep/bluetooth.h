#pragma once

#include "boardLed.h"
#include "pinouts.h"

#ifdef BT_ON
#include "../lib/arduino-esp32/libraries/BluetoothSerial/src/BluetoothSerial.h"
BluetoothSerial serialBT;
#endif

// pinout init, open BTSerial
void initBluetooth() {
#ifdef BT_ON
  serialBT.begin("a costly car");
  Serial.println("Bluetooth configured, now you can pair it!");
#endif
}

void btSend(int message) {
#ifdef BT_ON
  serialBT.println(message);
#endif
}

void btSend(char* message) {
#ifdef BT_ON
  serialBT.println(message);
#endif
}

// void btSend(const char* message) {
// #ifdef BT_ON
//   serialBT.write(message);
// #endif
// }

int btRecieve() {
#ifdef BT_ON
  //while (true) {
  if (serialBT.available()) {
    int message = serialBT.read();
    return message;
  }
  //}
#endif
  return -1;
}
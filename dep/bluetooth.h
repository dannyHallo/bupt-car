#pragma once

#include "boardLed.h"
#include "pinouts.h"

// the BT_ON define is in args.h, we can manually disable bluetooth functionality to greatly
// increase uploading speed (debug function)
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

// send information to other devices
void btSend(int message) {
#ifdef BT_ON
  serialBT.println(message);
#endif
}

// send information to other devices
void btSend(char* message) {
#ifdef BT_ON
  serialBT.println(message);
#endif
}

// recieve information from other devices
int btRecieve() {
#ifdef BT_ON
  if (serialBT.available()) {
    int message = serialBT.read();
    return message;
  }
#endif
  return -1;
}

// returns true when the device bluetooth is connected
bool btConnected() { return serialBT.connected(); }
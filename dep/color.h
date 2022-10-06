#pragma once

#include "../lib/arduino-esp32/libraries/Wire/src/Wire.h"
#include "boardLed.h"
#include "math.h"
#include "pinouts.h"

#define COLOR_RED 0x01
#define COLOR_YELLOW 0x02
#define COLOR_PINK 0x04
#define COLOR_WHITE 0x08
#define COLOR_BLACK 0x10
#define COLOR_GREEN 0x20
#define COLOR_DARK_BLUE 0x40
#define COLOR_BLUE 0x80

TwoWire* wire;
const uint8_t colorSensorAddr  = 0x5a; // 7 bit address: 0x5a (use this one); 8 bit address: 0xb4
const uint8_t colorBufferAddr  = 0x0f;
const long minimumSamplingTime = 100;

long timeStamp1 = 0;
long timeStamp2 = 0;
uint8_t lastSampleResult;

void setBrightness(uint8_t);
bool requestBuffer(uint8_t deviceAddr, uint8_t bufferAddr, uint8_t* buffer, uint8_t bufferSize);

void initColor() {
  wire = &Wire;
  //   wire->begin();     // Start master
  setBrightness(10); // Maximize the light
}

// Write communication test
void testColorLoop() {
  for (int b = 0; b <= 10; b++) {
    setBrightness(b);
    delay(200);
  }
}

// (0 -> 10) (dim -> light)
void setBrightness(uint8_t brightness) {
  clamp(brightness, static_cast<uint8_t>(0), static_cast<uint8_t>(10));

  brightness            = 10 - brightness; // invert brightness to meet the module's need :(
  uint8_t configCommand = 0x00;

  configCommand |= brightness << 4;

  wire->beginTransmission(colorSensorAddr); // Call device
  wire->write(0x10);                        // Ask to Config
  wire->write(configCommand);               // Actural command
  wire->endTransmission();                  // Stop transmission
}

uint8_t getColor() {
  timeStamp1 = millis();
  if (timeStamp1 - timeStamp2 < minimumSamplingTime) {
    return lastSampleResult;
  }
  timeStamp2 = timeStamp1;

  uint8_t buffAddr = 0x0f;
  uint8_t buff     = 0;

  requestBuffer(colorSensorAddr, colorBufferAddr, &buff, 1);

  lastSampleResult = buff;
  return buff;
}

bool requestBuffer(uint8_t deviceAddr, uint8_t bufferAddr, uint8_t* buffer, uint8_t bufferSize) {
  wire->beginTransmission(deviceAddr); // Call device
  wire->write(bufferAddr);             // Ask to Config
  wire->endTransmission(false);        // Stop transmission

  wire->requestFrom(deviceAddr, bufferSize);
  wire->readBytes(buffer, bufferSize);

  wire->endTransmission(); // Stop transmission

  return true;
}
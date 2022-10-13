#pragma once

#include "../lib/arduino-esp32/libraries/Wire/src/Wire.h"
#include "boardLed.h"
#include "math.h"
#include "oled.h"
#include "pinouts.h"

#define COLOR_RED 0
#define COLOR_GREEN 1
#define COLOR_BLUE 2
#define COLOR_YELLOW 3
#define COLOR_EMPTY 4

const char* colorLookupArray[5] = {"RED", "GREEN", "BLUE", "YELLOW", "EMPTY"};

const uint8_t colorSensorAddr  = 0x5a;
const uint8_t colorBufferAddr  = 0x00;
const long minimumSamplingTime = 100;

uint8_t rawBuff[6];
uint16_t rgb[3];
TwoWire* wire = &Wire;

void setBrightness(uint8_t);
bool requestBuffer(uint8_t deviceAddr, uint8_t bufferAddr, uint8_t* buffer, uint8_t bufferSize);
int parseColor();

void initColor() {
  setBrightness(10); // Maximize the light
}

// Write communication test
void testColor() {
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

void getColor() {
  // Resets the buffer
  memset(rawBuff, 0, 6);

  // Read buffers
  for (uint8_t i = 0; i < 6; i++) {
    requestBuffer(colorSensorAddr, colorBufferAddr + i, rawBuff + i, 1);
    delay(10);
  }
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

void getRGB(bool debug = false) {
  getColor();

  for (int i = 0; i < 3; i++) {
    rgb[i] = (*(rawBuff + i * 2) << 8) & 0xff00;
    rgb[i] |= *(rawBuff + i * 2 + 1);
  }

  if (debug) {
    for (uint8_t i = 0; i < 3; i++) {
      Serial.print(rgb[i], DEC);
      Serial.println("  ");
    }
  }

  oledClear();
  oledPrint(static_cast<int>(rgb[0]), "R", 0);
  oledPrint(static_cast<int>(rgb[1]), "G", 1);
  oledPrint(static_cast<int>(rgb[2]), "B", 2);
  oledPrint(colorLookupArray[parseColor()], 3);

  oledFlush();
}

int parseColor() {
  uint16_t maxVal  = 0;
  uint16_t largest = 0;

  for (int i = 0; i < 3; i++) {
    if (rgb[i] > maxVal) {
      maxVal  = rgb[i];
      largest = i;
    }
  }

  if (maxVal < 100)
    return COLOR_EMPTY;

  switch (largest) {
  case 0:
    if ((float)rgb[1] / (float)maxVal > 0.8f)
      return COLOR_YELLOW;
    return COLOR_RED;
  case 1:
    if ((float)rgb[0] / (float)maxVal > 0.8f)
      return COLOR_YELLOW;
    return COLOR_GREEN;
  case 2:
    return COLOR_BLUE;
  }
}
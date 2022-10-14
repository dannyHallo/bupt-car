#pragma once

#include "../lib/arduino-esp32/libraries/Wire/src/Wire.h"
#include "boardLed.h"
#include "math.h"
#include "oled.h"
#include "pinouts.h"

const int COLOR_RED = 0;
const int COLOR_GREEN = 1;
const int COLOR_BLUE = 2;
const int COLOR_YELLOW = 3;
const int COLOR_EMPTY = 4;

const char* colorLookupArray[5] = { "RED", "GREEN", "BLUE", "YELLOW", "EMPTY" };

const uint8_t colorSensorAddr = 0x5a;
const uint8_t colorBufferAddr = 0x00;
const long minimumSamplingTime = 100;

uint8_t rawBuff[6];
uint16_t rgb[3];
uint16_t blankRGB[3];
int outRGB[3];

TwoWire* wire = &Wire;

void initColor();
void colorSensorOn();
void colorSensorOff();
void testColor();
void setBrightness(uint8_t brightness);
void getColor();
bool requestBuffer(uint8_t deviceAddr,uint8_t bufferAddr,uint8_t* buffer,uint8_t bufferSize);
int getRGB(bool relativeVal = true);
int parseColor();

void initColor() { setBrightness(0); }
void colorSensorOn() { setBrightness(10); }
void colorSensorOff() { setBrightness(0); }

void setupBlankColor() {
  delay(400);
  getColor();
  delay(100);
  getColor();

  memcpy(blankRGB,rgb,sizeof(rgb[0])*3);

  oledClear();
  oledPrint(static_cast<int>(blankRGB[0]),"rawR",0);
  oledPrint(static_cast<int>(blankRGB[1]),"rawG",1);
  oledPrint(static_cast<int>(blankRGB[2]),"rawB",2);
  oledFlush();
  delay(1000);
}

// Write communication test
void testColor() {
  for (int b = 0; b<=10; b++) {
    setBrightness(b);
    delay(200);
  }
}

// (0 -> 10) (dim -> light)
void setBrightness(uint8_t brightness) {
  clamp(brightness,static_cast<uint8_t>(0),static_cast<uint8_t>(10));

  brightness = 10-brightness; // invert brightness to meet the module's need :(
  uint8_t configCommand = 0x00;

  configCommand |= brightness<<4;

  wire->beginTransmission(colorSensorAddr); // Call device
  wire->write(0x10);                        // Ask to Config
  wire->write(configCommand);               // Actural command
  wire->endTransmission();                  // Stop transmission
}

void getColor() {
  // Resets the buffer
  memset(rawBuff,0,6);

  // Read buffers
  for (uint8_t i = 0; i<6; i++) {
    requestBuffer(colorSensorAddr,colorBufferAddr+i,rawBuff+i,1);
    delay(10);
  }

  // Parse vals
  for (int i = 0; i<3; i++) {
    rgb[i] = (*(rawBuff+i*2)<<8)&0xff00;
    rgb[i] |= *(rawBuff+i*2+1);
  }
}

bool requestBuffer(uint8_t deviceAddr,uint8_t bufferAddr,uint8_t* buffer,uint8_t bufferSize) {
  wire->beginTransmission(deviceAddr); // Call device
  wire->write(bufferAddr);             // Ask to Config
  wire->endTransmission(false);        // Stop transmission

  wire->requestFrom(deviceAddr,bufferSize);
  wire->readBytes(buffer,bufferSize);

  wire->endTransmission(); // Stop transmission

  return true;
}

int color;

int getRGB(bool relativeVal) {
  getColor();

  for (int i = 0; i<3; i++) {
    outRGB[i] = static_cast<int>(rgb[i])-static_cast<int>(blankRGB[i]);
    outRGB[i] = (outRGB[i]>=0) ? outRGB[i] : 0;
  }

  oledClear();
  oledPrint(outRGB[0],"R",0);
  oledPrint(outRGB[1],"G",1);
  oledPrint(outRGB[2],"B",2);
  color = parseColor();
  oledPrint(colorLookupArray[color],3);

  oledFlush();
  return color;
}

int parseColor() {
  uint16_t maxVal = 0;
  uint16_t largest = 0;

  for (int i = 0; i<3; i++) {
    if (outRGB[i]>maxVal) {
      maxVal = outRGB[i];
      largest = i;
    }
  }

  if (maxVal<40)
    return COLOR_EMPTY;

  switch (largest) {
  case 0:
    if ((float)outRGB[1]/(float)maxVal>0.8f)
      return COLOR_YELLOW;
    return COLOR_RED;
  case 1:
    if ((float)outRGB[0]/(float)maxVal>0.8f)
      return COLOR_YELLOW;
    return COLOR_GREEN;
  case 2:
    return COLOR_BLUE;
  }
}
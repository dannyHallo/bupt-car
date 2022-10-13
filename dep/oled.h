#pragma once

// https://github.com/RalphBacon/ESP32-SSD1306-OLED/blob/master/ESP32_OLED_SSD1306_Adafruit.ino
#include "../lib/Adafruit_SSD1306/Adafruit_SSD1306.h"
#include "pinouts.h"

const int cLineSpacing = 8;

Adafruit_SSD1306 display(-1);

void initOled();
void oledPrint(const char* str, const int row = 0);
void oledPrint(const int val, const char* hint = "Val", const int row = 0);
void oledPrint(const float val, const char* hint = "Val", const int row = 0);
void oledPrintAndFlush(const char* str, const int row = 0);
void oledCountdown(std::string hint, const int delayMs = 500, const int row = 0);
void oledClear();
void oledFlush();

// set up display
void initOled() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // display.setRotation(2);

  // Set initial screen parameters
  display.setTextSize(1);
  display.setCursor(0, 0);
  oledClear();
  display.display();

  // Give user a chance to read display
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.println("Setup is OK");
  display.display();
}

void oledPrint(const char* str, const int row) {
  display.setCursor(0, cLineSpacing * row);
  display.println(str);
}

void oledPrint(const int val, const char* hint, const int row) {
  display.setCursor(0, cLineSpacing * row);
  char str[100];
  sprintf(str, "%s: %d", hint, val);
  display.println(str);
}

void oledPrint(const float val, const char* hint, const int row) {
  display.setCursor(0, cLineSpacing * row);
  char str[100];
  sprintf(str, "%s: %.2f", hint, val);
  display.println(str);
}

void oledPrintAndFlush(const char* str, const int row) {
  oledClear();
  oledPrint(str, row);
  oledFlush();
}

void oledCountdown(std::string hint, const int delayMs, const int row) {
  for (int i = 0; i < 3; i++) {
    hint += ".";
    oledPrintAndFlush(hint.c_str(), row);
    delay(delayMs);
  }
}

void oledClear() { display.clearDisplay(); }

void oledFlush() { display.display(); }
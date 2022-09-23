#pragma once

#include "../lib/Adafruit_SSD1306/Adafruit_SSD1306.h"
#include "pinouts.h"

extern Adafruit_SSD1306 display(-1);

void pinoutInitAndI2cConfigOled() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // display.setRotation(2);

  // Set initial screen parameters
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.clearDisplay();
  display.display();

  // Give user a chance to read display
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.println("Setup is OK");
  display.display();
}
#pragma once

#pragma once

#include "pinouts.h"

void pinoutInitBoardLed() {
  pinMode(PINOUT_BOARD_LED_PIN, OUTPUT);
  digitalWrite(PINOUT_BOARD_LED_PIN, LOW);
}

void boardLedOn() { digitalWrite(PINOUT_BOARD_LED_PIN, HIGH); }

void boardLedOff() { digitalWrite(PINOUT_BOARD_LED_PIN, LOW); }

void flipBoardLed() { digitalWrite(PINOUT_BOARD_LED_PIN, !digitalRead(PINOUT_BOARD_LED_PIN)); }
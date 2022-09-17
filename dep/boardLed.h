#pragma once

#pragma once

#include "pinouts.h"

void pinoutInitBoardLed()
{
    pinMode(PINOUT_BOARD_LED_PIN, OUTPUT);
    digitalWrite(PINOUT_BOARD_LED_PIN, LOW);
}
#pragma once

#include "boardLed.h"
#include "pinouts.h"


unsigned long lastTime    = 0;
unsigned long currentTime = 0;
unsigned long deltaTime   = 0;

unsigned long rotateCount = 0;

void resetSpeedCount();
float getSpeed();
void motorCountInterrupt();

// speed count pinout, attatch hall interrupt
void initSpeedControl() {
  pinMode(PINOUT_E2A, INPUT_PULLUP);
  attachInterrupt(PINOUT_E2A, motorCountInterrupt, FALLING);
}

void resetSpeedCount() {
  lastTime    = 0;
  currentTime = 0;
  rotateCount = 0;
}

float getSpeed() {
  float speed = 0;

  currentTime = millis();
  deltaTime   = currentTime - lastTime;
  lastTime    = currentTime;

  speed       = float(rotateCount) / float(deltaTime);
  rotateCount = 0;

  return speed;
}

void motorCountInterrupt() {
  rotateCount++;
  flipBoardLed();
}
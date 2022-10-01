#pragma once

#include "math.h"
#include "pinouts.h"
#include "speedControl.h"
#include "pid.h"
#include "oled.h"

const int cMotorResolution = 16; // Max: 16 bit
const int cDefaultPower = 30000;
const int cMaximumPower = 65535;
int currentPower;

float maxResolution = 0;

#define PWM_CHANNEL_LEFT_MOTOR_FRONT 2
#define PWM_CHANNEL_LEFT_MOTOR_BACK 3
#define PWM_CHANNEL_RIGHT_MOTOR_FRONT 4
#define PWM_CHANNEL_RIGHT_MOTOR_BACK 5

// init speed control, motor pins init, pwm init
void initMotor() {
  initSpeedControl();

  currentPower = cDefaultPower;

  ledcSetup(2,1000,cMotorResolution); // Channel 1, 1kHz, 16 bit resolution
  ledcSetup(3,1000,cMotorResolution); // Channel 2, 1kHz, 16 bit resolution
  ledcSetup(4,1000,cMotorResolution); // Channel 3, 1kHz, 16 bit resolution
  ledcSetup(5,1000,cMotorResolution); // Channel 4, 1kHz, 16 bit resolution

  ledcAttachPin(PINOUT_LEFT_MOTOR_FRONT,PWM_CHANNEL_LEFT_MOTOR_FRONT); // Channel 1: L motor front
  ledcAttachPin(PINOUT_LEFT_MOTOR_BACK,PWM_CHANNEL_LEFT_MOTOR_BACK);   // Channel 2: L motor back
  ledcAttachPin(PINOUT_RIGHT_MOTOR_FRONT,
    PWM_CHANNEL_RIGHT_MOTOR_FRONT);                         // Channel 3: R motor front
  ledcAttachPin(PINOUT_RIGHT_MOTOR_BACK,PWM_CHANNEL_RIGHT_MOTOR_BACK); // Channel 4: R motor back

  maxResolution = (1<<cMotorResolution)-1;
}

void motorControl(bool lFront,bool rFront,float lPower,float rPower) {
  clamp(lPower,0.0f,float(maxResolution));
  clamp(rPower,0.0f,float(maxResolution));

  if (lFront) {
    ledcWrite(PWM_CHANNEL_LEFT_MOTOR_FRONT,lPower);
    ledcWrite(PWM_CHANNEL_LEFT_MOTOR_BACK,0);
  } else {
    ledcWrite(PWM_CHANNEL_LEFT_MOTOR_FRONT,0);
    ledcWrite(PWM_CHANNEL_LEFT_MOTOR_BACK,lPower);
  }

  if (rFront) {
    ledcWrite(PWM_CHANNEL_RIGHT_MOTOR_FRONT,rPower);
    ledcWrite(PWM_CHANNEL_RIGHT_MOTOR_BACK,0);
  } else {
    ledcWrite(PWM_CHANNEL_RIGHT_MOTOR_FRONT,0);
    ledcWrite(PWM_CHANNEL_RIGHT_MOTOR_BACK,rPower);
  }
}

// Slow down slowly
void motorIdle() {
  ledcWrite(PWM_CHANNEL_LEFT_MOTOR_FRONT,0);
  ledcWrite(PWM_CHANNEL_LEFT_MOTOR_BACK,0);
  ledcWrite(PWM_CHANNEL_RIGHT_MOTOR_FRONT,0);
  ledcWrite(PWM_CHANNEL_RIGHT_MOTOR_BACK,0);
}

// Strong break
void motorBrake() {
  ledcWrite(PWM_CHANNEL_LEFT_MOTOR_FRONT,maxResolution);
  ledcWrite(PWM_CHANNEL_LEFT_MOTOR_BACK,maxResolution);
  ledcWrite(PWM_CHANNEL_RIGHT_MOTOR_FRONT,maxResolution);
  ledcWrite(PWM_CHANNEL_RIGHT_MOTOR_BACK,maxResolution);
}

void motorLoop() {
  motorControl(true,true,60000,60000);
  delay(500);
  motorIdle();
  delay(1000);

  motorControl(true,true,60000,60000);
  delay(500);
  motorBrake();
  delay(2000);
}

float baseSpeedP = 1000.0f;

pid motorPID(6e4,1e4,1e4);

void motorForward(float _aimSpeed,int& _currentPower,float& _currentSpeed) {
  float currentSpeed = getSpeed();
  float delta = _aimSpeed-currentSpeed;
  float p = currentPower+motorPID.update(delta);


  oledPrint(p,"power",0);
  oledPrint(currentSpeed,"speed",1);

  motorControl(true,true,p,p);

  //   float currentSpeed = getSpeed();
  //   float delta        = _aimSpeed - currentSpeed;

  // //   currentPower += baseSpeedP * delta;
  //   clamp(currentPower, 0, cMaximumPower);

  //   _currentPower = currentPower;
  //   _currentSpeed = currentSpeed;
}

void motorBackward() { motorControl(false,false,cDefaultPower,cDefaultPower); }
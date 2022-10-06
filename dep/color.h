#pragma once

#include "../lib/arduino-esp32/libraries/Wire/src/Wire.h"
#include "boardLed.h"
#include "math.h"
#include "pinouts.h"

TwoWire* wire;

typedef struct {
  uint16_t Red;
  uint16_t Green;
  uint16_t Blue;
  uint16_t Clear;
} RGB;

unsigned char Re_buf;
unsigned char sign = 0;

RGB rgb;

const uint8_t colorSensorAddr = 0x5a; // 7 bit address: 0x5a (use this one); 8 bit address: 0xb4

uint8_t color = 0, rgb_data[3] = {0};
uint16_t CT = 0, Lux = 0;

void setBrightness(uint8_t);

void initColor() {
  wire = &Wire;
  wire->begin();     // Start master
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
  uint8_t output = 0x00;
  int count      = 0;

  wire->beginTransmission(colorSensorAddr); // Call device
  wire->write(0x00);                        // Ask to Config
  wire->endTransmission();                  // Stop transmission

  wire->requestFrom(colorSensorAddr, static_cast<uint8_t>(8));
  uint8_t buff[8];
  for (int b = 0; b < 8; b++) {
    buff[b] = 0;
  }

  wire->readBytes(buff, static_cast<uint8_t>(8));

  Serial.println("----output: ");
  for (int b = 0; b < 8; b++) {
    Serial.println(buff[b], HEX);
  }

  return output;
}

// void colorLoop() {
//   unsigned char data[9] = {0};

//   if (!sign) {
//     iic_read(0x00, data, 8);
//     rgb.Red   = (data[0] << 8) | data[1];
//     rgb.Green = (data[2] << 8) | data[3];
//     rgb.Blue  = (data[4] << 8) | data[5];
//     rgb.Clear = (data[6] << 8) | data[7];
//     Serial.print("Red: ");
//     Serial.print(rgb.Red);
//     Serial.print(",Green: ");
//     Serial.print(rgb.Green);
//     Serial.print(",Blue");
//     Serial.print(rgb.Blue);
//     Serial.print(",Clear");
//     Serial.println(rgb.Clear);
//     iic_read(0x08, data, 4);
//     Lux = (data[0] << 8) | data[1];
//     CT  = (data[2] << 8) | data[3];

//     Serial.print("CT:");
//     Serial.print(CT);
//     Serial.print(",Lux:");
//     Serial.println(Lux);
//     iic_read(0x0c, data, 3);
//     rgb_data[0] = data[0];
//     rgb_data[1] = data[1];
//     rgb_data[2] = data[2];
//     Serial.print("r:");
//     Serial.print(rgb_data[0]);
//     Serial.print(",g:");
//     Serial.print(rgb_data[1]);
//     Serial.print(",b:");
//     Serial.println(rgb_data[2]);
//     iic_read(0x0f, data, 1);
//     color = data[0];
//     Serial.print(",color:");
//     Serial.println(color, HEX);
//   }
//   if (sign == 1) {
//     iic_read(0x10, &data[8], 1);
//     i2c_start_wait(0xb4);
//     i2c_write(0x10);
//     i2c_write(0x31);
//     // i2c_write((data[8]|0x01));
//     i2c_stop();
//     sign = 3;
//   }
//   delay(200);
// }

// void iic_read(unsigned char add, unsigned char* data, unsigned char len) {
//   i2c_start_wait(0xb4);
//   i2c_write(add);
//   i2c_start_wait(0xb5);
//   while (len - 1) {
//     *data++ = i2c_readAck();
//     len--;
//   }
//   *data = i2c_readNak();
//   i2c_stop();
// }

// void serialEvent() {
//   while (Serial.available()) {
//     Re_buf = (unsigned char)Serial.read();
//     if (Re_buf == 'a')
//       sign = 0;
//     if (Re_buf == 'b')
//       sign = 1;
//     Re_buf = 0;
//   }
// }
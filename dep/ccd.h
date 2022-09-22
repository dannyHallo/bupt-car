#pragma once

#include "math.h"
#include "pinouts.h"

// CCD hardware
const int cNumPixels  = 128;
const int cCountStart = 15;
const int cCountEnd   = 127;

// Line definition
const int cEffectiveLineWidthMin = 10;
const int cEffectiveLineWidthMax = 36;

// Dark / light dynamic propagation
const float cDarkRatioStart       = 0.0f;
const float cDarkRatioPropagation = 0.01f;
const float cDarkRatioEnd         = 1.0f;

float minDarkRatio                  = 1.0f;
const float cAbnormalDarkRatioDelta = 0.2f;
const float cAbnormalDarkRatio      = 0.5f;

int linearData[cNumPixels]{};
bool binaryData[cNumPixels]{};
bool binaryOnehotData[cNumPixels]{};

void pinoutInitCCD() {
  pinMode(PINOUT_CCD_SI, OUTPUT);
  pinMode(PINOUT_CCD_CLK, OUTPUT);
  pinMode(PINOUT_CCD_AO, INPUT);

  digitalWrite(PINOUT_CCD_SI, LOW);  // IDLE state
  digitalWrite(PINOUT_CCD_CLK, LOW); // IDLE state
}

void captrueCCD(int explosureTimeMs) {
  digitalWrite(PINOUT_CCD_CLK, LOW);
  delayMicroseconds(1);
  digitalWrite(PINOUT_CCD_SI, HIGH);
  delayMicroseconds(1);

  digitalWrite(PINOUT_CCD_CLK, HIGH);
  delayMicroseconds(1);
  digitalWrite(PINOUT_CCD_SI, LOW);
  delayMicroseconds(1);

  digitalWrite(PINOUT_CCD_CLK, LOW);
  delayMicroseconds(2);

  /* and now read the real image */

  for (int i = 0; i < cNumPixels; i++) {
    digitalWrite(PINOUT_CCD_CLK, HIGH);

    delayMicroseconds(2);
    linearData[i] = analogRead(PINOUT_CCD_AO); // 8-bit is enough
    digitalWrite(PINOUT_CCD_CLK, LOW);
    delayMicroseconds(2);
  }

  digitalWrite(PINOUT_CCD_CLK, HIGH);
  delayMicroseconds(2);

  for (int t = 0; t < explosureTimeMs * 250; t++) {
    digitalWrite(PINOUT_CCD_CLK, LOW);
    delayMicroseconds(2);

    digitalWrite(PINOUT_CCD_CLK, HIGH);
    delayMicroseconds(2);
  }
}

void printCCDLinearData(int maxVal) {
  for (int i = 0; i < cNumPixels; i++) {
    int t = floor(float(linearData[i]) / float(maxVal) * 10.0f - 0.1f);
    Serial.print(char(48 + t));
  }
  Serial.println();
}

void printCCDBinaryRawData() {
  for (int i = 0; i < cNumPixels; i++) {
    char c = binaryData[i] ? 'x' : '-';
    Serial.print(c);
  }
  Serial.println();
}

void printCCDOneHotData() {
  for (int i = 0; i < cNumPixels; i++) {
    char c = binaryOnehotData[i] ? '^' : ' ';
    Serial.print(c);
  }
  Serial.println();
}

void processLinearVals(int& minVal, int& maxVal, int& avgVal) {
  maxVal = 0;
  minVal = 1e6;

  for (int i = cCountStart; i < cCountEnd; i++) {
    int currentVal = linearData[i];

    if (maxVal < currentVal)
      maxVal = currentVal;
    if (minVal > currentVal)
      minVal = currentVal;
  }
  avgVal = customRound(float(minVal + maxVal) / 2.0f);
}

void linearToRawBinary(int avgVal, float darkRatio) {

  for (int i = 0; i < cNumPixels; i++) {
    binaryData[i] = false;
  }

  for (int i = cCountStart; i < cCountEnd; i++) {
    binaryData[i] = (float(linearData[i]) < (avgVal * darkRatio)) ? true : false;
  }
}

void drawOneHot(int point) {
  for (int i = 0; i < cNumPixels; i++) {
    if (i == point) {
      binaryOnehotData[i] = true;
      continue;
    }
    binaryOnehotData[i] = false;
  }
}

bool getTrackMidPixel(int fromPixel, int& trackMidPixel, int& trackEndPixel) {
  int accumulatedDarkPixel = 0;

  int trackLeftPixel   = -1;
  int trackRightPixel  = -1;
  int trackMidPixelTmp = -1;

  for (int i = fromPixel; i < cCountEnd; i++) {
    bool currentPixel = binaryData[i];

    // Dark pixel
    if (currentPixel == true) {
      if (trackLeftPixel == -1) {
        trackLeftPixel = i;
      }
      accumulatedDarkPixel++;
    }

    // White pixel
    else {
      if (accumulatedDarkPixel >= customRound(cEffectiveLineWidthMin) &&
          accumulatedDarkPixel <= customRound(cEffectiveLineWidthMax)) {
        trackRightPixel = i - 1;
        break;
      }

      accumulatedDarkPixel = 0;
      trackLeftPixel       = -1;
    }
  }

  // Parse invalid, retain last array
  if (trackLeftPixel == -1 || trackRightPixel == -1)
    return false;

  // Get mid point
  if ((trackRightPixel - trackLeftPixel) % 2 == 0) {
    trackMidPixelTmp = customRound((trackRightPixel + trackLeftPixel) / 2.0f);
  } else {
    int trackMidPixelCandidate1 = customRound((trackRightPixel + trackLeftPixel - 1) / 2.0f);
    int trackMidPixelCandidate2 = trackMidPixelCandidate1 + 1;

    trackMidPixelTmp = (linearData[trackMidPixelCandidate1] < linearData[trackMidPixelCandidate2])
                           ? trackMidPixelCandidate1
                           : trackMidPixelCandidate2;
  }

  trackMidPixel = trackMidPixelTmp;
  trackEndPixel = trackRightPixel + 1;
  return true;
}

bool formerOneIsCloserToCenter(int a, int b) {
  int midPoint = customRound((cCountEnd - cCountStart) / 2.0f);
  return abs(a - midPoint) < abs(b - midPoint);
}

int getTrackMidPixel() {
  int trackMidPixelTmp = -1;
  int trackMidPixel    = 0;
  int trackEndPixel    = cCountStart;

  while (getTrackMidPixel(trackEndPixel, trackMidPixel, trackEndPixel)) {
    if (formerOneIsCloserToCenter(trackMidPixel, trackMidPixelTmp))
      trackMidPixelTmp = trackMidPixel;
  }

  return trackMidPixelTmp;
}

// int getBias() {
//   int midPoint = cNumPixels / 2;
//   int bias     = 0;
//   for (int i = 0; i < cNumPixels; i++) {
//     if (binaryData[i]) {
//       bias += (i - midPoint);
//     }
//   }
//   return bias / 8;
// }

void processCCD(int& trackMidPixel, bool& isNormal) {
  int minVal = 0;
  int maxVal = 0;
  int avgVal = 0;
  isNormal   = true;

  // Capture
  captrueCCD(40);

  // Parse linear data
  processLinearVals(minVal, maxVal, avgVal);

  Serial.print("avg: ");
  Serial.print(avgVal);
  Serial.print("    ");
  Serial.print("min: ");
  Serial.print(minVal);
  Serial.print("    ");
  Serial.print("max: ");
  Serial.print(maxVal);
  Serial.print("    ");
  Serial.print("minRatio: ");
  Serial.print(minDarkRatio);
  Serial.println();

  // Find track dynamically
  for (float currentDarkRatio = cDarkRatioStart; currentDarkRatio < cDarkRatioEnd;
       currentDarkRatio += cDarkRatioPropagation) {
    linearToRawBinary(avgVal, currentDarkRatio);
    trackMidPixel = getTrackMidPixel();

    if (trackMidPixel != -1) {
      // Compare ratio
      if (currentDarkRatio > cAbnormalDarkRatio) {
        isNormal = false;
      }

      // Save min ratio if needed
      if (currentDarkRatio < minDarkRatio) {
        minDarkRatio = currentDarkRatio;
      }

      drawOneHot(trackMidPixel);
      Serial.print("currentDarkRatio: ");
      Serial.println(currentDarkRatio);
      break;
    }
  }

  // Cannot find track
  if (trackMidPixel == -1) {
    isNormal = false;
    return;
  }

  // Debug
  printCCDLinearData(maxVal);
  printCCDBinaryRawData();
  printCCDOneHotData();

  // Map pixel
  trackMidPixel =
      customRound(map(float(trackMidPixel), float(cCountStart), float(cCountEnd), 0.0f, 128.0f));
}
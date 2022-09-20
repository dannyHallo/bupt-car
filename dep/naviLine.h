#pragma once
#include "ccd.h"

const int MidPixel = cNumPixels/2;
const int histSize = 20;
const int maxNoiseTorelance = 2;

class naviLine {
public:
    int midLine = MidPixel;

    naviLine() {
        captrueCCD();
        LTBrightness = avgBrightness = getAvgBrightness();
        for (int i = 0; i<histSize; i++) {
            BrightnessHistory[i] = avgBrightness;
        }
    }

    int getMidLine() {
        captrueCCD();
        clockCycle++;;
        if (clockCycle%400==0) {
            updateLTBrightness();
        }

        /*
        findLB(MidPixel);
        findRB(MidPixel);

        //1. Line in the center
        int flag = isLine();
        if (flag==1) {
            return midLine = (lb+rb)/2;
        } else if (flag==-1) {
            return midLine = -1;
        }

        //2. Line in the left
        lb = MidPixel;
        while (true) {
            findLS(lb);
            rb = ls;
            findLB(ls);
            flag = isLine();
            if (flag==1) {
                return midLine = (lb+rb)/2;
            } else if (flag==-1) {
                return midLine = -1;
            }
        }
        */

        int flag = 0,tmpLine = MidPixel;
        rs = 0;
        while (rb<cNumPixels-2) {
            findRS(rs);
            lb = rs;
            findRB(lb);
            switch (isLine()) {
            case 1:
                midLine = (lb+rb)/2;
                if (abs(MidPixel-midLine)<abs(MidPixel-tmpLine)) {
                    tmpLine = midLine;
                } else if (midLine>MidPixel) {
                    return tmpLine;
                }
                break;
            case 0:
                flag = flag==-1 ? -1 : 0;
                break;
            case -1:
                return flag = -1;
                break;
            }
            rs = rb;
        }

        return tmpLine;
    }

private:
    int lb = MidPixel,rb = MidPixel,avgBrightness = 0,LTBrightness = 0,histI = 0,ls,rs;
    int BrightnessHistory[histSize];
    bool Binaries[cNumPixels];

    void Binarize() {
        for (int i = 0; i<cNumPixels; i++) {
            Binaries[i] = (linearPixelsData[i]<avgBrightness*0.8);
        }
    }

    int isLine() {
        if (abs(rb-lb)>minWidth&&abs(rb-lb)<maxWidth) {
            return 1;
        } else if (abs(rb-lb)>maxWidth&&((rb>=cNumPixels-2)||(lb<=2))) {
            return 1;
        } else if (abs(rb-lb)>maxWidth) {
            return -1;
        } else {
            return 0;
        }
    }

    int findLB(int start) {
        int i = start;
        while (true) {
            i--;
            if (i<2) {
                return lb = 1;
            } else if (!Binaries[i]) {
                int j = i-1;
                while (!Binaries[j]) {
                    j--;
                    if (j<2) {
                        return lb = 1;
                    } else if (i-j>maxNoiseTorelance) {
                        return lb = i;
                    }
                }
            }
        }
    }

    int findRB(int start) {
        int i = start;
        while (true) {
            i++;
            if (i>cNumPixels-2) {
                return rb = cNumPixels-1;
            } else if (!Binaries[i]) {
                int j = i+1;
                while (!Binaries[j]) {
                    j++;
                    if (j>cNumPixels-2) {
                        return rb = cNumPixels-1;
                    } else if (j-i>maxNoiseTorelance) {
                        return rb = i;
                    }
                }
            }
        }
    }

    int findLS(int start) {
        int i = start;
        while (i>2) {
            i--;
            if (Binaries[i]) {
                int j = i-1;
                while (Binaries[j]) {
                    j--;
                    if (j<2) {
                        return ls = 1;
                    } else if (i-j>maxNoiseTorelance) {
                        return ls = i;
                    }
                }
            }
        }
    }

    int findRS(int start) {
        int i = start;
        while (i<cNumPixels-2) {
            i++;
            if (Binaries[i]) {
                int j = i+1;
                while (Binaries[j]) {
                    j++;
                    if (j>cNumPixels-2) {
                        return rs = cNumPixels-1;
                    } else if (j-i>maxNoiseTorelance) {
                        return rs = i;
                    }
                }
            }
        }
    }

    int getAvgBrightness() {
        int sum = 0;
        for (int i = 0; i<cNumPixels; i++) {
            sum += linearPixelsData[i];
        }
        return sum/cNumPixels;
    }

    int updateLTBrightness() {
        int sum = LTBrightness*histSize;
        sum -= BrightnessHistory[histI];
        histI = (histI+1)%histSize;
        avgBrightness = getAvgBrightness();
        BrightnessHistory[histI] = avgBrightness;
        sum += avgBrightness;
        return LTBrightness = sum/histSize;
    }
};
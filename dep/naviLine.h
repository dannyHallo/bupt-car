#pragma once
#include "ccd.h"

const int MidPixel = cNumPixels/2;
const int histSize = 20;
const int maxNoiseTorelance = 2;

class naviLine {
private:
    int lb = MidPixel,rb = MidPixel,avgBrightness = 0,LTBrightness = 0,histI = 0,ls,rs;
    int BrightnessHistory[histSize];
    bool Binaries[cNumPixels];

    void Binarize() {
        LTBrightness = get3rdHighestBrigheness();
        for (int i = 0; i<cNumPixels; i++) {
            Binaries[i] = (linearPixelsData[i]<LTBrightness*0.75);
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
        return rs = cNumPixels-1;
    }

    int getAvgBrightness() {
        return get3rdHighestBrigheness();


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

    int get3rdHighestBrigheness() {
        int max1 = 0,max2 = 0,max3 = 0;
        for (int i = 0; i<cNumPixels; i++) {
            if (linearPixelsData[i]>max1) {
                max3 = max2;
                max2 = max1;
                max1 = linearPixelsData[i];
            } else if (linearPixelsData[i]>max2) {
                max3 = max2;
                max2 = linearPixelsData[i];
            } else if (linearPixelsData[i]>max3) {
                max3 = linearPixelsData[i];
            }
        }
        return max3;
    }


public:
    int midLine = MidPixel;

    void initNaviLine() {
        captrueCCD();
        Serial.println("NaviLine Init");
        LTBrightness = avgBrightness = getAvgBrightness();
        for (int i = 0; i<histSize; i++) {
            BrightnessHistory[i] = avgBrightness;
        }
        Serial.printf("LTBrightness: %d\n",LTBrightness);
    }

    int getMidLine() {
        captrueCCD();
        // clockCycle++;;
        // if (clockCycle%20==0) {
        //     updateLTBrightness();
        // }
        Binarize();

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
        rs = 0;rb = 0;
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
            // Serial.printf("rs: %d, rb: %d, lb: %d, midLine: %d, tmpLine: %d, flag: %d\n");
        }

        return tmpLine;

        // midLine = 0;
        // for (int i = 0;i<cNumPixels;i++) {
        //     midLine += Binaries[i]*(i-MidPixel);
        // }
        // return midLine;

    }

    void printBinarizedPixels() {
        for (int i = 0; i<cNumPixels; i++) {
            Serial.print(Binaries[i] ? '*' : '-');
        }
        Serial.println();
    }

    void printLinearPixels() {
        for (int i = 0; i<cNumPixels; i++) {
            Serial.printf("%d ",linearPixelsData[i]);
        }
        Serial.println();
    }

    void printLTBrightness() {
        Serial.printf("LTBrightness: %d\n",LTBrightness);
    }

};
#pragma once

#include "../args.h"

typedef struct {
    int count;
    int cargo_type[platform_num];
    char additional_info[16];
} bt_package;

class bt_data {
public:
    void set_cargo(int location,int color) {
        package.cargo_type[(location-1)%platform_num] = color;
    }
    void set_count(int count) {
        package.count = count;
    }
    char* encode() {
        sprintf(encode_data,"%d,%d,%d,%d,%d,%s",package.count,package.cargo_type[0],package.cargo_type[1],package.cargo_type[2],package.cargo_type[3],package.additional_info);
        return encode_data;
    }
private:
    bt_package package;
    char encode_data[128];
};
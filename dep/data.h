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
        char* data = (char*)malloc(sizeof(bt_package));
        memcpy(data,&package,sizeof(bt_package));
        return data;
    }
private:
    bt_package package;
};
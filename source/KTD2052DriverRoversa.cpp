#include "MicroBit.h"
//#include "MicroBitI2C.h"
#include "KTD2052DriverRoversa.h"

#include <cmath>
#include <stdint.h>

extern MicroBit uBit;
//extern MicroBitI2C i2c;

// RGB maximum current settings
const int rmax = 64; // 64/8 = 8mA
const int gmax = 64; // 64/8 = 8mA
const int bmax = 80; // 80/8 = 10mA

// Control Mode Configuration
const int en_mode = 2; // EnableMode: 1=night, 2=normal(day)
const int be_en = 1;   // BrightExtend: 0=disabled, 1=enabled
const int ce_temp = 2; // CoolExtend: 0=135C, 1=120C, 2=105C, 3=90C

// Calculations
const int on = en_mode * 64 + be_en * 32 + ce_temp * 8; // calculate global on
const int off = be_en * 32 + ce_temp * 8;               // calculate global off

// RGB list for several for-loops
const int rgb_list[] = {1, 2, 3, 4};

// Fade-rate exponential time-constant list
const double fade_list[] = {0.032, 0.063, 0.125, 0.25, 0.5, 1.0, 2.0, 4.0};

// Generate the scaled sRGB integer lists using floating-point precision
const double alpha = 0.04045;
const double phi = 12.92;
const double gam = 2.4;
int red_list[256];
int grn_list[256];
int blu_list[256];


void populate_RGB_arrays() {
    static bool array_calc = false; //run flag to only fill RGB array once
    if (!array_calc) {
        for (int i = 1; i < 24; i++) {
            red_list[i] = round(((i / 255.0) / phi) * rmax);
            grn_list[i] = round(((i / 255.0) / phi) * gmax);
            blu_list[i] = round(((i / 255.0) / phi) * bmax);
        }
        for (int i = 24; i < 256; i++) {
            red_list[i] = round(pow((i / 255.0 + alpha) / (1 + alpha), gam) * rmax);
            grn_list[i] = round(pow((i / 255.0 + alpha) / (1 + alpha), gam) * gmax);
            blu_list[i] = round(pow((i / 255.0 + alpha) / (1 + alpha), gam) * bmax);
        }
        array_calc = true;
    }
}

void i2c_write(uint8_t SID, uint8_t regAddr, uint8_t regData) {
    char buffer[2];
    buffer[0] = regAddr;
    buffer[1] = regData;
    uBit.i2c.write(SID << 1, (uint8_t*)buffer, 2);
}

uint8_t i2c_read(uint8_t SID, uint8_t regAddr) {
    char buffer[1];
    buffer[0] = regAddr;
    uBit.i2c.write(SID << 1, (uint8_t*)buffer, 1);
    uBit.i2c.read(SID << 1, (uint8_t*)buffer, 1);
    return buffer[0];
}

// Pattern Generator Registers Core Functions
void pattern_ctrl(int pg_mode, int pg_time, int fade1) {
    int regData = 64 * pg_mode + 8 * pg_time + fade1;
    i2c_write(SID, 0x0F, regData);
}

void pattern_fade(int regData) {
    i2c_write(SID, 0x10, regData);
}

void pattern_rgbn(int rgbn, int regData) {
    i2c_write(SID, 0x10 + rgbn, regData);
}

void pattern_all(int regData) {
    populate_RGB_arrays();
    for (unsigned int i = 0; i < sizeof(rgb_list) / sizeof(rgb_list[0]); i++) {
        pattern_rgbn(rgb_list[i], regData);
    }
}

// Cycles is number of cycles before watchdog times out.
// You can periodically refresh the cycles before it times out.
// When cycles times out, the chip fades to zero and goes to low current standby mode.
// 255 disables watchdog and runs until pattern generator mode is turned off.
// Note: always write to the watchdog register twice.
void pattern_watchdog(int cycles) {
    i2c_write(SID,0x15,cycles);
    i2c_write(SID,0x15,cycles);
}

// Control Register Core Functions
void global_on(int fade0) {
    i2c_write(SID, 0x02, on + fade0);
}

void global_off(int fade0) {
    i2c_write(SID, 0x02, off + fade0);
}

void global_reset() {
    i2c_write(SID, 0x02, 0xC0);
}

void fade_off() {
    global_off(2); // fade0=2
    pattern_ctrl(0, 0, 0); // turn pattern generator off
}

// Color Setting Registers Core Functions
void color_rgbn(int rgbn, int r, int g, int b) {
    populate_RGB_arrays();
    int regAddr = 3 * rgbn;
    i2c_write(SID, regAddr, red_list[r]);
    i2c_write(SID, regAddr + 1, grn_list[g]);
    i2c_write(SID, regAddr + 2, blu_list[b]);
}

void color_all(int r, int g, int b) {
    for (unsigned int i = 0; i < sizeof(rgb_list) / sizeof(rgb_list[0]); i++) {
        color_rgbn(rgb_list[i], r, g, b);
    }
}

// rgbn = 1, 2, 3 or 4
void color_rgbn_random(int rgbn) {
    int r = rand() % 255;
    int g = rand() % 255;
    int b = rand() % 255;
    color_rgbn(rgbn, r, g, b);
}
    
void color_all_random() {
    int r = rand() % 255;
    int g = rand() % 255;
    int b = rand() % 255;
    color_all(r, g, b);
}

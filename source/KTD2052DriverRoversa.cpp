/*
Adapted C++ code from "Rudolph KTD2052 Demo Software for QtPy RP2040 CircuitPython" by Karl Volk, Kinetic Technologies
Rewritten to support microbit and Roversa - Eric Bredder, 2025
*/

#include "MicroBit.h"
//#include "MicroBitI2C.h"
#include "KTD2052DriverRoversa.h"
#include <mutex>
#include <cmath>
#include <stdint.h>

extern MicroBit uBit;
//extern MicroBitI2C i2c;

/*
RGB maximum current settings per LED Temperature Derating, Ipulse(max) & per
good White Color Balance or the Application's Brightness requirement.
The sRGB color coordinates will be scaled by the below maximums.
*/
// RGB maximum current settings
const int rmax = 64; // 64/8 = 8mA
const int gmax = 64; // 64/8 = 8mA
const int bmax = 80; // 80/8 = 10mA

/*
In this case, 8mA is by brightness choice, but blue is increased for balance.
Generally, the Everbright RGB can run as high as 15mA for red
and 16mA for green and blue indefinitly up to Ta=50C,
per the manufacturer's recommendations.
*/

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

// Generate the scaled sRGB integer lists using doubles for sRGB calculations
const double alpha = 0.04045;
const double phi = 12.92;
const double gam = 2.4;
int red_list[256];
int grn_list[256];
int blu_list[256];

// Create a once_flag to ensure that RGB arrays are only populated once if they are used
std::once_flag RGB_flag;

//generate the RGB arrays for pattern_all() and color_rgbn() which are used in a few functions
void populate_RGB_arrays() {
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
    //uBit.display.scrollAsync("KTD2052 Calcs OK"); //check if this is run once

}

void RGB_calc() {
    std::call_once(RGB_flag, populate_RGB_arrays);
}

// i2c handlers for micro:bit
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
/*
pg_mode:  0=off, 1=4slots, 2=6slots, 3=8slots
pg_time:  0=188ms, 1=250ms, 2=375ms, 3=500ms, 4=750ms, 5=1s, 6=1.5s, 7=2s per slots
fade1: 0=31ms, 1=63ms, 2=125ms, 3=250ms, 4=500ms, 5=1s, 6=2s, 7=4s exponential time constant
*/
void pattern_ctrl(int pg_mode, int pg_time, int fade1) {
    int regData = 64 * pg_mode + 8 * pg_time + fade1;
    i2c_write(SID, 0x0F, regData);
}

void pattern_fade(int regData) {
    i2c_write(SID, 0x10, regData); // use binary for regData (e.g. 0b00011111)

}

void pattern_rgbn(int rgbn, int regData) {
    i2c_write(SID, 0x10 + rgbn, regData); // rgbn = 1, 2, 3 or 4, use binary for regData (e.g. 0b00011111)
}

void pattern_all(int regData) {
    RGB_calc();    
    for (unsigned int i = 0; i < sizeof(rgb_list) / sizeof(rgb_list[0]); i++) {
        pattern_rgbn(rgb_list[i], regData); // use binary for regData (e.g. 0b00011111)
    }
}

/* 
Cycles is number of cycles before watchdog times out.
You can periodically refresh the cycles before it times out.
When cycles times out, the chip fades to zero and goes to low current standby mode.
255 disables watchdog and runs until pattern generator mode is turned off.
Note: always write to the watchdog register twice.
*/ 
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
void color_rgbn(int rgbn, int r, int g, int b) { // rgbn = 1, 2, 3 or 4
    RGB_calc();
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

void color_rgbn_random(int rgbn) { // rgbn = 1, 2, 3 or 4
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

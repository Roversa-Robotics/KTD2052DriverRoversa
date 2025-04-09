#ifndef KTD2052DRIVER_H
#define KTD2052DRIVER_H
#include <stdint.h>

const uint8_t SID = 0x74;

void populate_RGB_arrays();
void i2c_write(uint8_t, uint8_t, uint8_t);
uint8_t i2c_read(uint8_t, uint8_t);
void pattern_ctrl(int, int, int);
void pattern_fade(int);
void pattern_rgbn(int, int);
void pattern_all(int);
void pattern_watchdog(int);
void global_on(int);
void global_off(int);
void global_reset();
void fade_off();
void color_rgbn(int, int, int, int);
void color_all(int, int, int);
void color_rgbn_random(int);
void color_all_random();

#endif // KTD2052DRIVER_H
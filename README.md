# Kinetic KTD2052 I2C LED Module Driver for micro:bit

This is a C++ version of the vendor driver from the Kinetic KTD2052 I2C LED module that was pre-loaded on the QTPy. This is specfically designed to work with the micro:bit using [codal-microbit-v2](https://github.com/lancaster-university/microbit-v2-samples). We designed this to integrate with Roversa, but can work without. You have immediate access to the driver board and the vendor functions. 

[KT2052 Datasheet](https://www.mouser.com/datasheet/2/936/KTD2052_04b-2887351.pdf)

To use the library, add:
`#include "KTD2052DriverRoversa.h"`
to **main.cpp** to start using the I2C LED module.

For example in **main.cpp**:
```cpp
#include "MicroBit.h"
#include "KTD2052DriverRoversa.h"

MicroBit uBit;

int main()
{
  uBit.init();
  while(1){
    fade_off()
    color_all(0,255,0)  // all green
    global_on(4)  // fade0=4
  }
}
```

### Control Register Core Functions

`global_off(int)` and `global_on(int)`

These functions are based on the control mode configuration. See the comments in the Control Mode Configuratino comment block. Default values are EnableMode = 2 (Normal), BrightExtend = 1 (Enabled), and CoolExtend = 2 (105C). The parameter for these functions controls the fade value.

`global_reset()` 

Full reset of values and settings.

`color_rgbn(int,int,int,int)`

Sets one LED RGB value starting at index 1. For example, `color_rgbn(2,255,0,0);` would set the second LED red.

`color_all(int,int,int)`

Sets all the LEDs at one RGB value. For example, `color_all(0,255,0);` would set all connected LEDs to green.



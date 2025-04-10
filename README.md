# Kinetic KTD2052 I2C LED Module Driver for micro:bit

This is a C++ version of the vendor driver from the Kinetic KTD2052 I2C LED module that was pre-loaded on the QTPy. This is specfically designed to work with the micro:bit using [codal-microbit-v2](https://github.com/lancaster-university/microbit-v2-samples). We designed this to integrate with Roversa, but can work without. You have immediate access to the driver board and the vendor functions. 

To use the library, add:
`#include "KTD2052DrifverRoversa.h"`
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
    global_off(0);
    color_rgbn_random(2);
    color_rgbn(3, 0, 0, 0);
    global_on(2);
    uBit.sleep(1000);
  }
}
```

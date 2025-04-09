# Kinetic KTD2052 I2C LED Module Driver for micro:bit | Roversa

This is a C++ version of the vendor driver from the Kinetic KTD2052 I2C LED module that was pre-loaded on the QTPy. This is specfically designed to work with the micro:bit using [codal-microbit-v2](https://github.com/lancaster-university/microbit-v2-samples). We designed this to integrate with Roversa, but can work without. You have immediate access to the driver board and the vendor functions. 


To use the library, add:
`#include "KTD2052DrifverRoversa.h"`
to **main.cpp** to start using the I2C LED module.

For example in **main.cpp**:

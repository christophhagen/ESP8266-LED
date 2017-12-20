/* An overview of the functions of the arduino core can be found here:
https://github.com/esp8266/Arduino/blob/master/doc/reference.md
*/
#include <Arduino.h>

#include "colors.h"
#include "customize.h"

#ifndef UDP_DEFAULT_PORT
#define UDP_DEFAULT_PORT  8000
#endif

#ifndef SERVER_PORT
#define SERVER_PORT       80
#endif

// Lets the user set up the led devices
void setupLEDs();

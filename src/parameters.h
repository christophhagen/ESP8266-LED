#ifndef __PARAMETERS_H
#define __PARAMETERS_H

/**
This project creates a url/udp api for controlling an LED strip connected
to an ESP8266 chip. It can be used in combination with a homebridge plug-In
to enable Siri voice control.

API structure:

/get/status
/get/hue
/get/saturation
/get/brightness
/get/color

/set/on
/set/off
/set/toggle

/set/hue
/set/saturation
/set/brightness
/set/color

*/

#include <FastLED.h>  /* LED strip control https://github.com/FastLED/FastLED */

// Defines for the Device
#define NR_OF_LEDS        299

#define STRIP_TYPE        WS2812B   // Your strip model
#define COLOR_TYPE        GRB       // My strip switches red and green

#define DATA_PIN          5
// #define CLOCK_PIN         0         // Use this if your strip has two lines

#define UDP_DEFAULT_PORT  8000
#define SERVER_PORT       80
#define EX_TIME           20

#define WIFI_SSID         "Skynet"
#define WIFI_PASSWORD     "outbound-admire-thimble-assert"


// The default color for turning the strip on (only on boot and when color is 0)
extern CHSV defaultHSV;

// Turn the strip on
void enable();

// Turn the strip off
void disable();

// Change off <-> on
void toggle();

// Get the current status of the strip
uint8_t getEnabled();

// Set the new HSB color. Defined in "main.cpp"
void setHSV(CHSV color);

// Get the color
CHSV getHSV();

// Setup the server. Defined in "url.cpp"
void setupServer();

// Handle url requests. Defined in "url.cpp"
void handleClient();

// Read the default color upon boot. Defined in "url.cpp"
void readDefaultColor();

#endif

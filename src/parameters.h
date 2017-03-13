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
#define DATA_PIN          5
#define STRIP_TYPE        WS2812B

#define UDP_DEFAULT_PORT  8000
#define SERVER_PORT       80
#define EX_TIME           20

#define WIFI_SSID         "Skynet"
#define WIFI_PASSWORD     "outbound-admire-thimble-assert"

// The current HSV color
extern CHSV endHSV;

void setHSV(CHSV color);

// Setup the server. Defined in "url.cpp"
void setupServer();

// Handle url requests. Defined in "url.cpp"
void handleClient();


#endif

/* An overview of the functions of the arduino core can be found here:
https://github.com/esp8266/Arduino/blob/master/doc/reference.md

*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>            // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFiUdp.h

/* Other libraries */
#include <FastLED.h>            /* LED strip control https://github.com/FastLED/FastLED */
#include <SimpleScheduler.h>    /* Simple task scheduling */

#include "parameters.h"

/* WiFi credentials */
const char* ssid = WIFI_SSID;
const char* pass = WIFI_PASSWORD;

WiFiUDP udp;                    // UDP instance to receive colors
CRGB colors[NR_OF_LEDS];        // Array with colors for the leds
CHSV endHSV;                    // HSV representation of current color
CHSV defaultHSV;                // The default color to go to

/* Scheduler functions */
void blendColor();
void receiveUDPPacket();

/* Scheduler tasks */
Task serverResponse(handleClient, 100);
Task receiveTask(receiveUDPPacket, 100);
Task blendTask(blendColor, EX_TIME, false);

/* colors needed for blending */
CRGB endRGB, cur;
uint8_t enabled = 0;

uint8_t getEnabled() {
    return enabled;
}

void enable() {
    enabled = 1;
    endRGB = CRGB(endHSV);
    if (endRGB == CRGB(0,0,0)) { // Set to default if enabled but color is 0
        endHSV = defaultHSV;
        endRGB = CRGB(defaultHSV);
    }
    blendTask.enable(); // Start blending
}

void disable() {
    enabled = 0;
    endRGB = CRGB(0,0,0);
    blendTask.enable(); // Start blending
}

void toggle() {
    (enabled) ? disable() : enable();
}

/* Enable or disable the strip
* 0: turn off
* 1: turn on
* other: toggle
*/
void setEnable(uint8_t newStatus) {
    switch (newStatus) {
        case 0:
        disable();
        return;

        case 1:
        enable();
        return;

        default:
        toggle();
    }
}


/* Set necessary values for blending and start */
void setHSV(CHSV color) {
    endHSV = color;

    // Turn strip off, if brightness is 0
    if (color.value == 0) {
        disable();
    } else {
        endRGB = CRGB(color); // Set end color
        enable();
    }
}

CHSV getHSV() {
    return endHSV;
}

/* Small steps towards the end color */
void blendColor() {
    // blend rgb, 1 step per channel
    for (uint8_t i = 0; i < 3; i++) {
        if (cur.raw[i] != endRGB.raw[i]) {
            cur.raw[i] += (cur.raw[i] > endRGB.raw[i]) ? -1 : 1;
        }
    }

#ifdef IS_WALL
    for(uint8_t i = 0; i < NR_LED_CHANGE; i++) {
        colors[i] = cur;
    }
    for(uint8_t i = NR_LED_CHANGE; i < NR_OF_LEDS; i++) {
        colors[i] = CRGB(cur.b, cur.g, cur.r);
    }
    FastLED.show();
#else
    FastLED.showColor(cur);
#endif

    if (cur == endRGB) { // Check for end of fade
        blendTask.disable();
    }
}

/**
Handle all packets received through UDP. A packet can either contain:
 1 byte: on (>0), off (== 0)
 2 byte: (param, value): 0: hue, 1: saturation, 2: brightness
 3 byte: hue, saturation, brightness

 Other packets will be ignored
*/
void receiveUDPPacket() {
    uint16_t bytes = udp.parsePacket();
    uint8_t param;
    CHSV color = getHSV();
    switch (bytes) {
        case 1:
        setEnable(udp.read());
        break;

        case 2:
        param = udp.read();
        if (param > 2) { break; }
        color.raw[param] = udp.read();
        setHSV(color);
        break;

        case 3:
        udp.read((char*) &color, 3); // Read the end color
        setHSV(color);
        break;

        default:
        break;
    }
    udp.flush();
}

/**
Set up UDP, WIFI, LEDs, Serial and Web server
*/
void setup() {
    #ifdef IS_WALL
    FastLED.addLeds<STRIP_TYPE, DATA_PIN, CLOCK_PIN, COLOR_TYPE>(colors, NR_OF_LEDS);
    #else
    FastLED.addLeds<STRIP_TYPE, DATA_PIN, COLOR_TYPE>(colors, NR_OF_LEDS);
    #endif

    FastLED.showColor(CRGB(0,0,0));

    readDefaultColor();

    WiFi.begin(ssid, pass);
    setupServer();
    udp.begin(UDP_DEFAULT_PORT);
}

/**
Run the task manager
*/
void loop() {
    Task::runTasks();
}

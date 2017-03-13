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

WiFiUDP udp;                          // UDP instance to receive colors
CRGB colors[NR_OF_LEDS];              // Array with colors for the leds
CHSV endHSV = CHSV(50, 150, 0);       // HSV representation of current color

/* Scheduler functions */
void blendColor();
void receiveUDPPacket();

/* Scheduler tasks */
Task serverResponse(handleClient, 100);
Task receiveTask(receiveUDPPacket, 100);
Task blendTask(blendColor, EX_TIME, false);

/* colors needed for blending */
CRGB endRGB, count, cur, steps, dir;

/* Set necessary values for blending and start */
void setHSV(CHSV color) {
    // Set end color
    endRGB = CRGB(color);

    // Get differences
    CRGB d;
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t c = cur.raw[i];
        uint8_t e = endRGB.raw[i];
        //(c > e) ? (d.raw[i] = c - e, dir.raw[i] = 1) : (d.raw[i] = e - c, dir.raw[i] = 0);
        if (c > e) {
            d.raw[i] = c - e;
            dir.raw[i] = 1;
        } else {
            d.raw[i] = e - c;
            dir.raw[i] = 0;
        }
    }

    // Get maximum
    uint8_t max = (d.r > d.g) ? d.r : d.g;
    max = (max > d.b) ? max : d.b;

    // Get step size
    steps.r = (d.r == 0) ? 0 : max / d.r;
    steps.g = (d.g == 0) ? 0 : max / d.g;
    steps.b = (d.b == 0) ? 0 : max / d.b;

    count = CRGB(0,0,0); // Set count to zero
    blendTask.enable(); // Start blending
}

/* This function will be executed in the specified interval */
void blendColor() {

    for (uint8_t i = 0; i < 3; i++) {
        count.raw[i] += 1;
        // If step count is reached, advance color
        if (count.raw[i] == steps.raw[i] && cur.raw[i] != endRGB.raw[i]) {
            cur.raw[i] += (dir.raw[i]) ? -1 : 1;
            count.raw[i] = 0;
        }
    }

    if (cur == endRGB) { // Check for end of fade
        blendTask.disable();
    }

    FastLED.showColor(cur);
    return;
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
    switch (bytes) {
        case 1:
        endHSV.value = udp.read() > 0 ? 255 : 0;
        break;

        case 2:
        param = udp.read();
        if (param > 2) { return; }
        endHSV.raw[param] = udp.read();
        break;

        case 3:
        udp.read((char*) &endHSV, 3); // Read the end color
        break;

        default:
        udp.flush();
        return;
    }
    setHSV(endHSV);
}

/**
Set up UDP, WIFI, LEDs, Serial and Web server
*/
void setup() {
    Serial.begin(115200);

    FastLED.addLeds<STRIP_TYPE, DATA_PIN, RGB>(colors, NR_OF_LEDS);
    FastLED.showColor(CRGB(0,0,0));

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

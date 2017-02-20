/* An overview of the functions of the arduino core can be found here:
https://github.com/esp8266/Arduino/blob/master/doc/reference.md

*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>   // SERVER - https://links2004.github.io/Arduino/d3/d58/class_e_s_p8266_web_server.html
#include <WiFiUdp.h>            // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFiUdp.h

/* Other libraries */
#include <FastLED.h>            /* LED strip control https://github.com/FastLED/FastLED */
#include <SimpleScheduler.h>    /* Simple task scheduling */

// Defines for the Device "Bed"
#define NROFLEDS          299
#define DATA_PIN          5

#define UDP_DEFAULT_PORT  8000
#define SERVER_PORT       80
#define EX_TIME           20

ESP8266WebServer server(SERVER_PORT); // Webserver to test online status
WiFiUDP udp;                          // UDP instance to receive colors
CRGB colors[NROFLEDS];                // Array with colors for the leds

/* Server functions */
void handleNotFound() { server.send(404, "text/plain", "on"); }
void respondOn() { server.send(200, "text/plain", "on"); }

/* Scheduler functions */
void blendColor();
void receiveUDPPacket();
void handleClient() { server.handleClient(); }

/* Scheduler tasks */
Task serverResponse(handleClient, 100);
Task receiveTask(receiveUDPPacket, 100);
Task blendTask(blendColor, EX_TIME, false);

/* WiFi credentials */
const char* ssid =  "Skynet";
const char* pass = "outbound-admire-thimble-assert";

/* colors needed for blending */
CRGB end, count, cur, steps, dir;
CRGB buttonOld = CRGB(255,255,255);

/* Set necessary values for blending */
void setBlending() {
    // Get differences
    CRGB d;
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t c = cur.raw[i];
        uint8_t e = end.raw[i];
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

    // Set count to zero
    count = CRGB(0,0,0);
}

/* This function will be executed in the specified interval */
void blendColor() {

    for (uint8_t i = 0; i < 3; i++) {
        count.raw[i] += 1;
        // If step count is reached, advance color
        if (count.raw[i] == steps.raw[i] && cur.raw[i] != end.raw[i]) {
            cur.raw[i] += (dir.raw[i]) ? -1 : 1;
            count.raw[i] = 0;
        }
    }

    // Check for end of fade
    if (cur == end) {
        blendTask.disable();
    }
    fill_solid(colors, NROFLEDS, cur);
    FastLED.show();
    return;
}

void receiveUDPPacket() {
    uint16_t bytes = udp.parsePacket();
    if (bytes == 0) {
        return;
    }

    if (bytes == 3) {
        CHSV hsvEnd;
        udp.read((char*) &hsvEnd, 3); // Read the end color
        end = CRGB(hsvEnd);

    } else if (bytes == 1) {
        if (cur == CRGB(0,0,0) || blendTask.isEnabled()) {
            end = buttonOld;
        } else {
            buttonOld = cur;
            end = CRGB(0,0,0);
        }
    }
    setBlending();
    blendTask.enable();
    udp.flush(); // If incorrect length, cancel
}

void setup() {
    //Serial.begin(115200);

    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(colors, NROFLEDS);
    FastLED.showColor(CRGB(0,0,0));

    WiFi.begin(ssid, pass);
    server.onNotFound(handleNotFound);
    server.on("/on", respondOn);
    server.begin();
    udp.begin(UDP_DEFAULT_PORT);
}

void loop() {
    Task::runTasks();
}

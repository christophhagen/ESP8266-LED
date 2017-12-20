#include "api.h"

/* Other libraries */
#include <SimpleScheduler.h>    /* Simple task scheduling */

// SERVER - https://links2004.github.io/Arduino/d3/d58/class_e_s_p8266_web_server.html
#include <ESP8266WebServer.h>

#include <WiFiUdp.h>            // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFiUdp.h

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Handle url requests. Defined in "url.cpp"
void handleClient();

void receiveUDPPacket();

Task serverResponse(handleClient, 100);

Task receiveTask(receiveUDPPacket, 100);

/*  Webserver to give access to url api */
ESP8266WebServer server(SERVER_PORT);

WiFiUDP udp;                       // UDP instance to receive colors

/* WiFi credentials */
const char* ssid = WIFI_SSID;
const char* pass = WIFI_PASSWORD;

static inline void printColor(CHSV color, char* mess) {
    sprintf(mess, "%02x%02x%02x", color.hue, color.sat, color.val);
}

static inline void printRGB(CHSV color, uint8_t index, char* mess) {
    CRGB rgb = CRGB(color);
    sprintf(mess, "%d", rgb[index]);
}

/**
 Process a string containing the 3 hex values of a HSV color.
*/
static CHSV colorFromString(String str) {
    uint8_t h = strtol(str.substring(0,2).c_str(), NULL, 16);
    uint8_t s = strtol(str.substring(2,4).c_str(), NULL, 16);
    uint8_t v = strtol(str.substring(4,6).c_str(), NULL, 16);
    return CHSV(h,s,v);
}

static void setColor(Device* device, String str) {
    CHSV color = colorFromString(str);
    setHSV(device, color);
}

static void setDefaultColor(Device* device, String str) {
    CHSV color = colorFromString(str);
    writeDefaultColor(device, color);
}

void get() {
    // Get device or cancel request
    Device* device = getDevice(server.arg("d").c_str());
    if (device == 0) {
        // TODO: Send error
        return;
    }
    // Get command or cancel request
    const char* command = server.arg("c").c_str();
    if (strlen(command) != 1) {
        // TODO: Send error
        return;
    }

    char mess[8];
    // Execute command
    switch (command[0]) {
        // Enabled
        case 'e': sprintf(mess, device->enabled ? "1" : "0"); break;

        case 'h': sprintf(mess, "%d", device->endHSV.h); break;
        case 's': sprintf(mess, "%d", device->endHSV.s); break;
        case 'v': sprintf(mess, "%d", device->endHSV.v); break;

        case 'c': printColor(device->endHSV, mess); break;
        case 'd': printColor(device->defaultColor, mess); break;

        case 'r': printRGB(device->endHSV, 0, mess); break;
        case 'g': printRGB(device->endHSV, 1, mess); break;
        case 'b': printRGB(device->endHSV, 2, mess); break;
        default:
        // TODO: Send error
        return;
    }
    server.send(200, "text/plain", mess);
}

void set() {
    // Get device or cancel request
    Device* device = getDevice(server.arg("d").c_str());
    if (device == 0) {
        // TODO: Send error
        return;
    }
    // Get command or cancel request
    const char* command = server.arg("c").c_str();
    if (strlen(command) != 1) {
        // TODO: Send error
        return;
    }
    // Get value or cancel request
    String valueString = server.arg("v");
    const char* value = valueString.c_str();
    if (strlen(command) == 0) {
        // TODO: Send error
        return;
    }
    // Execute command
    switch (command[0]) {
        // Enabled
        case 'e': enable(device); break;
        case 'o': disable(device); break; // OFF
        case 't': toggle(device); break;

        case 'h': setParamHSV(device, 0, value[0]); break;
        case 's': setParamHSV(device, 1, value[0]); break;
        case 'v': setParamHSV(device, 2, value[0]); break;

        case 'c': setColor(device, valueString); break;
        case 'd': setDefaultColor(device, valueString); break;

        case 'r': setParamRGB(device, 0, value[0]); break;
        case 'g': setParamRGB(device, 1, value[0]); break;
        case 'b': setParamRGB(device, 2, value[0]); break;
        default:
        // TODO: Send error
        return;
    }
    server.send(200, "text/plain", "ok");
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
    if (bytes == 0) {
        udp.flush();
        return;
    }
    // Get device or cancel request
    Device* device = getDeviceById(udp.read());
    if (device == 0) {
        udp.flush();
        return;
    }
    uint8_t param;
    CHSV color;
    switch (bytes) {
        case 2:
        setEnable(device, udp.read());
        break;

        case 3:
        param = udp.read();
        if (param > 2) { break; }
        setParamHSV(device, param, udp.read());
        break;

        case 4:
        udp.read((char*) &color, 3); // Read the end color
        setHSV(device, color);
        break;

        default:
        break;
    }
    udp.flush();
}

/* Server function for unknown urls */
static void handleNotFound() {
    server.send(404, "text/plain", "Page doesn't exist");
}

/* Set up the API, UDP, WIFI, and web server */
void setup() {
    setupLEDs();
    WiFi.begin(ssid, pass);

    server.onNotFound(handleNotFound);
    server.on("/get", get);
    server.on("/set", set);

    server.begin();
    udp.begin(UDP_DEFAULT_PORT);
}

/* This function is called regularly to handle url requests */
void handleClient() {
    server.handleClient();
}

/**
Run the task manager
*/
void loop() {
    Task::runTasks();
}

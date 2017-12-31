#include "api.h"

/* Other libraries */
#include <SimpleScheduler.h>    /* Simple task scheduling */

// SERVER - https://links2004.github.io/Arduino/d3/d58/class_e_s_p8266_web_server.html
#include <ESP8266WebServer.h>

// UDP - https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFiUdp.h
#include <WiFiUdp.h>

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

void process(void (*function) (Device*, uint8_t command)) {
    // Get device or cancel request
    if (!server.hasArg("d")) {
        server.send(400, "text/plain", "No device specified, use '?d='");
        return;
    }
    Device* device = getDeviceById(strtol(server.arg("d").c_str(),NULL,10));
    if (device == 0) {
        server.send(400, "text/plain", "Invalid device specified");
        return;
    }
    // Get command or cancel request
    if (!server.hasArg("c")) {
        server.send(400, "text/plain", "No command specified, use '?c='");
        return;
    }
    String commandString = server.arg("c");
    if (strlen(commandString.c_str()) != 1) {
        server.send(400, "text/plain", "Invalid command specified, use '?c='");
        return;
    }
    uint8_t command = commandString.c_str()[0];
    function(device, command);
}

static char mess[40];

void get(Device* device, uint8_t command) {

    // Execute command
    switch (command) {
        case 'i': deviceInfo(mess, device); break;
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
        server.send(400, "text/plain", "Unknown command");
        return;
    }
    server.send(200, "text/plain", mess);
}

// Wrapper function to handle getting variables
void handleGet() {
    process(get);
}

void set(Device* device, uint8_t command) {
    // Some command require values
    if (server.hasArg("v")) {
        Serial.println("Has value");
        // Get value or cancel request
        String valueString = server.arg("v");
        // Parse value argument as hex number
        uint8_t value = strtol(valueString.c_str(), NULL, 16);
        // Execute command
        switch (command) {
            case 'a': setEnable(device, value); break; // ACTIVATE

            case 'h': setParamHSV(device, 0, value); break;
            case 's': setParamHSV(device, 1, value); break;
            case 'v': setParamHSV(device, 2, value); break;

            case 'c': setColor(device, valueString); break;
            case 'd': setDefaultColor(device, valueString); break;

            case 'r': setParamRGB(device, 0, value); break;
            case 'g': setParamRGB(device, 1, value); break;
            case 'b': setParamRGB(device, 2, value); break;
            default:
            server.send(400, "text/plain", "Unknown command");
            return;
        }
    } else {
        Serial.println("No value");
        // Execute commands without values
        switch (command) {
            // Enabled
            case 'e': enable(device); break;
            case 'o': disable(device); break; // OFF
            case 't': toggle(device); break;
            default:
            server.send(400, "text/plain", "Unknown command, or no value specified");
            return;
        }
    }
    server.send(200, "text/plain", "ok");
}

// Wrapper function to handle setting variables
void handleSet() {
    Serial.println("Received command");
    process(set);
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
        case 1:
        toggle(device);
        break;

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

/**
 Server function for unknown urls
 */
static void handleNotFound() {
    server.send(404, "text/plain", "Page doesn't exist");
}

/**
 Set up the API, UDP, WIFI, and web server
 */
void setup() {
    setupLEDs();
    WiFi.begin(ssid, pass);

    server.onNotFound(handleNotFound);
    server.on("/get", handleGet);
    server.on("/set", handleSet);

    server.begin();
    udp.begin(UDP_DEFAULT_PORT);
}

/**
 This function is called regularly to handle url requests
 */
void handleClient() {
    server.handleClient();
}

/**
Run the task manager
*/
void loop() {
    Task::runTasks();
}

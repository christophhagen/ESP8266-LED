#include "parameters.h"

#include <EEPROM.h>             // Saves and reads the default 'on' color from persistant memory

// SERVER - https://links2004.github.io/Arduino/d3/d58/class_e_s_p8266_web_server.html
#include <ESP8266WebServer.h>

/*  Webserver to give access to url api */
ESP8266WebServer server(SERVER_PORT);

/* Forward declare API functions */
void handleNotFound();
void respondStatus();
void respondOn();
void respondOff();
void respondToggle();
void respondHue();
void respondSaturation();
void respondBrightness();
void respondColor();
void respondSetHue();
void respondSetSaturation();
void respondSetBrightness();
void respondSetColor();
void respondSetDefaultColor();

/* This function is called regularly to handle url requests */
void handleClient() {
    server.handleClient();
}

/* Set up the API */
void setupServer() {
    server.onNotFound(handleNotFound);
    server.on("/get/status", respondStatus);
    server.on("/get/hue", respondHue);
    server.on("/get/saturation", respondSaturation);
    server.on("/get/brightness", respondBrightness);
    server.on("/get/color", respondColor);

    server.on("/set/on", respondOn);
    server.on("/set/off", respondOff);
    server.on("/set/toggle", respondToggle);

    server.on("/set/hue", respondSetHue);
    server.on("/set/saturation", respondSetSaturation);
    server.on("/set/brightness", respondSetBrightness);
    server.on("/set/color", respondSetColor);
    server.on("/set/defaultColor", respondSetDefaultColor);

    server.begin();
}

/* Standard response for setting parameters */
inline void respondOK() {
    server.send(200, "text/plain", "ok");
}

/* Server function for unknown urls */
void handleNotFound() {
    server.send(404, "text/plain", "Page doesn't exist");
}

/**
Returns if the strip is on or off. Returns "1" if the brightness
is non-zero, and returns 0, if the brightness is 0.
*/
void respondStatus() {
    server.send(200, "text/plain", getEnabled() ? "1" : "0");
}

/* Return the value of a parameter */
void respondParam(uint8_t value) {
    char mess[4];
    sprintf(mess, "%d", value);
    server.send(200, "text/plain", mess);
}

/**
Return the current hsv hue of the strip.
The returned plaintext is from 0 to 255
*/
void respondHue() {
    respondParam(getHSV().hue);
}

/**
Return the current hsv saturation of the strip.
The returned plaintext is from 0 to 255
*/
void respondSaturation() {
    respondParam(getHSV().saturation);
}

/**
Return the current hsv brightness of the strip.
The returned plaintext is the brightness from 0 to 255
*/
void respondBrightness() {
    respondParam(getHSV().value);
}

/**
Return the current hsv color of the strip.
The returned plaintext is (hue,saturation,brightness) in 8 bit hex format
*/
void respondColor() {
    CHSV color = getHSV();
    char mess[8];
    sprintf(mess, "%02x%02x%02x", color.hue, color.sat, color.val);
    server.send(200, "text/plain", mess);
}

/**
Turn the strip on through a url
*/
void respondOn() {
    enable();
    respondOK();
}

/**
Turn the strip off through a url
*/
void respondOff() {
    disable();
    respondOK();
}

/**
Toggles the LEDs through a url.
Strip is on -> Strip will be turned off
Strip is off -> Strip will be turned on, color will be as before, or default
*/
void respondToggle() {
    toggle();
    respondOK();
}

/* Set one parameter of the color */
void respondSetParam(const char* arg, uint8_t type) {
    CHSV color = getHSV();
    color.raw[type] = strtol(server.arg(arg).c_str(), NULL, 16);
    setHSV(color);
    respondOK();
}

/**
Sets the hue through a url argument "h"
The argument is in 8 bit hex format,
for example: http://192.168.188.75/setHue?h=FF
*/
void respondSetHue() {
    respondSetParam("h", 0);
}

/**
Sets the brightness through a url argument "b"
The argument is in 8 bit hex format,
for example: http://192.168.188.75/setSaturation?s=FF
*/
void respondSetSaturation() {
    respondSetParam("s", 1);
}

/**
Sets the brightness through a url argument "b"
The argument is in 8 bit hex format,
for example: http://192.168.188.75/setBrightness?b=FF
*/
void respondSetBrightness() {
    respondSetParam("b", 2);
}

/**
Set the current color through a url argument "hsb"
The argument is (hue,saturation,brightness) in 8 bit hex format,
for example: http://192.168.188.75/setColor?hsb=FE7AFF
*/
void respondSetColor() {
    String str = server.arg("hsb");
    uint8_t h = strtol(str.substring(0,2).c_str(), NULL, 16);
    uint8_t s = strtol(str.substring(2,4).c_str(), NULL, 16);
    uint8_t v = strtol(str.substring(4,6).c_str(), NULL, 16);
    setHSV(CHSV(h,s,v));
    respondOK();
}

/**
Set the default color when the device is booted up. This will only influences
the color when the device is set to 'ON' right after boot.
Set the default color through a url argument "hsb".
The argument is (hue,saturation,brightness) in 8 bit hex format,
for example: http://192.168.188.75/setDefaultColor?hsb=FE7AFF

*/
void respondSetDefaultColor() {
    String str = server.arg("hsb");
    uint8_t h = strtol(str.substring(0,2).c_str(), NULL, 16);
    uint8_t s = strtol(str.substring(2,4).c_str(), NULL, 16);
    uint8_t v = strtol(str.substring(4,6).c_str(), NULL, 16);

    EEPROM.write(0,h);
    EEPROM.write(1,s);
    EEPROM.write(2,v);
    defaultHSV = CHSV(h,s,v);
    respondOK();
}

/**
Reads the color from EEPROM
*/
void readDefaultColor() {
    EEPROM.begin(3);
    defaultHSV.hue = EEPROM.read(0);
    defaultHSV.sat = EEPROM.read(1);
    defaultHSV.val = EEPROM.read(2);
}

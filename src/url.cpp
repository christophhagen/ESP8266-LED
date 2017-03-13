#include "parameters.h"

// SERVER - https://links2004.github.io/Arduino/d3/d58/class_e_s_p8266_web_server.html
#include <ESP8266WebServer.h>

// Webserver to give access to url api
ESP8266WebServer server(SERVER_PORT);

void handleNotFound();
void respondStatus();
void respondSwitchOn();
void respondSwitchOff();
void respondToggle();
void respondHue();
void respondSaturation();
void respondBrightness();
void respondColor();
void respondSetHue();
void respondSetSaturation();
void respondSetBrightness();
void respondSetColor();

/**
This function is called regularly to handle url requests
*/
void handleClient() {
    server.handleClient();
}

void setupServer() {
    server.onNotFound(handleNotFound);
    server.on("/get/status", respondStatus);
    server.on("/get/hue", respondHue);
    server.on("/get/saturation", respondSaturation);
    server.on("/get/brightness", respondBrightness);
    server.on("/get/color", respondColor);

    server.on("/set/on", respondSwitchOn);
    server.on("/set/off", respondSwitchOff);
    server.on("/set/toggle", respondToggle);

    server.on("/set/hue", respondSetHue);
    server.on("/set/saturation", respondSetSaturation);
    server.on("/set/brightness", respondSetBrightness);
    server.on("/set/color", respondSetColor);
    server.begin();
}

/**
Standard response for setting parameters
*/
inline void respondOK() {
    server.send(200, "text/plain", "ok");
}

/* Server function for unknown urls */
void handleNotFound() {
    respondOK();
}

/**
Returns if the strip is on or off. Returns "1" if the brightness
is non-zero, and returns 0, if the brightness is 0.
*/
void respondStatus() {
    server.send(200, "text/plain", endHSV.value == 0 ? "0" : "1");
}

/**
Return the current hsv hue of the strip.
The returned plaintext is from 0 to 255 in 8 bit hex format
*/
void respondHue() {
    char mess[4];
    sprintf(mess, "%02x", endHSV.hue);
    server.send(200, "text/plain", mess);
}

/**
Return the current hsv saturation of the strip.
The returned plaintext is from 0 to 255 in 8 bit hex format
*/
void respondSaturation() {
    char mess[4];
    sprintf(mess, "%02x", endHSV.saturation);
    server.send(200, "text/plain", mess);
}

/**
Return the current hsv brightness of the strip.
The returned plaintext is the brightness from 0 to 255 in 8 bit hex format
*/
void respondBrightness() {
    char mess[4];
    sprintf(mess, "%02x", endHSV.value);
    server.send(200, "text/plain", mess);
}

/**
Return the current hsv color of the strip.
The returned plaintext is (hue,saturation,brightness) in 8 bit hex format
*/
void respondColor() {
    char mess[8];
    sprintf(mess, "%02x%02x%02x", endHSV.hue, endHSV.sat, endHSV.val);
    server.send(200, "text/plain", mess);
}

/**
Turn the strip on through a url
*/
void respondSwitchOn() {
    endHSV.value = 255;
    setHSV(endHSV);
    respondOK();
}

/**
Turn the strip off through a url
*/
void respondSwitchOff() {
    endHSV.value = 0;
    setHSV(endHSV);
    respondOK();
}

/**
Toggles the LEDs through a url.
Strip is on -> Strip will be turned off
Strip is off -> Set to full brightness
*/
void respondToggle() {
    endHSV.value = (endHSV.value == 0) ? 255 : 0;
    setHSV(endHSV);
    respondOK();
}
/**
Sets the hue through a url argument "h"
The argument is in 8 bit hex format,
for example: http://192.168.188.75/setHue?h=FF
*/
void respondSetHue() {
    endHSV.hue = strtol(server.arg("h").c_str(), NULL, 16);
    setHSV(endHSV);
    respondOK();
}

/**
Sets the brightness through a url argument "b"
The argument is in 8 bit hex format,
for example: http://192.168.188.75/setSaturation?s=FF
*/
void respondSetSaturation() {
    endHSV.saturation = strtol(server.arg("s").c_str(), NULL, 16);
    setHSV(endHSV);
    respondOK();
}

/**
Sets the brightness through a url argument "b"
The argument is in 8 bit hex format,
for example: http://192.168.188.75/setBrightness?b=FF
*/
void respondSetBrightness() {
    endHSV.value = strtol(server.arg("b").c_str(), NULL, 16);
    setHSV(endHSV);
    respondOK();
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
    endHSV = CHSV(h,s,v);
    setHSV(endHSV);
    respondOK();
}

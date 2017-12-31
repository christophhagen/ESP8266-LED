#ifndef __COLORS_H
#define __COLORS_H

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>  /* LED strip control https://github.com/FastLED/FastLED */

// Access user defines
#include "customize.h"

// Defines the maximum number of devices
#ifndef DEVICES_MAX
#define DEVICES_MAX       4
#endif

// Defines the time between blending steps (in ms)
#ifndef EX_TIME
#define EX_TIME           20
#endif

struct Device {
    // A pointer to the colors
    CRGB* colors;
    // The number of leds in the strip
    uint16_t leds;
    // The controller for the leds
    CLEDController* controller;
    // The default color of the device
    CHSV defaultColor;
    // End color in HSB format
    CHSV endHSV;
    // End color in RGB format
    CRGB endRGB;
    // Current color in RGB format
    CRGB currentRGB;
    // Indicate if blending towards end color
    bool blending;
    // Index of the device within the devices array
    uint8_t index;
    // Indicate if device is currently enabled
    bool enabled;
};

void addDevice(Device device);

Device* getDeviceById(uint8_t id);

void enable(Device* device);

void disable(Device* device);

void toggle(Device* device);

void setEnable(Device* device, uint8_t newStatus);

void setParamHSV(Device* device, uint8_t type, uint8_t value);

void setParamRGB(Device* device, uint8_t type, uint8_t value);

void setHSV(Device* device, CHSV color);

void writeDefaultColor(Device* device, CHSV color);

void printDeviceInfo();

const char* deviceInfo(char* mess, Device* device);

#endif

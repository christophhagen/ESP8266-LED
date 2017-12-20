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
    const char* name;
    CRGB* colors;
    uint16_t leds;
    CLEDController& controller;
    CHSV endHSV;
    CHSV defaultColor;
    CRGB endRGB;
    CRGB currentRGB;
    uint8_t enabled;
    uint8_t index;
};

void addDevice(Device device);

Device* getDevice(const char* name);

Device* getDeviceById(uint8_t id);

void enable(Device* device);

void disable(Device* device);

void toggle(Device* device);

void setEnable(Device* device, uint8_t newStatus);

void setParamHSV(Device* device, uint8_t type, uint8_t value);

void setParamRGB(Device* device, uint8_t type, uint8_t value);

void setHSV(Device* device, CHSV color);

void writeDefaultColor(Device* device, CHSV color);

#endif

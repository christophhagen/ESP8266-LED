
#include "colors.h"

/* Other libraries */
#include <SimpleScheduler.h>    /* Simple task scheduling */

#include <EEPROM.h>             // Saves and reads the default 'on' color from persistant memory

void blendColors();

Task blendTask(blendColors, EX_TIME, false);

static Device devices[DEVICES_MAX];
static uint8_t deviceCount = 0;

const char* deviceInfo(char* mess, Device* device) {
    sprintf(mess, "%02d: %s RGB: (%03d,%03d,%03d)",
    device->index,
    device->enabled ? "enabled " : "disabled",
    device->endRGB[0], device->endRGB[1], device->endRGB[2]);
}

void printDeviceInfo() {
    Serial.print(deviceCount);
    Serial.println(" devices registered");
    char mess[40];
    for (uint8_t i = 0; i < deviceCount; i += 1) {
        Serial.println(deviceInfo(mess, &devices[i]));
    }
}

/**
Set the color to use when when the device is booted up. This will only influence
the color when the device is set to 'ON' before a different color is set.
*/
void writeDefaultColor(Device* device, CHSV color) {
    uint16_t offset = (uint16_t) device->index * 3;
    device->defaultColor = color;
    EEPROM.write(offset,     color.h);
    EEPROM.write(offset + 1, color.s);
    EEPROM.write(offset + 2, color.v);
}

/**
Reads the color from EEPROM.
*/
void readDefaultColor(Device* device) {
    uint16_t offset = (uint16_t) device->index * 3;
    EEPROM.begin(offset + 3);
    uint8_t hue = EEPROM.read(offset);
    uint8_t sat = EEPROM.read(offset + 1);
    uint8_t val = EEPROM.read(offset + 2);
    device->defaultColor = CHSV(hue, sat, val);
}

void addDevice(Device device) {
    if (deviceCount == DEVICES_MAX) {
        return;
    }
    device.index = deviceCount;
    device.controller->showColor(CRGB(0,0,0));
    device.blending = false;
    device.enabled = false;
    readDefaultColor(&device);
    devices[deviceCount] = device;
    deviceCount += 1;
}

Device* getDeviceById(uint8_t id) {
    if (id >= deviceCount) {
        return 0;
    }
    return &devices[id];
}

void startBlend(Device* device) {
    device->blending = true;
    Serial.println("Start blending");
    blendTask.enable(); // Start blending
}

void enable(Device* device) {
    if (device->enabled) {
        return;
    }
    device->enabled = true;
    if (device->endRGB == CRGB(0,0,0)) {
        device->endHSV = device->defaultColor;
        device->endRGB = CRGB(device->defaultColor);
    }
    startBlend(device);
}

void disable(Device* device) {
    if (!device->enabled) {
        return;
    }
    device->enabled = false;
    startBlend(device);
}

void toggle(Device* device) {
    (device->enabled) ? disable(device) : enable(device);
}

/* Enable or disable the strip
* 0: turn off
* 1: turn on
* other: toggle
*/
void setEnable(Device* device, uint8_t newStatus) {
    switch (newStatus) {
        case 0:  disable(device); return;
        case 1:  enable(device);  return;
        default: toggle(device);
    }
}

void didSetParam(Device* device) {
    // Turn strip off if brightness is 0
    if (device->endRGB == CRGB(0,0,0)) {
        device->enabled = false;
    } else {
        device->enabled = true;

    }
    startBlend(device);
}

/*
Set one parameter of a HSV color from a string.
*/
void setParamHSV(Device* device, uint8_t type, uint8_t value) {
    device->endHSV.raw[type] = value;
    device->endRGB = CRGB(device->endHSV);
    didSetParam(device);
}

void setParamRGB(Device* device, uint8_t type, uint8_t value) {
    device->endRGB[type] = value;
    didSetParam(device);
}

/* Set necessary values for blending and start */
void setHSV(Device* device, CHSV color) {
    device->endHSV = color;
    device->endRGB = CRGB(device->endHSV);
    didSetParam(device);
}

/* Small steps towards the end color */
void blendColor(Device* device) {
    if (!device->blending) {
        return;
    }

    CRGB current = device->currentRGB;
    CRGB end = device->enabled ? device->endRGB : CRGB(0,0,0);

    // blend rgb, 1 step per channel
    for (uint8_t i = 0; i < 3; i++) {
        if (current[i] != end[i]) {
            current[i] += (current[i] > end[i]) ? -1 : 1;
        }
    }
    device->currentRGB = current;
    device->controller->showColor(current);

    if (current == end) { // Check for end of fade
        device->blending = false;
    }
}

void blendColors() {
    bool blending = false;
    for (uint8_t i = 0; i < deviceCount; i += 1) {
        blendColor(&devices[i]);
        blending |= devices[i].blending;
    }
    if (!blending) {
        Serial.println("Stop blending");
        blendTask.disable();
    }
}

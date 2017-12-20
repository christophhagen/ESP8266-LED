
#include "colors.h"

/* Other libraries */
#include <SimpleScheduler.h>    /* Simple task scheduling */

#include <EEPROM.h>             // Saves and reads the default 'on' color from persistant memory

void blendColors();

Task blendTask(blendColors, EX_TIME, false);

static Device* devices[DEVICES_MAX];
static uint8_t deviceCount = 0;

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
    devices[deviceCount] = &device;
    device.controller.showColor(CRGB(0,0,0));
    device.enabled = 0;
    readDefaultColor(&device);
    device.index = deviceCount;
    deviceCount += 1;
}

Device* getDevice(const char* name) {
    for (uint8_t i = 0; i < deviceCount; i += 1) {
        if (strcmp(devices[i]->name, name) == 0) {
            return devices[i];
        }
    }
    return 0;
}

Device* getDeviceById(uint8_t id) {
    if (id >= deviceCount) {
        return 0;
    }
    return devices[id];
}


void enable(Device* device) {
    device->enabled = 1;
    device->endRGB = CRGB(device->endHSV);
    if (device->endRGB == CRGB(0,0,0)) { // Set to default if enabled but color is 0
        device->endHSV = device->defaultColor;
        device->endRGB = CRGB(device->defaultColor);
    }
    blendTask.enable(); // Start blending
}

void disable(Device* device) {
    device->enabled = 0;
    device->endRGB = CRGB(0,0,0);
    blendTask.enable(); // Start blending
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

/*
Set one parameter of a HSV color from a string.
*/
void setParamHSV(Device* device, uint8_t type, uint8_t value) {
    device->endHSV.raw[type] = value;
    // Turn strip off if brightness is 0
    if (device->endHSV.value == 0) {
        disable(device);
    } else {
        device->endRGB = CRGB(device->endHSV); // Set end color
        device->enabled = 1;
        blendTask.enable(); // Start blending
    }
}

void setParamRGB(Device* device, uint8_t type, uint8_t value) {
    device->endRGB.raw[type] = value;
    device->enabled = 1;
    blendTask.enable(); // Start blending
}

/* Set necessary values for blending and start */
void setHSV(Device* device, CHSV color) {
    device->endHSV = color;

    // Turn strip off, if brightness is 0
    if (color.value == 0) {
        disable(device);
    } else {
        device->endRGB = CRGB(color); // Set end color
        enable(device);
    }
}


/* Small steps towards the end color */
void blendColor(Device* device) {
    // blend rgb, 1 step per channel
    for (uint8_t i = 0; i < 3; i++) {
        if (device->currentRGB.raw[i] != device->endRGB.raw[i]) {
            if (device->currentRGB.raw[i] > device->endRGB.raw[i]) {
                device->currentRGB.raw[i] -= 1;
            } else {
                device->currentRGB.raw[i] += 1;
            }
        }
    }
    device->controller.showColor(device->currentRGB);

    if (device->currentRGB == device->endRGB) { // Check for end of fade
        device->enabled = 0;
    }
}

void blendColors() {
    uint8_t enabled = 0;
    for (uint8_t i = 0; i < deviceCount; i += 1) {
        blendColor(devices[i]);
        enabled |= devices[i]->enabled;
    }
    if (!enabled) {
        blendTask.disable();
    }
}

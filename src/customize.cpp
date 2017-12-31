
#include "api.h"

// Defines for the devices

#define STRIP_TYPE        WS2812B
#define COLOR_TYPE        GRB     // My strip switches red and green

#define WALL_NR_OF_LEDS   60
#define WALL_DATA_PIN     12       // The data line of the strip on the wall

#define BED_NR_OF_LEDS    40
#define BED_DATA_PIN      14

CRGB wall_colors[WALL_NR_OF_LEDS]; // Array with colors for the leds
CRGB bed_colors[BED_NR_OF_LEDS];   // Array with colors for the leds

void setupLEDs() {
    CLEDController &wall_controller = FastLED.addLeds<STRIP_TYPE, WALL_DATA_PIN, COLOR_TYPE>(wall_colors, WALL_NR_OF_LEDS);
    Device wall_device = {
        wall_colors,
        WALL_NR_OF_LEDS,
        &wall_controller
    };
    addDevice(wall_device);

    CLEDController &bed_controller = FastLED.addLeds<STRIP_TYPE, BED_DATA_PIN, COLOR_TYPE>(bed_colors, WALL_NR_OF_LEDS);
    Device bed_device = {
        bed_colors,
        BED_NR_OF_LEDS,
        &bed_controller
    };
    addDevice(bed_device);
}

# ESP8266-LED

A simple program to control one or multiple LED strips connected to an ESP8266 chip. Provides a URL/UDP interface to easily set the color of the strip. I use it together with a [homebridge plug-in](https://github.com/christophhagen/homebridge-ESP-HSV) to control my room lighting with Siri.

## Usage guide

### Hardware Setup

Connect all the necessary pins to be able to flash the software onto the chip. Have a look [here](https://github.com/christophhagen/ESP8266-LED/blob/master/hardware.md) for the setup I use with the ESP12F.

### Connect

Use [PlatformIO](http://platformio.org) (or equivalent software, e.g. the [Arduino IDE](https://www.arduino.cc/en/main/software) with the [ESP8266 Arduino Core](https://github.com/esp8266/Arduino/blob/master/doc/reference.md)) to connect to your `ESP8266`.

### WiFi credentials and other parameters

Set your device parameters and Wifi credentials in `customize.h`. Uncomment specific defines if you want to override default parameters, such as the maximum number of devices.

### LED setup

This library allows the control of several distinct led strips (or parts of a strip). To set up the devices, simply add them in `customize.cpp` within the `void setupLEDs()` function:

````c++

#define STRIP_TYPE   WS2812B // The led strip model
#define COLOR_TYPE   GRB     // My strip switches red and green

#define NR_OF_LEDS   60      // The number of the leds on the strip
#define DATA_PIN     12      // The GPIO pin connected to the strip

// Array with colors for the leds
CRGB strip_colors[NR_OF_LEDS];

void setupLEDs() {
    CLEDController &controller1 = FastLED.addLeds<STRIP_TYPE, DATA_PIN, COLOR_TYPE>(strip_colors, NR_OF_LEDS);
    Device my_device = {
        "MyDevice", // Name of the device to access it through the api.
        strip_colors, // Pointer to the array of the colors
        NR_OF_LEDS,
        wall_controller // The controller created above
    };

    // Add the device to be able to access it
    addDevice(my_device);

    // Add more devices here
}
````

For all the possible hardware models please consult the [FastLED documentation](https://github.com/FastLED/FastLED/wiki/Overview).

### Flash the chip

Now compile and upload the program to the ESP. Once the device is connected to your wifi network, you can control it over http requests or through udp packets.

### URL API

The API is a combination of urls and UDP packets. All functions are available through URLs, while UDP can be used to set colors and brightness faster and with less overhead.

#### Getting data

The base url for all `get` operations is `http://YOUR_IP/get`.
The target device is specified by the url parameter `?d=` (e.g. `http://YOUR_IP/get?d=MyDevice`).
The type of request is specified by the url parameters `?c=`:

| Function           | Command `?c=` | Returned data (text)         |
| ------------------ |:------------- |:---------------------------- |
| Determine ON/OFF   | `e`           | ON: `1`, OFF: `0`            |
| Current Hue        | `h`           | 1x 8-bit HEX (e.g. `EF`)     |
| Current saturation | `s`           | 1x 8-bit HEX (e.g. `EF`)     |
| Current brightness | `v`           | 1x 8-bit HEX (e.g. `EF`)     |
| Current HSB color  | `c`           | 3x 8-bit HEX (e.g. `EFC4FF`) |

#### Setting data

Similar to `get`, except that an additional parameter `?v=` specifies the value(s) to set:

| Function           | URL parameter `?c=` | URL Parameter `?v=`            |
| ------------------ |:------------------- |:------------------------------ |
| Turn on            | `e`                 | None                           |
| Turn off           | `o`                 | None                           |
| Toggle (Full/off)  | `t`                 | None                           |
| Set new hue        | `h`                 | `8 bit HEX` (e.g. `EF`)        |
| Set new saturation | `s`                 | `8 bit HEX` (e.g. `EF`)        |
| Set new brightness | `v`                 | `8 bit HEX` (e.g. `EF`)        |
| Set new HSB color  | `c`                 | `3x 8 bit HEX` (e.g. `EFCDAB`) |
| Set default color  | `d`                 | `3x 8 bit HEX` (e.g. `EFCDAB`) |
| Set new red        | `r`                 | `8 bit HEX` (e.g. `EF`)        |
| Set new green      | `g`                 | `8 bit HEX` (e.g. `EF`)        |
| Set new blue       | `b`                 | `8 bit HEX` (e.g. `EF`)        |

For example, setting the hue to 234 on the device `MyDevice`:
`http://YOUR_IP/get?d=MyDevice?c=h?v=234`

### UDP API

For UDP the first byte to send is the device identifier, which corresponds to the order in which the devices are added through `addDevice()`. The following bytes can be:

| Function       | Packet length | Included bytes                  |
| -------------- |:------------- |:------------------------------- |
| Set on/off     | 2 byte        | id, 1/0                         |
| Set hue        | 3 byte        | id, 0, hue (0-255)              |
| Set saturation | 3 byte        | id, 1, saturation (0-255)       |
| Set brightness | 3 byte        | id, 2, brightness (0-255)       |
| Set HSV color  | 4 byte        | id, hue, saturation, brightness |

An example request to set the HSV color of a device with identifier `0` to `hue = 123`, `saturation = 234`, `brightness = 45` would simply be: `[0, 123,234,45]`

## Thanks

This code uses the [FastLED library](http://fastled.io) to control the LED strip. It's a really cool project and makes this stuff so much easier.

## Other

I'm using a simple [Scheduler](https://github.com/christophhagen/ArduinoScheduler) to execute tasks. Basically just a glorified if/else loop, but maybe useful to someone.

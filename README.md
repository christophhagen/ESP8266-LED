# ESP8266-LED

A simple program to control an LED strip connected to a ESP8266 chip. Provides a URL interface and a UDP interface to easily set the color of the strip. I use it together with a [homebridge plug-in](https://github.com/christophhagen/homebridge-ESP-HSV) to control my room lighting with Siri.

## Setup

1. Use [PlatformIO](http://platformio.org) (or equivalent software) to connect to your `ESP8266`.

2. Set your device parameters and Wifi credentials in `parameters.h`

3. Compile and upload the code to your chip.

4. Use the API

## API

The API is a combination of urls and UDP packets. All functions are available through URLs, while UDP can be used to set colors and brightness faster and with less traffic.

### URLs

Getting data:

| Function           | URL              | Returned data              |
| ------------------ |:---------------- |:-------------------------- |
| Determine ON/OFF   | /get/status      | ON: `1`, OFF: `0`          |
| Current Hue        | /get/hue         | 1x 8-bit HEX (e.g. EF)     |
| Current saturation | /get/saturation  | 1x 8-bit HEX (e.g. EF)     |
| Current brightness | /get/brightness  | 1x 8-bit HEX (e.g. EF)     |
| Current HSB color  | /get/color       | 3x 8-bit HEX (e.g. EFC4FF) |

Setting data:

| Function               | URL               | URL Parameter       |
| ---------------------- |:----------------- |:------------------- |
| Set to full brightness | /set/on           | None                |
| Turn off               | /set/off          | None                |
| Toggle (Full/off)      | /set/toggle       | None                |
| Set new hue            | /set/hue          | h = `8 bit HEX`     |
| Set new saturation     | /set/saturation   | s = `8 bit HEX`     |
| Set new brightness     | /set/brightness   | b = `8 bit HEX`     |
| Set new HSB color      | /set/color        | hsb = `3x 8-bit HEX`|
| Set default color      | /set/defaultColor | hsb = `3x 8-bit HEX`|

### UDP

| Function       | Packet length | Included data                                  |
| -------------- |:------------- |:---------------------------------------------- |
| Set on/off     | 1 byte        | on: >0, off: 0                                 |
| Set hue        | 2 byte        | First byte: 0, Second byte: hue (0-255)        |
| Set saturation | 2 byte        | First byte: 1, Second byte: saturation (0-255) |
| Set brightness | 2 byte        | First byte: 2, Second byte: brightness (0-255) |
| Set HSB color  | 3 byte        | hue, saturation, brightness (each 1 byte)      |

## Thanks

This code uses the [FastLED library](fastled.io) to control the LED strip. It's a really cool project and makes my life so much easier.

I'm using a simple [Scheduler](https://github.com/christophhagen/ArduinoScheduler) to execute tasks. Basically just a glorified if/else loop, but maybe useful to someone.

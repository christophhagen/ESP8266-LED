# ESP8266-LED

A simple program to control one or multiple LED strips connected to an ESP8266 chip. Provides a URL/UDP interface to easily set the color of the strip. I use it together with a [homebridge plug-in](https://github.com/christophhagen/homebridge-ESP-HSV) to control my room lighting with Siri.

## Setup

1. Use [PlatformIO](http://platformio.org) (or equivalent software) to connect to your `ESP8266`. This project uses the [Arduino Core](https://github.com/esp8266/Arduino/blob/master/doc/reference.md) for the ESP8266.

2. Set your device parameters and Wifi credentials in `customize.h`.

3. Create the devices you want to use in `customize.cpp`.

3. Compile and upload the code to your chip.

4. Use the API

## API

The API is a combination of urls and UDP packets. All functions are available through URLs, while UDP can be used to set colors and brightness faster and with less overhead.

### URLs

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

### UDP

For UDP the first byte to send is the device identifier, which corresponds to the order in which the devices are added through `addDevice()`. The following bytes can be:

| Function       | Packet length | Included data                                  |
| -------------- |:------------- |:---------------------------------------------- |
| Set on/off     | 1 byte        | on: > 0, off: 0                                |
| Set hue        | 2 byte        | First byte: 0, Second byte: hue (0-255)        |
| Set saturation | 2 byte        | First byte: 1, Second byte: saturation (0-255) |
| Set brightness | 2 byte        | First byte: 2, Second byte: brightness (0-255) |
| Set HSV color  | 3 byte        | hue, saturation, brightness (each 1 byte)      |

An example request to set the HSV color of a device with identifier `0` to `hue = 123, saturation = 234, brightness = 45` would simply be: `[0, 123,234,45]`

## Thanks

This code uses the [FastLED library](fastled.io) to control the LED strip. It's a really cool project and makes this stuff so much easier.

## Hardware setup

Have a look [here](https://github.com/christophhagen/ESP8266-LED/blob/master/hardware.md)

## Other

I'm using a simple [Scheduler](https://github.com/christophhagen/ArduinoScheduler) to execute tasks. Basically just a glorified if/else loop, but maybe useful to someone.

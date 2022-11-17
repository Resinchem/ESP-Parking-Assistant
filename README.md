## An ESP8266 and WS2812b Parking Assistant

A simple to build and use garage parking assistant using an ESP8266 and WS2812b LEDs.  Using a low cost Wemos D1 Mini, a short strip of around 20-40 WS2812b LED pixels and TFMini-s LIDAR distance sensor, a visual parking assistant system for putting your car in the same location in the garage each time can easily be created.

Some key features of the system include:
- 4 variable distance parking zones, including a wake zone, an active zone, a parked zone and a backup zone.
- Each zone can have its own unique LED color, specified by the user
- The active zone has 5 different approach effects that can be used to visually show the car approaching the final parked position
- Automatically goes to standby or sleep mode and only awakens when a car enters the wake zone
- Supports any number of LED pixels, up to 100, and is designed so the LED strip can be mounted horizontally or vertically
- All options and settings made through a web interface
- Over-the-air firmware updates, with a manual OTA option available for uploading your own modified source code
- **Optional** MQTT integration so you can use the parking assistant in Home Assistant or other automation systems

### See the [wiki](https://github.com/Resinchem/ESP-Parking-Assistant/wiki) for full details on installation, configuration, settings and options.

**Note**: This repo only deals with the firmware/software application. For an overview of the build details and to see some of the features in use, please see this YouTube video.

Full step-by-step build instructions, including parts lists, wiring diagrams and more can be found in my blog article: [A New Parking Assistant using an ESP8266 and WS2812b LEDs](https://resinchemtech.blogspot.com/2022/11/esp-parking-assistant.html)

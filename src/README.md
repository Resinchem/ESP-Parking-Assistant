### If you just want to install the firmware on your ESP32, you do not need to download the source code!
Just download the latest **ParkingAsst_vx.xx_ESP32.bin** file from the [Releases page](https://github.com/Resinchem/ESP-Parking-Assistant/releases) and follow the installation and instructions that are found in the [wiki](https://github.com/Resinchem/ESP-Parking-Assistant/wiki).  Please try the steps in the wiki before posting questions about how to install the firmware.

## Source Code
This directory contains the source code .ino sketch for the parking assistant.  You can download and modify this to meet your own needs.  Both the .ino and .h files are reqired. The Parking Assistant supports OTA updates from the Arduino IDE (or other appropriate IDEs).  See the wiki section on Performing Updates and Upgrades for more information on OTA updates using an IDE.

You will need to have the ESP8266/ESP32 boards installed in your environment, plus all the appropriate libraries listed at the top of the sketch file.  If you've not setup the Arduino IDE for the ESP before, this video might help: [Arduino IDE: Basics for Non-Coders](https://youtu.be/KS5HOJat88k)

### ESP8266 support has ended
Support for the ESP8266 ended with release v0.52.  If you need to install a version for the ESP8266, you must download and use the _esp8266.bin file for release v0.51 or earlier.  The source code may still contains stubs and references to the ESP8266 (likely removed in future versions as well), but the additional size and features of the firmware make it unstable on the ESP8266 beginning with release v0.52.

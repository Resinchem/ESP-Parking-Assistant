### If you just want to install the firmware on your ESP32, you do not need to download any files!

Starting with release v0.60, just go to the [Web Installer](https://resinchem.github.io/ESP-Parking-Assistant/installation.html#-install-full-firmware-here) to install the latest firmware release.  All you need is a compatible browser (Chrome, Edge, Brave, Opera, Arc or most other Chromimum-based browsers) and a USB data cable!

## Source Code
This directory contains the source code files for the parking assistant.  You can download and modify this to meet your own needs.  Both the .ino and .h files are reqired. The Parking Assistant supports OTA updates from the Arduino IDE (or other appropriate IDEs).  See the wiki section on [Modifying the Firmware](https://resinchem.github.io/ESP-Parking-Assistant/modifications.html) for more information on OTA updates using an IDE.

You will need to have the ESP32 boards installed in your environment, plus all the appropriate libraries listed at the top of the sketch file.  If you've not setup the Arduino IDE for the ESP before, this video might help: [Arduino IDE: Basics for Non-Coders](https://youtu.be/KS5HOJat88k)

### ESP8266 support has ended
Support for the ESP8266 ended with release v0.52.  If you need to install a version for the ESP8266, you must download and use the _esp8266.bin file for release v0.51 or earlier.  Later versions (v0.52+) are unstable on the ESP8266 due to the overall sketch size and the complexity of new features.

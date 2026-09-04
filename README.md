# An ESP32 LED Parking Assistant

<img width="400" height="225" alt="Image" src="https://github.com/user-attachments/assets/a166f596-bfcc-403d-a03d-d33a84d8622e" />

### Version 0.60 is now out with a complete rebuild!!

Using a low cost ESP32 Mini, a short strip of around 20-40 WS2812b LED pixels and TFMini-s LIDAR distance sensor, you can create an easy-to-build  visual parking assistant system for putting your car in the same location in the garage each time.

Some key features of the system include:
- 4 variable distance parking zones, including a wake zone, an active zone, a parked zone and a backup zone.
- Each zone can have its own unique LED color, specified by the user
- The active zone has 5 different approach effects that can be used to visually show the car approaching the final parked position.
- Automatically goes to standby or sleep mode and only awakens when a car enters the wake zone.
- Supports any number of LED pixels, up to 100, and is designed so the LED strip can be mounted horizontally or vertically.
- All options and settings made through an embedded web interface. No external apps needed.
- Over-the-air firmware updates, with a manual OTA option available for uploading your own modified source code.
- **Optional** Lateral guidance via a secondary sensor.
- **Optional** AP Mode (no-WiFi) allows the system to be used in locations where reliable WiFi may not be available.
- **Optional** MQTT integration so you can use the parking assistant with other automation systems.
- **Optional** 1-Click Home Assistant Discovery.  Add the device to Home Assistant with no YAML or manual configuration!

If you are just looking to install the firmware, just go to the [Installation Page]( https://resinchem.github.io/ESP-Parking-Assistant/installation.html#-install-full-firmware-here) in the firmware guide.  _You do not need to download any files_ from this repo nor do you need to clone the repository.  The installer will handle everything for you!

## Additional Information and Resources

> [!IMPORTANT]
> _The new build guide and update video will be released Saturday morning, September 5 at 8:00am EDT.  Until that time, the links marked with (\*) will not be available.**_

Resource | Details
------|-----
[Build Guide*](https://resinchemtech.blogspot.com/2026/08/parking-assistant-2026.html) |A how-to written guide including parts, schematics and overall assembly instructions.
[Firmware Guide](https://resinchem.github.io/ESP-Parking-Assistant/)|Firwmare installer, onboarding and initial system configuration, and overall general application use.

### Video Overviews
YouTube Video | Details
-------|-------
[ESP32 Parking Assistant Overhaul*](https://youtu.be/pNj6GjKsijo)|Primarily focuses on v0.60 new features, with brief overviews of operation, parts and assembly processes.
[Parking Assistant Upgrades: Lateral Guidance](https://youtu.be/Eps6QCgKzaM)|Covers new features introduced starting with v0.50.  Primarily focuses on the side sensor addition, wiring and use.
[Original Parking Assistant Video](https://youtu.be/HqqlY4_3kQ8)|Now a bit outdated, as the original version used an ESP8266 and substandard shifter.  But still contains useful background information, including a comparison of distance sensors and why the TFMini-S was selected.

## Special Upgrade Notice
If you are running v0.5x of the firmware, a special one-time migration is **highly recommended**.  This process is described in a special [Migration Page](https://resinchem.github.io/ESP-Parking-Assistant/migration) within the firmware documentation.  

_**DO NOT** attempt to upgrade your firmware using the existing v0.5x web app until you've reviewed the migration page information!_

---

## Information for Older Releases (v0.52 and earlier)
Version 0.60 involved a complete rewrite of the firmware and much of the documentation.  However, if you landed here looking for information for older versions, it has been maintained (for now).

### ESP8266 Support Has Ended (v0.52)

Due to the continued addition (and complexity) of new features, it has unfortunately become necessary to drop support for the ESP8266 beginning with release v0.52.  If you have or wish to use an ESP8266, then you must use firmware v0.51 or earlier.  I've tried to update the wiki and related pages, but if you still see references to the ESP8266 please be aware that this applies to firmware v0.51 or earlier.  Newer features shown in the wiki and discussed elsewhere also may not be available in the older ESP8266 versions.  It is highly recommended to now use an ESP32 for all new builds.

**ADDTIONAL RESOURCES** (ONLY for pre-v0.60 versions!)

Resource | Notes
-----|----
[Firmware Installation and Use](https://github.com/Resinchem/ESP-Parking-Assistant/wiki)|Maintained in this repo's old Wiki (v0.52 and earlier)
[Original Build Guide](https://resinchemtech.blogspot.com/2022/11/esp-parking-assistant.html)|Original written build guide that covers up to v0.52

### Supporting this Project and Future Developement 

### _Always free!_

It takes substantial time, effort and cost to develop and maintain this project and all the related documentation. But my goal is to always provide the code and documentation for free... **without paywalls, subscriptions or implementing some sort of half-assed "freemimum" model.** 

<i>The system will also never collect your data or force any annoying ads down your throat!</i>

If you find this project helpful and would like to say 'thanks', please consider supporting it and future development at:

<a href="https://www.buymeacoffee.com/resinchemtech" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" ></a>


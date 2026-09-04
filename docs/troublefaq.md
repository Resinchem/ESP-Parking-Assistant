---
layout: default
title: FAQ & Getting Help
parent: Troubleshooting
nav_order: 4
---

# FAQ & Getting Help
{: .no_toc }

---

<p align="center">
  <img src="images/troublefaq_01.jpg" alt="[PLACEHOLDER: FAQ and Help]">
</p>

Before you open a GitHub issue or head to the discussions board, check these Frequently Asked Questions. Most "mysteries" have a simple explanation rooted in hardware limitations or configuration settings.  Note that this is a "living" document that will be updated as common questions are brought to light.  Initially, the list is pretty short!

---

## Frequently Asked Questions

### Can I use a different ESP32 board, different sensor type or different "X" component?
{: style="color: #edbd32; margin-top: 2rem;" }


**Short answer:** Maybe...maybe not!  

**Long answer:** The firmware is written and compiled with specific libraries and hardware register commands for the components listed in the [Build Guide](https://resinchemtech.blogspot.com/2026/08/parking-assistant-2026.html). This guide includes a parts list that indicates which components can be substituted and those likely to require a firmware modification.  While you *can* modify the code to work with other hardware, I cannot provide support for these versions.

**ESP32's/MCU:**  The firmware _should_ work with any standard ESP32, regardless of form factor (30-pin, 38-pin, Mini, etc.).  The firmware is compiled using the "ESP32_DEV" board, so any board that compiles under that definition should work.  However, if you wish to use a different ESP32 variant, such as a -S3, -C6, etc., it is likely that the firmware will at least require recompilation using the specified board.  Source code modifications may or may not be necessary. 

**Sensors:**  You cannot substitute other sensor types without also modifying the source code and compiling your own version.  **Period. End of story.** 

- If you really want to use other sensor types (I2C, SPI) or something like the HC-SR04 for distance, then <u>source code modifications will be required</u> as the libraries to read the sensors (and the syntax to interface with them) will be different than the default provided interfaces for the TFMini-s and the optional VL53L0X side sensor.

> **⚠️ Support Only Offered for Listed Hardware**<br>The system has only been fully tested using the components listed in the build guide.  As much as I'd like, I simply do not have the capacity to support multiple builds of the firmware for various hardware variations.   If you swap a component, you are officially the lead developer for your specific fork!  I'll try to answer questions as time allows, but please do not request firmware modifications for hardware other than listed in the Build Guide.
{: .important }


### Can I control the system with Alexa or Google Home?
{: style="color: #edbd32; margin-top: 2rem;" }


Not directly from the controller itself. The system is designed for **local control** to protect your privacy and eliminate the need for Internet connectivity. However, if you integrate the Parking Assistant into **Home Assistant** via Discovery, you can then expose those entities to Alexa or Google Home through the standard Home Assistant cloud or Nabu Casa integrations.

📢 _"Hey Google. Turn on the Parking Assistant LEDs and set to 50% brightness"_

## Getting Additional Help

If your question isn't answered above, don't worry—the project has several avenues for support.

### [GitHub Discussions](https://github.com/Resinchem/ESP-Parking-Assistant/discussions) (Recommended)
This is the best place for "How do I...?" questions or "Is this normal?" concerns. Discussions allow for code snippets and images, making it much easier for me (and the community) to see what’s going on. 
* **Etiquette:** Include your firmware version and a description of what you've already tried. "It won't work" is a riddle; "My MQTT status is 'Offline' despite the correct IP" is a solvable problem!

### [GitHub Issues](https://github.com/Resinchem/ESP-Parking-Assistant/issues)
Please reserve Issues for **confirmed or suspected bugs** in the official firmware. 
* If you find a bug, please provide steps to reproduce it. 
* If the issue is related to custom hardware or modified code, it will be moved to the Discussions area or closed.

_Please don't email me directly unless specifically requested.  The volume of email received means your message will likely be lost in the "noise" and may not receive a response.  The best way to initially reach out is via the [Discussions](https://github.com/Resinchem/ESP-Parking-Assistant/discussions) area._

> **💡 A Final Note on Support**<br>I’m an electronics hobbyist, just like you. I maintain these repos in my spare time between coffee refills and actual work. Please be patient—I’ll do my best to help you get your Parking Assistant working.
{: .note }

<div style="display: flex; justify-content: space-between; align-items: center; margin-top: 40px; border-top: 1px solid #333; padding-top: 20px;">
  <a href="{{ '/troublediscovery' | relative_url }}" class="btn btn-outline"><- Previous: Home Assistant & MQTT</a>
  <a href="{{ '/support' | relative_url }}" class="btn btn-purple">Next: Supporting this Project -></a>
</div>
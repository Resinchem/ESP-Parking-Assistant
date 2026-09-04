---
layout: default
title: API HTTP Command List
parent: Optional Integrations
nav_order: 3
---

# HTTP API Command List
{: .no_toc }

---

<p align="center">
  <img src="images/api_01.jpg" alt="HTTP API Overview">
</p>

**The HTTP API is not available in [AP Mode] - WiFi is required!**
{: .label .label-yellow }

The API allows for "headless" control of the Parking Assistant system via simple HTTP requests. It's essentially a remote control that lives in your browser's address bar or your favorite automation script. Unlike MQTT, the API does not require third-party intermediaries (like a broker) or complex configuration. Commands are issued as standard URLs:

`http://[your-controller-ip]/api?...`

> **⚠️ Assign a Static or Reserved IP Address to the Controller**<br>As covered in the [Onboarding](onboarding) topic, it is recommended that you assign a static or reserved IP address to the controller.  Otherwise, if you create HTTP API calls in a third-party system (such as Home Assistant) and the controller's IP address changes, these automations will break and require an updating of the automation to use the new IP.
{: .important }

Even though the method to send commands is different, the HTTP API has many of the same caveats and restrictions as using a similar MQTT command.  Some of those limitations are listed here, but a review of the [MQTT commands](mqtttopics#command-topics) may be helpful even if you aren't planning on using MQTT.

## Command Types
The HTTP API consists of two different types of commands:
 * **SET Commands**: These 'set' a value or parameter on the system.  Unless otherwise noted, if a command is valid and accepted, a status code of 200 and simple "OK" text string will be returned to the browser or calling application.  If invalid, then a status code of 404 and an error message will be returned..

   * Some calls may offer more than one option (e.g. `?ledbrightness=` and `?brightness=`) that perform the same operation.  Others accept alternate values (e.g. `ON`, `1`, `TRUE`) that are equivalent.

 * **GET Commands**: These do not modify the system, but simply return data from the system back to the calling routine or browser.  All GET commands must be issued separately.

### Command Chaining (SET Commands)
Some SET commands can be combined into a single HTTP call by concatenating them with an ampersand (`&`). 

**Example:**
`http://[controller-ip-address]/api?ledbrightness=120&ledcolor=ff0000`

>🔍 **Combined Command Format**<br>When combining commands, the first command following the `/api` is prepended with the question mark (`?`) and any additional commands are prepended with the ampersand (`&`) as shown in the above example.
{: .note}

Other commands must be sent as the only command in the URL.   

---

## SET Commands
These commands, unless otherwise noted, are used to modify the ACTIVE state of the system. Commands marked **NO** in the "Multi" column must be sent as standalone requests and cannot be combined with other commands.

>🔍 **Passing Colors**<br>Color code parameters can be passed as either an RGB string (e.g. `?color=255,255,0`) or a HEX color string (`?color=ffff00`), but you <b>must</b> omit including a `#` with HEX strings as this symbol has special meaining in a URL string.
{: .note}

Command | Parameter(s) | Returns | Multi | Example / Notes 
:--- | :---: | :---: | :---: | :--- 
`sensoroverride`<br>`overridesensor`| `ON`/`1`/`TRUE` or<br>`OFF`/`0`/`FALSE`| 200 / `OK`|Yes|Enables or disables the sensor override.
`ledstate`<br>`state`| `ON`/`1`/`TRUE` or<br>`OFF`/`0`/`FALSE`| 200 / `OK`|Yes|Turns the LED strip on or off.
`ledcolor`<br>`color`<br>`manualcolor`|RGB or HEX<br>color string| 200 / `OK`|Yes|Sets the active LED color.
`ledbrightness`<br>`brightness`|0 - 255|200 / `OK`|Yes|Sets the active LED brightness.  If `0` is passed, the LED state will also be set to off.  Values greater than `0` will toggle the LEDs on if they were off.
`sleepbrightness`<br>`brightnesssleep`|0 - 255|200 / `OK`|Yes|Sets the brightness of the indicator LEDs when the system is in standby mode. Will not impact current state of the LEDs.
`colorwake`|RGB or HEX<br>color string| 200 / `OK`|Yes|Sets the LED color for the Wake Zone.  Can pass an RGB or HEX string (without leading `#`).
`coloractive`|RGB or HEX<br>color string| 200 / `OK`|Yes|Sets the LED color for the Active (approach) Zone.  Can pass an RGB or HEX string (without leading `#`).
`colorparked`|RGB or HEX<br>color string| 200 / `OK`|Yes|Sets the LED color for the Parked Zone.
`colorbackup`|RGB or HEX<br>color string| 200 / `OK`|Yes|Sets the LED color for the Backup Zone. Also used for the lateral guidance color if a side sensor is installed and enabled.
`colorstandby`|RGB or HEX<br>color string| 200 / `OK`|Yes|Sets the LED color for indicator LEDs when the system is in standby mode.
`ledeffect`<br>`effect1`|`Out-In`, `In-Out`, `Full-Strip`, `Full-Strip-Inv`, `Solid`| 200 / `OK`|Yes|Sets LED effect for the Active (approach) zone. Must match one of the predefined effects.
`distwake`| 12-192 (inches)<br>305-4980 (mm)| 200 / `OK`|Yes|Sets the Wake zone distance. Parameter range depends upon the system's unit settings (inches or millimeters). <i>Out-of-range values will be constrained to the appropriate minimum or maximum value.</i>
`distactive`<br>`diststart`| 12-192 (inches)<br>305-4980 (mm)| 200 / `OK`|Yes|Sets the Active (approach) zone distance.
`distpark`| 12-192 (inches)<br>305-4980 (mm)| 200 / `OK`|Yes|Sets the Parked zone distance.
`distbackup`| 12-192 (inches)<br>305-4980 (mm)| 200 / `OK`|Yes|Sets the Backup zone distance.
`sidedistleft`|2 - 48 (inches)<br>50 - 1220 (mm)| 200 / `OK`|Yes|Sets the Side sensor LEFT distance. Ignored if side sensor is not installed or disabled.
`sidedistright`|2 - 48 (inches)<br>50 - 1220 (mm)| 200 / `OK`|Yes|Sets the Side sensor RIGHT distance. Ignored if side sensor is not installed or disabled.

### Additional Command Notes

>⚠️**THE API ONLY UPDATES "ACTIVE" SETTINGS**<br>When system settings (such as LED brightness or zone color/distances) are updated via the API, it only changes the current 'Active' setting.  If the system reboots for any reason, changes will be lost and the last saved 'defaults' will be loaded.<br><br>To update save any changes made as the new 'Defaults', a direct command can be issued <i>(see below)</i> to force a save of the current active values.
{: .important}

<b><u>Constraining Numeric Values</u></b><br>
For commands where the parameter is a numeric value range, out-of-range values are 'constrained' to a valid value.  For example, given a payload range of `50-100`, passed values < 50 will be set to `50`.  Passed values > 100 will be set to `100`.

However, the API processing does not validate distances as it does on the web page.  Using the API, you can set a parked zone that is further away than the active zone.  In this case, the system would not respond appropriately.  Use caution when manually setting distances via the API and assure the final zone settings are appropriate.

---

## GET Commands
These commands retrieve data and do not modify settings. They must be sent individually and cannot be combined in a single URL statement.  A parameter is required, but ignored for processing.  Using a parameter of `1` is recommended.

| Command | Parameter(s) | Returns | Example / Notes |
| :--- | :---: | :---: | :--- |
| `ping` | 1 | OK<br>or<br>-48 / Timeout | `/api?ping=1`<br>Returns `OK` if successful.<br>Returns `ERR_CONNECTION_TIMED_OUT` if not found. |
| `ipaddress` | 1 | IP Address | `/api?ipaddr=1`<br>Returns IP address of controller. |
| `macaddress` | 1 | MAC Address | Returns the MAC address of the ESP32 |

---

## DIRECT Commands
The commands loosely correspond to the [Controller Commands](commands) found on the main web page, but provide a way to initiate some system functions via MQTT.

>1️⃣ **Payloads Ignored**<br>For these commands, a payload is required but ignored when processed.  It is recommended that you simply pass a "1" as the payload for each of these special direct commands.
{: .note}

 Note the **omission** of `/api` in the URL.  If a browser is used to send the command, then the corresponding web app page is returned/opened.

### Controller-Specific Direct Calls

<i>Note: Those commands shown with **Browser Only** in the notes are only applicable when called from a browser as they open an interactive web page from the application.</i>

| Command<br>`http://[ip_address]...` | Parameter(s) | Returns | Notes |
|---|:---:|:---:|---|
| `/restart` | none | Web Page | Reboots the targeted controller. |
| `/otaupdate` | none | Web Page | Places targeted unit in OTA Update mode. |
| `/firmwareupdate`|none | Web Page | **Browser only**. Launches the firmware update page.
| `/configdump` | none | Web Page | **Browser only**. Opens up the configuration dump page from the web application and lists all settings currently saved in the `config.json` file.
| `/info` | none | Web Page | **Browser only**. Opens up the system information page from the web application with system and config details.
| `/infojson` | none | JSON Payload | Returns a JSON payload with the system information.  This could be parsed and used by a third-party system.


<div style="display: flex; justify-content: space-between; align-items: center; margin-top: 40px; border-top: 1px solid #333; padding-top: 20px;">
  <a href="{{ '/mqtttopics' | relative_url }}" class="btn btn-outline"><- Previous: MQTT Topics &amp; Payloads</a>
  <a href="{{ '/discoverymain' | relative_url }}" class="btn btn-purple">Next: Home Assistant Discovery -></a>
</div>

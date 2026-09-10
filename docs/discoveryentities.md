---
layout: default
title: Updating or Hiding Discovered Entities
parent: Home Assistant Discovery
nav_order: 2
---

# Updating or Hiding Discovered Entities
{: .no_toc }

---

<p align="center">
  <img src="images/discoveryentities_01.jpg" alt="[Top banner image]">
</p>

One of the most powerful features of the Discovery system is the ability to choose exactly which components of the system are exposed to Home Assistant. You can add or remove groups of entities at any time to keep your dashboard clean and relevant.

---

## Updating an Existing Discovery
The primary controller stores your previous settings in a `discovery.json` file. This allows you to modify your entity list without creating duplicates in Home Assistant.

### Modifying Entity Groups
1. Navigate to the **Discovery** page in the **Integrations** menu.
2. Your previously selected groups will be pre-populated.
3. Check any additional groups you wish to add, or uncheck groups you wish to remove.
4. Click the **UPDATE DISCOVERY** button.

> **💡 Device Name Limitation**<br>The **Device Name** is used to generate unique MQTT discovery topics. Because of this, you cannot change the Device Name once Discovery is enabled. To change the name, you must first [Remove Discovery]({{ '/discoverymanage' | relative_url }}) and then recreate it with the new name.
{: .note }

### The Update Button
When you click **UPDATE DISCOVERY**, the system immediately sends "clearance" messages for any removed groups and "discovery" messages for any new ones. 

> **⚠️ Dashboard Warning**<br>If you remove an entity group that is currently in use on a Home Assistant dashboard or in an automation, those items will show as "Unavailable" or may break entirely.
{: .warning }

---

## Handling Hardware Changes and Firmware Upgrades
Because Discovery relies on a unique string (Device Name + MAC Address), certain hardware events require attention:

* **System Reset (Same ESP32):** If you perform a factory reset but use the same hardware and the same Device Name, running Discovery again will simply "take over" the existing entities in Home Assistant.
* **New ESP32 / Different Name:** If you replace the controller or change the name, Home Assistant will treat it as a brand-new device. In this case, it is best to delete the old device from the Home Assistant **Devices** page to avoid clutter.
* **Firmware Updates:** Some firmware updates may introduce new features like LED effects that might require an update to Discovery.  If needed, this will be noted in the release notes for the updated version.

If you make any hardware changes to your system after Discovery is enabled (e.g. enable/disable a side sensor, changing the number of LEDs, etc.), you will need to update or remove/reenable your Discovery to reflect these changes.  

* If you are making permanent hardware changes, then the recommendation is to completely remove and then reenable Discovery.  This assures the correct type of entities and properties are created for the hardware changes.

* If you just want to change the exposed entities, you just make your selected changes and update the Discovery. A complete removal and reenable of Discovery isn't needed just to change exposed entities.

## Entities Created Reference Table
The following table lists the entities created in Home Assistant based on your selections. Note that settings, such as the number or type of sensors used, will also impact the entities created.  

*Note: `device_name` refers to your specified Discovery Device Name (converted to lowercase with underscores).*

 Entity Group | Entities Created | Friendly Name 
 :--- | :--- | :--- 
**Controls**|`light.device_name_led_strip`<br>*(Includes brightness &amp; color attributes)*<br>`switch.device_name_sensor_override`|LED Strip<br><br>Sensor Override
**Sensors**|`binary_sensor.device_name_car_presence`<br>`sensor.device_name_front_distance`<br>`sensor.device_name_side_distance`<br>`sensor.device_name_zone`|Car Presence<br>Front Distance<br>Side Distance<br>Zone
**Configuration**|`number.device_name_brightness_standby`<br>`text.device_name_color1_wake`<br>`text.device_name_color2_active`<br>`text.device_name_color3_parked`<br>`text.device_name_color4_backup`<br>`text.device_name_color5_standby`<br>`select.device_name_led_effect`<br>`number.device_name_dist1_wake`<br>`number.device_name_dist2_active`<br>`number.device_name_dist3_park`<br>`number.device_name_dist4_backup`<br>`number.device_name_side_dist_left`<br>`number.device_name_side_dist_left`|Brightness Standby<br>Color1 Wake<br>Color2 Active<br>Color3 Parked<br>Color4 Backup<br>Color5 Standby<br>LED Effect<br>Dist1 Wake<br>Dist2 Active<br>Dist3 Park<br>Dist4 Backup<br>Side Dist Left<br>Side Dist Right
**Diagnostics**|`button.device_name_controller_restart`<br>`sensor.device_name_ip_address`<br>`sensor.device_name_mac_address`|Controller Restart<br>IP Address<br>MAC Address

### Additional Notes
* If your system is configured without a side sensor, then all side sensor-related entities will be omitted and will not be created.
* To control the LEDs manually via the Light entity or via automations, then assure you first enable the `Sensor Override` switch.  See more information on manually controlling the system under the [MQTT](mqtt) and [API](api) topics.  Toggle this setting back off to reenable normal system operation.

> **💡 Customization**<br>You can safely rename these entities or change their icons directly within the Home Assistant UI without affecting the controller's firmware.  But note that future Discovery updates may overwrite your changes or create duplicate entities.
{: .note }

<div style="display: flex; justify-content: space-between; align-items: center; margin-top: 40px; border-top: 1px solid #333; padding-top: 20px;">
  <a href="{{ '/discoverymanage' | relative_url }}" class="btn btn-outline"><- Previous: Enabling and Disabling Discovery</a>
  <a href="{{ '/discoverymanual' | relative_url }}" class="btn btn-purple">Next: Manually Creating MQTT Entities -></a>
</div>
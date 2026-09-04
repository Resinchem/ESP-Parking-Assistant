---
layout: default
title: Partitions and Flashing
parent: Advanced Technical Info
nav_order: 2
---

# Partitions and Flashing
{: .no_toc }

---

<p align="center">
  <img src="images/advancedpartitions_01.jpg" alt="[Partitions image]">
</p>

This information is critical if you are manually flashing source code via the Arduino IDE or a third-party utility. If you are simply using the web-based "Firmware Upgrade" feature with the provided `.bin` files, the system handles the memory offsets for you. However, for the developers out there, here is how we manage the "real estate" inside the ESP32.

### The "Sketch Too Large" Problem
Due to the complexity of the firmware and the inclusion of the web application strings, future updates and new features may cause the binary to grow too large to fit into a standard ESP32 partition. To solve this, we "borrow" space normally reserved for the SPIFFS (the local file system). Since our configuration files are tiny JSON strings, we can safely shrink the SPIFFS area and give that extra room to the primary sketch.

>⚠️ **Special Note for Users Current on version 0.52 or Lower!**<br>This "migration" to a new partition layout was first introduced beginning with version 0.60.  To upgrade from v0.52 or earlier to v0.60+, you should complete a [One Time Migration](migration) of your system.
{: .important}

<p align="center">
  <img src="images/advancedpartitions_migrate.jpg" alt="partitions" width="100%">
</p>

---

### Arduino IDE Configuration
When using the Arduino IDE to compile and flash the source code, you **must** change the partition scheme from the "Default" setting. Failure to do this may result in a compilation error stating that the firmware is too large for the available space or you might overwrite your current configurations, requiring onboarding and initial setup again.

In the **Tools** menu, ensure your settings match the following:

* **Board:** `ESP32 Dev Module`
* **Partition Scheme:** `Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)`

![Arduino IDE Partition Settings](images/advanced_partitions.jpg)

---

### Manual Partition Table
If you are using a different IDE or a specialized flashing utility (like the Espressif Download Tool), you will need the specific memory offsets and sizes. If these aren't set correctly, the ESP32 will likely fall into a "boot loop" as it tries to find its code in the wrong neighborhood.

| Partition Name | Type | Subtype | Offset | Size |
| :--- | :--- | :--- | :--- | :--- |
| **nvs** | data | nvs | `0x9000` | `0x5000` (20KB) |
| **otadata** | data | ota | `0xe000` | `0x2000` (8KB) |
| **app0** | app | ota_0 | `0x10000` | `0x1E0000` (1.875MB) |
| **app1** | app | ota_1 | `0x1F0000` | `0x1E0000` (1.875MB) |
| **spiffs** | data | spiffs | `0x3D0000` | `0x30000` (~190KB) |

> **💡 Fun Fact**<br>By using this "Minimal SPIFFS" layout, we still have two identical `app` slots (OTA0 and OTA1). This is what allows a controller to perform a "Safe Update"—it downloads the new firmware into the empty slot and only switches over if the download is successful. It’s like wearing a belt and suspenders at the same time!
{: .note }



<div style="display: flex; justify-content: space-between; align-items: center; margin-top: 40px; border-top: 1px solid #333; padding-top: 20px;">
  <a href="{{ '/advancedconfig' | relative_url }}" class="btn btn-outline"><- Previous: Configuration Files</a>
  <a href="{{ '/troubleshooting' | relative_url }}" class="btn btn-purple">Next: Troubleshooting -></a>
</div>
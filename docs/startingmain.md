---
layout: default
title: Getting Started
nav_order: 2
has_children: true
has_toc: false
---

# Getting Started
{: .no_toc }

---

<p align="center">
  <img src="images/start_green_flag.png" alt="Project Overview" width="80">
</p>

This guide focuses strictly on the installation, configuration, and operation of the provided firmware. It assumes you have already assembled your hardware, whether as a completed build or a breadboard prototype.

**🛠️ Hardware & Build Reference**<br>If you are looking for wiring diagrams, assembly instructions, or a complete parts list, please refer to the following external resources:
* **[YouTube Video Overview]({{ site.links.youtube_video }})**
* **[Written Build Guide](https://resinchemtech.blogspot.com/2026/08/parking-assistant-2026.html)**
{: .note }

---

## Setup Roadmap
To achieve a fully functional system, you must complete the following steps in the specific order listed below. Each section builds upon the previous one.  If building multiple systems (e.g. one for each side of a two-car garage), you should complete the installation and onboarding of the first system before starting on any additional systems.

### 1. [Firmware Installation]({{ '/installation' | relative_url }})
The first step is flashing the custom compiled C++ firmware (.bin) to the controllers using the installer provided on the next page.

### 2. [Network Onboarding]({{ '/onboarding' | relative_url }})
Once flashed, you will connect the controller to your local Wi-Fi network. Remember: this system is designed for local-first operation and does not require an active internet connection for core functionality.  While WiFi is required for this initial onboarding, after configuration, you may switch the system to "Access Point (No WiFi)" mode and then move it to the final installation location which may not have WiFi available.

### 3. [Complete Initial Hardware Configuration]({{ '/setupmain' | relative_url }})
Establish the number of LEDs, enable or disable certain options and configure your zone distances and desired LED colors/effect.

## What's Next?
The system will not be fully functional until all three steps above are completed. Once finished, you can proceed to the **General Use** and the remaining topics to customize your system, and enable _optional_ integrations, such as MQTT or Home Assistant Discovery.

<div style="display: flex; justify-content: space-between; align-items: center; margin-top: 40px; border-top: 1px solid #333; padding-top: 20px;">
  <a href="{{ '/concepts' | relative_url }}" class="btn btn-outline"><- Previous: Concepts & Terminology</a>
  <a href="{{ '/installation' | relative_url }}" class="btn btn-purple">Next: Firmware Installation -></a>
</div>
---
layout: default
title: Optional Integrations
nav_order: 5
has_children: true
has_toc: false
---

# Optional Integrations
{: .no_toc }

---

<p align="center">
  <img src="images/integrationmain_01.jpg" alt="Integrations Overview">
</p>
This section is for those who prefer to let their LEDs take orders from a script rather than a sensor. Whether you're a Home Assistant power user or a terminal enthusiast, these tools let you bypass the UI and talk directly to the hardware. 🛠️

<i>Note that use of these integration methods are entirely optional and are not required for any functionality of the Parking Assistant.</i>

>⚠️ **WiFi Required**<br>If you are operating your system in Access Point (no WiFi) mode, you will not be able to enable or use MQTT or the HTTP API as these methods require WiFi for communication.
{: .important}

> **<img src="images/ha_icon.png">&nbsp; Attention Home Assistant Users!**<br>
While you can certainly manually integrate the Parking Assistant using MQTT, check out the [Home Assistant Discovery](discoverymain) section for how you can safely add, edit and even remove the Parking Assistant to or from Home Assistant instantly with a single button click.  **No YAML needed!** 👍
{: .note }

When it comes to interacting with the Parking Assistant system via third party systems, there are two primary options available:

### MQTT
Message Queuing Telemetry Transport (MQTT) is a lightweight messaging protocol designed for IoT communication. If you have an MQTT broker and a system that can publish/subscribe to topics, you can fully automate the system’s states and settings. 

* **Learn more at:** [MQTT.org](https://mqtt.org/)
* **Implementation &amp; Topics:** [MQTT Setup & Topics]({{ '/mqtt' | relative_url }})

### HTTP API
The only requirement for using this method is that a third party system must be able to POST a message to a URL (the IP address of the controller).  For example, the following could be used to turn on the LEDs:

`http://[your_controller_ip]/api?ledstate=on`

<i>Note:  Since these are just URLs, you could also POST messages to the system via a browser, using any device on the same network.</i>

* **Implementation &amp; Commands:** [API HTTP Command List]({{ '/api' | relative_url }})

<div style="display: flex; justify-content: space-between; align-items: center; margin-top: 40px; border-top: 1px solid #333; padding-top: 20px;">
  <a href="{{ '/commands' | relative_url }}" class="btn btn-outline"><- Previous: Controller Commands</a>
  <a href="{{ '/mqtt' | relative_url }}" class="btn btn-purple">Next: MQTT Setup & Config -></a>
</div>

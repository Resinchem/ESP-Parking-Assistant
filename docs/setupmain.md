---
layout: default
title: Setting Up the System
nav_order: 3
has_children: true
has_toc: false
---

# Setting Up the System
{: .no_toc }

---

<p align="center">
  <img src="images/setupmain_01.jpg" alt="System Setup Header">
</p>

Once the system has been onboarded and the controller is booting normally, you can begin to configure the system for your particular hardware and needs.  This includes options like the number of LEDs, sensor settings and other global options.

This section covers the core configuration of your hardware and default settings:

* **[Web App Overview]({{ '/webapp' | relative_url }})** - Accessing the app and understanding the interface.
* **[Initial Hardware Configuration]({{ '/initconfig' | relative_url }})** - Setting up the system for your exact hardware.
* **[Sensor Calibration]({{ '/sensorcalibrate' | relative_url }})** - See raw readings from your sensor(s).  Used to assist in setting zone distances.

> **💡 Configuration Logic**<br>It is important to understand that the system manages two distinct "sets" of settings:
> * **Default Settings:** Saved values loaded when the system first starts up or reboots.
> * **Active Settings:** Values currently in use (which may be temporary until the next reboot).
>
> This distinction is covered in more detail in the [Web App Overview]({{ '/webapp' | relative_url }}).
{: .note }

---

<div style="display: flex; justify-content: space-between; align-items: center; margin-top: 40px; border-top: 1px solid #333; padding-top: 20px;">
  <a href="{{ '/booting' | relative_url }}" class="btn btn-outline"><- Previous: The Boot Process</a>
  <a href="{{ '/webapp' | relative_url }}" class="btn btn-purple">Next: Web App Overview -></a>
</div>
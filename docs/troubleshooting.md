---
layout: default
title: Troubleshooting
nav_order: 9
has_children: true
has_toc: false
---

# Troubleshooting
{: .no_toc }

---

<p align="center">
  <img src="images/troubleshooting_01.jpg" alt="Troubleshooting Overview">
</p>

If you find there is a "ghost in the machine" or your system isn't behaving as expected, don't panic. Electronics can be finicky, but most issues can be resolved with a bit of systematic checking. 

> **🔍 Living Document**<br>
> This troubleshooting topic will be updated if common issues/workarounds are discovered over time.  If you develop a problem or issue after you've been using the system for a while, be sure to check this topic again, as it may have been updated with new information
{: .note }

Before diving into the specific fixes below, first verify that you have reviewed the applicable sections throughout this document and the [Build Guide](https://resinchemtech.blogspot.com/2026/08/parking-assistant-2026.html) to ensure a step wasn't missed during the assembly process.  In addition, you can often find helpful information or potential problems/fixes in the following areas of the repository:
* **<a href="https://github.com/Resinchem/ESP-Parking-Assistant/discussions">Repository Discussions</a>**
* **<a href="https://github.com/Resinchem/ESP-Parking-Assistant/issues">Repository Issues</a>**


---

### 🛑 The Golden Rule of Hardware
As emphasized throughout this project, **the firmware is designed to work with specific hardware components.** 

> **❗ Support Disclaimer**<br>
> I am unable to provide technical support for builds that use substituted hardware components or modified firmware. If you decide to go "off-book" with different functions, sensors, or controllers, you are officially the lead engineer for that version! Please don't ask for support when using custom firmware modifications or unsupported hardware.
{: .important }

---

### Triage: Where is the problem?

To help you find a solution without scrolling through pages of unrelated text, the troubleshooting guide is broken down into the following categories:

* **[Initial Setup & Connectivity]({{ '/troublesetup' | relative_url }})** – Issues with COM ports, USB drivers, flashing errors, and the initial onboarding hotspot.
* **[Configuration And Hardware]({{ '/troubleconfig' | relative_url }})** - Issues with configuration and hardware setup.
* **[Daily Operation & Use]({{ '/troubleoperation' | relative_url }})** – Problems with "Save & Reboot" persistence, firmware update failures, and reviewing Config Dumps.
* **[Home Assistant & MQTT]({{ '/troublediscovery' | relative_url }})** – Discovery issues, "stuck" MQTT topics, and integration prerequisites.
* **[FAQ & Getting Help]({{ '/troublefaq' | relative_url }})** – Frequently Asked Questions and the proper etiquette for opening a GitHub Issue or starting a Discussion.

---

<div style="display: flex; justify-content: space-between; align-items: center; margin-top: 40px; border-top: 1px solid #333; padding-top: 20px;">
  <a href="{{ '/advancedpartitions' | relative_url }}" class="btn btn-outline"><- Previous: Partitions &amp; Flashing</a>
  <a href="{{ '/troublesetup' | relative_url }}" class="btn btn-purple">Next: Initial Setup Troubleshooting -></a>
</div>
## MQTT / Home Assistant Examples

**Starting with release v0.45, Home Assistant Discovery is now available**

MQTT Discover can be launched from within the Parking Assistant web application and it will automatically create a new device and entities in Home Assistant for you, without the need to create any manual YAML or do any other configuration in Home Assistant. Discovery is the **recommended method** of integrating your Parking Assistant.  Please see [Home Assistant Discovery](https://resinchem.github.io/ESP-Parking-Assistant/discoverymain.html) topic for more information.

However, if the discovery process does not work for you or you simply wish to completely control how the entities are created in Home Assistant, you can manually create them via YAML. The basic code for creating these entities is provided in the yaml examples above.  They do not contain additional fields like last-will-and-testament nor any device information. You can tweak these to use your own preferred names, add default icons, or any other valid properties.

Once created, the entities should be available in Home Assistant for use on your dashboards or in your automations.

![HA_Entities_Blurred_Small](https://github.com/Resinchem/ESP-Parking-Assistant/assets/55962781/b8085110-e003-4587-a734-4a63344e53ab)

Note that when first created, these entities may have a state of 'Unknown' until the controller updates its MQTT values based on the telemetry period set in the controller's MQTT options.  You can force an immediate refresh of all topics by any of the following methods:
- Issue an mqtt command to: `cmnd/your_subscribe_topic/refresh` with a payload of "1"
- Use a browser to send the command: `http://[your_ip_address]/api?refresh=1`
- Reboot the Parking Assistant controller.

<i>Creating individual entities as shown in the examples will <b>not</b> create a device like when using Discovery.  To manually add the entities to a common device, you'd need to add the device information to each entity's YAML definition.  See the [Home Assistant MQTT documentation](https://www.home-assistant.io/integrations/mqtt/) for more information on how to add device properties to an entity.</i>

Note that MQTT is optional for the project and it is not required and can be disabled at any time via the web settings.

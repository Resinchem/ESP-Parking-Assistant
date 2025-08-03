## MQTT / Home Assistant Examples
This provides examples of how to setup sensors in Home Assistant to use data published by the parking assistant via MQTT.

**Starting with release v0.45, Home Assistant MQTT Discovery is now available**

MQTT Discover can be launched from within the Parking Assistant web application and it will automatically create a new device and entities in Home Assistant for you, without the need to create any manual YAML or do any other configuration in Home Assistant.  Please see Wiki [MQTT and Home Assistant](https://github.com/Resinchem/ESP-Parking-Assistant/wiki/08-MQTT-and-Home-Assistant) topic for more information.  You must have a properly installed and configured MQTT broker for this feature (the Home Assistant MQTT add-on is OK).

However, if the discovery process does not work for you or you simply wish to completely control how the entities are created in Home Assistant, you can manually create them via YAML. Sample code for creating these entities is provided in the sensors.yaml file above.  You can tweak these to use your own preferred names, add default icons, etc.

Once created, the entities should be available in Home Assistant for use on your dashboards or in your automations.

![HA_Entities_Blurred_Small](https://github.com/Resinchem/ESP-Parking-Assistant/assets/55962781/b8085110-e003-4587-a734-4a63344e53ab)

Note that when first created, these entities may have a state of 'Unknown' until the controller updates its MQTT values based on the telemetry period set in the controller's MQTT options.  See the wiki section on Using the Web Interface and MQTT for more details.

Note that MQTT is optional for the project and it is not required and can be disabled at any time via the web settings.

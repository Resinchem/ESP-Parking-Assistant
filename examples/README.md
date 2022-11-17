## MQTT / Home Assistant Examples
This provides examples of how to setup sensors in Home Assistant to use data published by the parking assistant via MQTT.

To use the exported MQTT data from the parking assistant in Home Assistant, you must first create MQTT sensors.  Sample code for creating these entities is provided in the sensors.yaml file above.  You can tweak these to use your own preferred names, add default icons, etc.

Once created, the entities should be available in Home Assistant for use on your dashboards or in your automations.

![HA_MQTT_Entities](https://user-images.githubusercontent.com/55962781/202325999-6794bae9-5fa3-4b59-9b48-055500748423.jpg)

Note that when first created, these entities may have a state of 'Unknown' until the controller updates its MQTT values based on the telemetry period set in the controller's MQTT options.  See the wiki section on Using the Web Interface and MQTT for more details.

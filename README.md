# INFORMATION SYSTEM AND VISUALIZATION OF PARKING AVAILABILITY BASED ON MQTT PROTOCOL IN MULTI-STOREY PARKING LOTS

This repo is made in order to finish my thesis. In this repo you will find information regarding my thesis (the title).

This system is planted above parking slots to detect if is there any car at the parking slots.

The information system made use of ESP32 to gather information regarding distance using HC-SR04 ultrasonic sensor. Distance data is forwarded to Node-RED using Mosquitto MQTT. Node-RED will process the distance into state of each parking slots. The state data is then sent back to ESP32 for physical indicator with LED and virtual indicator inside Node-RED dashboard.

The dasboard is available at 192.168.1.100:1880/ui and has to be connected to WiFi : parkirAP_B1 or parkirAP_B2 at Robotic Laboratory of Electrical Eng. at Maranatha Christian Uni. Contact me for WiFi info.

Result Video is available at https://youtu.be/-B2HvUjEWWI

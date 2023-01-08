# ESP-NOW water leakage sensor for ESP8266

ESP-NOW based water leakage sensor for ESP8266. Alternate firmware for Tuya/SmartLife WiFi water leakage sensors.

## Features

1. 2 possiility operating modes - NORMAL and LITE.
2. Average response time of 1.8 second (NORMAL), 0.4 second (LITE) (depends on the MCU of the sensor).
3. Triggered in 2 cases: contact closure, contact drying (only if the contacts were previously closed).
4. When triggered transmits system information and sensor status (ALARM/DRY) at NORMAL mode and always ALARM at LITE mode.
5. In setup/update mode creates an access point named "ESP-NOW Water XXXXXXXXXXXX" with password "12345678" (IP 192.168.4.1) at NORMAL mode only.
6. Automatically adds sensor configuration to Home Assistan via MQTT discovery as a binary_sensor.
7. Possibility firmware update over OTA (if is allows the size of the flash memory) at NORMAL mode only.
8. Web interface for settings at NORMAL mode only.
  
## Notes

1. ESP-NOW mesh network based on the library [ZHNetwork](https://github.com/aZholtikov/ZHNetwork).
2. For enter to setup/update mode press the button for > 2 seconds. The LED blinks fast. Access point will be available during 120 seconds before the module is powered off.

## Tested on

See [here](https://github.com/aZholtikov/ESP-NOW-Water-Leakage-Sensor/tree/main/hardware).

## Attention

1. A gateway is required. For details see [ESP-NOW Gateway](https://github.com/aZholtikov/ESP-NOW-Gateway).
2. ESP-NOW network name must be set same of all another ESP-NOW devices in network.
3. Upload the "data" folder (with web interface) into the filesystem before flashing for NORMAL mode only.
4. For using this firmware on Tuya/SmartLife WiFi water leakage sensors, the WiFi module must be replaced with an ESP8266 compatible module (if necessary).
5. Highly recommended connect an external power supply during setup/upgrade.
6. Because this sensor is battery operated, it has an additional controller (MCU) that controls the power of the WiFi module (Module) and transmits data to it for transmission to the network. The communication is done via UART at 9600 speed. Make sure that the protocol is correct before flashing. Details [here](https://github.com/aZholtikov/ESP-NOW-Water-Leakage-Sensor/tree/main/doc) for NORMAL mode only.

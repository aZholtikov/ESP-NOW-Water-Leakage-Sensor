# Communication protocol

The protocol can be read with 2 USB-TTL converters. RX of the first one is connected to the RX of the WiFi module to read the MCU data. Connect the RX of the second to the TX of the WiFi module to read the module's data. Don't forget to connect all GND (of both converters and module).

Communication protocol used in the firmware (only necessary "cuts" from the original protocol):

```text
1. Normal mode. Sensor triggering.

      Module power is on
      Module sends            55 AA 00 01 00 00 00                                        (Initial message)
      MCU returns             55 AA 00 01 00 00 ............                              (MCU system information)
      Module sends            55 AA 00 02 00 01 04 06                                     (Network connection established)
      MCU returns             55 AA 00 02 00 00 01                                        (Confirmation message)
      MCU sends               55 AA 00 05 00 05 01 04 00 01 00 0F                         (Leak status. 00 0F - leak, 01 10 - dry)
      Module returns          55 AA 00 05 00 01 00 05                                     (Confirmation message)
      MCU sends               55 AA 00 05 00 05 03 04 00 01 02 13                         (Battery status. 02 13 - high, 01 12 - medium, 00 11 - low)
      Module returns          55 AA 00 05 00 01 00 05                                     (Confirmation message)
      Module power off

2. Update mode. Pressing the button for > 2 seconds - the LED blinks fast.

      Module power is on
      Module sends            55 AA 00 01 00 00 00                                        (Initial message)
      MCU returns             55 AA 00 01 00 00 ............                              (MCU system information)
      Module sends            55 AA 00 02 00 01 04 06                                     (Network connection established)
      MCU returns             55 AA 00 02 00 00 01                                        (Confirmation message)
      MCU sends               55 AA 00 04 00 00 01 00 04                                  (Message for switching to setting mode)
      Module returns          55 AA 00 04 00 00 03                                        (Confirmation message)

      Update mode has started. Will be available during 120 seconds until the module is powered off.
      After updating and rebooting the module will return to normal mode - the LED will go off.
      Highly recommended connect an external power supply during update.

Battery level sending is not used in the firmware because it is not needed.
```

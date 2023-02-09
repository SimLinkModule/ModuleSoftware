# SimLinkModule-Software

This is the software for a BLE game controller expansion module for the TBS Tango 2. The data transfer between the module and the remote control takes place via CRSF. The first eight channels are transmitted analog and the remaining eight channels are transmitted digital. An output for the BLE connection status is done via an OLED display type SSD1306. ESP-32 Dev Kit C V4 from AZ- Delivery used.

## Tested operating systems
- [x] Windows 11 (tested with Velocidrone)
- [x] Android 10 (tested with FPV.SkyDive)
- [x] Pop! OS 22.04 LTS (tested with Velocidrone)

## Links to the other parts of the project
- [ ] [PCB](https://github.com/SimLinkModule/PCB)
- [ ] [Shell](https://github.com/SimLinkModule/Shell)
- [ ] [Doc](https://github.com/SimLinkModule/documentation)

## Known bugs / issues
- [ ] Display shows wrong output for some GAP events
- [ ] iOS not working. Connection can be established and automatic reconnection also works. RPA works. HID data is also subscribed and sent, but not processed by iOS.
    - Probably needs a closer look with XCode and Packetlogger (Mac required :thinking: )
    - Or a ticket to the Apple Developer Technical Support Team (Apple Developer account required :thinking: )
- [ ] Pop! OS kernel doesn't recognize HID version when connecting via BLE (always version 0.0) &rarr; Linux Kernel output is correct with an XBox controller
- [ ] For an XBox controller, the client configuration descriptor of the report is set to 01 00 (by default). Here the default is 00 00 &rarr; Should be according to the specification.

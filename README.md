# SimLinkModule-Software

This is the software for a BLE game controller expansion module for the TBS Tango 2. The data transfer between the module and the remote control takes place via CRSF. An output for the BLE connection status is done via an OLED display type SSD1306.

## Tested operating systems
- [x] Windows 11
- [x] Android 10
- [x] Pop! OS 22.04 LTS

## TODO
- [ ] Create a case &rarr; will be provided to another repo
- [ ] Creating a PCB &rarr; will be provided to another repo
- [ ] Add a status led

## Known bugs / issues
- [ ] Display shows wrong output for some GAP events
- [ ] iOS not working. Connection can be established and automatic reconnection also works. RPA works. HID data is also subscribed and sent, but not processed by iOS.
    - Probably needs a closer look with XCode and Packetlogger (Mac required :thinking: )
    - Or a ticket to the Apple Developer Technical Support Team (Apple Developer account required :thinking: )
- [ ] Pop! OS kernel doesn't recognize HID version when connecting via BLE (always version 0.0) &rarr; Works with an XBox Controller
- [ ] For an Xbox controller, the client configuration descriptor of the report is set to 01 00 (by default). Here the default is 00 00 &rarr; Should be according to the specification.
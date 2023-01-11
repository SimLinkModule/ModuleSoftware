# iOS
- On iOS, the [LightBlue](https://punchthrough.com/lightblue/) app can be used. Note that Apple does not pass the raw data to 3rd party apps for some services [S. 197](https://developer.apple.com/accessories/Accessory-Design-Guidelines.pdf). This means that some data cannot be debugged. One service, for example, is HID.
- Another possibility for debugging for iOS is with a Mac running XCode. For this there is [PacketLogger](https://developer.apple.com/bluetooth/) which can log Bluetooth packets live. Additionally, a logging profile must be downloaded and installed on the iOS device.

# Android
- For Android, the apps [LightBlue](https://punchthrough.com/lightblue/) and [nRF Connect](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&hl=en&gl=US) can be used. There you can read more data / services as on iOS.
- In the developer options "Bluetooth HCI snoop protocol can be enabled" and the data can be logged and later inspected using Wirekshark.

# Linux
- Tools: hcitool, gatttool, bluetoothctl, hidraw-dump, Bluepy

## General
Edit `/lib/systemd/system/bluetooth.service` and change to:

`ExecStart=/usr/libexec/bluetooth/bluetoothd --noplugin=input,hog`

Then Bluetooth daemon won't grab input or hog devices and won't create system input nodes. Then the HID GATT service and corresponding characteristics are available via BlueZ to application.

## hcitool

### Available Bluetooth device at the Linux computer
`hcitool dev`

### Scan for BLE-Devices
`sudo hcitool lescan`

## gatttool

### Establish connection
1. `sudo gatttool [-t random] -b <BLE ADDRESS> -I`
2. `connect`

### Show list of all existing services
`primary`

### List of all existing handles
Each handle is a connection point where data can be read or written.

`char-desc`

### Read a handle
`char-read-hnd <handle>`

### Write a value to the handle
`char-write-req <handle> <data>`

## bluetoothctl
`sudo bluetoothctl`

### Search devices
`scan le`

### Connect device
`connect <BLE ADDRESS>`

### Device info
`info`

### Change menu to work with GATT
`menu gatt`

#### list all attributes
`list-attributes`

#### read attribute
- `select-attribute <path>`
- `read`

## hidraw-dump

### Installation
- `sudo apt-get install build-essential pkg-config libudev-dev`
- `git clone https://github.com/todbot/hidraw-dump`
- `cd hidraw-dump`
- `make`

### GET all HID descriptors
`sudo ./hidraw-dump`

### Make HID descriptor human readable
Use the [website](https://eleccelerator.com/usbdescreqparser/).

## Bluepy

### Installation
1. `sudo apt-get install bluetooth build-essential libglib2.0-dev libdbus-1-dev`
2. `git clone https://github.com/IanHarvey/bluepy.git`
3. `cd bluepy/bluepy`
4. `make`

### Usage

#### start
`./bluepy-helper`

#### connect
`conn <BT ADDRESS>`

#### List of services
`svcs`

## Note:
- Debugging the XBox controller only worked if it was not connected in the Bluetooth settings and if the XBox controller was only switched to connecting mode when gatttools was open and `connect` was called. The option random shouldn't be activated in the gatttool.
# iOS
- Unter iOS kann die App [LightBlue](https://punchthrough.com/lightblue/) verwendet werden. Zu beachten ist das Apple bei einigen Diensten nicht die rohen Daten an 3rd party Apps weitergeben [S. 197](https://developer.apple.com/accessories/Accessory-Design-Guidelines.pdf). Dadurch können manche Daten nicht gedebugged werden. Ein Service ist beispielsweise HID.
- Eine weitere Möglichkeit für das Debugging für iOS ist mit einem Macbook auf dem XCode läuft. Dafür gibt es [PacketLogger](https://developer.apple.com/bluetooth/) womit Bluetooth-Pakete live gelogged werden können. Zusätzlich muss auf dem iOS-Gerät ein Logging-Profil heruntergeladen werden und installiert werden.

# Android
- Unter Android können die Apps [LightBlue](https://punchthrough.com/lightblue/) und [nRF Connect](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&hl=en&gl=US) verwendet werden. Dort können mehr Daten / Dienste wie unter iOS ausgelesen werden.
- In den Entwickleroptionen kann "Bluetooth HCI-Snoop-Protokoll aktiviert" werden und die Daten gelogged werden und später mittels Wirekshark ausgelesen werden.

# Linux
- Tools: hcitool, gatttool, bluetoothctl, hidraw-dump, Bluepy

## hcitool

### Verfügbares Bluetoothgerät am Linuxrechner
`hcitool dev`

### Scan for BLE-Devices
`sudo hcitool lescan`

## gatttool

### Verbindung aufbauen
1. `sudo gatttool [-t random] -b <BLE ADDRESS> -I`
2. `connect`

### Liste aller vorhandenen Dienste aufzeigen
`primary`

### Liste aller vorhandenen Handles
Jeder Handle ist ein Verbindungspunkt, wo Daten gelesen oder geschrieben werden können.

`char-desc`

### Ein Handle auslesen
`char-read-hnd <handle>`

### Schreiben eines Werts in ein Handle
`char-write-req <handle> <data>`

## bluetoothctl
`sudo bluetoothctl`

### Geräte suchen
`scan le`

### Gerät verbinden
`connect <BLE ADDRESS>`

### Info des Geräts
`info`

### Menü wechseln, damit mit GATT gearbeitet werden kann
`menu gatt`

#### Attributliste
`list-attributes`

#### Attribut auslesen
- `select-attribute <Pfad>`
- `read`

## hidraw-dump

### Installation
- `sudo apt-get install build-essential pkg-config libudev-dev`
- `git clone https://github.com/todbot/hidraw-dump`
- `cd hidraw-dump`
- `make`

### Auslesen aller HID-Deskriptoren
`sudo ./hidraw-dump`

### HID Deskriptor menschen lesbar machen
Auf dieser [Website](https://eleccelerator.com/usbdescreqparser/) umwandeln.

## Bluepy

### Installation
1. `sudo apt-get install bluetooth build-essential libglib2.0-dev libdbus-1-dev`
2. `git clone https://github.com/IanHarvey/bluepy.git`
3. `cd bluepy/bluepy`
4. `make`

### Verwendung

#### Starten
`./bluepy-helper`

#### Verbinden
`conn <BT ADDRESS>`

#### Liste von Diensten
`svcs`

## Hinweis:
- Debugging des XBox controllers hat nur verbunden, wenn man ihn nicht in den Bluetooth-Einstellungen verbunden hatte und wenn mann ihn erst connecting modus bringt wenn man gatttools offen hat und danach connect aufruft.
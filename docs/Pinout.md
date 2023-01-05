# ESP-32

| Pin | Anmerkung                   |
| --- | --------------------------- |
| 32  | ADC-Eingang nicht benötigt  |
| 16  | UART-Eingang                |
| 21  | SDA                         |
| 22  | SCK                         |
| 25  | rechter Button              |
| 26  | linker Button               |

**Anmerkung:**

- Wenn der ADC für die Ermittlung der Spannung verwendet werden soll. (Wenn ein Pin für die Batteriespannung vorhanden ist) Dann muss ein Spannungsteiler verwendet werden, welche die Maximalspannung der Batterie auf 3.3V herunterregelt. Des Weiteren muss noch die Formel in der Batteriekomponente angepasst werden. Im Testaufbau war R1 650 Ohm und R2 2000 Ohm.
- Die Buttons sind jeweils mit GND verbunden. Ein interner Pullup-Widerstand im ESP zieht den Pin im Ruhezustand auf 3.3V.

# OLED-Display
| Pin | Anmerkung |
| --- | --------- |
| GND | -         |
| VCC | 3.3V      |
| SCK | zu SCK    |
| SDA | zu SDA    |

**Anmerkungen:** -

# Drohnenfernsteuerung

| Pin | Anmerkung                           |
| --- | ----------------------------------- |
| 0   | S.Port - Pin für UART-Übertragungen |
| 1   | GND                                 |
| 2   | VMAIN - konstante Spannungsregelung |
| 3   | HEARTBEAT - nicht verwendet         |
| 4   | PXX_OUT - nicht verwendet           |
| 5   | PXX_IN - nicht verwendet            |
| 6   | CANH - nicht verwendet              |
| 7   | CANL - nicht verwendet              |

**Anmerkungen:**

- Die Pins werden betrachtet, wenn die Fernsteuerung umgedreht ist und die Antenne von einem weg zeigt.
- Pin 0 befindet sich ganz rechts und Pin MAX ist ganz links
- VMAIN ist bei der TBS Tango 2 konstant 6V. Es gibt aber auch Dokumente da steht zwischen 5V und 12V konstant.
- Es gibt keinen Pin, welcher die Batteriespannung ausgibt
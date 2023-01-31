# ESP-32

| Pin | Note                        |
| --- | --------------------------- |
| 32  | ADC inut, not used          |
| 16  | UART input                  |
| 21  | SDA                         |
| 22  | SCK                         |
| 25  | right button                |
| 26  | left button                 |

**Note:**

- If the ADC should be used to determine the voltage, (If there is a pin for the battery voltage) then a voltage divider must be used, which regulates the maximum voltage of the battery down to 3.3V. Also the formula in the battery component has to be adjusted. In the test setup R1 was 650 Ohm and R2 2000 Ohm.
- The buttons are connected to GND. An internal pullup resistor in the ESP pulls the pin to 3.3V in idle state.

# OLED-Display
| Pin | Note      |
| --- | --------- |
| GND | -         |
| VCC | 3.3V      |
| SCK | to SCK    |
| SDA | to SDA    |

**Note:** -

# Drone remote controller

| Pin | Not                                 |
| --- | ----------------------------------- |
| 0   | S.Port - Pin for UART transmissions |
| 1   | GND                                 |
| 2   | VMAIN - constant voltage regulation |
| 3   | HEARTBEAT - not used                |
| 4   | PXX_OUT - not used                  |
| 5   | PXX_IN - not used                   |
| 6   | CANH - not used                     |
| 7   | CANL - not used                     |

**Note:**

- The pins are in the right order when the remote control is upside down and the antenna is pointing away from you.
- Pin 0 is on the far right and pin MAX is on the far left
- VMAIN is constant 6V for the TBS Tango 2. But there are also documents that state constant between 5V and 12V.
- There is no pin which outputs the battery voltage

#ifndef SSD1306_IMAGES_H
#define SSD1306_IMAGES_H

//drawn in: https://www.pixilart.com/draw
//converted from png to bitstring with: https://www.dcode.fr/binary-image
//convert bitstring to hex array: Bin2Hex.py <filename> in tools folder

const uint8_t connectedImage[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xF8,0xFC,0xFC,0xFC,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0x00,0x00,0x00,0x00,0x00,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFC,0xFC,0xFC,0xF8,0xF8,0xF0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xC0,0xE0,0xE0,0x60,0x60,0x60,0x60,0xE0,0xC0,0xC0,0xC0,0xC0,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0xF0,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x06,0x06,0x06,0x06,0x06,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0xF0,0xE0,0xE0,0x60,0x60,0x60,0x70,0x30,0x30,0x30,0x38,0x18,0x18,0x18,0x18,0x18,0x1C,0x0C,0x0E,0x0E,0x06,0x06,0x06,0x06,0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1C,0x1C,0x18,0x18,0x38,0x38,0x30,0x30,0x70,0xE0,0xE0,0xC0,0x80,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x03,0x03,0x07,0x06,0x0E,0x0E,0x0C,0x1C,0x18,0x18,0x18,0x18,0x18,0x1C,0x0C,0x0C,0x0C,0x0C,0x0E,0x0E,0x06,0x06,0x06,0x07,0x03,0x03,0x03,0x03,0x01,0x01,0x0F,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x60,0x60,0x60,0x60,0x60,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x0F,0x1F,0x1F,0x3F,0x3F,0x3F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x3F,0x3F,0x3F,0x1F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

#endif
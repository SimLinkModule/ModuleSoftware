#ifndef SSD1306_FONT_H
#define SSD1306_FONT_H

#define CHAR_WIDTH  10
#define CHAR_HEIGHT 14

//Ein Hex wert stellt eine Spalte eines Zeichen dar
//lsb sind die unteren pixel des zeichens
//die 2 msb werden immer übersprungen
const uint16_t VCR_OSD_MONO[43][10] = {
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}, // 
    {0x0038,0x00FE,0x19C7,0x1983,0x1983,0x1983,0x1983,0x1CC6,0x0FFF,0x07FF}, // a
    {0x3FFF,0x3FFF,0x01FC,0x038E,0x0707,0x0603,0x0707,0x038E,0x01FC,0x00F8}, // b
    {0x01FC,0x03FE,0x0707,0x0603,0x0603,0x0603,0x0603,0x0707,0x038E,0x018C}, // c
    {0x00F8,0x01FC,0x038E,0x0707,0x0603,0x0707,0x038E,0x01FC,0x3FFF,0x3FFF}, // d
    {0x01FC,0x03FE,0x0767,0x0663,0x0663,0x0663,0x0663,0x0767,0x03E6,0x01C4}, // e
    {0x0000,0x0300,0x0300,0x0300,0x1FFF,0x3FFF,0x3300,0x3300,0x3300,0x0000}, // f
    {0x0380,0x07C6,0x0EE7,0x0C63,0x0C63,0x0C63,0x0C63,0x0EC7,0x07FE,0x03FC}, // g
    {0x3FFF,0x3FFF,0x01C0,0x0380,0x0700,0x0600,0x0700,0x0380,0x01FF,0x00FF}, // h
    {0x0000,0x0000,0x0180,0x0180,0x19FC,0x19FE,0x0007,0x0003,0x0000,0x0000}, // i
    {0x0000,0x0000,0x0003,0x0003,0x0603,0x0607,0x37FE,0x37FC,0x0000,0x0000}, // j
    {0x3FFF,0x3FFF,0x001C,0x0038,0x0070,0x00F8,0x01DC,0x038E,0x0707,0x0603}, // k
    {0x0000,0x0000,0x0000,0x0000,0x3FFF,0x3FFF,0x0000,0x0000,0x0000,0x0000}, // l
    {0x07FF,0x07FF,0x0300,0x0600,0x07FF,0x07FF,0x0600,0x0700,0x03FF,0x01FF}, // m
    {0x07FF,0x07FF,0x0300,0x0600,0x0600,0x0600,0x0600,0x0700,0x03FF,0x01FF}, // n
    {0x01FC,0x03FE,0x0707,0x0603,0x0603,0x0603,0x0603,0x0707,0x03FE,0x01FC}, // o
    {0x0FFF,0x0FFF,0x07F0,0x0E38,0x0C18,0x0C18,0x0C18,0x0E38,0x07F0,0x03E0}, // p
    {0x03E0,0x07F0,0x0E38,0x0C18,0x0C18,0x0C18,0x0E38,0x07F0,0x0FFF,0x0FFF}, // q
    {0x07FF,0x07FF,0x01C0,0x0380,0x0700,0x0600,0x0600,0x0700,0x0380,0x0180}, // r
    {0x018C,0x03CE,0x07C7,0x0663,0x0663,0x0673,0x0633,0x073F,0x039E,0x018C}, // s
    {0x0000,0x0000,0x0600,0x0600,0x3FFC,0x3FFE,0x0607,0x0603,0x0000,0x0000}, // t
    {0x07FC,0x07FE,0x0007,0x0003,0x0003,0x0003,0x0003,0x0007,0x07FE,0x07FC}, // u
    {0x07F0,0x07F8,0x001C,0x000E,0x0007,0x0007,0x000E,0x001C,0x07F8,0x07F0}, // v
    {0x07FC,0x07FE,0x0007,0x0003,0x007E,0x007E,0x0003,0x0007,0x07FE,0x07FC}, // w
    {0x0603,0x0707,0x038E,0x01DC,0x00F8,0x00F8,0x01DC,0x038E,0x0707,0x0603}, // x
    {0x0FC0,0x0FE0,0x0073,0x0033,0x0033,0x0073,0x00E3,0x03C7,0x0FFE,0x0FFC}, // y
    {0x0603,0x0607,0x060F,0x061F,0x063B,0x0673,0x06E3,0x07C3,0x0783,0x0703}, // z
    {0x0FFC,0x1FFE,0x3837,0x3073,0x30E3,0x31C3,0x3383,0x3B07,0x1FFE,0x0FFC}, // 0
    {0x0000,0x0000,0x0C03,0x1C03,0x3FFF,0x3FFF,0x0003,0x0003,0x0000,0x0000}, // 1
    {0x0C3F,0x1C7F,0x38E3,0x30C3,0x30C3,0x30C3,0x30C3,0x39C3,0x1F83,0x0F03}, // 2
    {0x0C0C,0x1C0E,0x3807,0x3003,0x30C3,0x30C3,0x30C3,0x39E7,0x1FFE,0x0F3C}, // 3
    {0x00F0,0x01F0,0x03B0,0x0730,0x0E30,0x1C30,0x3FFF,0x3FFF,0x0030,0x0030}, // 4
    {0x3F0C,0x3F0E,0x3307,0x3303,0x3303,0x3303,0x3303,0x3387,0x31FE,0x30FC}, // 5
    {0x0FFC,0x1FFE,0x38C7,0x30C3,0x30C3,0x30C3,0x30C3,0x38E7,0x1C7E,0x0C3C}, // 6
    {0x3000,0x3000,0x3000,0x3000,0x307F,0x30FF,0x31C0,0x3380,0x3F00,0x3E00}, // 7
    {0x0F3C,0x1FFE,0x39E7,0x30C3,0x30C3,0x30C3,0x30C3,0x39E7,0x1FFE,0x0F3C}, // 8
    {0x0F0C,0x1F8E,0x39C7,0x30C3,0x30C3,0x30C3,0x30C3,0x38C7,0x1FFE,0x0FFC}, // 9
    {0x1E0F,0x3F1F,0x3338,0x3F70,0x1EE0,0x01DE,0x03BF,0x0733,0x3E3F,0x3C1E}, // %
    {0x0C00,0x1C00,0x3800,0x3000,0x3033,0x3073,0x30E0,0x39C0,0x1F80,0x0F00}, // ?
    {0x0000,0x0000,0x0000,0x0000,0x07F3,0x07F3,0x0000,0x0000,0x0000,0x0000}, // !
    {0x0000,0x0000,0x0000,0x0000,0x0003,0x0003,0x0000,0x0000,0x0000,0x0000}, // .
    {0x0000,0x00C0,0x00C0,0x00C0,0x00C0,0x00C0,0x00C0,0x00C0,0x00C0,0x0000}, // -
    {0x0000,0x3FFF,0x3FFF,0x3003,0x3003,0x3003,0x3003,0x3FFF,0x3FFF,0x0000}, // unknown
};

#endif
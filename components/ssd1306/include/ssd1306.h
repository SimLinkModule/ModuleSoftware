#ifndef SSD1306_H
#define SSD1306_H

//credits:  https://github.com/nopnop2002/esp-idf-ssd1306
//          http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html

#include "driver/i2c.h"
#include "esp_log.h"
#include <string.h>
#include <stdbool.h>

//SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define SSD1306_ADDRESS                     0x3C

//allgemeine infos
//startup des ssd1306 wird durch Kondensatoren auf dem PCB geregelt --> beachten bei pcb design
#define SSD1306_WIDTH                       128
#define SSD1306_HEIGHT                      32
#define SSD1306_PAGES                       8
#define SSD1306_TAG                         "SSD1306"
#define SSD1306_I2C_NUM_CON                 I2C_NUM_0

//setzen des controllbytes für die art der daten die folgend kommen
#define SSD1306_CONTROL_BYTE_CMD_STREAM    0x00
#define SSD1306_CONTROL_BYTE_CMD_SINGLE    0x80
#define SSD1306_CONTROL_BYTE_DATA_SINGLE   0xC0
#define SSD1306_CONTROL_BYTE_DATA_STREAM   0x40

//grundbefehle
#define SSD1306_CMD_SET_CONTRAST           0x81    // gefolgt von 0x7F
#define SSD1306_CMD_DISPLAY_RAM            0xA4
#define SSD1306_CMD_DISPLAY_ALLON          0xA5
#define SSD1306_CMD_DISPLAY_NORMAL         0xA6
#define SSD1306_CMD_DISPLAY_INVERTED       0xA7
#define SSD1306_CMD_DISPLAY_OFF            0xAE
#define SSD1306_CMD_DISPLAY_ON             0xAF

//GDDRAM (Graphic Display Data RAM) adressierung
#define SSD1306_CMD_SET_MEMORY_ADDR_MODE   0x20
#define SSD1306_CMD_SET_HORI_ADDR_MODE     0x00    // horizontaler adressierungsmodus
#define SSD1306_CMD_SET_VERT_ADDR_MODE     0x01    // vertikaler adressierungsmodus
#define SSD1306_CMD_SET_PAGE_ADDR_MODE     0x02    // page adressierungsmodus
#define SSD1306_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define SSD1306_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

//Hardwarekonfiguration
#define SSD1306_CMD_SET_DISPLAY_START_LINE 0x40
#define SSD1306_CMD_SET_SEGMENT_REMAP_0    0xA0    
#define SSD1306_CMD_SET_SEGMENT_REMAP_1    0xA1    
#define SSD1306_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define SSD1306_CMD_SET_COM_SCAN_MODE_1    0xC8
#define SSD1306_CMD_SET_COM_SCAN_MODE_0    0xC0
#define SSD1306_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define SSD1306_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define SSD1306_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme
#define SSD1306_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define SSD1306_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define SSD1306_CMD_SET_VCOMH_DESELECT     0xDB    // follow with 0x30

// Charge Pump
#define SSD1306_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

// Scrolling Befehle
#define SSD1306_CMD_DEACTIVE_SCROLL        0x2E

//buffer damit der aktuelle Output auch im Speicher des ESP vorhanden ist
uint8_t* ssd1306_buffer;

void ssd1306_init();
void I2C_master_init();
void ssd1306_clear();
void ssd1306_display();
void ssd1306_setPixel(uint8_t x, uint8_t y, bool status);
void ssd1306_setChar(char c, uint8_t x, uint8_t y);
void ssd1306_setString(const char* str, uint8_t x, uint8_t y);

#endif
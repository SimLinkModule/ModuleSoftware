#ifndef SSD1306_H
#define SSD1306_H

//credits:  https://github.com/nopnop2002/esp-idf-ssd1306
//          http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html

#include <stdint.h>
#include <stdbool.h>

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  32

void ssd1306_init();
void I2C_master_init();
void ssd1306_clear();
void ssd1306_display();
void ssd1306_setPixel(uint8_t x, uint8_t y, bool status);
void ssd1306_setChar(char c, uint8_t x, uint8_t y);
void ssd1306_setString(const char* str, uint8_t x, uint8_t y);
void ssd1306_setConnectedImage();

#endif
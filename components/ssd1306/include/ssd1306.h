/*credits:  https://github.com/nopnop2002/esp-idf-ssd1306
 *          http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html
*/

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  32

/**
 * setup the ssd1306 display
 */
void ssd1306_init();

/**
 * setup the I2C interface on the ESP
 */
void I2C_master_init();

/**
 * clear the memory of the selected pixels. Only on the ESP side.
 */
void ssd1306_clear();

/**
 * display the current stored pixel data on the ESP
 */
void ssd1306_display();

/**
 * Set a specific pixel in the pixel data. Axis origin is the left top.
 *
 * @param x                  x coord
 * @param y                  y coord
 * @param status             enable or disable the bit
 */
void ssd1306_setPixel(uint8_t x, uint8_t y, bool status);

/**
 * Set a char at a specific position in the pixel data.
 *
 * @param c                  the character to be written
 * @param x                  x coord
 * @param y                  y coord
 */
void ssd1306_setChar(char c, uint8_t x, uint8_t y);

/**
 * set a string at a specific position in the pixel data.
 *
 * @param str                The string to be written
 * @param x                  x coord
 * @param y                  y coord
 */
void ssd1306_setString(const char* str, uint8_t x, uint8_t y);

/**
 * set an image in the whole pixel data with a connected icon.
 */
void ssd1306_setConnectedImage();

#endif
#include "ssd1306.h"
#include "ssd1306_font.h"
#include "ssd1306_images.h"

#include "driver/i2c.h"
#include "esp_log.h"
#include <string.h>

//SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define SSD1306_ADDRESS                     0x3C

//general info
//startup of the ssd1306 is controlled by capacitors on the PCB
#define SSD1306_PAGES                       8
#define SSD1306_TAG                         "SSD1306"
#define SSD1306_I2C_NUM_CON                 I2C_NUM_0

//set the control byte for the type of data that comes after the control byte
#define SSD1306_CONTROL_BYTE_CMD_STREAM    0x00
#define SSD1306_CONTROL_BYTE_CMD_SINGLE    0x80
#define SSD1306_CONTROL_BYTE_DATA_SINGLE   0xC0
#define SSD1306_CONTROL_BYTE_DATA_STREAM   0x40

//basic commands
#define SSD1306_CMD_SET_CONTRAST           0x81    //followed by 0x7F
#define SSD1306_CMD_DISPLAY_RAM            0xA4
#define SSD1306_CMD_DISPLAY_ALLON          0xA5
#define SSD1306_CMD_DISPLAY_NORMAL         0xA6
#define SSD1306_CMD_DISPLAY_INVERTED       0xA7
#define SSD1306_CMD_DISPLAY_OFF            0xAE
#define SSD1306_CMD_DISPLAY_ON             0xAF

//GDDRAM (Graphic Display Data RAM) addressing
#define SSD1306_CMD_SET_MEMORY_ADDR_MODE   0x20
#define SSD1306_CMD_SET_HORI_ADDR_MODE     0x00    // horizontal addressing mode
#define SSD1306_CMD_SET_VERT_ADDR_MODE     0x01    // vertical addressing mode
#define SSD1306_CMD_SET_PAGE_ADDR_MODE     0x02    // page addressing mode
#define SSD1306_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define SSD1306_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

//hardware configuration
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

// scroll command
#define SSD1306_CMD_DEACTIVE_SCROLL        0x2E

//buffer so that the current output is also available in the memory of the ESP
static uint8_t* ssd1306_buffer;

void ssd1306_init(){

    //create buffer for i2c commands
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    //Set address of display and enter write mode --> allow ack
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    //set control byte that a command stream comes to set the oled for startup
    i2c_master_write_byte(cmd, SSD1306_CONTROL_BYTE_CMD_STREAM, true);

    //Sequence in datasheet
    //turn display off
    i2c_master_write_byte(cmd, SSD1306_CMD_DISPLAY_OFF, true);
    //set mux ratio to adjust the number of lines
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_MUX_RATIO, true);
    i2c_master_write_byte(cmd, 0x1F, true);
    //reset display offset to 0
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_DISPLAY_OFFSET, true);
	i2c_master_write_byte(cmd, 0x00, true);
    //set the RAM display start line to 0
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_DISPLAY_START_LINE, true);
    //mirror the x-axis. alternative = SSD1306_CMD_SET_SEGMENT_REMAP_1
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_SEGMENT_REMAP_0, true);  
    //mirror the y-axis. alternative = SSD1306_CMD_SET_COM_SCAN_MODE_1
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_COM_SCAN_MODE_0, true);	
    //set the COM PinMap to fit 32 lines
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_COM_PIN_MAP, true);
    i2c_master_write_byte(cmd, 0x02, true);
    //set contrast to highest level
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_CONTRAST, true);
	i2c_master_write_byte(cmd, 0xFF, true);
    //Render from RAM
    i2c_master_write_byte(cmd, SSD1306_CMD_DISPLAY_RAM, true);
    //V_COMH deselect voltage set to max (default 0x20)
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_VCOMH_DESELECT, true);
	i2c_master_write_byte(cmd, 0x30, true);
    //Set memory addressing mode
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_MEMORY_ADDR_MODE, true);
	i2c_master_write_byte(cmd, SSD1306_CMD_SET_HORI_ADDR_MODE, true);
    //Disable scroll for startup
    i2c_master_write_byte(cmd, SSD1306_CMD_DEACTIVE_SCROLL, true);
    //Set precharge cycles to high cap type --> is default
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_PRECHARGE, true);
    i2c_master_write_byte(cmd, 0x22, true);
    //set if pixel should be inverted or not --> 1 is on
    i2c_master_write_byte(cmd, SSD1306_CMD_DISPLAY_NORMAL, true);
    //Set Display Clock divide Ratio/Oscillator Frequency (default set)
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_DISPLAY_CLK_DIV, true);
	i2c_master_write_byte(cmd, 0x80, true);
    //charge pump must be activated when the display will be on
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);
    //activate display
    i2c_master_write_byte(cmd, SSD1306_CMD_DISPLAY_ON, true);

    //add stop signal to queue
    i2c_master_stop(cmd);

    //transfer all buffered commands values mutex saved at once via i2c
    //last parameter is the time to wait at the ack before it should be evaluated as timeout
    esp_err_t espRc = i2c_master_cmd_begin(SSD1306_I2C_NUM_CON, cmd, 10/portTICK_PERIOD_MS);

	if (espRc == ESP_OK) {
		ESP_LOGI(SSD1306_TAG, "OLED configured successfully");
	} else {
		ESP_LOGE(SSD1306_TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}

    //clear link buffer with the i2c commands
    i2c_cmd_link_delete(cmd);

    //create buffer
    //each page has 128 segments with 8 bits each. For a height of 32 pixels 4 pages are needed.
    //ssd1780-datasheet page 37 & 38
    ssd1306_buffer = (uint8_t *)malloc(SSD1306_WIDTH * (SSD1306_HEIGHT / 8));
    ssd1306_clear();
    ssd1306_display();
}

void I2C_master_init(){
    //setup the i2c driver
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 21,
        .scl_io_num = 22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(SSD1306_I2C_NUM_CON, &conf);
    i2c_driver_install(SSD1306_I2C_NUM_CON, I2C_MODE_MASTER, 0, 0, 0);
}

void ssd1306_clear(){
    memset(ssd1306_buffer, 0, SSD1306_WIDTH * (SSD1306_HEIGHT / 8));
}

void ssd1306_display(){
    i2c_cmd_handle_t cmd;

    //create buffer for i2c commands
    cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

    //Set address of display and enter write mode --> allow ack
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    //set control byte that a command stream comes to set the size of the display
    i2c_master_write_byte(cmd, SSD1306_CONTROL_BYTE_CMD_STREAM, true);
    //Set column width
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_COLUMN_RANGE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x7F, true);
    //set number of lines
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_PAGE_RANGE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x07, true);

    //send configuration
    i2c_master_stop(cmd);
	i2c_master_cmd_begin(SSD1306_I2C_NUM_CON, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

    //send data
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, SSD1306_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, ssd1306_buffer, SSD1306_WIDTH * (SSD1306_HEIGHT / 8), true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(SSD1306_I2C_NUM_CON, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}

void ssd1306_setPixel(uint8_t x, uint8_t y, bool status){
    if(x>=SSD1306_WIDTH){
        return;
    }
    if(y>=SSD1306_HEIGHT){
        return;
    }
    uint16_t buffer_idx = x+y/8*SSD1306_WIDTH;
    if(status){
        ssd1306_buffer[buffer_idx] |= 0x01<<y%8;
    } else {
        ssd1306_buffer[buffer_idx] &= ~(0x01<<y%8);
    }
}

void ssd1306_setChar(char c, uint8_t x, uint8_t y){
    //calc the index in the font array
    if(c >= 'a' && c <= 'z'){
        c = c-'a'+1;
    } else if (c >= '0' && c <= '9'){
        c = c-'0'+27;
    } else if (c >= 'A' && c <= 'Z'){
        c = c-'A'+1;
    } else if (c == '%') {
        c = 37;
    } else if (c == '?') {
        c = 38;
    } else if (c == '!') {
        c = 39;
    } else if (c == '.') {
        c = 40;
    } else if (c == '-') {
        c = 41;
    } else {
        c = 42;
    }

    const uint16_t* chr = VCR_OSD_MONO[(uint8_t)c];
    for (uint8_t j=0; j<CHAR_WIDTH; j++) {
        for (uint8_t i=0; i<=CHAR_HEIGHT-1; i++) {
            if (chr[j] & (1<<i)) {
                ssd1306_setPixel(x+j, y+(CHAR_HEIGHT-1)-i, true);
            }
        }
    }
}

void ssd1306_setString(const char* str, uint8_t x, uint8_t y) {
    while (*str) {
        ssd1306_setChar(*str++, x, y);
        x += CHAR_WIDTH+1;
    }
}

void ssd1306_setConnectedImage(){
    memcpy(ssd1306_buffer,connectedImage,SSD1306_WIDTH * (SSD1306_HEIGHT / 8));
}
#include "ssd1306.h"
#include "ssd1306_font.h"
#include "ssd1306_images.h"

#include "driver/i2c.h"
#include "esp_log.h"
#include <string.h>

//SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define SSD1306_ADDRESS                     0x3C

//allgemeine infos
//startup des ssd1306 wird durch Kondensatoren auf dem PCB geregelt --> beachten bei pcb design
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
static uint8_t* ssd1306_buffer;

void ssd1306_init(){

    //buffer für i2c befehle erstellen
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    //Adresse des Displays setzen und in den Schreibmodus übergehen --> ack zulassen
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    //kontroll-byte setzen, dass ein command stream kommt, um das oled für den start zu setzen
    i2c_master_write_byte(cmd, SSD1306_CONTROL_BYTE_CMD_STREAM, true);

    //Reihenfolge anscheinend im Datasheet
    //Display ausschalten
    i2c_master_write_byte(cmd, SSD1306_CMD_DISPLAY_OFF, true);
    //mux ratio setzen, um die Anzahl der zeilen anzupassen
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_MUX_RATIO, true);
    i2c_master_write_byte(cmd, 0x1F, true);
    //display offset zurücksetzen auf 0
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_DISPLAY_OFFSET, true);
	i2c_master_write_byte(cmd, 0x00, true);
    //setzen des RAM Display start zeile auf 0
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_DISPLAY_START_LINE, true);
    //spiegeln der x-achse alternative = SSD1306_CMD_SET_SEGMENT_REMAP_0
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_SEGMENT_REMAP_1, true);  
    //spiegeln der y-achse alternative = SSD1306_CMD_SET_COM_SCAN_MODE_0
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_COM_SCAN_MODE_1, true);	
    //setzen der COM-PinMap für 32 Zeilen anpassen
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_COM_PIN_MAP, true);
    i2c_master_write_byte(cmd, 0x02, true);
    //contrast auf höchste stufe setzen
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_CONTRAST, true);
	i2c_master_write_byte(cmd, 0xFF, true);
    //Rendern vom RAM
    i2c_master_write_byte(cmd, SSD1306_CMD_DISPLAY_RAM, true);
    //Set the V_COMH deselect volatage to max (default 0x20)
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_VCOMH_DESELECT, true);
	i2c_master_write_byte(cmd, 0x30, true);
    //Speicheradressierungsmodus setzen
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_MEMORY_ADDR_MODE, true);
	i2c_master_write_byte(cmd, SSD1306_CMD_SET_HORI_ADDR_MODE, true);
    //scroll für den start deaktivieren
    i2c_master_write_byte(cmd, SSD1306_CMD_DEACTIVE_SCROLL, true);
    //Set precharge cycles to high cap type --> ist default
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_PRECHARGE, true);
    i2c_master_write_byte(cmd, 0x22, true);
    //setzen ob pixel invertiert werden sollen oder nicht --> 1 ist an
    i2c_master_write_byte(cmd, SSD1306_CMD_DISPLAY_NORMAL, true);
    //Set Display Clock divide Ratio/Oscillator Frequency (default setzen)
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_DISPLAY_CLK_DIV, true);
	i2c_master_write_byte(cmd, 0x80, true);
    //charge pump muss aktiviert sein wenn das display an sein wird
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);
    //display aktivieren
    i2c_master_write_byte(cmd, SSD1306_CMD_DISPLAY_ON, true);

    //stop signal zur queue hinzufügen
    i2c_master_stop(cmd);

    //alle gebufferten befehle werten mutex gesichert auf einmal über i2c übertragen
    //letzer parameter ist die zeit die gewartet werden soll bei den ack bevor es als timeout gewertet werden soll
    esp_err_t espRc = i2c_master_cmd_begin(SSD1306_I2C_NUM_CON, cmd, 10/portTICK_PERIOD_MS);

	if (espRc == ESP_OK) {
		ESP_LOGI(SSD1306_TAG, "OLED configured successfully");
	} else {
		ESP_LOGE(SSD1306_TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}

    //link buffer löschen mit den i2c befehlen
    i2c_cmd_link_delete(cmd);

    //buffer erstellen
    // jede page hat 128 segmenente mit jeweils 8 Bit. Für höhe von 32 Pixel werden 4 Pages benötigt
    // ssd1780-datasheet seite 37&38
    ssd1306_buffer = (uint8_t *)malloc(SSD1306_WIDTH * (SSD1306_HEIGHT / 8));
    ssd1306_clear();
    ssd1306_display();
}

void I2C_master_init(){
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

    //buffer für i2c befehle erstellen
    cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

    //Adresse des Displays setzen und in den Schreibmodus übergehen --> ack zulassen
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    //kontroll-byte setzen, dass ein command stream kommt, um die größe des displays festzulegen
    i2c_master_write_byte(cmd, SSD1306_CONTROL_BYTE_CMD_STREAM, true);
    //spaltenbreite festlegen
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_COLUMN_RANGE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x7F, true);
    //zeilenanzahl festlegen
    i2c_master_write_byte(cmd, SSD1306_CMD_SET_PAGE_RANGE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x07, true);

    //Konfiguration übertragen
    i2c_master_stop(cmd);
	i2c_master_cmd_begin(SSD1306_I2C_NUM_CON, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

    //Datenübertragen
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
#include "crsf.h"

#include "hal/uart_hal.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdbool.h>
#include "host/ble_hs.h"

#include "gap.h"
#include "gatt.h"
#include "host/ble_gatt.h"

static uint8_t crcSingleChar(uint8_t crc, uint8_t a);
static uint8_t crcMessage(uint8_t message[], uint8_t length);
static uint16_t scale_Range_Analog(uint16_t value);
static uint8_t scale_Range_Digital(uint16_t value);

ChannelDataStruct channelData = {0};

/**
 * Calc crc for a single char.
 * 
 * @param crc       current crc value 
 * @param a         char to check
 * @return          new crc value
 */
static uint8_t crcSingleChar(uint8_t crc, uint8_t a)
{
    //move char bitstring eight times to the left and every time there is a 1 at the msb then xor with the generatorpolynom 0xD5 --> normal crc calculation
    crc ^= a;
    for (int i = 0; i < 8; i++) {
        crc = (crc << 1) ^ ((crc & 0x80) ? 0xD5 : 0);
    }
    return crc;
}

/**
 * CRC calculation for a whole string
 * 
 * @param message       get crc for this string
 * @param length        length of the string
 * @return              return the crc value
 */
static uint8_t crcMessage(uint8_t message[], uint8_t length)
{
    uint8_t crc = 0;
    for (int i = 0; i < length; i++) {
        crc = crcSingleChar(crc, message[i]);
    }
    return crc;
}

/**
 * scale range from [173,1811] to [0,2047]
 * 
 * @param value         value to scale
 * @return              new scaled value 
 */
static uint16_t scale_Range_Analog(uint16_t value){
    return (uint16_t)((value-173)*1.25);
}

/**
 * @brief scale range from [173,1811] to [0,1]
 * 
 * @param value         value to scale
 * @return              new scaled value
 */
static uint8_t scale_Range_Digital(uint16_t value){
    if(value <= 992){
        return 0;
    } else {
        return 1;
    }
}

void initCRSF_read(){
    /* configure the uart driver
     * the uart driver creates his own interrupt
     */
    uart_config_t uart_config = {
        .baud_rate = 420000,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    //use uart 2, because uart 0 is used for the console and uart1 could be used for spi
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 1024 * 2, 0, 0, NULL, 0));
    uart_set_line_inverse(UART_NUM_2,UART_SIGNAL_RXD_INV);
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    //trigger interrupt after the timeout (primary) or if a overflow happens
    uart_intr_config_t uart_intr = {
        .intr_enable_mask = UART_INTR_RXFIFO_TOUT | UART_INTR_RXFIFO_FULL,
        .rx_timeout_thresh = 10,
        .rxfifo_full_thresh = 200,
    };

    //store the interrupt thresholds
    ESP_ERROR_CHECK(uart_intr_config(UART_NUM_2, &uart_intr));
    ESP_ERROR_CHECK(uart_enable_rx_intr(UART_NUM_2));
}

void crsf_get_ChannelData_task(void *arg)
{
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(1024);

    while (1) {
        //bool if channel data changed from channeldata t-1. Used to send a BLE update
        bool changed = false;

        // get size in uart buffer
        int length = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_2, (size_t*)&length));

        //work with the data, if data exists
        if(length){

            //read uart data
            int len = uart_read_bytes(UART_NUM_2, data, length, 20 / portTICK_PERIOD_MS);

            //clear rx buffer after the data is in the temp buffer
            uart_flush(UART_NUM_2);

            //check again the length of the data in the temp buffer
            if(len > 0){
                //loop through data
                for(int i = 0; i < len; i++){
                    //check for packets with the data: 0xEE(address) + 0x18(length in Bytes) + 0x16(typefield: channel data)
                    if((data[i] == 0xEE) && (i+25)<len){
                        if(data[i+2] == 0x16){
                            //CRC check. should be zero
                            if(crcMessage(data+i+2, data[i+1]) == 0){

                                //used = How many bits were used in a temp buffer byte.
                                int used = 0;
                                //dataIndex = Index of the data byte in the buffer
                                int dataIndex = i+3;

                                //loop through all 16 channels
                                for(int j = 0; j<16; j++){
                                    //Set the remaining existing data of a temp buffer byte to LSB.
                                    uint16_t value = data[dataIndex] >> used;

                                    //use a new byte from the temp buffer.
                                    dataIndex++;

                                    //Number of remaining channel bits of a byte in the started temp buffer byte.
                                    uint8_t availableBits = 8-used;

                                    if(11-availableBits > 8){
                                        //3bytes are required if more than 8 bits are still needed for one channel (11bit)

                                        //use entire second byte
                                        value = value | (data[dataIndex] << availableBits);
                                        //use a new Byte
                                        dataIndex++;

                                        //from the third byte still shift the needed bits into the MSB
                                        value = value | ((data[dataIndex]&((1U << (11-(8+availableBits))) - 1U)) << (8+availableBits));

                                        //Recalculate number of used bits in current temp buffer byte
                                        used = 11-(8+availableBits);
                                    } else {
                                        //If only a second byte is needed for 11Bit data

                                        //Move the missing bits from the second byte to the MSB positions. (11Bit per channel)
                                        value = value | ((data[dataIndex]&((1U << (11-availableBits)) - 1U)) << availableBits);

                                        //calculate how many bits in the current temp buffer byte have already been used
                                        used = 11-availableBits;
                                    }

                                    //store channel data --> first 8 channel are analog rest ist digital
                                    uint16_t newVal;
                                    uint8_t origDataButtons;
                                    switch(j){
                                        case 0:
                                            newVal = scale_Range_Analog(value);
                                            if(channelData.throttle != newVal){
                                                changed = true;
                                                channelData.throttle = newVal;
                                            }
                                            break;
                                        case 1:
                                            newVal = scale_Range_Analog(value);
                                            if(channelData.yaw != newVal){
                                                changed = true;
                                                channelData.yaw = newVal;
                                            }
                                            break;
                                        case 2:
                                            newVal = scale_Range_Analog(value);
                                            if(channelData.pitch != newVal){
                                                changed = true;
                                                channelData.pitch = newVal;
                                            }
                                            break;
                                        case 3:
                                            newVal = scale_Range_Analog(value);
                                            if(channelData.roll != newVal){
                                                changed = true;
                                                channelData.roll = newVal;
                                            }
                                            break;
                                        case 4:
                                            newVal = scale_Range_Analog(value);
                                            if(channelData.aux1 != newVal){
                                                changed = true;
                                                channelData.aux1 = newVal;
                                            }
                                            break;
                                        case 5:
                                            newVal = scale_Range_Analog(value);
                                            if(channelData.aux2 != newVal){
                                                changed = true;
                                                channelData.aux2 = newVal;
                                            } 
                                            break;
                                        case 6:
                                            newVal = scale_Range_Analog(value);
                                            if(channelData.aux3 != newVal){
                                                changed = true;
                                                channelData.aux3 = newVal;
                                            }
                                            break;
                                        case 7:
                                            newVal = scale_Range_Analog(value);
                                            if(channelData.aux4 != newVal){
                                                changed = true;
                                                channelData.aux4 = newVal;
                                            }
                                            break;
                                        case 8:
                                        case 9:
                                        case 10:
                                        case 11:
                                        case 12:
                                        case 13:
                                        case 14:
                                        case 15:
                                            origDataButtons = channelData.buttons;
                                            if(scale_Range_Digital(value) == 1){
                                                channelData.buttons = channelData.buttons | (0x01 << (j-8));
                                            } else {
                                                channelData.buttons = channelData.buttons & (~(0x01 << (j-8)));
                                            }

                                            if(origDataButtons != channelData.buttons){
                                                changed = true;
                                            }

                                            break;
                                    }
                                }

                                if(changed){
                                    if(notify_state_report_data){
                                        struct os_mbuf *om;

                                        //set size to 17, because of padding bits in the struct channelData calculates 18
                                        om = ble_hs_mbuf_from_flat(&channelData, 17);
                                        //Deprecated. Should not be used. Use ble_gatts_notify_custom instead.
                                        ble_gattc_notify_custom(conn_handle, report_data_handle, om);
                                    }
                                }
                                //if esp_logi is used then the watchdog timer for the task may not be reset
                                //ESP_LOGI("Channel-Data","%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d", channelData.throttle, channelData.yaw, channelData.pitch, channelData.roll, channelData.aux1, channelData.aux2, channelData.aux3, channelData.aux4, (channelData.buttons & (0x01<<0)), (channelData.buttons & (0x01<<1)), (channelData.buttons & (0x01<<2)), (channelData.buttons & (0x01<<3)), (channelData.buttons & (0x01<<4)), (channelData.buttons & (0x01<<5)), (channelData.buttons & (0x01<<6)), (channelData.buttons & (0x01<<7)));
                                break;
                            }
                        }
                    }
                }
            }
        } else {
            //Create a delay if no data could be loaded
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}
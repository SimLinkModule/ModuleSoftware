#include "crsf.h"

ChannelDataStruct channelData = {0};

uint8_t crcSingleChar(uint8_t crc, uint8_t a)
{
    crc ^= a;
    //zeichen wird acht mal nach links geschoben und jedes mal geschaut ob höchste stelle eine 1 ist, wenn dann polynom xor --> klassisches crc rechnen
    for (int i = 0; i < 8; i++) {
        crc = (crc << 1) ^ ((crc & 0x80) ? 0xD5 : 0);
    }
    return crc;
}

//get crc from message
uint8_t crcMessage(uint8_t message[], uint8_t length)
{
    uint8_t crc = 0;
    for (int i = 0; i < length; i++) {
        crc = crcSingleChar(crc, message[i]);
    }
    return crc;
}

//scale from range [173,1811] to range [0,2047]
uint16_t scale_Range_Analog(uint16_t value){
    return (uint16_t)((value-173)*1.25);
}

//scale from range [173,1811] to range [0,1]
uint8_t scale_Range_Digital(uint16_t value){
    if(value <= 992){
        return 0;
    } else {
        return 1;
    }
}

void initCRSF_read(){
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    //uart driver erstellt eigene interrupts
    uart_config_t uart_config = {
        .baud_rate = 420000,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    //UART 2 verwenden, da uart 0 für console benötigt wird und uart1 hab ich gelesen für spi oder irgendetwas anderes
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 1024 * 2, 0, 0, NULL, 0));
    uart_set_line_inverse(UART_NUM_2,UART_SIGNAL_RXD_INV);
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    //interrupt auslösen bei timeout (primär) und bei überlauf eines thresholds
    uart_intr_config_t uart_intr = {
        .intr_enable_mask = UART_INTR_RXFIFO_TOUT | UART_INTR_RXFIFO_FULL,
        .rx_timeout_thresh = 10,
        .rxfifo_full_thresh = 200,
    };
    //diese interrupt schwellwellen speichern
    ESP_ERROR_CHECK(uart_intr_config(UART_NUM_2, &uart_intr));
    ESP_ERROR_CHECK(uart_enable_rx_intr(UART_NUM_2));
}

void crsf_get_ChannelData_task(void *arg)
{
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(1024);

    while (1) {
        //bool if channel data changed from channeldata t-1
        bool changed = false;

        // get size in uart buffer
        int length = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_2, (size_t*)&length));

        //mit daten arbeiten, wenn welche vorhanden sind
        if(length){

            //read uart data
            int len = uart_read_bytes(UART_NUM_2, data, length, 20 / portTICK_RATE_MS);

            //RX Buffer leeren wenn Frame im temporären buffer
            uart_flush(UART_NUM_2);

            //len of data read from rx buffer
            if(len > 0){
                //daten durchgehen
                for(int i = 0; i < len; i++){
                    //Paketaufbau: 0xEE (Addresse) + 0x18 (Länge in Byte) + 0x16 (Typenfeld: Kanaldaten)
                    if((data[i] == 0xEE) && (i+25)<len){
                        if(data[i+2] == 0x16){
                            //CRC überprüfen ob kein rest rauskommst
                            if(crcMessage(data+i+2, data[i+1]) == 0){

                                //used = wie viel Bits bereits in einen Byte verwendet wurden
                                int used = 0;
                                //dataIndex = Index des Datenbytes im Buffer
                                int dataIndex = i+3;

                                //Da 16 kanäle vorhanden sind muss es 16 mal durchgegangen werden
                                for(int j = 0; j<16; j++){
                                    //Die restlichen vorhandenen Daten eines Bytes an LSB setzen
                                    uint16_t value = data[dataIndex] >> used;

                                    //ein neues Byte muss verwendet werden
                                    dataIndex++;

                                    //Anzahl der noch vorhanden Bits eines Bytes im angefangenen Byte
                                    uint8_t availableBits = 8-used;

                                    if(11-availableBits > 8){
                                        //3Bytes werden benötigt, wenn für ein Kanal (11Bit) noch mehr als 8 Bit benötigt werden

                                        //komplettes zweites Byte verwenden
                                        value = value | (data[dataIndex] << availableBits);
                                        //ein neues Byte muss verwendet werden
                                        dataIndex++;

                                        //vom dritten Byte noch die benötigten Bits in die MSB stellen schieben
                                        value = value | ((data[dataIndex]&((1U << (11-(8+availableBits))) - 1U)) << (8+availableBits));

                                        //Anzahl der verwendeten Bits im akutellen Byte neu berechnen
                                        used = 11-(8+availableBits);
                                    } else {
                                        //Wenn nur ein zweites Byte für 11Bit-Daten benötigt wird

                                        //Aus dem zweiten Byte noch die fehlenden Bits an die MSB stellen schieben von 11 Bit
                                        value = value | ((data[dataIndex]&((1U << (11-availableBits)) - 1U)) << availableBits);

                                        //berechnen wie viele Bits im aktuellen Byte schon verwendet wurden
                                        used = 11-availableBits;
                                    }

                                    //Kanaldaten abspeichern --> ersten 8 kanäle sind analog rest digital
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
                                //Kanaldaten ausgeben
                                //wenn esp_logi ausgegeben wird dann kann es sein das watchdog timer für den task nicht zurückgesetzt wird ist aber nicht so schlimm solang der output einfach weggelassen wird in stpäteren code
                                if(changed){
                                    int rc;
                                    struct os_mbuf *om;

                                    if(notify_state){
                                        om = ble_hs_mbuf_from_flat(&channelData, sizeof(channelData));
                                        rc = ble_gattc_notify_custom(conn_handle, report_data_handle, om);

                                        //assert(rc == 0);
                                    }
                                }
                                //ESP_LOGI("Channel-Data","%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d", channelData.throttle, channelData.yaw, channelData.pitch, channelData.roll, channelData.aux1, channelData.aux2, channelData.aux3, channelData.aux4, (channelData.buttons & (0x01<<0)), (channelData.buttons & (0x01<<1)), (channelData.buttons & (0x01<<2)), (channelData.buttons & (0x01<<3)), (channelData.buttons & (0x01<<4)), (channelData.buttons & (0x01<<5)), (channelData.buttons & (0x01<<6)), (channelData.buttons & (0x01<<7)));
                                break;
                            }
                        }
                    }
                }
            }
        } else {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}
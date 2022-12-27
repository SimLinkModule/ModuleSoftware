#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

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

static void echo_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
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
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 4, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(1024);

    //store channel data in array
    uint16_t channelData[16] = {0};

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(UART_NUM_2, data, 1024, 20 / portTICK_RATE_MS);

        //mit daten arbeiten, wenn welche vorhanden sind
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

                                //Kanaldaten abspeichern
                                channelData[j] = value;
                            }
                            //Kanaldaten ausgeben
                            ESP_LOGI("Channel-Data","%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d", channelData[0], channelData[1], channelData[2], channelData[3], channelData[4], channelData[5], channelData[6], channelData[7], channelData[8], channelData[9], channelData[10], channelData[11], channelData[12], channelData[13], channelData[14], channelData[15]);
                            break;
                        }
                    }
                }
            }
            //RX Buffer leeren wenn Frame gelesen wurde
            uart_flush(UART_NUM_2);
        }
    }
}

void app_main(void)
{
    xTaskCreate(echo_task, "uart_echo_task", 2048, NULL, 10, NULL);
}

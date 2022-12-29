#ifndef CRSF_H
#define CRSF_H

#include "hal/uart_hal.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "esp_log.h"

//store channel data in array
static uint16_t channelData[16] = {0};

uint8_t crcSingleChar(uint8_t crc, uint8_t a);
uint8_t crcMessage(uint8_t message[], uint8_t length);
void initCRSF_read();
void crsf_get_ChannelData_task(void *arg);

#endif
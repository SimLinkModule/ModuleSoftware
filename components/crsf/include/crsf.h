#ifndef CRSF_H
#define CRSF_H

#include "hal/uart_hal.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdbool.h>

#include "gap.h"

//struct for channel data
typedef struct ChannelDataStruct{
  uint16_t roll;        //roll = x
  uint16_t pitch;       //pitch = y
  uint16_t aux3;        //aux3 = z
  uint16_t yaw;         //yaw = rx
  uint16_t aux1;        //aux1 = rz
  uint16_t throttle;    //throttle = ry
  uint16_t aux4;        //aux4 = slide
  uint16_t aux2;        //aux2 = slide
  uint8_t buttons;      //buttons = aux12(b8) .. aux5(b1)
} ChannelDataStruct;

//store for channeldata
extern ChannelDataStruct channelData;

uint8_t crcSingleChar(uint8_t crc, uint8_t a);
uint8_t crcMessage(uint8_t message[], uint8_t length);
void initCRSF_read();
void crsf_get_ChannelData_task(void *arg);
uint16_t scale_Range_Analog(uint16_t value);
uint8_t scale_Range_Digital(uint16_t value);

#endif
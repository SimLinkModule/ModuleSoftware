#ifndef CRSF_H
#define CRSF_H

#include <stdint.h>

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

//global store for channeldata
extern ChannelDataStruct channelData;

/**
 * initializes UART to read remote control channel data.
 */
void initCRSF_read();

/**
 * Read the UART data. Check if it is the right frame and make a CRC check. Then process the channel data and convert it to a range from 0 to 2047.
 * 
 * @param arg Required for FreeRTOS. Not used.
 */
void crsf_get_ChannelData_task(void *arg);

#endif
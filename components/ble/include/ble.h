#ifndef BLE_H
#define BLE_H

#include <stdint.h>

//global store for the BLE address type
extern uint8_t bleAddressType;

/**
 * setup nimble for the BLE communication
 */
void initBLE();

/**
 * the main freertos task for nimble
 * 
 * @param param     default param for the task. not used.
 */
void bleHostTask(void *param);

/**
 * output a BLE address via ESP_LOGI
 * 
 * @param addr      pointer to the start of the BLE address
 */
void print_addr(const void *addr);

#endif
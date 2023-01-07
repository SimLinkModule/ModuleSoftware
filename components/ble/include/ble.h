#ifndef BLE_H
#define BLE_H

#include <stdint.h>

extern uint8_t bleAddressType;

void initBLE();
void bleHostTask(void *param);
void print_addr(const void *addr);

#endif
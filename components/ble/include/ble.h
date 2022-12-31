#ifndef BLE_H
#define BLE_H

#include "esp_log.h"
#include "nvs_flash.h"

#include "gap.h"
#include "gatt.h"

#include "nimble/nimble_port.h"
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "host/ble_hs_pvcy.h"
#include "host/util/util.h"
#include "nimble/nimble_port_freertos.h"

extern uint8_t bleAddressType = BLE_ADDR_RANDOM;
static const char *tag_BLE = "SimLinkModule_BLE";

void initBLE();
void bleOnSync(void);
void bleOnReset(int reason);
void bleHostTask(void *param);

void print_addr(const void *addr);

#endif
#ifndef GAP_H
#define GAP_H

#include <string.h>

#include "host/ble_hs.h"
#include "esp_peripheral.h"

#include "esp_log.h"

#include "gatt.h"
#include "ble.h"
#include "ssd1306.h"

static const char *tag_GAP = "SimLinkModule_GAP";
static const ble_uuid16_t hid_service_uuid = BLE_UUID16_INIT(0x1812);

//eindeutiges handle was bei einem verbindungsaufbau einer verbindung zugeordnet wird
extern uint16_t conn_handle;
extern bool notify_state_report_data;
extern bool notify_state_battery_status;

void bleAdvertise(void);
int bleGAPEevent(struct ble_gap_event *event, void *arg);

#endif
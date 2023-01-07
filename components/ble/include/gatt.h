#ifndef GATT_H
#define GATT_H

#include "host/ble_hs.h"

//The value handle of the relevant characteristic for notification
extern uint16_t report_data_handle;     //report daten
extern uint16_t battery_status_handle;  //battery status

void gattSvrRegisterCb(struct ble_gatt_register_ctxt *ctxt, void *arg);
int gattSvrInit(void);

#endif
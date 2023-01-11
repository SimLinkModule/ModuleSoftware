#ifndef GATT_H
#define GATT_H

#include "host/ble_hs.h"

//The data/value/status handle of the relevant characteristic for notification
extern uint16_t report_data_handle;     //global report data handle
extern uint16_t battery_status_handle;  //global battery status handle

/**
 * optional callback. Called when a GATT resource (characteristic, descriptor, sevice) is added.
 * 
 * @param ctxt      struct with info about the GATT resource
 * @param arg       predefined argument. not used
 */
void gattSvrRegisterCb(struct ble_gatt_register_ctxt *ctxt, void *arg);

/**
 * init GATT server (includes GATT and gap service)
 * 
 * @return          Whether init has worked
 */
int gattSvrInit(void);

#endif
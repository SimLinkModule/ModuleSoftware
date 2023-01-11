#ifndef GAP_H
#define GAP_H

#include <stdbool.h>
#include <stdint.h>

//global unique handle for a BLE connection
extern uint16_t conn_handle;

//Global indicators whether data should be sent to subscribed clients.
extern bool notify_state_report_data;
extern bool notify_state_battery_status;

/**
 * start the BLE Advertising
 */
void bleAdvertise(void);

#endif
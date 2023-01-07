#ifndef GAP_H
#define GAP_H

#include <stdbool.h>
#include <stdint.h>

//eindeutiges handle was bei einem verbindungsaufbau einer verbindung zugeordnet wird
extern uint16_t conn_handle;
extern bool notify_state_report_data;
extern bool notify_state_battery_status;

void bleAdvertise(void);

#endif
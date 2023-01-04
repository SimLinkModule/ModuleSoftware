#include "battery.h"

static const char *tag_BAT = "SimLinkModule_BAT";

uint8_t batteryPercentage = 0;

void initBatteryRead(){

}

void battery_Timer_Event(TimerHandle_t ev){
    //https://esp32.com/viewtopic.php?t=1459
    
    struct os_mbuf *om;
    int rc;

    batteryPercentage = (batteryPercentage < 100) ? batteryPercentage+10 : 0;

    if(notify_state_battery_status){
        om = ble_hs_mbuf_from_flat(&batteryPercentage, sizeof(batteryPercentage));
        //Deprecated. Should not be used. Use ble_gatts_notify_custom instead.
        rc = ble_gattc_notify_custom(conn_handle, battery_status_handle, om);
    }
}
#include "main.hpp"

int gatt_svr_init(void){
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    //TODO
    /*Adjusts a host configuration object's settings to accommodate the specified
     service definition array.  This function adds the counts to the appropriate
     fields in the supplied configuration object without clearing them first, so
     it can be called repeatedly with different inputs to calculate totals.  Be
     sure to zero the GATT server settings prior to the first call to this
     function.*/
    /*rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }*/

    /*
    Queues a set of service definitions for registration.  All services queued
    in this manner get registered when ble_gatts_start() is called.
    */
    /*rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }*/

    ble_gatts_start();

    return 0;
}
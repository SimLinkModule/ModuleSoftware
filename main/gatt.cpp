#include "main.hpp"

int gatt_svr_init(void){
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    //TODO
    rc = ble_gatts_count_cfg(gatt_svr_srvcs);

}
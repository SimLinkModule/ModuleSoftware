#include "main.hpp"

void bleprph_host_task(void *param){
    ESP_LOGI(tag, "BLE Host Task Started");
    /*This function will return only when nimble_port_stop() is executed*/
    nimble_port_run();

    nimble_port_freertos_deinit();
}

static void bleprhp_on_reset(int reason){
    ESP_LOGE(tag, "Resetting state; reason=%d\n",reason);
}

static void bleprhp_on_sync(void){
    //save return code
    int rc;

    /*use privacy*/
    rc = ble_hs_id_infer_auto(0, &bleprhp_addr_type);
    assert(rc == 0);

    //TODO
    //bleprhp_advertise();
}
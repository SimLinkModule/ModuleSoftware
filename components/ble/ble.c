#include "ble.h"

void initBLE(){
    //store for returncodes
    //mynewt.apache.org/latest/network/ble_hs/ble_hs_return_codes.html
    int rc;

    /* Initialize NVS — it is used to store PHY calibration data */
    //NVS = Non-volatile storage library is designed to store key-balue pairs in flash
    //Ablauf ist in esp idf beschrieben
    //docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/nimble/index.html
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());

    nimble_port_init();

    /* Initialize the NimBLE host configuration */
    ble_hs_cfg.sync_cb = bleOnSync;
    ble_hs_cfg.reset_cb = bleOnReset;

    /**
     * Round-robin status callback.  If a there is insufficient storage capacity
     * for a new record, delete the oldest bond and proceed with the persist
     * operation.
     *
     * Note: This is not the best behavior for an actual product because
     * uninteresting peers could cause important bonds to be deleted.  This is
     * useful for demonstrations and sample apps.
     */
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    //optional wird ausgeführt wenn ein gatt resource (characteristic, descriptor, sevice) hinzugefügt wird. Also nicht benötigt
    ble_hs_cfg.gatts_register_cb = gattSvrRegisterCb;

    /*Security Manager local input output capabilities*/
    //io types zum aufbau einer sicheren verbindung
    //BLE_SM_IO_CAP_DISP_ONLY = Display only
    //BLE_SM_IO_CAP_DISP_YES_NO = Display & yes & no buttons
    //BLE_SM_IO_CAP_KEYBOARD_ONLY = Keyboard only
    //BLE_SM_IO_CAP_NO_IO = just work
    //BLE_SM_IO_CAP_KEYBOARD_DISP = Keyboard and display
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;
    /*Security Manager secure connections flag
    if set proper flag in pairing request/response will be set. this results in using LE Secure Connections for pairing if also supported by remote device. Fallback to legacy pairing if not supported by remote.*/
    ble_hs_cfg.sm_sc = 1;
    /*security Manager bond flag
    if set proper flag in Pairing request/response will be set. This results in storing keys distributed during bonding.*/
    ble_hs_cfg.sm_bonding = 1;
    /*security manager mitm flag
    if set proper flag in pairing request/response will be set. This results in requiring man-in-the-middle protection when pairing.*/
    ble_hs_cfg.sm_mitm = 1;
    /*Security Manager Local Key Distribution Mask*/
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    /* Refer components/nimble/nimble/nimble/host/include/host/ble_sm.h for
     * more information */
    /*Security Manager Remote Key Distribution Mask*/
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

    rc = gattSvrInit();
    assert(rc == 0);

    /* Set the default device name */
    rc = ble_svc_gap_device_name_set(CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME);
    assert(rc == 0);
}

//This callback is executed when the host and controller become synced. This happens at startup and after a reset
void bleOnSync(void){
    int rc;

    /* Generate a non-resolvable private address. */
    //ble_app_set_addr();

    ble_hs_pvcy_rpa_config(1);

    /* Make sure we have proper identity address set (public preferred) */
    rc = ble_hs_util_ensure_addr(0);


    /*use privacy*/
    rc = ble_hs_id_infer_auto(bleAddressType, &bleAddressType);
    assert(rc == 0);

    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(bleAddressType, addr_val, NULL);

    ESP_LOGI(tag_BLE, "Device Address: ");
    print_addr(addr_val);
    ESP_LOGI(tag_BLE, "\n");

    taskYIELD();

    /*start advertising, when controller and host are in sync*/
    bleAdvertise();
}

//This callback is executed when the host resets itself and the controller
void bleOnReset(int reason){
    ESP_LOGI(tag_BLE, "Resetting state; reason=%d\n", reason);
}

//start nimble in a task
void bleHostTask(void *param)
{
    ESP_LOGI(tag_BLE, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

void print_addr(const void *addr){
    const uint8_t *u8p;

    u8p = addr;
    ESP_LOGI(tag_BLE, "%02x:%02x:%02x:%02x:%02x:%02x",
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}
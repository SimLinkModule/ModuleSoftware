#include "main.hpp"

extern "C" void app_main(void)
{
    //returncodes von nimble functionen zwischenspeichern --> geben viele informationen über den status der funktionen
    //mynewt.apache.org/latest/network/ble_hs/ble_hs_return_codes.html
    int rc;

    /*Initialize NVS - it is used to store PHY calibration data*/
    //Ablauf ist in esp idf beschrieben
    //docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/nimble/index.html
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    nimble_port_init();
    /*Initialize the NimBLE host configuration.*/
    ble_hs_cfg.sync_cb = bleprhp_on_sync;
    ble_hs_cfg.reset_cb = bleprhp_on_reset;
    /*Security Manager local input output capabilities*/
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_DISP_YES_NO;
    /*security Manager bond flag
    if set proper flag in Pairing request/response will be set. This results in storing keys distributed during bonding.*/
    ble_hs_cfg.sm_bonding = 1;
    /*security manager mitm flag
    if set proper flag in pairing request/response will be set. This results in requiring man-in-the-middle protection when pairing.*/
    ble_hs_cfg.sm_mitm = 1;
    /*Security Manager secure connections flag
    if set proper flag in pairing request/response will be set. this results in using LE Secure Connections for pairing if also supported by remote device. Fallback to legacy pairing if not supported by remote.*/
    ble_hs_cfg.sm_sc = 1;
    /*Security Manager Local Key Distribution Mask*/
    ble_hs_cfg.sm_our_key_dist = 1;
    /*Security Manager Remote Key Distribution Mask*/
    ble_hs_cfg.sm_their_key_dist = 1;

    //TODO
    //rc = gatt_svr_init();
    assert(rc == 0);

    /*Set the default device name. */
    rc = ble_svc_gap_device_name_set(CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME);
    assert(rc == 0);

    nimble_port_freertos_init(bleprph_host_task);
}
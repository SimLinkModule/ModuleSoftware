#include "ble.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "gap.h"
#include "gatt.h"

#include "nimble/nimble_port.h"
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "host/ble_hs_pvcy.h"
#include "host/util/util.h"
#include "nimble/nimble_port_freertos.h"

/**
 * This callback is executed when the host and controller become synced. This happens at startup and after a reset
 */
static void bleOnSync(void);

/**
 * This callback is executed when the host resets itself and the controller
 * 
 * @param reason    not used
 */
static void bleOnReset(int reason);

/**
 * Define template prototype for store. located in nimble/host/store/config/src/ble_store_config.c. Not defined in any .h nimble file.
 */
void ble_store_config_init(void);

static const char *tag_BLE = "SimLinkModule_BLE";

uint8_t bleAddressType = BLE_ADDR_RANDOM;

void initBLE(){
    //list of returncodes: mynewt.apache.org/latest/network/ble_hs/ble_hs_return_codes.html
    int rc;

    /*
     * Initialize NVS — it is used to store PHY calibration data
     * NVS = Non-volatile storage library is designed to store key-value pairs in flash
     * docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/nimble/index.html
     */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /*
     *Required in older ESP-IDF version.
     *Controller initialization, enable and HCI initialization calls have been moved to nimble_port_init. This function can be deleted directly.
     *ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
     */

    nimble_port_init();

    //Initialize the NimBLE host configuration
    ble_hs_cfg.sync_cb = bleOnSync;
    ble_hs_cfg.reset_cb = bleOnReset;

    /*
     * Round-robin status callback.  If there is insufficient storage capacity
     * for a new record, delete the oldest bond and proceed with the persist
     * operation.
     *
     * Note: This is not the best behavior for an actual product because
     * uninteresting peers could cause important bonds to be deleted.  This is
     * useful for demonstrations and sample apps.
     */
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    //optional callback. Called when a GATT resource (characteristic, descriptor, sevice) is added.
    ble_hs_cfg.gatts_register_cb = gattSvrRegisterCb;

    /*Security Manager local input output capabilities
     *io types to establish a secure connection
     *BLE_SM_IO_CAP_DISP_ONLY = Display only
     *BLE_SM_IO_CAP_DISP_YES_NO = Display & yes & no buttons
     *BLE_SM_IO_CAP_KEYBOARD_ONLY = Keyboard only
     *BLE_SM_IO_CAP_NO_IO = just work
     *BLE_SM_IO_CAP_KEYBOARD_DISP = Keyboard and display
     *BLE_SM_IO_CAP_KEYBOARD_ONLY && BLE_SM_IO_CAP_KEYBOARD_DISP not implemented
    */
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_DISP_YES_NO;

    /*Security Manager secure connections flag
     *if set proper flag in pairing request/response will be set. this results in using LE Secure Connections for pairing if also supported by remote device. Fallback to legacy pairing if not supported by remote.
     */
    ble_hs_cfg.sm_sc = 1;

    /*Security Manager bond flag
     *if set proper flag in Pairing request/response will be set. This results in storing keys distributed during bonding.
     */
    ble_hs_cfg.sm_bonding = 1;

    /*Security manager MITM flag
     *if set proper flag in pairing request/response will be set. This results in requiring man-in-the-middle protection when pairing.
     */
    ble_hs_cfg.sm_mitm = 1;

    /*Security Manager Local Key Distribution Mask
     *Refer components/nimble/nimble/nimble/host/include/host/ble_sm.h for
     * more information
     */
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    //Security Manager Remote Key Distribution Mask
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

    rc = gattSvrInit();
    assert(rc == 0);

    /* Set the default device name */
    rc = ble_svc_gap_device_name_set(CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME);
    assert(rc == 0);

    /*set the appearance of the device
     *0x03c3 = joystick; 0x03c4 = gamepad
     */
    ble_svc_gap_device_appearance_set(0x03C4);

    /*store the identity resolving key (IRK) to establish a connection after restarting the esp.
     *https://github.com/espressif/esp-nimble/issues/33
     */
    ble_store_config_init();
}

static void bleOnSync(void){
    //rpa = resolvable private address; Address randomly generated from an identity address and an identity resolving key (IRK).
    ble_hs_pvcy_rpa_config(1);

    /* Make sure we have proper identity address set (public preferred)
     * Works without that
     * rc = ble_hs_util_ensure_addr(0);
     */


    /* set the BLE address type
     * Works without that
     * rc = ble_hs_id_infer_auto(bleAddressType, &bleAddressType);
     */

    uint8_t addr_val[6] = {0};
    if(BLE_HS_ENOADDR == ble_hs_id_copy_addr(BLE_ADDR_PUBLIC, addr_val, NULL)) {
        ble_hs_id_copy_addr(BLE_ADDR_RANDOM, addr_val, NULL);
        ESP_LOGI(tag_BLE, "Device Address-Type: random");
    } else {
        ESP_LOGI(tag_BLE, "Device Address-Type: public");
    }

    ESP_LOGI(tag_BLE, "Device Address: ");
    print_addr(addr_val);
    ESP_LOGI(tag_BLE, "\n");

    //used to request a context switch to another task
    taskYIELD();

    //start advertising, when controller and host are in sync
    bleAdvertise();
}

static void bleOnReset(int reason){
    ESP_LOGI(tag_BLE, "Resetting state; reason=%d\n", reason);
}

void bleHostTask(void *param)
{
    ESP_LOGI(tag_BLE, "BLE Host Task Started");
    
    //This function will return only when nimble_port_stop() is executed
    nimble_port_run();

    nimble_port_freertos_deinit();
}

void print_addr(const void *addr){
    const uint8_t *u8p;

    u8p = addr;
    ESP_LOGI(tag_BLE, "%02x:%02x:%02x:%02x:%02x:%02x",
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}
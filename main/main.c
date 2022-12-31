#include <stdio.h>

#include "crsf.h"
#include "ssd1306.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOSConfig.h"
/* BLE */
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "blehr_sens.h"
#include "host/ble_uuid.h"
#include "esp_peripheral.h"
#include "host/ble_hs_pvcy.h"

static const char *tag = "SimLinkModule_BLE";
static const ble_uuid16_t hid_service_uuid = BLE_UUID16_INIT(0x1812);

static xTimerHandle blehr_tx_timer;

static bool notify_state;

//eindeutiges handle was bei einem verbindungsaufbau einer verbindung zugeordnet wird
static uint16_t conn_handle;

static const char *device_name = "HID Device";

static int blehr_gap_event(struct ble_gap_event *event, void *arg);

static uint8_t blehr_addr_type = BLE_ADDR_RANDOM;

static bool volumeUp = true;

static void
ble_app_set_addr(void)
{
    ble_addr_t addr;
    int rc;

    /* generate new non-resolvable private address */
    rc = ble_hs_id_gen_rnd(0, &addr);
    assert(rc == 0);

    /* set generated address */
    rc = ble_hs_id_set_rnd(addr.val);

    assert(rc == 0);
}

/**
 * Utility function to log an array of bytes.
 */
void
print_bytes(const uint8_t *bytes, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        ESP_LOGI("ASDF", "%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

void
print_addr(const void *addr)
{
    const uint8_t *u8p;

    u8p = addr;
    ESP_LOGI("ASDF", "%02x:%02x:%02x:%02x:%02x:%02x",
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}


/*
 * Enables advertising with parameters:
 *     o General discoverable mode
 *     o Undirected connectable mode
 */
static void
blehr_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    /*
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info)
     *     o Advertising tx power
     *     o Device name
     */
    memset(&fields, 0, sizeof(fields));

    /*
     * Advertise two flags:
     *      o Discoverability in forthcoming advertisement (general)
     *      o BLE-only (BR/EDR unsupported)
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /*
     * Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.name = (uint8_t *)CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME;
    fields.name_len = strlen(CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME);
    fields.name_is_complete = 1;

    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 0;
    fields.uuids16 = (ble_uuid16_t[]){
        hid_service_uuid
    };
    //geht nur eins weil sonst die adv packetgröße nicht reicht

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGI("ASDF", "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising */
    memset(&adv_params, 0, sizeof(adv_params));
    //undirected-connectable
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    //general-discoverable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(blehr_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, blehr_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGI("ASDF", "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

static void
blehr_tx_hrate_stop(void)
{
    volumeUp = true;
    xTimerStop( blehr_tx_timer, 1000 / portTICK_PERIOD_MS );
}

/* Reset heart rate measurement */
static void
blehr_tx_hrate_reset(void)
{

    int rc;

    if (xTimerReset(blehr_tx_timer, 1000 / portTICK_PERIOD_MS ) == pdPASS) {
        rc = 0;
    } else {
        rc = 1;
    }

    assert(rc == 0);

}

/* This function simulates heart beat and notifies it to the client */
static void
blehr_tx_hrate(xTimerHandle ev)
{
    int rc;
    struct os_mbuf *om;

    if (!notify_state) {
        blehr_tx_hrate_stop();
        return;
    }

    /* Simulation of volume */
    ESP_LOGW("ASDF", "%d",volumeUp);
    if(volumeUp){
        reportData[0] = 0b00000100;
    } else {
        reportData[0] = 0b00001000;
    }
    volumeUp = !volumeUp;

    om = ble_hs_mbuf_from_flat(reportData, sizeof(reportData));
    rc = ble_gattc_notify_custom(conn_handle, report_data_handle, om);

    assert(rc == 0);

    blehr_tx_hrate_reset();
}

//The callback to associate with this advertising procedure. If advertising ends, the event is reported through this callback. If advertising results in a connection, the connection inherits this callback as its event-reporting mechanism.
static int
blehr_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed */
        ESP_LOGI("ASDF", "connection %s; status=%d\n",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising */
            blehr_advertise();
        } else {
            struct ble_gap_upd_params connectionParameters = {
                //itvl: These determine how often the devices will "ping-pong" each other and also when they will send any data required. So if you set the value to something like 20, that would mean packets are sent every 25ms, which will obviously consume more power than say a value of 80 (100ms). The reason for the min max values is so the devices can negotiate a compromise for the best possible communication, you can set these to the same value if you prefer.
                .itvl_min = (int)(11.25/1.25), //1.25ms units; laut apple 11.25 minimum fuer hid
                .itvl_max = (int)(20/1.25),    //minimum ist laut apple eigentlich 15ms deswegen etwas höher setzen
                //latency: This is how many "ping-pong" (connection interval) events the slave(server) device is allowed to skip without the client device considering the connection terminated. So if you had a 25ms connection interval and you wanted to sleep for 1 second you could set this value to 40 and the client would consider the connection active for up to 40 skipped intervals.
                .latency = 30,            //up to 30 connection intervals
                //timeout: This is the absolute (disconnection) timeout, if no packets are received by either device within this time the connection is considered terminated.
                .supervision_timeout = 1860/10 //10ms units, laut apple größer als itvl_max * (latency + 1) * 3
            };

            //ESP_ERROR_CHECK(ble_gap_update_params(event->connect.conn_handle, &connectionParameters));
        }


        conn_handle = event->connect.conn_handle;

        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("ASDF", "disconnect; reason=%d\n", event->disconnect.reason);
        //531 = Remote User Terminated Connection

        /* Connection terminated; resume advertising */
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("ASDF", "adv complete; reason = %d\n", event->adv_complete.reason);
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI("ASDF", "subscribe event; cur_notify=%d\n value handle; val_handle=%d\n", event->subscribe.cur_notify, report_data_handle);
        
        rc = ble_gap_conn_find(event->subscribe.conn_handle, &desc);
        if (rc != 0) {
            break;
        }

        //muss eigentlich nur gemacht werden bei den attr_handle wo es auf encrypted nur gelesen wird --> input report
        if(!desc.sec_state.encrypted) {
            ble_gap_security_initiate(event->subscribe.conn_handle);
        }

        if (event->subscribe.attr_handle == report_data_handle) {
            notify_state = event->subscribe.cur_notify;
            blehr_tx_hrate_reset();
        } else if (event->subscribe.attr_handle != report_data_handle) {
            notify_state = event->subscribe.cur_notify;
            blehr_tx_hrate_stop();
        }
        ESP_LOGI("BLE_GAP_SUBSCRIBE_EVENT", "conn_handle from subscribe=%d", conn_handle);
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI("ASDF", "mtu update event; mtu already updated; nothing todo; conn_handle=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.value);
        break;
    case BLE_GAP_EVENT_ENC_CHANGE:
        ESP_LOGI("ASDF", "encryption change event; status=%d ",
                    event->enc_change.status);
        break;
    case BLE_GAP_EVENT_REPEAT_PAIRING:
        /* We already have a bond with the peer, but it is attempting to
         * establish a new secure link.  This app sacrifices security for
         * convenience: just throw away the old bond and accept the new link.
         */
        ESP_LOGI("ASDF", "establisch new secure link");

        /* Delete the old bond. */
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        assert(rc == 0);
        ble_store_util_delete_peer(&desc.peer_id_addr);

        /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
         * continue with the pairing operation.
         */
        return BLE_GAP_REPEAT_PAIRING_RETRY;
        break;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
        ESP_LOGI("ASDF", "PASSKEY_ACTION_EVENT started \n");
        
        struct ble_sm_io pkey = {0};
        int key = 0;

        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            pkey.action = event->passkey.params.action;
            pkey.passkey = 123456; // This is the passkey to be entered on peer
            ESP_LOGI(tag, "Enter passkey %d on the peer side", pkey.passkey);
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(tag, "ble_sm_inject_io result: %d\n", rc);
        } else if (event->passkey.params.action == BLE_SM_IOACT_NUMCMP) {
            ESP_LOGI(tag, "Passkey on device's display: %d", event->passkey.params.numcmp);
            ESP_LOGI(tag, "Accept or reject the passkey through console in this format -> key Y or key N");
            pkey.action = event->passkey.params.action;
            if (scli_receive_key(&key)) {
                pkey.numcmp_accept = key;
            } else {
                pkey.numcmp_accept = 0;
                ESP_LOGE(tag, "Timeout! Rejecting the key");
            }
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(tag, "ble_sm_inject_io result: %d\n", rc);
        } else if (event->passkey.params.action == BLE_SM_IOACT_OOB) {
            static uint8_t tem_oob[16] = {0};
            pkey.action = event->passkey.params.action;
            for (int i = 0; i < 16; i++) {
                pkey.oob[i] = tem_oob[i];
            }
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(tag, "ble_sm_inject_io result: %d\n", rc);
        } else if (event->passkey.params.action == BLE_SM_IOACT_INPUT) {
            ESP_LOGI(tag, "Enter the passkey through console in this format-> key 123456");
            pkey.action = event->passkey.params.action;
            if (scli_receive_key(&key)) {
                pkey.passkey = key;
            } else {
                pkey.passkey = 0;
                ESP_LOGE(tag, "Timeout! Passing 0 as the key");
            }
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(tag, "ble_sm_inject_io result: %d\n", rc);
        }
        return 0;
    case BLE_GAP_EVENT_NOTIFY_TX:
        //Represents a transmitted ATT notification or indication, or a
        //completed indication transaction.
        ESP_LOGI("asdf", "notify tx event occured");
        return 0;
    default:
        ESP_LOGI("ASDF", "GAP EVENT ID: %d\n",event->type);
    }

    return 0;
}

//This callback is executed when the host and controller become synced. This happens at startup and after a reset
static void
blehr_on_sync(void)
{
    int rc;

    /* Generate a non-resolvable private address. */
    //ble_app_set_addr();

    ble_hs_pvcy_rpa_config(1);

    /* Make sure we have proper identity address set (public preferred) */
    rc = ble_hs_util_ensure_addr(0);


    /*use privacy*/
    rc = ble_hs_id_infer_auto(blehr_addr_type, &blehr_addr_type);
    assert(rc == 0);

    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(blehr_addr_type, addr_val, NULL);

    ESP_LOGI("ASDF", "Device Address: ");
    print_addr(addr_val);
    ESP_LOGI("ASDF", "\n");

    taskYIELD();

    /*start advertising, when controller and host are in sync*/
    blehr_advertise();
}

//This callback is executed when the host resets itself and the controller
static void
blehr_on_reset(int reason)
{
    ESP_LOGI("ASDF", "Resetting state; reason=%d\n", reason);
}

//start nimble in a task
void blehr_host_task(void *param)
{
    ESP_LOGI(tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

void app_main(void)
{
    //returncodes von nimble functionen zwischenspeichern --> geben viele informationen über den status der funktionen
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
    ble_hs_cfg.sync_cb = blehr_on_sync;
    ble_hs_cfg.reset_cb = blehr_on_reset;
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
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;

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

    /* name, period/time,  auto reload, timer ID, callback */
    blehr_tx_timer = xTimerCreate("blehr_tx_timer", pdMS_TO_TICKS(2000), pdTRUE, (void *)0, blehr_tx_hrate);

    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name */
    rc = ble_svc_gap_device_name_set(CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME);
    assert(rc == 0);

    //init i2c for the oled display
    I2C_master_init();
    ssd1306_init();

    //init uart for crsf
    initCRSF_read();

    //welcome screen
    ssd1306_setString("Welcome",25,9);
    ssd1306_display();

    //display screen one second before starting up ble and crsf read
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ssd1306_clear();
    ssd1306_display();

    /* Start ble task */
    nimble_port_freertos_init(blehr_host_task);

    //task to read crsf uart ata
    xTaskCreate(crsf_get_ChannelData_task, "crsf_task", 2048, NULL, 10, NULL);
}

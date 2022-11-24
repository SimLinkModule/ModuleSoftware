// USE LINUX TOOL FOR HID PROFILE --> SONST FUNKTIONIERT ES NICHT
//https://macchina.io/blog/internet-of-things/communication-with-low-energy-bluetooth-devices-on-linux/


//TODO: alle geräte funktionieren außer ios; hier reinschauen wie notify gemacht wird; muss wahrscheinlich secure verbindung sein
//https://github.com/Xi-MingYu/Central/tree/394ccfb02f2c58d8ea7eb0e72b0abe927602f5ea/lib/NimBLE/src
//vielleicht eine secure verbindung aufbauen https://github.com/h2zero/NimBLE-Arduino/issues/222
//https://github.com/wakwak-koba/ESP32-NimBLE-Keyboard/blob/ec4ce707f00d260f30286191fe63dc23d65a5934/BleKeyboard.cpp#L141
// Diese verschlüsselung mal hinzufügen

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

static const char *tag = "NimBLE_BLE_HeartRate";
static const ble_uuid16_t hid_service_uuid = BLE_UUID16_INIT(0x1812);

static xTimerHandle blehr_tx_timer;

static bool notify_state;

static uint16_t conn_handle;

static const char *device_name = "HID Device";

static int blehr_gap_event(struct ble_gap_event *event, void *arg);

static uint8_t blehr_addr_type;

static bool volumeUp = true;

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

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
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
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
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

static int
blehr_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed */
        ESP_LOGI("ASDF", "connection %s; status=%d\n",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising */
            blehr_advertise();
        }

        struct ble_gap_upd_params connectionParameters = {
            //itvl: These determine how often the devices will "ping-pong" each other and also when they will send any data required. So if you set the value to something like 20, that would mean packets are sent every 25ms, which will obviously consume more power than say a value of 80 (100ms). The reason for the min max values is so the devices can negotiate a compromise for the best possible communication, you can set these to the same value if you prefer.
            .itvl_min = (int)(11.25/1.25), //1.25ms units; laut apple 11.25 minimum fuer hid
            .itvl_max = (int)(20/1.25),    //minimum ist laut apple eigentlich 15ms deswegen etwas höher setzen
            //latency: This is how many "ping-pong" (connection interval) events the slave(server) device is allowed to skip without the client device considering the connection terminated. So if you had a 25ms connection interval and you wanted to sleep for 1 second you could set this value to 40 and the client would consider the connection active for up to 40 skipped intervals.
            .latency = 30,            //up to 30 connection intervals
            //timeout: This is the absolute (disconnection) timeout, if no packets are received by either device within this time the connection is considered terminated.
            .supervision_timeout = 1860/10 //10ms units, laut apple größer als itvl_max * (latency + 1) * 3
        };

        ESP_ERROR_CHECK(ble_gap_update_params(event->connect.conn_handle, &connectionParameters));

        conn_handle = event->connect.conn_handle;
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("ASDF", "disconnect; reason=%d\n", event->disconnect.reason);

        /* Connection terminated; resume advertising */
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("ASDF", "adv complete; reason = %d\n", event->adv_complete.reason);
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI("ASDF", "subscribe event; cur_notify=%d\n value handle; val_handle=%d\n", event->subscribe.cur_notify, report_data_handle);
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

    }

    return 0;
}

static void
blehr_on_sync(void)
{
    int rc;

    rc = ble_hs_id_infer_auto(0, &blehr_addr_type);
    assert(rc == 0);

    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(blehr_addr_type, addr_val, NULL);

    ESP_LOGI("ASDF", "Device Address: ");
    print_addr(addr_val);
    ESP_LOGI("ASDF", "\n");

    /* Begin advertising */
    blehr_advertise();
}

static void
blehr_on_reset(int reason)
{
    ESP_LOGI("ASDF", "Resetting state; reason=%d\n", reason);
}

void blehr_host_task(void *param)
{
    ESP_LOGI(tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

void app_main(void)
{
    int rc;

    /* Initialize NVS — it is used to store PHY calibration data */
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

    /* name, period/time,  auto reload, timer ID, callback */
    blehr_tx_timer = xTimerCreate("blehr_tx_timer", pdMS_TO_TICKS(2000), pdTRUE, (void *)0, blehr_tx_hrate);

    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name */
    rc = ble_svc_gap_device_name_set(device_name);
    assert(rc == 0);

    /* Start the task */
    nimble_port_freertos_init(blehr_host_task);

}

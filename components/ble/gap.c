#include "gap.h"

uint16_t conn_handle = 0;
bool notify_state_report_data = false;
bool notify_state_battery_status = false;

/*
 * Enables advertising with parameters:
 *     o General discoverable mode
 *     o Undirected connectable mode
 */
void bleAdvertise(void){
    ssd1306_clear();
    ssd1306_setString("Connecting",10,9);
    ssd1306_display();

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
        ESP_LOGI(tag_GAP, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising */
    memset(&adv_params, 0, sizeof(adv_params));
    //undirected-connectable
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    //general-discoverable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(bleAddressType, NULL, BLE_HS_FOREVER,
                           &adv_params, bleGAPEevent, NULL);
    if (rc != 0) {
        ESP_LOGI(tag_GAP, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

//The callback to associate with this advertising procedure. If advertising ends, the event is reported through this callback. If advertising results in a connection, the connection inherits this callback as its event-reporting mechanism.
int bleGAPEevent(struct ble_gap_event *event, void *arg) {
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed */
        ESP_LOGI(tag_GAP, "connection %s; status=%d\n",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising */
            bleAdvertise();
        } else {
            ssd1306_clear();
            ssd1306_display();

            struct ble_gap_upd_params connectionParameters = {
                //itvl: These determine how often the devices will "ping-pong" each other and also when they will send any data required. So if you set the value to something like 20, that would mean packets are sent every 25ms, which will obviously consume more power than say a value of 80 (100ms). The reason for the min max values is so the devices can negotiate a compromise for the best possible communication, you can set these to the same value if you prefer.
                .itvl_min = (int)(11.25/1.25), //1.25ms units; laut apple 11.25 minimum fuer hid
                .itvl_max = (int)(20/1.25),    //minimum ist laut apple eigentlich 15ms deswegen etwas höher setzen
                //latency: This is how many "ping-pong" (connection interval) events the slave(server) device is allowed to skip without the client device considering the connection terminated. So if you had a 25ms connection interval and you wanted to sleep for 1 second you could set this value to 40 and the client would consider the connection active for up to 40 skipped intervals.
                .latency = 30,            //up to 30 connection intervals
                //timeout: This is the absolute (disconnection) timeout, if no packets are received by either device within this time the connection is considered terminated.
                .supervision_timeout = 1860/10 //10ms units, laut apple größer als itvl_max * (latency + 1) * 3
            };

            //Update connection parameters are not allowed during iPhone HID encryption, slave turns off the ability to automatically update connection parameters during encryption.
            ESP_ERROR_CHECK(ble_gap_update_params(event->connect.conn_handle, &connectionParameters));
        }


        conn_handle = event->connect.conn_handle;

        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(tag_GAP, "disconnect; reason=%d\n", event->disconnect.reason);
        ESP_LOGI(tag_GAP, "%d",event->disconnect.conn.conn_handle);
        print_addr(event->disconnect.conn.peer_id_addr.val);
        //531 = Remote User Terminated Connection
        //517 = Authentication Failure
        //573 = Connection Terminated due to MIC Failure

        notify_state_report_data = false;
        notify_state_battery_status = false;

        /* Connection terminated; resume advertising */
        bleAdvertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(tag_GAP, "adv complete; reason = %d\n", event->adv_complete.reason);
        if(event->adv_complete.reason != 0){
            bleAdvertise();
        }
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(tag_GAP, "subscribe event; cur_notify=%d\n value handle; val_handle=%d\n", event->subscribe.cur_notify, report_data_handle);
        
        rc = ble_gap_conn_find(event->subscribe.conn_handle, &desc);
        if (rc != 0) {
            break;
        }

        //muss eigentlich nur gemacht werden bei den attr_handle wo es auf encrypted nur gelesen wird --> input report
        if(!desc.sec_state.encrypted) {
            ble_gap_security_initiate(event->subscribe.conn_handle);
        }

        if (event->subscribe.attr_handle == report_data_handle) {
            //check if the device subscribed to the report data notifies
            notify_state_report_data = event->subscribe.cur_notify;
        } else if (event->subscribe.attr_handle == battery_status_handle) {
            //check if the device subscribed to the battery status notifies
            notify_state_battery_status = event->subscribe.cur_notify;
        }


        ESP_LOGI(tag_GAP, "conn_handle from subscribe=%d", conn_handle);
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(tag_GAP, "mtu update event; mtu already updated; nothing todo; conn_handle=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.value);
        break;
    case BLE_GAP_EVENT_ENC_CHANGE:
        ESP_LOGI(tag_GAP, "encryption change event; status=%d ",
                    event->enc_change.status);
        if(event->enc_change.status == 0){
            ssd1306_setConnectedImage();
            ssd1306_display();
        }
        break;
    case BLE_GAP_EVENT_REPEAT_PAIRING:
        //https://ubuntu.com/core/docs/bluez/reference/pairing/introduction
        /* We already have a bond with the peer, but it is attempting to
         * establish a new secure link.  This app sacrifices security for
         * convenience: just throw away the old bond and accept the new link.
         */
        ESP_LOGI(tag_GAP, "establish new secure link");

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
        //https://community.nxp.com/t5/Wireless-Connectivity-Knowledge/KW36-Enabling-Out-Of-Band-Pairing-on-a-Bluetooth-LE-Central-and/tac-p/1238698
        ESP_LOGI(tag_GAP, "PASSKEY_ACTION_EVENT started \n");
        
        struct ble_sm_io pkey = {0};
        BUTTON selectedButton = RIGHTBUTTON;

        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            //BLE_SM_IO_CAP_DISP_ONLY option
            pkey.action = event->passkey.params.action;
            
            pkey.passkey = esp_random();
	        /* Max value is 999999 */
	        pkey.passkey %= 1000000;
            
            char pinStr[12];
            snprintf(pinStr, 12,"%06d", (int)pkey.passkey);

            ssd1306_clear();
            ssd1306_setString("PIN",48,0);
            ssd1306_setString(pinStr,31,18);
            ssd1306_display();
            //display passkey for the other side

            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(tag_GAP, "ble_sm_inject_io result: %d\n", rc);
        } else if (event->passkey.params.action == BLE_SM_IOACT_NUMCMP) {
            //BLE_SM_IO_CAP_DISP_YES_NO option

            //gegenseite kann auch mit yes oder no bestätigen den pin
            //verify with buttons Yes or No --> add timeout after x seconds
            char pinStr[12];
            snprintf(pinStr, 12,"%06d", (int)event->passkey.params.numcmp);
            ssd1306_clear();
            ssd1306_setString(pinStr,0,0);
            ssd1306_setString("ja",0,18);
            ssd1306_setString("nein",128-43,18);
            ssd1306_display();

            pkey.action = event->passkey.params.action;

            clearStoredButtons();
            if (getButton(&selectedButton)) {
                if(selectedButton == LEFTBUTTON){
                    ESP_LOGI(tag_GAP,"LEFT BUTTON");
                    pkey.numcmp_accept = true;
                } else {
                    pkey.numcmp_accept = false;
                }
            } else {
                pkey.numcmp_accept = false;
                ESP_LOGE(tag_GAP, "Timeout! Rejecting the key");
            }

            ESP_LOGI(tag_GAP, "NUMCP_ACCEPT: %d", pkey.numcmp_accept);
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            if(rc != 0){
                //https://github.com/espressif/esp-idf/issues/8555#issuecomment-1066591071
                // 0x05 = Authentication Failure error code
                ble_gap_terminate(conn_handle, 0x05);
            }
            ESP_LOGI(tag_GAP, "ble_sm_inject_io result: %d\n", rc);
        }
        return 0;
    case BLE_GAP_EVENT_NOTIFY_TX:
        //Represents a transmitted ATT notification or indication, or a
        //completed indication transaction.
        return 0;
    case BLE_GAP_EVENT_IDENTITY_RESOLVED:
        ssd1306_setConnectedImage();
        ssd1306_display();
        return 0;
    default:
        ESP_LOGI(tag_GAP, "GAP EVENT ID: %d\n",event->type);
    }

    return 0;
}
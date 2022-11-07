#include "main.hpp"

static void bleprhp_advertise(void){
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    /*
    set the advertisement data included in our advertisements:
    -flags (indicates advertisement type and other gernal info)
    -advertising tx power
    -device name
    */
    memset(&fields, 0, sizeof(fields));

    /*advertise two flags:
    -discoverability in forhtcoming advertisement (general)
    -ble-only (br/edr unsupported)
    */
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    /*inidcate that the tx power level field should be included; have the stack fill this value automatically. This is done ba assigning the special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
    */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.name = (uint8_t *)CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME;
    fields.name_len = strlen(CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0){
        ESP_LOGE(tag, "error setting advertisement data; rc=%d\n",rc);
    }

    /*Begin advertising*/
    memset(&adv_params, 0, sizeof(adv_params));
    //undirected-connectable
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    //general-discoverable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(bleprhp_addr_type, NULL, BLE_HS_FOREVER, &adv_params, bleprhp_gap_event, NULL);
    if (rc != 0){
        ESP_LOGE(tag, "error enabling advertisement; rc=%d\n",rc);
    }
}

static int bleprhp_gap_event(struct ble_gap_event *event, void *arg){
    switch(event->type){
        case BLE_GAP_EVENT_CONNECT:
            //A new connection was establied or a connection attempt failed
            ESP_LOGI(tag, "connection %s; status=%d\n",event->connect.status == 0 ? "established" : "failed",event->connect.status);

            if(event->connect.status != 0){
                //Conection failed; resume advertising
                bleprhp_advertise();
            }
            conn_handle = event->connect.conn_handle;
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(tag, "disconnect; reason%d\n", event->disconnect.reason);
            //connection terminated; resume advertising
            bleprhp_advertise();
            break;
        case BLE_GAP_EVENT_ADV_COMPLETE:
            //means advertising has been completed because connection has been established
            ESP_LOGI(tag, "adv complete\n");
            bleprhp_advertise();
            break;
        case BLE_GAP_EVENT_SUBSCRIBE:
            //TODO
            /*ESP_LOGI(tag, "subscribe event; cur_notify=%d\n value handle; val_handle=%d\n", event->subscribe.cur_notify, hrs_hrm_handle);
            if(event->subscribe.attr_handle == hrs_hrm_handle){
                notify_state = event->subscribe.cur_notify;
                blehr_tx_hrate_reset();
            } else if (event->subscribe.attr_handle != hrs_hrm_handle){
                notify_state = event->subscribe.cur_notify;
                blehr_tx_hrate_stop();
            }*/
            ESP_LOGI(tag, "conn_handle from subscribe=%d", conn_handle);
            break;
        case BLE_GAP_EVENT_MTU:
            ESP_LOGI(tag, "mtu update event; conn_handle=%d mtu=%d\n", event->mtu.conn_handle, event->mtu.value);
            break;
    }

    return 0;
}
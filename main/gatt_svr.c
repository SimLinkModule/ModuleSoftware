#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "blehr_sens.h"

static const char *manuf_name = "Apache Mynewt ESP32 devkitC";
static const char *model_num = "Mynewt HR Sensor demo";
static const char *firmware_rev = "2.0";
static const char *software_rev = "4.8";

static const uint8_t hidReportMap[] = {
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        // Report Id (1)
    0x05, 0x0C,        //   Usage Page (Consumer)
    0x09, 0x86,        //   Usage (Channel)
    0x15, 0xFF,        //   Logical Minimum (-1)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x02,        //   Report Size (2)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x46,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,Null State)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x15, 0x00,        //   Logical Minimum (0)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x04,        //   Report Size (4)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};

static const uint8_t hidInfo[HID_INFORMATION_LEN] = {
    0x11, 0x01,                                       // bcdHID (USB HID version) --> Version 1.11
    0x00,                                             // bCountryCode
    HID_KBD_FLAGS                                     // Flags
};

static const uint8_t reportReferenceChar[2] = {
    0x01,                                               //report-id des reports vom report deskriptor
    0x01                                                //input: 0x01, output: 0x02, feature: 0x03
};

uint16_t report_data_handle;

static int
gatt_svr_chr_access_heart_rate(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

static int
gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg);

static int
gatt_svr_chr_hid(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg);

static int
report_descriptor_callback(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /* Service: Device Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                /* Characteristic: Manufacturer name */
                .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                /* Characteristic: Model number string */
                .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                /* Characteristic: Firmware Revision string */
                .uuid = BLE_UUID16_DECLARE(GATT_FIRMWARE_REVISION_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                /* Characteristic: Software Revision string */
                .uuid = BLE_UUID16_DECLARE(GATT_SOFTWARE_REVISION_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                0, /* No more characteristics in this service */
            },
        }
    },

    {
        /* Service: Battery Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_BATTERYS_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        {   {
                /* Characteristic: Battery Level */
                .uuid = BLE_UUID16_DECLARE(GATT_BATTERY_LEVEL_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                0, /* No more characteristics in this service */
            },
        }
    },

    {
        /* Service: HID */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_HIDS_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                /* Characteristic: Report Map */
                .uuid = BLE_UUID16_DECLARE(GATT_HID_REPORT_MAP_UUID),
                .access_cb = gatt_svr_chr_hid,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                /* Characteristic: HID Information */
                .uuid = BLE_UUID16_DECLARE(GATT_HID_INFORMATION_UUID),
                .access_cb = gatt_svr_chr_hid,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                /* Characteristic: HID Control Point */
                .uuid = BLE_UUID16_DECLARE(GATT_HID_CONTROL_POINT_UUID),
                .access_cb = gatt_svr_chr_hid,
                .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
            }, {
                /* Characteristic: Report Input, Pro Report Collection wird ein Reportmerkmal benötigt */
                .uuid = BLE_UUID16_DECLARE(GATT_HID_REPORT_UUID),
                .access_cb = gatt_svr_chr_hid,
                .val_handle = &report_data_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .descriptors = (struct ble_gatt_dsc_def[]){
                    //client configuration descriptor soll nicht manuell hinzugefügt werden, da dieser mittels dem flag notify automatisch hinzugefügt wird
                    {   
                        .att_flags = BLE_ATT_F_READ | BLE_ATT_F_READ_ENC,
                        .access_cb = report_descriptor_callback,
                        .uuid = BLE_UUID16_DECLARE(GATT_REPORT_REFERENCE_CHAR_UUID) //damit wird angegeben welche report id und report type abgedeckt werden
                    }, {
                        0, /* No more descriptors */
                    },
                }
            }, {
                0, /* No more characteristics in this service */
            },
        }
    },

    {
        0, /* No more services */
    },
};

static int report_descriptor_callback(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_REPORT_REFERENCE_CHAR_UUID) {
        //report id soll ungleich 0 sein, wenn es mehr als einen reportmerkmal gibt für einen bestimmten typen
        rc = os_mbuf_append(ctxt->om, reportReferenceChar, sizeof(reportReferenceChar)/sizeof(reportReferenceChar[0]));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static int gatt_svr_chr_hid(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_HID_REPORT_MAP_UUID) {
        rc = os_mbuf_append(ctxt->om, hidReportMap, sizeof(hidReportMap)/sizeof(hidReportMap[0]));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_HID_INFORMATION_UUID) {
        rc = os_mbuf_append(ctxt->om, hidInfo, sizeof(hidInfo)/sizeof(hidInfo[0]));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_HID_CONTROL_POINT_UUID) {
            int *test = OS_MBUF_DATA(ctxt->om,int *);
            //00 == hid host is entering the suspend state
            //01 == hid host is exiting the suspend state
            ESP_LOGW("ASDF", "WRITE TO CONTROL POINT %d",*test);
			return 0;
    }

    //Daten des reports übermitteln
    if (uuid == GATT_HID_REPORT_UUID) {
            rc = os_mbuf_append(ctxt->om, reportData, sizeof(reportData)/sizeof(reportData[0]));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static int
gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_MODEL_NUMBER_UUID) {
        rc = os_mbuf_append(ctxt->om, model_num, strlen(model_num));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_MANUFACTURER_NAME_UUID) {
        rc = os_mbuf_append(ctxt->om, manuf_name, strlen(manuf_name));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_FIRMWARE_REVISION_UUID) {
        rc = os_mbuf_append(ctxt->om, firmware_rev, strlen(firmware_rev));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_SOFTWARE_REVISION_UUID) {
        rc = os_mbuf_append(ctxt->om, software_rev, strlen(software_rev));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_BATTERY_LEVEL_UUID) {
        int percentage = 99;
        rc = os_mbuf_append(ctxt->om, &percentage, sizeof(percentage));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

void
gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGI("ASDF", "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGI("ASDF", "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGI("ASDF", "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

int
gatt_svr_init(void)
{
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

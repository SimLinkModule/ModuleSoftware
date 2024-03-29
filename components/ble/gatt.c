#include "gatt.h"

#include "services/gatt/ble_svc_gatt.h"
#include "services/gap/ble_svc_gap.h"

#include "esp_log.h"

#include "crsf.h"
#include "gap.h"
#include "battery.h"

//BLOCK OF BLE ATTRIBUTEN UUIDs
//device info configuration
#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24
#define GATT_FIRMWARE_REVISION_UUID             0x2A26
#define GATT_SOFTWARE_REVISION_UUID             0x2A28
#define GATT_PNP_ID_UUID                        0x2A50

//battery configuration
#define GATT_BATTERYS_UUID                      0x180f
#define GATT_BATTERY_LEVEL_UUID                 0x2a19

//hid configuration
#define GATT_HIDS_UUID                          0x1812
#define GATT_HID_REPORT_MAP_UUID                0x2A4B
#define GATT_HID_INFORMATION_UUID               0x2A4A
#define GATT_HID_CONTROL_POINT_UUID             0x2A4C
#define GATT_HID_REPORT_UUID                    0x2A4D

//hid report configuration
#define GATT_REPORT_REFERENCE_CHAR_UUID         0x2908



/*hid information infos*/
#define HID_FLAGS_REMOTE_WAKE           0x01      // RemoteWake
#define HID_FLAGS_NORMALLY_CONNECTABLE  0x02      // NormallyConnectable
#define HID_KBD_FLAGS                   HID_FLAGS_REMOTE_WAKE | HID_FLAGS_NORMALLY_CONNECTABLE
#define HID_INFORMATION_LEN             4         // HID Information

/**
 * @brief 
 * 
 * @param conn_handle 
 * @param attr_handle 
 * @param ctxt 
 * @param arg 
 * @return int 
 */
static int gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

/**
 * @brief 
 * 
 * @param conn_handle 
 * @param attr_handle 
 * @param ctxt 
 * @param arg 
 * @return int 
 */
static int gatt_svr_chr_hid(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

/**
 * @brief 
 * 
 * @param conn_handle 
 * @param attr_handle 
 * @param ctxt 
 * @param arg 
 * @return int 
 */
static int report_descriptor_callback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const char *tag_GATT = "SimLinkModule_GATT";

uint16_t report_data_handle;
uint16_t battery_status_handle;

//infos about the manufacturer, model, firmware, ... for the device information service 
static const char *manuf_name = "SimLinkModule";
static const char *model_num = "P-1.0";
static const char *firmware_rev = "1.01";
static const char *software_rev = "1.01";

static const uint8_t hidInfo[HID_INFORMATION_LEN] = {
    0x01, 0x01,                                       // bcdHID (USB HID version) --> Version 1.01
    0x00,                                             // bCountryCode
    HID_KBD_FLAGS                                     // Flags
};

//info about the report from the hid descriptor
static const uint8_t reportReferenceChar[2] = {
    0x01,                                               //report-id of the reports from the report deskriptor
    0x01                                                //input: 0x01, output: 0x02, feature: 0x03
};


/*
 *HID report descriptor created from following sources:
 *https://github.com/betaflight/betaflight/blob/025ee87a7aca068e3659fd066b8a9afbed123361/lib/main/STM32_USB_Device_Library/Class/hid/src/usbd_hid_core.c#L239
 *https://github.com/betaflight/betaflight/blob/c5468981e68795c674b76788abe820a7870f62a8/src/main/io/usb_cdc_hid.c#L56
 *original TBS Tango 2 USB HID report descriptor
 */
static const uint8_t hidReportMap[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        // Report Id (1)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x32,        //     Usage (Z)
    0x09, 0x33,        //     Usage (Rx)
    0x09, 0x35,        //     Usage (Rz)
    0x09, 0x34,        //     Usage (Ry)
    0x09, 0x36,        //     Usage (Slider)
    0x09, 0x36,        //     Usage (Slider)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x07,  //     Logical Maximum (2047)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x08,        //     Report Count (8)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x08,        //     Usage Maximum (0x08)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x08,        //     Report Count (8)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};


//definition of all services and characteristics of the GATT server
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        //Service: Device Information
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                //Characteristic: Manufacturer name
                .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
            }, {
                //Characteristic: Model number string
                .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
            }, {
                //Characteristic: Firmware Revision string
                .uuid = BLE_UUID16_DECLARE(GATT_FIRMWARE_REVISION_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
            }, {
                //Characteristic: Software Revision string
                .uuid = BLE_UUID16_DECLARE(GATT_SOFTWARE_REVISION_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
            }, {
                0, //No more characteristics in this service
            },
        }
    },

    {
        //Service: Battery Information
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_BATTERYS_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        {   {
                //Characteristic: Battery Level
                .uuid = BLE_UUID16_DECLARE(GATT_BATTERY_LEVEL_UUID),
                .access_cb = gatt_svr_chr_access_device_info,
                .val_handle = &battery_status_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_NOTIFY,
            }, {
                0, //No more characteristics in this service
            },
        }
    },

    {
        //Service: HID
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_HIDS_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                //Characteristic: Report Map
                .uuid = BLE_UUID16_DECLARE(GATT_HID_REPORT_MAP_UUID),
                .access_cb = gatt_svr_chr_hid,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
            }, {
                //Characteristic: HID Information
                .uuid = BLE_UUID16_DECLARE(GATT_HID_INFORMATION_UUID),
                .access_cb = gatt_svr_chr_hid,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
            }, {
                //Characteristic: HID Control Point
                .uuid = BLE_UUID16_DECLARE(GATT_HID_CONTROL_POINT_UUID),
                .access_cb = gatt_svr_chr_hid,
                .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
            }, {
                //Characteristic: Report Input, per report collection is a report characterstic required
                .uuid = BLE_UUID16_DECLARE(GATT_HID_REPORT_UUID),
                .access_cb = gatt_svr_chr_hid,
                .val_handle = &report_data_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_NOTIFY,
                .descriptors = (struct ble_gatt_dsc_def[]){
                    //dont add the client configuration descriptor manual. Is added automaticly via the flag notify
                    {   
                        .att_flags = BLE_ATT_F_READ | BLE_ATT_F_READ_ENC,
                        .access_cb = report_descriptor_callback,
                        .uuid = BLE_UUID16_DECLARE(GATT_REPORT_REFERENCE_CHAR_UUID)
                        //declares the report type for a specif report id
                    }, {
                        0, //No more descriptors
                    },
                }
            }, {
                0, //No more characteristics in this service
            },
        }
    },

    {
        0, //No more services
    },
};

static int gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    //send model number
    if (uuid == GATT_MODEL_NUMBER_UUID) {
        rc = os_mbuf_append(ctxt->om, model_num, strlen(model_num));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    //send manufacturer name
    if (uuid == GATT_MANUFACTURER_NAME_UUID) {
        rc = os_mbuf_append(ctxt->om, manuf_name, strlen(manuf_name));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    //send firmware revision
    if (uuid == GATT_FIRMWARE_REVISION_UUID) {
        rc = os_mbuf_append(ctxt->om, firmware_rev, strlen(firmware_rev));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    //send software revision
    if (uuid == GATT_SOFTWARE_REVISION_UUID) {
        rc = os_mbuf_append(ctxt->om, software_rev, strlen(software_rev));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    //send battery level
    if (uuid == GATT_BATTERY_LEVEL_UUID) {
        int percentage = batteryPercentage;
        rc = os_mbuf_append(ctxt->om, &percentage, sizeof(percentage));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static int gatt_svr_chr_hid(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    //send HID report map
    if (uuid == GATT_HID_REPORT_MAP_UUID) {
        rc = os_mbuf_append(ctxt->om, hidReportMap, sizeof(hidReportMap)/sizeof(hidReportMap[0]));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    //send HID information
    if (uuid == GATT_HID_INFORMATION_UUID) {
        rc = os_mbuf_append(ctxt->om, hidInfo, sizeof(hidInfo)/sizeof(hidInfo[0]));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    //receive HID control point data
    if (uuid == GATT_HID_CONTROL_POINT_UUID) {
            int *controlPointData = OS_MBUF_DATA(ctxt->om,int *);
            //00 == hid host is entering the suspend state
            //01 == hid host is exiting the suspend state
            //only observe LSB bit
            //
            //In ios, the suspend state is already changed when the device is only turned over and the display is not yet turned on. :)
            int wakeupInfo = *controlPointData & 0b11;
            notify_state_report_data = wakeupInfo;
            notify_state_battery_status = wakeupInfo;
            ESP_LOGW(tag_GATT, "WRITE TO CONTROL POINT %d",wakeupInfo);
			return 0;
    }

    //send HID report data
    if (uuid == GATT_HID_REPORT_UUID) {
            rc = os_mbuf_append(ctxt->om, &channelData, sizeof(channelData));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static int report_descriptor_callback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    //send report reference char
    if (uuid == GATT_REPORT_REFERENCE_CHAR_UUID) {
        //report id should be non-zero if there is more than one report id for a given type.
        rc = os_mbuf_append(ctxt->om, reportReferenceChar, sizeof(reportReferenceChar)/sizeof(reportReferenceChar[0]));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

void gattSvrRegisterCb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGI(tag_GATT, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGI(tag_GATT, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGI(tag_GATT, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

int gattSvrInit(void) {
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    //Adjusts a host configuration object's settings to accommodate the specified service definition array.  This function adds the counts to the appropriate fields in the supplied configuration object without clearing them first, so it can be called repeatedly with different inputs to calculate totals.  Be sure to zero the GATT server settings prior to the first call to this function.
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    //Queues a set of service definitions for registration.  All services queued in this manner get registered when ble_gatts_start() is called.
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
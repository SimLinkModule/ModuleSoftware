#ifndef H_BLEHR_SENSOR_
#define H_BLEHR_SENSOR_

#include "nimble/ble.h"
#include "modlog/modlog.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Heart-rate configuration */
#define GATT_HRS_UUID                           0x180D
#define GATT_HRS_MEASUREMENT_UUID               0x2A37
#define GATT_HRS_BODY_SENSOR_LOC_UUID           0x2A38

/* device info configuration */
#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24
#define GATT_FIRMWARE_REVISION_UUID             0x2A26
#define GATT_SOFTWARE_REVISION_UUID             0x2A28
#define GATT_PNP_ID_UUID                        0x2A50

/* battery configuration */
#define GATT_BATTERYS_UUID                      0x180f
#define GATT_BATTERY_LEVEL_UUID                 0x2a19

/* hid configuration */
#define GATT_HIDS_UUID                          0x1812
#define GATT_HID_REPORT_MAP_UUID                0x2A4B
#define GATT_HID_INFORMATION_UUID               0x2A4A
#define GATT_HID_CONTROL_POINT_UUID             0x2A4C
#define GATT_HID_REPORT_UUID                    0x2A4D

/*hid report configuration*/
#define GATT_REPORT_REFERENCE_CHAR_UUID         0x2908

/*hid information infos*/
#define HID_FLAGS_REMOTE_WAKE           0x01      // RemoteWake
#define HID_FLAGS_NORMALLY_CONNECTABLE  0x02      // NormallyConnectable
#define HID_KBD_FLAGS                   HID_FLAGS_REMOTE_WAKE
#define HID_INFORMATION_LEN             4         // HID Information

//The value handle of the relevant characteristic for notification
extern uint16_t report_data_handle;

//zu verschickender report muss immer so groß sein wie auch der report groß ist sonst geht es nicht
static uint8_t reportData[1] = {
    0x00
};

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
int gatt_svr_init(void);

#ifdef __cplusplus
}
#endif

#endif

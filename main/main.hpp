#ifndef H_MAIN
#define H_MAIN

#include <cstdlib>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"

//esp32
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

extern "C" {

static const char *tag = "SimLinkModule_BLE";

static uint8_t bleprhp_addr_type;

//eindeutiges handle was bei einem verbindungsaufbau einer verbindung zugeordnet wird
static uint16_t conn_handle;

//start nimble in a task
void bleprph_host_task(void *param);
//This callback is executed when the host resets itself and the controller
static void bleprhp_on_reset(int reason);
//This callback is executed when the host and controller become synced. This happens at startup and after a reset
static void bleprhp_on_sync(void);
//enables advertising with parameters: general discoverable mode and unidrect connectable mode
static void bleprhp_advertise(void);
//The callback to associate with this advertising procedure. If advertising ends, the event is reported through this callback. If advertising results in a connection, the connection inherits this callback as its event-reporting mechanism.
static int bleprhp_gap_event(struct ble_gap_event *event, void *arg);
//init gatt server
int gatt_svr_init(void);
}

#endif
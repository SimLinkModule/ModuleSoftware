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

extern "C" {

static const char *tag = "SimLinkModule_BLE";

static uint8_t bleprhp_addr_type;

//start nimble in a task
void bleprph_host_task(void *param);
//This callback is executed when the host resets itself and the controller
static void bleprhp_on_reset(int reason);
//This callback is executed when the host and controller become synced. This happens at startup and after a reset
static void bleprhp_on_sync(void);
}

#endif
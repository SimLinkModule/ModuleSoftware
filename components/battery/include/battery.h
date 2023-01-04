#ifndef BATTERY_H
#define BATTERY_H

#include "esp_log.h"

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "host/ble_gatt.h"

#include "gatt.h"
#include "gap.h"

extern uint8_t batteryPercentage;

void initBatteryRead();
void battery_Timer_Event(TimerHandle_t ev);

#endif
#ifndef BATTERY_H
#define BATTERY_H

#include "esp_log.h"

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "host/ble_gatt.h"

#include "gatt.h"
#include "gap.h"

extern uint8_t batteryPercentage;

void initBatteryRead();
void battery_Timer_Event(TimerHandle_t ev);
bool adc_calibration(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);

#endif
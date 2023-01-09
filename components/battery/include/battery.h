#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

//global storage var of the battery percentage
extern uint8_t batteryPercentage;

/**
 * initialize the required ADC.
 */
void initBatteryRead();

/**
 * Timer event which determines a percentage value from the measured voltage.
 *
 * @param ev                  timer handle. Not used.
 */
void battery_Timer_Event(TimerHandle_t ev);

#endif
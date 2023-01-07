#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

extern uint8_t batteryPercentage;

void initBatteryRead();
void battery_Timer_Event(TimerHandle_t ev);

#endif
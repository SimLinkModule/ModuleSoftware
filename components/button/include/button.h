#ifndef BUTTON_H
#define BUTTON_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "esp_log.h"
//#include "rom/ets_sys.h"

typedef enum BUTTON {
    LEFTBUTTON,
    RIGHTBUTTON,
} BUTTON;

static QueueHandle_t buttonQueue = NULL;

void initButtons();
static void IRAM_ATTR buttonISRHandler(void* arg);
int getButton(BUTTON *selectedButton);
void clearStoredButtons();

#endif
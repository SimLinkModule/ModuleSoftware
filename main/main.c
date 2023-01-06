#include <stdio.h>
#include "esp_log.h"

#include "crsf.h"
#include "ssd1306.h"
#include "ble.h"
#include "battery.h"
#include "button.h"

//#include "freertos/projdefs.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/timers.h"
#include "freertos/task.h"

static TimerHandle_t batteryTimerHandle;

void app_main(void)
{
    //init and configure nimble
    initBLE();

    //init i2c for the oled display
    I2C_master_init();
    ssd1306_init();

    //welcome screen
    ssd1306_setString("Welcome",25,9);
    ssd1306_display();

    //display screen 2 seconds before starting up ble and crsf read
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ssd1306_clear();
    ssd1306_display();

    /* Start ble task */
    nimble_port_freertos_init(bleHostTask);

    //init uart for crsf
    initCRSF_read();

    //task to read crsf uart data
    xTaskCreate(crsf_get_ChannelData_task, "crsf_task", 4096, NULL, 10, NULL);

    //setup buttons
    initButtons();

    /*##########################################################
    ############################################################
    # nicht verwendet, da keine Batteriespannung vorhanden ist #
    ############################################################
    ############################################################
    //timer to read battery percentage via ADC
    initBatteryRead();
    //wert jede sekunde auslesen --> aus 10 messungen wird der mittelwert gebildet --> vermindern von rauschen
    batteryTimerHandle = xTimerCreate("battery_timer", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, battery_Timer_Event);

    //start the timer
    if (xTimerStart(batteryTimerHandle, 1000 / portTICK_PERIOD_MS ) != pdPASS) {
        //display a default value if the timer cannot be started
        batteryPercentage = 7;
    }
    */
}

#include <stdio.h>
#include "esp_log.h"

#include "crsf.h"
#include "ssd1306.h"
#include "ble.h"

#include "freertos/FreeRTOSConfig.h"

static xTimerHandle blehr_tx_timer;

static bool volumeUp = true;

/**
 * Utility function to log an array of bytes.
 */
void
print_bytes(const uint8_t *bytes, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        ESP_LOGI("VOLUME-SERVICE", "%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

static void
blehr_tx_hrate_stop(void)
{
    volumeUp = true;
    xTimerStop( blehr_tx_timer, 1000 / portTICK_PERIOD_MS );
}

/* Reset heart rate measurement */
static void
blehr_tx_hrate_reset(void)
{

    int rc;

    if (xTimerReset(blehr_tx_timer, 1000 / portTICK_PERIOD_MS ) == pdPASS) {
        rc = 0;
    } else {
        rc = 1;
    }

    assert(rc == 0);

}

/* This function simulates heart beat and notifies it to the client */
static void
blehr_tx_hrate(xTimerHandle ev)
{
    int rc;
    struct os_mbuf *om;

    if (!notify_state) {
        blehr_tx_hrate_stop();
        return;
    }

    /* Simulation of volume */
    ESP_LOGW("VOLUME-SERVICE", "%d",volumeUp);
    if(volumeUp){
        reportData[0] = 0b00000100;
    } else {
        reportData[0] = 0b00001000;
    }
    volumeUp = !volumeUp;

    om = ble_hs_mbuf_from_flat(reportData, sizeof(reportData));
    rc = ble_gattc_notify_custom(conn_handle, report_data_handle, om);

    assert(rc == 0);

    blehr_tx_hrate_reset();
}

void app_main(void)
{
    //init and configure nimble
    initBLE();

    //init i2c for the oled display
    I2C_master_init();
    ssd1306_init();

    //init uart for crsf
    initCRSF_read();

    //welcome screen
    ssd1306_setString("Welcome",25,9);
    ssd1306_display();

    //display screen 2 seconds before starting up ble and crsf read
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ssd1306_clear();
    ssd1306_display();

    /* name, period/time,  auto reload, timer ID, callback */
    blehr_tx_timer = xTimerCreate("blehr_tx_timer", pdMS_TO_TICKS(2000), pdTRUE, (void *)0, blehr_tx_hrate);

    /* Start ble task */
    nimble_port_freertos_init(bleHostTask);

    //task to read crsf uart data
    xTaskCreate(crsf_get_ChannelData_task, "crsf_task", 2048, NULL, 10, NULL);
}

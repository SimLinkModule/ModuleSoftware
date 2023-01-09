#include "battery.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "host/ble_gatt.h"
#include "esp_log.h"

#include "gatt.h"
#include "gap.h"

static const char *tag_BAT = "SimLinkModule_BAT";

uint8_t batteryPercentage = 0;

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t adc_cal_handle = NULL;
static bool adc_calibrated = false;
static int measurementCount = 0;
static int sumVoltage = 0; //voltage in mV

static bool adc_calibration(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);
static int32_t getVoltage();


/**
 * determine the esp ADC reference voltage which is around 1100 mV
 */
static bool adc_calibration(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *return_handle){
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool returnValue = false;

    adc_cali_line_fitting_config_t cal_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_12,
    };

    //eFuse Vref represents the true ADC reference voltage. This value is measured and burned into eFuse BLOCK0 during factory calibration.
    ret = adc_cali_create_scheme_line_fitting(&cal_config, &handle);
    if (ret == ESP_OK) {
        returnValue = true;
    }

    *return_handle = handle;
    return returnValue;
}

void initBatteryRead(){
    //ADC init
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    /*ADC configuration
     * if .atten is set to 0db attenuation, the possible value range is from 0 to 1.1V
     * at .atten 12db the value range from 0 to theoretically 3.9V (3.55*1.1V) is used but limited by VDD to 3.3V 
     * recommended range between 150 to 2450 mV at 12db
     */
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_11,
    };
    //esp32_technical_reference_manual_en.pdf page 629 for the pin/channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_4, &config));

    //ADC calibration
    adc_calibrated = adc_calibration(ADC_UNIT_1, ADC_ATTEN_DB_11, &adc_cal_handle);
}

/**
 * determines the applied voltage at the ADC. Either calibrated or raw.
 */
static int32_t getVoltage(){
    int raw_val, voltage;
    adc_oneshot_read(adc_handle, ADC_CHANNEL_4, &raw_val);
    if(adc_calibrated){
        //voltage in mV
        adc_cali_raw_to_voltage(adc_cal_handle, raw_val, &voltage);
    } else {
        //rough value
        voltage = (raw_val*3.3/4096)*1000;
    }

    return voltage;
}

void battery_Timer_Event(TimerHandle_t ev){
    /* If there is too much output, the ESP crashes
     * https://esp32.com/viewtopic.php?t=1459
     */


    getVoltage();
    if(measurementCount < 10){
        sumVoltage += getVoltage();
        measurementCount++;
    } else {
        int avgVoltage = sumVoltage/10;
        sumVoltage = 0;
        measurementCount = 0;

        //battery range between 3.4 and 4.2V
        //r2/(r1+r2) = 0.755 with the used resistors
        //vmax_in = 4.2*0.755 = 3.171
        //vmin_in = 3.8*0.755 = 2.869
        //steps = (3171-2869)/100 = 302/100
        int8_t newPercentage = 0;
        if(avgVoltage > 2869) {
            newPercentage = avgVoltage*302/100;
        }

        ESP_LOGI(tag_BAT, "%d", newPercentage);

        if(newPercentage != batteryPercentage){
            if(notify_state_battery_status){
                struct os_mbuf *om;

                om = ble_hs_mbuf_from_flat(&batteryPercentage, sizeof(batteryPercentage));
                //Deprecated. Should not be used. Use ble_gatts_notify_custom instead.
                ble_gattc_notify_custom(conn_handle, battery_status_handle, om);
            }
            newPercentage = batteryPercentage;
        }
    }
}
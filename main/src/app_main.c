#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_check.h>

#include "adc_setup.h"

#define APP_ADC_PIN GPIO_NUM_2

#define LOOP_DELAY pdMS_TO_TICKS(1000)

void app_main(void)
{
    app_adc_state conf;
    esp_err_t r = setup_adc(GPIO_NUM_2, &conf);
    ESP_RETURN_VOID_ON_ERROR(r, __func__, "ADC setup failed");

    int raw, v_inter = -1, v_dyi = -2;
    for(;;) {
        adc_oneshot_read(conf.unit_h, conf.chan, &raw);

        adc_cali_raw_to_voltage(conf.inter_cali_h, raw, &v_inter);
        adc_cali_raw_to_voltage(conf.dyi_cali_h, raw, &v_dyi);

        ESP_LOGI(__func__, "raw: %d, v_inter: %d, v_dyi: %d", raw, v_inter, v_dyi);

        vTaskDelay(LOOP_DELAY);
    }
}

#pragma once

#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>

typedef struct {
    adc_oneshot_unit_handle_t unit_h;
    adc_channel_t chan;
    adc_cali_handle_t inter_cali_h;
    adc_cali_handle_t dyi_cali_h;
} app_adc_state;

esp_err_t setup_adc(gpio_num_t pin, app_adc_state* conf);

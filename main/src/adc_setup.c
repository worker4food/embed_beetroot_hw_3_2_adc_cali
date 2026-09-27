#include <stdio.h>
#include <math.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <adc_cali_interface.h>
#include <esp_check.h>

#include "adc_setup.h"

#define APP_ADC_ATTEN ADC_ATTEN_DB_12
#define APP_ADC_BITWIDTH ADC_BITWIDTH_12

#define MAX_RAW 4000
#define MAX_VOLTAGE 3100

typedef struct {int raw; int voltage;} segment_t;

static segment_t segments[] = {
    {   0,    0},
    { 499,  440},
    {1011,  880},
    {1999, 1690},
    {2520, 2120},
    {3047, 2530},
    {3512, 2850},
    {MAX_RAW, MAX_VOLTAGE},
};

esp_err_t dyi_raw_to_voltage(void *ctx, int raw, int *voltage);

static adc_cali_scheme_t s_dyi_cali_scheme = {
    .raw_to_voltage = dyi_raw_to_voltage,
    .ctx = segments
};

esp_err_t setup_inter_calibration(adc_unit_t unit_id, adc_channel_t channel, adc_cali_handle_t *p_inter_cali_h) {
    adc_cali_scheme_ver_t supported_cali_schemes;
    esp_err_t r = adc_cali_check_scheme(&supported_cali_schemes);
    ESP_RETURN_ON_ERROR(r, __func__, "adc_cali_check_scheme failed");

    if (!(supported_cali_schemes & ADC_CALI_SCHEME_VER_CURVE_FITTING)) {
        ESP_LOGE(__func__, "Curve fitting calibration doesn't supoorted");
        return ESP_ERR_NOT_SUPPORTED;
    }

    const adc_cali_curve_fitting_config_t c = {
        .unit_id = unit_id,
        .atten = APP_ADC_ATTEN,
        .chan = channel,
        .bitwidth = APP_ADC_BITWIDTH,
    };

    return adc_cali_create_scheme_curve_fitting(&c, p_inter_cali_h);
}

esp_err_t setup_unit_and_channel(adc_unit_t unit, adc_channel_t chan, adc_oneshot_unit_handle_t *p_adc_unit_h) {
    esp_err_t r;

    const adc_oneshot_unit_init_cfg_t unit_conf = {
        .unit_id = unit,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    r = adc_oneshot_new_unit(&unit_conf, p_adc_unit_h);
    ESP_RETURN_ON_ERROR(r, __func__, "adc_oneshot_new_unit failed: %s", esp_err_to_name(r));

    adc_oneshot_chan_cfg_t chan_conf = {
        .atten = APP_ADC_ATTEN,
        .bitwidth = APP_ADC_BITWIDTH
    };
    r = adc_oneshot_config_channel(*p_adc_unit_h, chan, &chan_conf);
    ESP_RETURN_ON_ERROR(r, __func__, "adc_oneshot_config_channel failed: %s", esp_err_to_name(r));

    return ESP_OK;
}

esp_err_t setup_pin(gpio_num_t pin) {
    const gpio_config_t gpio_adc = {
        .pin_bit_mask = 1 << pin,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    return gpio_config(&gpio_adc);
}

esp_err_t dyi_raw_to_voltage(void *, int raw, int *voltage) {

    if (raw < 0) {
        return ESP_ERR_INVALID_ARG;
    } else if (raw >= MAX_RAW) {
        *voltage = MAX_VOLTAGE;
        return ESP_OK;
    }

    size_t i = 1;
    while (segments[i].raw <= raw)
        i++;

    const segment_t *p_lo = &segments[i - 1];
    const segment_t *p_hi = &segments[i];

    double k = (double) (p_hi->voltage - p_lo->voltage) / (p_hi->raw - p_lo->raw);
    double b = (double) p_lo->voltage - k*p_lo->raw;

    *voltage = (int) roundl(k*raw + b);

    return ESP_OK;
}

esp_err_t setup_adc(gpio_num_t pin, app_adc_state* conf) {
    esp_err_t r;

    adc_unit_t unit;
    r = adc_oneshot_io_to_channel(pin, &unit, &conf->chan);
    ESP_RETURN_ON_ERROR(r, __func__, "Wrong input pin %d: %s", pin, esp_err_to_name(r));

    r = setup_pin(pin);
    ESP_RETURN_ON_ERROR(r, __func__, "gpio_config({.pin: %d}) failed: %s", pin, esp_err_to_name(r));

    r = setup_inter_calibration(unit, conf->chan, &conf->inter_cali_h);
    ESP_RETURN_ON_ERROR(r, __func__, "Integrated curve fitting setup failed: %s", esp_err_to_name(r));

    conf->dyi_cali_h = &s_dyi_cali_scheme;

    r = setup_unit_and_channel(unit, conf->chan, &conf->unit_h);
    ESP_RETURN_ON_ERROR(r, __func__, "ADC channel setup failed");

    return ESP_OK;
}

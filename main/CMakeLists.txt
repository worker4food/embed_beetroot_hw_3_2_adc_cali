idf_component_register(
    SRCS "src/app_main.c" "src/adc_setup.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES "esp_adc" "esp_driver_gpio"
)

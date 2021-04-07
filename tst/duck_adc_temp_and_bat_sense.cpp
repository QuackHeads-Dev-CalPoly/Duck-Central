extern "C"
{
    #include <stdio.h>
    #include "pico/stdlib.h"
    #include "hardware/gpio.h"
    #include "hardware/adc.h"
    #include "batt_pwr_and_temp_sensor.h"
}

#define LED_PIN 25

int main()
{
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    uint8_t led_val = 1;
    gpio_put(LED_PIN, led_val);

    init_batt_pwr_and_temp_sensor();

    while(1) 
    {
        (led_val > 0) ? led_val = 0 : led_val = 1;
        gpio_put(LED_PIN, led_val);
        printf("VBUS: %f\n", get_bus_voltage_float());
        printf("VBAT: %f\n", get_batt_voltage_float());
        sleep_ms(500);
    }

    return 0;
}
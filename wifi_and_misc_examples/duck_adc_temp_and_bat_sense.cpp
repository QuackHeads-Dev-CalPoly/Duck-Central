extern "C"
{
    #include <stdio.h>
    #include "pico/stdlib.h"
    #include "hardware/gpio.h"
    #include "hardware/adc.h"
}

int main()
{
    stdio_init_all();

    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    while(1) 
    {
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t result = adc_read();
        float conversion = result * conversion_factor;
        float temp = 27.0f- (conversion - 0.706f)/(0.001721f);
        printf("Raw value: 0x%03x, voltage: %f V\n", result, conversion);
        printf("Temp: %f\n", temp);
        sleep_ms(500);
    }

    return 0;
}
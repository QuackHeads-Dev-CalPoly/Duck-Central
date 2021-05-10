#include "batt_pwr_and_temp_sensor.h"
#include <stdint.h>

float get_temp(); // Should only be used internally

void init_batt_pwr_and_temp_sensor()
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_gpio_init(BUS_ADC_PIN);
    adc_gpio_init(BATT_ADC_PIN);
}

float get_batt_voltage_float()
{
    adc_select_input(BATT_ADC_CHANNEL);
    float conversion = adc_read() * CONVERSION_FACTOR;
    conversion += OFFSET_CORRECTION;
    conversion *= DIVISION_SCALING;
    return conversion;
}

void get_batt_voltage_string(char* batt_voltage)
{

}

float get_bus_voltage_float()
{
    adc_select_input(BUS_ADC_CHANNEL);
    float conversion = adc_read() * CONVERSION_FACTOR;
    conversion += OFFSET_CORRECTION;
    conversion *= DIVISION_SCALING;
    return conversion;
}

void get_bus_voltage_string(char* bus_voltage)
{

}

float get_temp_centigrade_float()
{
    float temp = 27.0f - ((get_temp() - 0.706f) / (0.0017121f));
}

void get_temp_centigrade_string(char* temp_str)
{
    float temp = 27.0f - ((get_temp() - 0.706f) / (0.001712f));
}

float get_temp_fahrenheit_float()
{

}

void get_temp_fahrenheit_string(char* temp_str)
{

}

float get_temp()
{
    adc_select_input(4);
    uint16_t result = adc_read();
    float temp = 27.0f - (((CONVERSION_FACTOR * result) - 0.706f) / (0.0017121f));
    return (CONVERSION_FACTOR * result);
}
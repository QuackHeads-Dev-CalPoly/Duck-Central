#ifndef __BATT_PWR_AND_TEMP_SENSOR_H__
#define __BATT_PWR_AND_TEMP_SENSOR_H__

#define BUS_ADC_PIN 27
#define BUS_ADC_CHANNEL 1
#define BATT_ADC_PIN 26
#define BATT_ADC_CHANNEL 0

#define CONVERSION_FACTOR (3.3f / (1 << 12))
#define DIVISION_SCALING 1.5f
#define OFFSET_CORRECTION 0.1f

/* Init to setup ADC. Must be called for sensor to work */
/* TODO: maybe add a static var or something to see if it works */
void init_batt_pwr_and_temp_sensor();

/* Voltage getters */
float get_batt_voltage_float();
void get_batt_voltage_string(char*);

float get_bus_voltage_float();
void get_bus_voltage_string(char*);

/* Temperature getters */
float get_temp_centrigrade_float();
void get_temp_centrigrade_string(char*);

float get_temp_fahrenheit_float();
void get_temp_fahrenheit_string(char*);

#endif /* */
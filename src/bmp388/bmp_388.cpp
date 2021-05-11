#include "bmp_388.h"
#include "driver/bmp3.h"
#include "driver/bmp3_selftest.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <unistd.h>
#include "pico/stdlib.h"


#ifndef BMP3_DOUBLE_PRECISION_COMPENSATION

/* 0 degree celsius */
#define BMP3_MIN_TEMPERATURE  INT16_C(0)

/* 40 degree celsius */
#define BMP3_MAX_TEMPERATURE  INT16_C(4000)

/* 900 hecto Pascals */
#define BMP3_MIN_PRESSURE     UINT32_C(90000)

/* 1100 hecto Pascals */
#define BMP3_MAX_PRESSURE     UINT32_C(110000)

#else

/* 0 degree celsius */
#define BMP3_MIN_TEMPERATURE  (0.0f)

/* 40 degree celsius */
#define BMP3_MAX_TEMPERATURE  (40.0f)

/* 900 hecto Pascals */
#define BMP3_MIN_PRESSURE     (900.0f)

/* 1100 hecto Pascals */
#define BMP3_MAX_PRESSURE     (1100.0f)
#endif

static int8_t analyze_sensor_data(const struct bmp3_data *sens_data);

// Hardware address for BMP 388
static uint8_t addr = 0x76;

// Constructor: performs all required pre-setup tasks.
BMP::BMP() {
    int8_t result = 0;
    int i = 0;
    uint8_t buff[3] = { 0 };

    stdio_init_all();

    // set up i2c stuff on the Pico
    i2c_init(i2c0, 400*1000);
    
    gpio_set_function(PICO_BMP_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_BMP_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_BMP_I2C_SDA_PIN);
    gpio_pull_up(PICO_BMP_I2C_SCL_PIN);

    // Set the BMP388 struct to use I2C and self-defined read/write/delay funcs
    bmp388.intf = BMP3_I2C_INTF;
    //bmp388.settings.op_mode = BMP3_MODE_NORMAL;
    //bmp388.settings.press_en = BMP3_SEL_PRESS_EN;
    //bmp388.settings.temp_en = BMP3_SEL_PRESS_EN;
    //bmp388.chip_id = BMP3_CHIP_ID;
    bmp388.delay_us = (bmp3_delay_us_fptr_t)&bmpDelay;
    bmp388.read = (bmp3_read_fptr_t)&bmpRead;
    bmp388.write = (bmp3_write_fptr_t)&bmpWrite;

    // perform selftest for error checking
    //printf("Selftest result is: %d\n", bmp3_selftest_check(&bmp388));
    //fflush(stdout);

    /* Variable used to select the sensor component */
    uint8_t sensor_comp;

    // Use Pressure and Temperature readings    
    sensor_comp = BMP3_PRESS | BMP3_TEMP;

    /* Used to select the settings user needs to change */
    uint16_t settings_sel;

    /* Variable used to store the compensated data */
    struct bmp3_data data = { 0 };


    result = bmp3_soft_reset(&bmp388);

    printf("Result of soft reset is %d\n", result);

    bmp388.delay_us(40000, bmp388.intf_ptr);


    result = bmp3_init(&bmp388);

    printf("Result of init is %d\n", result);

    bmp388.delay_us(40000, bmp388.intf_ptr);

    //validate_trimming_param(&bmp388);

    // bmp388.settings.press_en = BMP3_ENABLE;
    // bmp388.settings.temp_en = BMP3_ENABLE;
    // bmp388.settings.odr_filter.press_os = BMP3_OVERSAMPLING_8X;
    // bmp388.settings.odr_filter.temp_os = BMP3_OVERSAMPLING_2X;
    // bmp388.settings.odr_filter.iir_filter = BMP3_IIR_FILTER_COEFF_15;
    // bmp388.settings.odr_filter.odr = BMP3_ODR_50_HZ;

    bmp388.settings.press_en = BMP3_ENABLE;
    bmp388.settings.temp_en = BMP3_ENABLE;
    //bmp388.settings.odr_filter.press_os = BMP3_OVERSAMPLING_8X;
    //bmp388.settings.odr_filter.temp_os = BMP3_OVERSAMPLING_2X;
    //bmp388.settings.odr_filter.iir_filter = BMP3_IIR_FILTER_COEFF_15;
    bmp388.settings.odr_filter.odr = BMP3_ODR_25_HZ;

    //settings_sel = BMP3_SEL_PRESS_EN | BMP3_SEL_TEMP_EN | BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS | BMP3_SEL_ODR;
    settings_sel = BMP3_SEL_PRESS_EN | BMP3_SEL_TEMP_EN | BMP3_SEL_ODR;

    //settings_sel = 0xB7;

    printf("The settings sel struct is : %04x\n", settings_sel);

    bmpRead(0x1B, buff, 1, bmp388.intf_ptr);
    printf("The settings reg is BEFORE: %02x\n", buff[0]);

    result = bmp3_set_sensor_settings(settings_sel, &bmp388);

    bmp388.delay_us(40000, bmp388.intf_ptr);

    printf("The sensor set result was %d\n", result);
    fflush(stdout);

    bmpRead(0x1B, buff, 1, bmp388.intf_ptr);
    printf("The status reg is AFTER: %02x\n", buff[0]);


    // bmp388.settings.op_mode = BMP3_MODE_FORCED;
    // bmp3_set_op_mode(&bmp388);

    bmp388.settings.op_mode = BMP3_MODE_NORMAL;
    bmp3_set_op_mode(&bmp388);
    printf("power mode set result is: %d\n", result);

    uint8_t pwr = 0;
    result = bmp3_get_regs(BMP3_REG_PWR_CTRL, &pwr, 1, &bmp388);
    printf("Chip id is: %x, result is: %d\n", pwr, result);


    // bmp3_get_sensor_data(sensor_comp, &data, &bmp388);

    // printf("Sensor data temp: %lf press: %lf\n", data.temperature, data.pressure/100.0);

    // printf("Analyzed sensor data: %d\n", analyze_sensor_data(&data));
    // fflush(stdout);

    // // for reply
    // uint8_t chip_id = 0;
    // result = bmp3_get_regs(BMP3_REG_CHIP_ID, &chip_id, 1, &bmp388);
    // printf("Chip id is: %x, result is: %d\n", chip_id, result);

    // uint8_t read_data = 1;
    // uint8_t reg = 0x1D;
    // result = bmp3_get_regs(reg, &read_data, 1, &bmp388);
    // printf("ODR reg is : %x, result: %d\n", read_data, result);
    // read_data = BMP3_ODR_25_HZ;
    // result = bmp3_set_regs(&reg, &read_data, 1, &bmp388);
    // printf("Result after write is: %d\n", result);
    // result = bmp3_get_regs(reg, &read_data, 1, &bmp388);
    // printf("ODR reg is : %x, result: %d\n", read_data, result);


    // while(1) {
    //     bmp388.delay_us(1000000, bmp388.intf_ptr);

    //     bmp3_get_sensor_data(sensor_comp, &data, &bmp388);

    //     printf("Sensor data temp: %lf press: %lf\n", data.temperature, data.pressure/100.0);

    //     printf("Analyzed sensor data: %d\n", analyze_sensor_data(&data));
    //     fflush(stdout);
    //     printf("\n");
    //     bmp388.delay_us(40000, bmp388.intf_ptr);
    // }

}

BMP3_INTF_RET_TYPE bmpWrite(uint8_t reg_addr, const uint8_t *read_data, uint32_t len, void *intf_ptr) {
    // Signal the reg addr to be written to
    i2c_write_blocking(i2c0, addr, &reg_addr, sizeof(uint8_t), true);
    
    //sleep_ms(40);

    // Write to reg
    int8_t write_res = i2c_write_blocking(i2c0, addr, read_data, len, false);
    printf("Read data after write is: %02x\n", read_data[0]);
    return write_res > 0 ? BMP3_INTF_RET_SUCCESS : BMP3_E_COMM_FAIL;
}

BMP3_INTF_RET_TYPE bmpRead(uint8_t reg_addr, uint8_t *read_data, uint32_t len, void *intf_ptr) {
    // To read, first write the register address wanting to be read
    i2c_write_blocking(i2c0, addr, &reg_addr, sizeof(uint8_t), true);

    sleep_ms(40);

    int8_t write_result = i2c_read_blocking(i2c0, addr, read_data, len, false);

    return write_result > 0 ? BMP3_INTF_RET_SUCCESS : BMP3_E_COMM_FAIL;
}

BMP3_INTF_RET_TYPE bmpDelay(uint32_t amt, void *intf_ptr) {
    sleep_us(amt);
    return BMP3_INTF_RET_SUCCESS;
}


static int8_t analyze_sensor_data(const struct bmp3_data *sens_data)
{
    int8_t rslt = BMP3_SENSOR_OK;

    if ((sens_data->temperature < BMP3_MIN_TEMPERATURE) || (sens_data->temperature > BMP3_MAX_TEMPERATURE))
    {
        rslt = BMP3_IMPLAUSIBLE_TEMPERATURE;
    }

    if (rslt == BMP3_SENSOR_OK)
    {
        if ((sens_data->pressure / 100 < BMP3_MIN_PRESSURE) || (sens_data->pressure / 100 > BMP3_MAX_PRESSURE))
        {
            rslt = BMP3_IMPLAUSIBLE_PRESSURE;
        }
    }

    return rslt;
}

/*
===============================================
bmx160 magnetometer/accelerometer/gyroscope library for Intel(R) Curie(TM)
devices. Copyright (c) 2015 Intel Corporation.  All rights reserved. Based on
MPU6050 Arduino library provided by Jeff Rowberg as part of his excellent I2Cdev
device library: https://github.com/jrowberg/i2cdevlib
===============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/
#include "bmx_160.h"

#include <cstdlib>

BMX160::BMX160(i2c_inst_t *i2c_inst, int clock_speed, int pin_sck, int pin_sda) {
    i2c_init(i2c_inst, clock_speed);
    gpio_set_function(pin_sck, GPIO_FUNC_I2C);
    gpio_set_function(pin_sda, GPIO_FUNC_I2C);
    gpio_pull_up(pin_sck);
    gpio_pull_up(pin_sda);
}

const uint8_t int_mask_lookup_table[13] = {BMX160_INT1_SLOPE_MASK,
                                           BMX160_INT1_SLOPE_MASK,
                                           BMX160_INT2_LOW_STEP_DETECT_MASK,
                                           BMX160_INT1_DOUBLE_TAP_MASK,
                                           BMX160_INT1_SINGLE_TAP_MASK,
                                           BMX160_INT1_ORIENT_MASK,
                                           BMX160_INT1_FLAT_MASK,
                                           BMX160_INT1_HIGH_G_MASK,
                                           BMX160_INT1_LOW_G_MASK,
                                           BMX160_INT1_NO_MOTION_MASK,
                                           BMX160_INT2_DATA_READY_MASK,
                                           BMX160_INT2_FIFO_FULL_MASK,
                                           BMX160_INT2_FIFO_WM_MASK};

bool BMX160::begin() {
    if (scan() == true) {
        soft_reset();
        write_bmx_reg(BMX160_COMMAND_REG_ADDR, 0x11);
        sleep_ms(50);
        /* Set gyro to normal mode */
        write_bmx_reg(BMX160_COMMAND_REG_ADDR, 0x15);
        sleep_ms(100);
        /* Set mag to normal mode */
        write_bmx_reg(BMX160_COMMAND_REG_ADDR, 0x19);
        sleep_ms(10);
        config_magnetometer();
        return true;
    } else {
        return false;
    }
}

void BMX160::set_low_power() {
    soft_reset();
    sleep_ms(100);
    config_magnetometer();
    sleep_ms(100);
    write_bmx_reg(BMX160_COMMAND_REG_ADDR, 0x12);
    sleep_ms(100);
    /* Set gyro to normal mode */
    write_bmx_reg(BMX160_COMMAND_REG_ADDR, 0x17);
    sleep_ms(100);
    /* Set mag to normal mode */
    write_bmx_reg(BMX160_COMMAND_REG_ADDR, 0x1B);
    sleep_ms(100);
}

void BMX160::wake() {
    soft_reset();
    sleep_ms(100);
    config_magnetometer();
    sleep_ms(100);
    write_bmx_reg(BMX160_COMMAND_REG_ADDR, 0x11);
    sleep_ms(100);
    /* Set gyro to normal mode */
    write_bmx_reg(BMX160_COMMAND_REG_ADDR, 0x15);
    sleep_ms(100);
    /* Set mag to normal mode */
    write_bmx_reg(BMX160_COMMAND_REG_ADDR, 0x19);
    sleep_ms(100);
}

bool BMX160::soft_reset() {
    int8_t rslt = BMX160_OK;
    if (Obmx160 == NULL) {
        rslt = BMX160_E_NULL_PTR;
    }

    rslt = soft_reset(Obmx160);
    printf("Reset %d\n", rslt);

    if (rslt == 0) {
        return true;
    } else {
        return false;
    }
}

int8_t BMX160::soft_reset(struct bmx160Dev *dev) {
    int8_t rslt = BMX160_OK;
    uint8_t data = BMX160_SOFT_RESET_CMD;
    if (dev == NULL) {
        rslt = BMX160_E_NULL_PTR;
    }
    write_bmx_reg(BMX160_COMMAND_REG_ADDR, data);
    sleep_ms(BMX160_SOFT_RESET_DELAY_MS);
    if (rslt == BMX160_OK) {
        BMX160::set_sensor_defaults(dev);
    }
    return rslt;
}

void BMX160::set_sensor_defaults(struct bmx160Dev *dev) {
    // Initializing accel and gyro params with
    dev->gyroCfg.bw = BMX160_GYRO_BW_NORMAL_MODE;
    dev->gyroCfg.odr = BMX160_GYRO_ODR_100HZ;
    dev->gyroCfg.power = BMX160_GYRO_SUSPEND_MODE;
    dev->gyroCfg.range = BMX160_GYRO_RANGE_2000_DPS;
    dev->accelCfg.bw = BMX160_ACCEL_BW_NORMAL_AVG4;
    dev->accelCfg.odr = BMX160_ACCEL_ODR_100HZ;
    dev->accelCfg.power = BMX160_ACCEL_SUSPEND_MODE;
    dev->accelCfg.range = BMX160_ACCEL_RANGE_2G;

    dev->prevMagnCfg = dev->magnCfg;
    dev->prevGyroCfg = dev->gyroCfg;
    dev->prevAccelCfg = dev->accelCfg;
}

void BMX160::config_magnetometer() {
    write_bmx_reg(BMX160_MAGN_IF_0_ADDR, 0x80);
    sleep_ms(50);
    // Sleep mode
    write_bmx_reg(BMX160_MAGN_IF_3_ADDR, 0x01);
    write_bmx_reg(BMX160_MAGN_IF_2_ADDR, 0x4B);
    // REPXY regular preset
    write_bmx_reg(BMX160_MAGN_IF_3_ADDR, 0x04);
    write_bmx_reg(BMX160_MAGN_IF_2_ADDR, 0x51);
    // REPZ regular preset
    write_bmx_reg(BMX160_MAGN_IF_3_ADDR, 0x0E);
    write_bmx_reg(BMX160_MAGN_IF_2_ADDR, 0x52);

    write_bmx_reg(BMX160_MAGN_IF_3_ADDR, 0x02);
    write_bmx_reg(BMX160_MAGN_IF_2_ADDR, 0x4C);
    write_bmx_reg(BMX160_MAGN_IF_1_ADDR, 0x42);
    write_bmx_reg(BMX160_MAGN_CONFIG_ADDR, 0x08);
    write_bmx_reg(BMX160_MAGN_IF_0_ADDR, 0x03);
    sleep_ms(50);
}

void BMX160::set_gyro_range(eGyroRange_t bits) {
    switch (bits) {
        case eGyroRange_125DPS:
            gyroRange = BMX160_GYRO_SENSITIVITY_125DPS;
            break;
        case eGyroRange_250DPS:
            gyroRange = BMX160_GYRO_SENSITIVITY_250DPS;
            break;
        case eGyroRange_500DPS:
            gyroRange = BMX160_GYRO_SENSITIVITY_500DPS;
            break;
        case eGyroRange_1000DPS:
            gyroRange = BMX160_GYRO_SENSITIVITY_1000DPS;
            break;
        case eGyroRange_2000DPS:
            gyroRange = BMX160_GYRO_SENSITIVITY_2000DPS;
            break;
        default:
            gyroRange = BMX160_GYRO_SENSITIVITY_250DPS;
            break;
    }
}

void BMX160::set_accel_range(eAccelRange_t bits) {
    switch (bits) {
        case eAccelRange_2G:
            accelRange = BMX160_ACCEL_MG_LSB_2G * 10;
            break;
        case eAccelRange_4G:
            accelRange = BMX160_ACCEL_MG_LSB_4G * 10;
            break;
        case eAccelRange_8G:
            accelRange = BMX160_ACCEL_MG_LSB_8G * 10;
            break;
        case eAccelRange_16G:
            accelRange = BMX160_ACCEL_MG_LSB_16G * 10;
            break;
        default:
            accelRange = BMX160_ACCEL_MG_LSB_2G * 10;
            break;
    }
}

void BMX160::get_all_data(struct bmx160SensorData *magn,
                        struct bmx160SensorData *gyro,
                        struct bmx160SensorData *accel) {

    uint8_t data[23] = {0};
    
    read_reg(BMX160_MAG_DATA_ADDR, data, 23);

    if (magn) {
        magn->x = (int16_t)((data[1] << 8) | data[0]);
        magn->y = (int16_t)((data[3] << 8) | data[2]);
        magn->z = (int16_t)((data[5] << 8) | data[4]);
        magn->x *= BMX160_MAGN_UT_LSB;
        magn->y *= BMX160_MAGN_UT_LSB;
        magn->z *= BMX160_MAGN_UT_LSB;
    }

    if (gyro) {
        gyro->x = (int16_t)((data[9] << 8) | data[8]);
        gyro->y = (int16_t)((data[11] << 8) | data[10]);
        gyro->z = (int16_t)((data[13] << 8) | data[12]);
        gyro->x *= gyroRange;
        gyro->y *= gyroRange;
        gyro->z *= gyroRange;
    }

    if (accel) {
        accel->x = (int16_t)((data[15] << 8) | data[14]);
        accel->y = (int16_t)((data[17] << 8) | data[16]);
        accel->z = (int16_t)((data[19] << 8) | data[18]);
        accel->x *= accelRange;
        accel->y *= accelRange;
        accel->z *= accelRange;
    }
}

int8_t BMX160::read_bmx_reg(uint8_t reg) {
    uint8_t buf[1] = {0};
    read_reg(reg, buf, sizeof(buf));
    return buf[1];
}

void BMX160::write_bmx_reg(uint8_t reg, uint8_t value) {
    uint8_t buffer[1] = {value};
    write_reg(reg, buffer, 1);
}

void BMX160::write_reg(uint8_t reg, uint8_t *pBuf, uint16_t len) {
    uint8_t con_buff[2] = {reg, pBuf[0]};
    i2c_write_blocking(i2c0, _addr, con_buff, 2, false);
}

void BMX160::read_reg(uint8_t reg, uint8_t *pBuf, uint16_t len) {
    i2c_write_blocking(i2c0, _addr, &reg, 1, true);
    i2c_read_blocking(i2c0, _addr, pBuf, len, false);
}

bool BMX160::scan() {
    int val = i2c_write_blocking(i2c0, _addr, 0x00, 0, false);
    if (val < 0) {
        return false;
    }
    return true;
}

void BMX160::enable_low_g_interrupt() {
    // INT 1 is GP6
    // INT 2 is GP26

    // enable the low g interrupt on INT1
    write_bmx_reg(BMX160_INT_ENABLE_1_ADDR, BMX160_LOW_G_INT_EN_MASK);

    // push-pull interrupt pin
    write_bmx_reg(BMX160_INT_OUT_CTRL_ADDR,
                BMX160_INT1_OUTPUT_EN_MASK |  // enable this as an output interrupt on INT1
                BMX160_INT1_OUTPUT_TYPE_MASK  // active high
    );

    // make the low g interrupt come out on INT1
    write_bmx_reg(BMX160_INT_MAP_0_ADDR, BMX160_INT1_LOW_G_MASK);

    // must hold below threshold + hysteresis value for 300ms
    write_bmx_reg(BMX160_INT_LOWHIGH_0_ADDR, BMX160_INT_LOWHIGH_TIME_THRESH_300_MS);

    // 0.1g is our threshold
    write_bmx_reg(BMX160_INT_LOWHIGH_1_ADDR, BMX160_INT_LOWHIGH_G_THRESH_0_1_G);

    write_bmx_reg(BMX160_INT_LOWHIGH_2_ADDR, 
                BMX160_INT_LOWHIGH_SUMMING_MODE |  // 3-axis sum of accelerometer
                BMX160_INT_LOWHIGH_HYST_125_MG     // 0.125g hysteresis value
    );
}
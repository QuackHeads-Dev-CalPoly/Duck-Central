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
        softReset();
        writeBmxReg(BMX160_COMMAND_REG_ADDR, 0x11);
        sleep_ms(50);
        /* Set gyro to normal mode */
        writeBmxReg(BMX160_COMMAND_REG_ADDR, 0x15);
        sleep_ms(100);
        /* Set mag to normal mode */
        writeBmxReg(BMX160_COMMAND_REG_ADDR, 0x19);
        sleep_ms(10);
        setMagnConf();
        return true;
    } else {
        return false;
    }
}

void BMX160::setLowPower() {
    softReset();
    sleep_ms(100);
    setMagnConf();
    sleep_ms(100);
    writeBmxReg(BMX160_COMMAND_REG_ADDR, 0x12);
    sleep_ms(100);
    /* Set gyro to normal mode */
    writeBmxReg(BMX160_COMMAND_REG_ADDR, 0x17);
    sleep_ms(100);
    /* Set mag to normal mode */
    writeBmxReg(BMX160_COMMAND_REG_ADDR, 0x1B);
    sleep_ms(100);
}

void BMX160::wakeUp() {
    softReset();
    sleep_ms(100);
    setMagnConf();
    sleep_ms(100);
    writeBmxReg(BMX160_COMMAND_REG_ADDR, 0x11);
    sleep_ms(100);
    /* Set gyro to normal mode */
    writeBmxReg(BMX160_COMMAND_REG_ADDR, 0x15);
    sleep_ms(100);
    /* Set mag to normal mode */
    writeBmxReg(BMX160_COMMAND_REG_ADDR, 0x19);
    sleep_ms(100);
}

bool BMX160::softReset() {
    int8_t rslt = BMX160_OK;
    if (Obmx160 == NULL) {
        rslt = BMX160_E_NULL_PTR;
    }

    rslt = softReset(Obmx160);
    printf("Reset %d\n", rslt);

    if (rslt == 0) {
        return true;
    } else {
        return false;
    }
}

int8_t BMX160::softReset(struct bmx160Dev *dev) {
    int8_t rslt = BMX160_OK;
    uint8_t data = BMX160_SOFT_RESET_CMD;
    if (dev == NULL) {
        rslt = BMX160_E_NULL_PTR;
    }
    printf("Writing....... \n");
    writeBmxReg(BMX160_COMMAND_REG_ADDR, data);
    printf("Write done.....\n");
    sleep_ms(BMX160_SOFT_RESET_DELAY_MS);
    if (rslt == BMX160_OK) {
        BMX160::defaultParamSettg(dev);
    }
    return rslt;
}

void BMX160::defaultParamSettg(struct bmx160Dev *dev) {
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

void BMX160::setMagnConf() {
    writeBmxReg(BMX160_MAGN_IF_0_ADDR, 0x80);
    sleep_ms(50);
    // Sleep mode
    writeBmxReg(BMX160_MAGN_IF_3_ADDR, 0x01);
    writeBmxReg(BMX160_MAGN_IF_2_ADDR, 0x4B);
    // REPXY regular preset
    writeBmxReg(BMX160_MAGN_IF_3_ADDR, 0x04);
    writeBmxReg(BMX160_MAGN_IF_2_ADDR, 0x51);
    // REPZ regular preset
    writeBmxReg(BMX160_MAGN_IF_3_ADDR, 0x0E);
    writeBmxReg(BMX160_MAGN_IF_2_ADDR, 0x52);

    writeBmxReg(BMX160_MAGN_IF_3_ADDR, 0x02);
    writeBmxReg(BMX160_MAGN_IF_2_ADDR, 0x4C);
    writeBmxReg(BMX160_MAGN_IF_1_ADDR, 0x42);
    writeBmxReg(BMX160_MAGN_CONFIG_ADDR, 0x08);
    writeBmxReg(BMX160_MAGN_IF_0_ADDR, 0x03);
    sleep_ms(50);
}

void BMX160::setGyroRange(eGyroRange_t bits) {
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

void BMX160::setAccelRange(eAccelRange_t bits) {
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

void BMX160::getAllData(struct bmx160SensorData *magn,
                        struct bmx160SensorData *gyro,
                        struct bmx160SensorData *accel) {

    uint8_t data[23] = {0};
    
    readReg(BMX160_MAG_DATA_ADDR, data, 23);

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

int8_t BMX160::readBmxReg(uint8_t reg) {
    uint8_t buf[1] = {0};
    readReg(reg, buf, sizeof(buf));
    return buf[1];
}

void BMX160::writeBmxReg(uint8_t reg, uint8_t value) {
    uint8_t buffer[1] = {value};
    writeReg(reg, buffer, 1);
}

void BMX160::writeReg(uint8_t reg, uint8_t *pBuf, uint16_t len) {
    uint8_t con_buff[2] = {reg, pBuf[0]};
    i2c_write_blocking(i2c0, _addr, con_buff, 2, false);
}

void BMX160::readReg(uint8_t reg, uint8_t *pBuf, uint16_t len) {
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
    writeBmxReg(BMX160_INT_ENABLE_1_ADDR, BMX160_LOW_G_INT_EN_MASK);

    // push-pull interrupt pin
    writeBmxReg(BMX160_INT_OUT_CTRL_ADDR,
                BMX160_INT1_OUTPUT_EN_MASK |  // enable this as an output interrupt on INT1
                BMX160_INT1_OUTPUT_TYPE_MASK  // active high
    );

    // make the low g interrupt come out on INT1
    writeBmxReg(BMX160_INT_MAP_0_ADDR, BMX160_INT1_LOW_G_MASK);

    // must hold below threshold + hysteresis value for 300ms
    writeBmxReg(BMX160_INT_LOWHIGH_0_ADDR, BMX160_INT_LOWHIGH_TIME_THRESH_300_MS);

    // 0.1g is our threshold
    writeBmxReg(BMX160_INT_LOWHIGH_1_ADDR, BMX160_INT_LOWHIGH_G_THRESH_0_1_G);

    writeBmxReg(BMX160_INT_LOWHIGH_2_ADDR, 
                BMX160_INT_LOWHIGH_SUMMING_MODE |  // 3-axis sum of accelerometer
                BMX160_INT_LOWHIGH_HYST_125_MG     // 0.125g hysteresis value
    );
}

// void anyMotionInterrupt_set() {

//       int_config.int_channel = BMI160_INT_CHANNEL_1;   // na - Select the
//       interrupt channel int_config.int_type = BMI160_ACC_ANY_MOTION_INT; //
//       na - choosing Any Motion Interrupt

//       // na - the following configuration is written to registers (0x53)
//       INT_OUT_CTRL & (0x54) INT_LATCH  see datasheet pg71 section 2.11.20
//       int_config.int_pin_settg.output_en = BMI160_ENABLE;         // na -
//       Enabling interrupt pin as output -> register (0x53)
//       int_config.int_pin_settg.output_mode = BMI160_DISABLE;      // na -
//       Selecting push-pull mode for interrupt pin -> register (0x53)
//       int_config.int_pin_settg.output_type = BMI160_DISABLE;      // na -
//       Setting interrupt pin to active low -> register (0x53)
//       int_config.int_pin_settg.edge_ctrl = BMI160_ENABLE;         // na -
//       Enabling edge trigger -> register (0x53)
//       int_config.int_pin_settg.input_en = BMI160_DISABLE;         // na -
//       Disabling interrupt pin to act as input -> register (0x54)
//       int_config.int_pin_settg.latch_dur = BMI160_LATCH_DUR_NONE; // na -
//       non-latched output -> register (0x54)

//       // na - Select the Any Motion Interrupt parameter
//       int_config.int_type_cfg.acc_any_motion_int.anymotion_en =
//       BMI160_ENABLE; // na - 1- Enable the any-motion, 0- disable any-motion
//       int_config.int_type_cfg.acc_any_motion_int.anymotion_x = BMI160_ENABLE;
//       // na - Enabling x-axis for any motion interrupt - monitor x axis
//       int_config.int_type_cfg.acc_any_motion_int.anymotion_y = BMI160_ENABLE;
//       // na - Enabling y-axis for any motion interrupt - monitor y axis
//       int_config.int_type_cfg.acc_any_motion_int.anymotion_z = BMI160_ENABLE;
//       // na - Enabling z-axis for any motion interrupt - monitor z axis
//       int_config.int_type_cfg.acc_any_motion_int.anymotion_dur = 2; // na -
//       any-motion duration. This is the consecutive datapoints -> see
//       datasheet pg32 section 2.6.1 <int_anym_dur> and pg78
//       int_config.int_type_cfg.acc_any_motion_int.anymotion_thr = 20; // na -
//       An interrupt will be generated if the absolute value of two consecutive
//       accelarion signal exceeds the threshold value -> see datasheet pg32
//       section 2.6.1 <int_anym_th> and pg78 INT_MOTION[1]
//                                                                                // na - (2-g range) -> (anymotion_thr) * 3.91 mg, (4-g range) -> (anymotion_thr) * 7.81 mg, (8-g range) ->(anymotion_thr) * 15.63 mg, (16-g range) -> (anymotion_thr) * 31.25 mg

//       rslt = bmi160_set_int_config(&int_config, &sensor); // na - Set
//       Any-motion interrupt NRF_LOG_INFO("rslt: %d", rslt);

//       if (rslt != BMI160_OK) {
//         NRF_LOG_INFO("BMI160 Any-motion interrupt configuration failure!\n");
//       } else {
//         NRF_LOG_INFO("BMI160 Any-motion interrupt configuration done!\n");
//       }
//     }
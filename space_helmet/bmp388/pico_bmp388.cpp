/*! 
 * @file pico_bmp388.cpp
 * 
 * Implementation of BMP388 driver code 
 * adapted from Adafruit Arduino code for the BMP388
 * 
 * 
 * Written by Kevin Nottberg
 * 
 * BSD license, all text here must be included in any redistribution.
 * 
 */

#include "pico_bmp388.h"

// Hardware interface functions
static int8_t i2c_write(uint8_t reg_addr,
                        const uint8_t* reg_data,
                        uint32_t len,
                        void* intf_ptr);
static int8_t i2c_read(uint8_t reg_addr,
                        uint8_t* reg_data,
                        uint32_t len,
                        void* intf_ptr);

static int8_t spi_write(uint8_t reg_addr,
                        const uint8_t *reg_data,
                        uint32_t len,
                        void* intf_ptr);

static int8_t spi_read(uint8_t reg_addr,
                        uint8_t* reg_data,
                        uint32_t len,
                        void* intf_ptr);

static void delay_usec(uint32_t us, void* intf_ptr);
static int8_t validate_trimming_param(struct bmp3_dev *dev);
static int8_t cal_crc(uint8_t seed, uint8_t data);

/*!
    @brief Instantiates sensor
*/
Pico_BMP388::Pico_BMP388(void)
{
    _meas_end = 0;
    _filter_enabled = _temp_os_enabled = _pres_os_enabled = false;
}

/*!
    @brief Initializes the sensor

    I2C hardware is configured for the device

    @param addr Optional parameter for the I2C address of BMP3.
    @param i2c I2C port
    @param sda_pin GPIO pin that will be used for SDA pin
    @param scl_pin GPIO pin that will be used for SCL pin

    @return True on sensor initialization success. False of failure.
*/
bool Pico_BMP388::begin_I2C(uint8_t addr, 
                            i2c_inst_t* i2c, 
                            uint8_t sda_pin,
                            uint8_t scl_pin)
{
    the_sensor.chip_id = addr;
    the_sensor.intf = BMP3_I2C_INTF;
    the_sensor.read = &i2c_read;
    the_sensor.write = &i2c_write;
    the_sensor.intf_ptr = i2c;
    the_sensor.dummy_byte = 0;

    return _init();
}








/*!
 * @file pico_bmp388.h
 * 
 * Pico SDK implementation for BMP388 temp and barometric pressure sensor
 * 
 * Adapted from Adafruit_BMP3XX library for Arduino
 * 
 * These sensors use I2C or SPI; Only implementing I2C
 * 
 * Written by Kevin Nottberg
 * 
 * BSD license, all text here must be included in any redistribution.
 * 
 */

#ifndef __BMP388_H__
#define __BMP388_H__

#include "bmp3.h"

extern "C" {
    #include "hardware/i2c.h"
}

// Include hardware for I2C and SPI
/*===========================================*/
/*============= I2C Address =================*/
#define BMP388_DEFAULT_ADDRESS (0x77) 
#define BMP388_MULTI_ADDRESS (0x76) // Used for when multiple BMP are on a bus
/*===========================================*/
/*============= SPI SCLK ====================*/
#define BMP388_DEFAULT_SPI_FREQ (1000000) // Default SPI Clock speed

class Pico_BMP388 {
public:
    Pico_BMP388();

    bool begin_I2C(uint8_t i2c_port, uint32_t baudrate,
                    uint8_t SDA_PIN, uint8_t SCL_PIN,
                    uint8_t addr = BMP388_DEFAULT_ADDRESS);

    uint8_t chip_id(void);
    float read_temperature(void);
    float read_pressure(void);
    float read_altitude(float sea_level);

    bool set_temperature_oversampling(uint8_t os);
    bool set_pressure_oversampling(uint8_t os);
    bool set_IIR_filter_coeff(uint8_t fs);
    bool set_output_data_rate(uint8_t odr);

    /// Perform a reading in blocking mode
    bool perform_reading(void);

    double temperature; // Celsius; Assigned after calling perform_reading()
    double pressure; // Pascals; Assigned after calling perform_reading()

private:
    bool _init(void);

    i2c_inst_t* i2c_port;
    uint8_t sda_pin;
    uint8_t scl_pin;

    bool _filter_enabled, _temp_os_enabled, _pres_os_enabled, _odr_enabled;
    uint8_t _i2c_addr;
    int32_t _sensor_id;
    int8_t _cs;
    unsigned long _meas_end;

    uint8_t spixfer(uint8_t x);

    struct bmp3_dev the_sensor;
};

#endif /* pico_bmp333.h */


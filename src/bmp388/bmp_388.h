#ifndef BMP_388_H
#define BMP_388_H

#include "pico/stdlib.h"
#include "driver/bmp3.h"

#define PICO_BMP_I2C_SCL_PIN 21
#define PICO_BMP_I2C_SDA_PIN 20
//#define I2C_DEFAULT i2c_inst_t

class BMP {
    public:
        struct bmp3_dev bmp388;
        BMP();
};

BMP3_INTF_RET_TYPE bmpWrite(uint8_t reg_addr, const uint8_t *read_data, uint32_t len, void *intf_ptr);
BMP3_INTF_RET_TYPE bmpRead(uint8_t reg_addr, uint8_t *read_data, uint32_t len, void *intf_ptr);
BMP3_INTF_RET_TYPE bmpDelay(uint32_t amt, void *intf_ptr);

#endif
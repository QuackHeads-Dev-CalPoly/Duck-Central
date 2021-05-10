
extern "C"
{
    #include "pico/stdlib.h"
    #include "hardware/i2c.h"
    #include <stdio.h>
}

#include "LSM9DS1.h"

/*!
    @brief Initialize I2C interface
    @param i2c_port port number for Pico either 0 or 1
    @param sda_pin GPIO number for I2C SCL pin
    @param scl_pin GPIO number for I2C SDA pin
    @param init_i2c True: I2C modules needs to be configured | False if already configured
*/
Pico_LSM9DS1::Pico_LSM9DS1(uint8_t i2c_port, uint8_t sda_pin,
                            uint8_t scl_pin, uint8_t init_i2c )
{

    (i2c_port) ? _i2c_port = i2c1 : _i2c_port = i2c0;
    _i2c_sda_pin = sda_pin;
    _i2c_scl_pin = scl_pin;
    printf("SCL PIN: %d\n", scl_pin);
    printf("SDA PIN: %d\n", sda_pin);

    if(init_i2c)
    {
        // Configure to run the bus at 400 KHz
        // LSMD9S1 support I2C fast mode
        i2c_init(_i2c_port, 400 * 1000);
        printf("I2C port inited\n");
        gpio_init(_i2c_sda_pin);
        gpio_init(_i2c_scl_pin);
        gpio_set_function(_i2c_sda_pin, GPIO_FUNC_I2C);
        gpio_set_function(_i2c_scl_pin, GPIO_FUNC_I2C);
        gpio_pull_up(_i2c_sda_pin);
        gpio_pull_up(_i2c_scl_pin);       
        printf("I2C setup\n");
        printf("\tSCL Pin: %d\n", _i2c_scl_pin);
        printf("\tSDA Pin: %d\n", _i2c_sda_pin);
    }

    return;
}

/*!
    @brief Initialize I2C sensors and detect if they are on the bus
    @return True if both subsensors are detected on bus
*/
bool Pico_LSM9DS1::begin()
{
    int ret;
    uint8_t rxdata;
    ret = i2c_read_blocking(_i2c_port, LSM9DS1_ADDRESS_MAG, &rxdata, 1, false);
    if(ret < 0)
    {
        printf("Did not find LSM9DS1 on i2c bus\n");
        return false;
    }

    // soft reset & reboot accel/gyro
    write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG8, 0x05);

    sleep_ms(10); // Time to allow for reset

    uint8_t id = read8(XGTYPE, LSM9DS1_REGISTER_WHO_AM_I_XG);
    if( id != LSM9DS1_XG_ID )
    {   
        printf("Wrong XGTYPE ID\n");
        return false;
    }
    printf("XGTYPE ID: 0b%b\n", id);
    id = read8(MAGTYPE, LSM9DS1_REGISTER_WHO_AM_I_XG);
    if( id != 0b00111101 )
    {
        printf("Wrong MAGTYPE ID\n");
        return false;
    }
    printf("MAGTYPE ID: 0b%b\n", id);

    // enable gyro continuous
    write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG1_G, 0xC0);

    // enable accelerometer continuous
    write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG5_XL, 0x38); // Enable x, y, z
    write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG6_XL, 0xC0); // 1 KHz data rate

    // enable mag continuous
    write8(MAGTYPE, LSM9DS1_REGISTER_CTRL_REG8, LIS3MDL_CONTINUOUSMODE);

    setup_accel(LSM9DS1_ACCELRANGE_2G);
    setup_mag(LSM9DS1_MAGGAIN_4GAUSS);
    setup_gyro(LSM9DS1_GYROSCALE_245DPS);
    return true;
}

/*!
    @brief Read all four sensor subcomponents
*/
void Pico_LSM9DS1::read()
{
    /* Read all the sensors. */
    read_accel();
    read_gyro();
    read_temp();
    read_mag();
}

/*!
    @brief Read the sensor magnetometer sensor component
*/
void Pico_LSM9DS1::read_mag()
{
    // TODO: Have to look into the magnetometer code specifically because it relies on that
    // LIS3MDL_REG_OUT_X_L (0x28) => LSM9DS1_REG_OUT_X_L_XL
    uint8_t buffer[6];
    readBuffer(MAGTYPE, 0x80 | LSM9DS1_REGISTER_OUT_X_L_XL, 6, buffer);

    uint8_t xlo = buffer[0];
    int16_t xhi = buffer[1];
    uint8_t ylo = buffer[2];
    int16_t yhi = buffer[3];
    uint8_t zlo = buffer[4];
    int16_t zhi = buffer[5];

    xhi <<= 8;
    xhi |= xlo;
    yhi <<= 8;
    yhi |= ylo;
    zhi <<= 8;
    zhi |= zlo;

    mag_data.x = xhi;
    mag_data.y = yhi;
    mag_data.z = zhi;
}

/*!
    @brief Read the sensor accelerometer sensor component
*/
void Pico_LSM9DS1::read_accel()
{
    // Read the accelerometer
    uint8_t buffer[6];
    readBuffer(XGTYPE, 0x80 | LSM9DS1_REGISTER_OUT_X_L_XL, 6, buffer);

    uint8_t xlo = buffer[0];
    int16_t xhi = buffer[1];
    uint8_t ylo = buffer[2];
    int16_t yhi = buffer[3];
    uint8_t zlo = buffer[4];
    int16_t zhi = buffer[5];

    // Shift values to create properly formed integer (low byte first)
    xhi <<= 8;
    xhi |= xlo;
    yhi <<= 8;
    yhi |= ylo;
    zhi <<= 8;
    zhi |= zlo;

    accel_data.x = xhi;
    accel_data.y = yhi;
    accel_data.z = zhi;
}

/*! 
    @brief Read the sensor gyroscope sensor component
*/
void Pico_LSM9DS1::read_gyro()
{
    // Read gyro
    uint8_t buffer[6];
    readBuffer(XGTYPE, 0x80 | LSM9DS1_REGISTER_OUT_X_L_G, 6, buffer);

    uint8_t xlo = buffer[0];
    int16_t xhi = buffer[1];
    uint8_t ylo = buffer[2];
    int16_t yhi = buffer[3];
    uint8_t zlo = buffer[4];
    int16_t zhi = buffer[5];

    // Shift values to create properly formed integer (low byte first)
    xhi <<= 8;
    xhi |= xlo;
    yhi <<= 8;
    yhi |= ylo;
    zhi <<= 8;
    zhi |= zlo;

    gyro_data.x = xhi;
    gyro_data.y = yhi;
    gyro_data.z = zhi;
}

void Pico_LSM9DS1::read_temp()
{
    uint8_t buffer[2];
    readBuffer(XGTYPE, 0x80 | LSM9DS1_REGISTER_TEMP_OUT_L, 2, buffer);
    uint8_t xlo = buffer[0];
    int16_t xhi = buffer[1];

    xhi <<= 8;
    xhi |= xlo;

    // Shift values to create properly formed integer (low byte first)
    temperature = xhi;
}

/*========================================================
    Setup functions for configuring the sensors
==========================================================*/

/*!
    @brief Configure the accelerometer ranging
    @param range Can be LSM9DS1_ACCELRANGE_2G, LSM9DS1_ACCELRANGE_4G,
    LSM9DS1_ACCELRANGE_8G, LSM9DS1_ACCELRANGE_16G
*/
void Pico_LSM9DS1::setup_accel(lsm9ds1AccelRange_t range)
{
    uint8_t reg = read8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG6_XL);
    reg &= ~(0b00011000);
    reg |= range;
    write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG6_XL, reg);

    switch (range) 
    {
        case LSM9DS1_ACCELRANGE_2G:
            _accel_mg_lsb = LSM9DS1_ACCEL_MG_LSB_2G;
            break;
        case LSM9DS1_ACCELRANGE_4G:
            _accel_mg_lsb = LSM9DS1_ACCEL_MG_LSB_4G;
            break;
        case LSM9DS1_ACCELRANGE_8G:
            _accel_mg_lsb = LSM9DS1_ACCEL_MG_LSB_8G;
            break;
        case LSM9DS1_ACCELRANGE_16G:
            _accel_mg_lsb = LSM9DS1_ACCEL_MG_LSB_16G;
            break;
    }
}

/*! 
    @brief Configure the gyroscope scaling
    @param scale Can be LSM9DS1_GYROSCALE_245DPS, LSM9DS1_GYROSCALE_500DPS,
    or LSM9DS1_GYROSCALE_2000DPS
*/
void Pico_LSM9DS1::setup_gyro(lsm9ds1GyroScale_t scale)
{
    uint8_t reg = read8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG1_G);
    reg &= ~(0b00011000);
    reg |= scale;
    write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG1_G, reg);

    switch(scale)
    {
        case LSM9DS1_GYROSCALE_245DPS:
            _gyro_dps_digit = LSM9DS1_GYRO_DPS_DIGIT_245DPS;
            break;
        case LSM9DS1_GYROSCALE_500DPS:
            _gyro_dps_digit = LSM9DS1_GYRO_DPS_DIGIT_500DPS;
            break;
        case LSM9DS1_GYROSCALE_2000DPS:
            _gyro_dps_digit = LSM9DS1_GYRO_DPS_DIGIT_2000DPS;
            break;
    }
}

/*!
    @brief Configure teh magnetometer gain
    @param gain Can be LSM9DS1_MAGGAIN_4GAUSS, LSM9DS1_MAGGAIN_8GAUSS,
            LSM9DS1_MAGGAIN_12GAUSS, LSM9DS1_MAGGAIN_16GAUSS
*/
void Pico_LSM9DS1::setup_mag(lsm9ds1MagGain_t gain)
{
    // Need to write to the LIS3MDL_REG_CTRL_REG2 register
    // LIS3MDL_REG_CTRL_REG2 (0x21) => LSM9DS1_REGISTER_CTRL_REG7_XL (0x21)
    uint8_t reg = read8(MAGTYPE, LSM9DS1_REGISTER_CTRL_REG7_XL);
    reg &= ~(0b01100000);
    reg |= gain;
    write8(MAGTYPE, LSM9DS1_REGISTER_CTRL_REG7_XL, reg);

    _mag_gain = gain;
}

/*========================================================
    STDOUT functions to scale and output sensor readings
==========================================================*/
void Pico_LSM9DS1::print_accel_data_raw()
{
    printf("Accel X: %d\n", accel_data.x);
    printf("Accel Y: %d\n", accel_data.y);
    printf("Accel Z: %d\n", accel_data.z);
}

void Pico_LSM9DS1::print_accel_data()
{
    float acceleration_x, acceleration_y, acceleration_z;

    acceleration_x = accel_data.x * _accel_mg_lsb;
    acceleration_x /= 1000;
    acceleration_x *= SENSORS_GRAVITY_STANDARD;
    acceleration_y = accel_data.y * _accel_mg_lsb;
    acceleration_y /= 1000;
    acceleration_y *= SENSORS_GRAVITY_STANDARD;
    acceleration_z = accel_data.z * _accel_mg_lsb;
    acceleration_z /= 1000;
    acceleration_z *= SENSORS_GRAVITY_STANDARD;

    printf("Accel X: %.6f\n", acceleration_x);
    printf("Accel Y: %.6f\n", acceleration_y);
    printf("Accel Z: %.6f\n", acceleration_z);
}

void Pico_LSM9DS1::print_gyro_data_raw()
{
    printf("Gyro X: %.6f\n", gyro_data.x);
    printf("Gyro Y: %.6f\n", gyro_data.y);
    printf("Gyro Z: %.6f\n", gyro_data.z);
}

void Pico_LSM9DS1::print_gyro_data()
{
    float gyro_x, gyro_y, gyro_z;
    gyro_x = gyro_data.x * _gyro_dps_digit * SENSORS_DPS_TO_RADS;
    gyro_y = gyro_data.y * _gyro_dps_digit * SENSORS_DPS_TO_RADS;
    gyro_z = gyro_data.z * _gyro_dps_digit * SENSORS_DPS_TO_RADS;

    printf("Gyro X: %.6f\n", gyro_x);
    printf("Gyro Y: %.6f\n", gyro_y);
    printf("Gyro Z: %.6f\n", gyro_z);
}

void Pico_LSM9DS1::print_mag_data_raw()
{
    printf("Mag X: %.6f\n", mag_data.x);
    printf("Mag Y: %.6f\n", mag_data.y);
    printf("Mag Z: %.6f\n", mag_data.z);
}

void Pico_LSM9DS1::print_mag_data()
{
    float mag_x, mag_y, mag_z;
    float scale = 1;
    if(_mag_gain == LSM9DS1_MAGGAIN_4GAUSS)
        scale = 1711;
    else if(_mag_gain == LSM9DS1_MAGGAIN_8GAUSS)
        scale = 2281;
    else if(_mag_gain == LSM9DS1_MAGGAIN_12GAUSS)
        scale = 3421;
    else if(_mag_gain == LSM9DS1_MAGGAIN_16GAUSS)
        scale = 6842;

    mag_x = (float)mag_data.x / scale;
    mag_y = (float)mag_data.y / scale;
    mag_z = (float)mag_data.z / scale;

    printf("Mag X: %.6f\n", mag_x);
    printf("Mag Y: %.6f\n", mag_y);
    printf("Mag Z: %.6f\n", mag_z);
}

// Temperature readings do not work currently
void Pico_LSM9DS1::print_temp_data_raw()
{
    printf("Temp raw: %d\n", temperature);
}

void Pico_LSM9DS1::print_temp_data()
{
    float temp;
    temp = (temperature / 16) + 27.5;

    printf("Temp: %.6f \370C\n", temp);
}
/*========================================================
    Private functions for I2C bus interfacing
==========================================================*/
void Pico_LSM9DS1::write8(bool type, uint8_t reg, uint8_t value)
{
    uint8_t address;   
    if (type == MAGTYPE) {
        address = LSM9DS1_ADDRESS_MAG;
    } else {
        address = LSM9DS1_ADDRESS_ACCELGYRO;
    }

    uint8_t write_buffer[2];
    write_buffer[0] = reg;
    write_buffer[1] = value;

    int ret;
    // No stop needs to be false to release I2C bus once done ending transmission
    //ret = i2c_write_blocking(_i2c_port, address, &reg, 1, true);
    //if(ret != 1)
    //    printf("Failed to write location reg: 0x%x\n", reg);
    //ret = i2c_write_blocking(_i2c_port, address, &value, 1, false);
    //if(ret != 1)
    //    printf("Failed to write data to register: 0x%x\n", value);
    ret = i2c_write_blocking(_i2c_port, address, write_buffer, 2, false);
    if( ret != 2 )
        printf("Failed to write data (0x%x) to reg: %x\n", write_buffer[0], write_buffer[1]);
}

uint8_t Pico_LSM9DS1::read8(bool type, uint8_t reg)
{
    uint8_t value;
    readBuffer(type, reg, 1, &value);

    return value;
}

uint8_t Pico_LSM9DS1::readBuffer(bool type, uint8_t reg,
                                    uint8_t len, uint8_t* buffer) 
{
    uint8_t address;
    if(type == MAGTYPE)
        address = LSM9DS1_ADDRESS_MAG;
    else
        address = LSM9DS1_ADDRESS_ACCELGYRO;

    int ret;
    ret = i2c_write_blocking(_i2c_port, address, &reg, 1, true);
    ret = i2c_read_blocking(_i2c_port, address, buffer, len, false);

    if( ret == len )
        return ret;
    else
    {
        printf("Failed to read %d bytes from LSMD9S1. Error: %d\n", 
                len, ret);
        return -1;
    }
}
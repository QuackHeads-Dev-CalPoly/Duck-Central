#ifndef __LSM9DS1_H__
#define __LSM9DS1_H__

extern "C"
{
    #include "hardware/i2c.h"
    #include <stdint.h>
}

/*=================================================
        Sensor constants 
===================================================*/
#define SENSORS_GRAVITY_EARTH (9.80665F)
#define SENSORS_GRAVITY_MOON (1.6F)
#define SENSORS_GRAVITY_SUN (275.0F)
#define SENSORS_GRAVITY_STANDARD (SENSORS_GRAVITY_EARTH)

/* Maximum magnetic field on Earth's surface */
#define SENSORS_MAGFIELD_EARTH_MAX (60.0F)
#define SENSORS_MAGFIELD_EARTH_MIN (30.0F)

/* Average sea level pressure is 1013.25 hPa */
#define SENSORS_PRESSURE_SEALEVELHPA (1013.25F)

/* Degrees to rad/s scaler */
#define SENSORS_DPS_TO_RADS (0.017453293F)
#define SENSORS_RADS_TO_DPS (57.29566693F)

/* Gauss to micro-Tesla scaler */
#define SENSORS_GAUSS_TO_MICROTESLA (100)

/*================================================
        LSM9DS1 specificc constants
==================================================*/
#define LSM9DS1_ADDRESS_ACCELGYRO (0x6B)
#define LSM9DS1_ADDRESS_MAG (0x1E)
#define LSM9DS1_XG_ID (0b01101000)

// Linear Acceleration: mg per LSB
#define LSM9DS1_ACCEL_MG_LSB_2G (0.061F)
#define LSM9DS1_ACCEL_MG_LSB_4G (0.122F)
#define LSM9DS1_ACCEL_MG_LSB_8G (0.244F)
#define LSM9DS1_ACCEL_MG_LSB_16G (0.732F)

// Magnetic Field Strength: gauss range
#define LSM9DS1_MAG_MGAUSS_4GAUSS (0.14F)
#define LSM9DS1_MAG_MGAUSS_8GAUSS (0.29F)
#define LSM9DS1_MAG_MGAUSS_12GAUSS (0.43F)
#define LSM9DS1_MAG_MGAUSS_16GAUSS (0.58F)

// Angular Rate: dps per LBS
#define LSM9DS1_GYRO_DPS_DIGIT_245DPS (0.00875F)
#define LSM9DS1_GYRO_DPS_DIGIT_500DPS (0.01750F)
#define LSM9DS1_GYRO_DPS_DIGIT_2000DPS (0.07000F)

// Temperature: LSB per degree celsius
#define LSM9DS1_TEMP_LSB_DEGREE_CELSIUS (8) // 1 deg C = 8, 25 deg C = 200

#define MAGTYPE (true)
#define XGTYPE (false)

// Reference required for function pointers
class Pico_LSM9DS1;

/* Pointer to member functions for read, get event, and get sensor. These
 * are used by to read and rerieve individual sensors */

class Pico_LSM9DS1 {
    public:
        Pico_LSM9DS1(uint8_t i2c_port, 
                            uint8_t scl, uint8_t sda, 
                            uint8_t init_i2c);
        
        /**! Register mapping for accel/gyro component */
        typedef enum {
            LSM9DS1_REGISTER_WHO_AM_I_XG = 0x0F,
            LSM9DS1_REGISTER_CTRL_REG1_G = 0x10,
            LSM9DS1_REGISTER_CTRL_REG2_G = 0x11,
            LSM9DS1_REGISTER_CTRL_REG3_G = 0x12,
            LSM9DS1_REGISTER_TEMP_OUT_L = 0x15,
            LSM9DS1_REGISTER_TEMP_OUT_H = 0x16,
            LSM9DS1_REGISTER_STATUS_REG = 0x17,
            LSM9DS1_REGISTER_OUT_X_L_G = 0x18,
            LSM9DS1_REGISTER_OUT_X_H_G = 0x19,
            LSM9DS1_REGISTER_OUT_Y_L_G = 0x1A,
            LSM9DS1_REGISTER_OUT_Y_H_G = 0x1B,
            LSM9DS1_REGISTER_OUT_Z_L_G = 0x1C,
            LSM9DS1_REGISTER_OUT_Z_H_G = 0x1D,
            LSM9DS1_REGISTER_CTRL_REG4 = 0x1E,
            LSM9DS1_REGISTER_CTRL_REG5_XL = 0x1F,
            LSM9DS1_REGISTER_CTRL_REG6_XL = 0x20,
            LSM9DS1_REGISTER_CTRL_REG7_XL = 0x21,
            LSM9DS1_REGISTER_CTRL_REG8 = 0x22,
            LSM9DS1_REGISTER_CTRL_REG9 = 0x23,
            LSM9DS1_REGISTER_CTRL_REG10 = 0x24,

            LSM9DS1_REGISTER_OUT_X_L_XL = 0x28,
            LSM9DS1_REGISTER_OUT_X_H_XL = 0x29,
            LSM9DS1_REGISTER_OUT_Y_L_XL = 0x2A,
            LSM9DS1_REGISTER_OUT_Y_H_XL = 0x2B,
            LSM9DS1_REGISTER_OUT_Z_L_XL = 0x2C,
            LSM9DS1_REGISTER_OUT_Z_H_XL = 0x2D,
        } lsm9ds1AccGyroRegisters_t;

        /**! Enumeration for accelerometer range (2/4/8/16 g) */
        typedef enum {
            LSM9DS1_ACCELRANGE_2G = (0b00 << 3),
            LSM9DS1_ACCELRANGE_16G = (0b01 << 3),
            LSM9DS1_ACCELRANGE_4G = (0b10 << 3),
            LSM9DS1_ACCELRANGE_8G = (0b11 << 3),
        } lsm9ds1AccelRange_t;

        /**! Enumeration for accelerometer data range 3.125 - 1600 Hz */
        typedef enum {
            LSM9DS1_ACCELDATARATE_POWERDWON = (0b0000 << 4),
            LSM9DS1_ACCELDATARATE_3_125HZ = (0b0001 << 4),
            LSM9DS1_ACCELDATARATE_6_25HZ = (0b0010 << 4),
            LSM9DS1_ACCELDATARATE_25HZ = (0b0011 << 4),
            LSM9DS1_ACCELDATARATE_50HZ = (0b0100 << 4),
            LSM9DS1_ACCELDATARATE_100HZ = (0b0101 << 4),
            LSM9DS1_ACCELDATARATE_200HZ = (0b0110 << 4),
            LSM9DS1_ACCELDATARATE_400HZ = (0b0111 << 4),
            LSM9DS1_ACCELDATARATE_800HZ = (0b1000 << 4),
            LSM9DS1_ACCELDATARATE_1600HZ = (0b1001 << 4),    
        } lsm9ds1AccelDataRate_t;

        /**! Enumeration for gyroscope scaling (245/500/2000 dps) */
        typedef enum {
            LSM9DS1_GYROSCALE_245DPS = 
                (0b00 << 3), // +/- 245 per second rotation
            LSM9DS1_GYROSCALE_500DPS =
                (0b01 << 3), // +/- 500 degress per second rotation
            LSM9DS1_GYROSCALE_2000DPS =
                (0b11 << 3), // +/- 2000 degrees per second rotation
        } lsm9ds1GyroScale_t;

        /**! Enueration for magnetometer scaling (4/8/12/16 gauss) */
        typedef enum {
            LSM9DS1_MAGGAIN_4GAUSS = (0b00 << 5),   // +/- 4 gauss
            LSM9DS1_MAGGAIN_8GAUSS = (0b01 << 5),   // +/- 8 gauss
            LSM9DS1_MAGGAIN_12GAUSS = (0b10 << 5),  // +/- 12 gauss
            LSM9DS1_MAGGAIN_16GAUSS = (0b11 << 5)   // +/- 16 gauss 
        } lsm9ds1MagGain_t;

        /** The magnetometer data rate, includes FAST_ODR bit */
        typedef enum {
            LIS3MDL_DATARATE_0_625_HZ = 0b0000, ///<  0.625 Hz
            LIS3MDL_DATARATE_1_25_HZ = 0b0010,  ///<  1.25 Hz
            LIS3MDL_DATARATE_2_5_HZ = 0b0100,   ///<  2.5 Hz
            LIS3MDL_DATARATE_5_HZ = 0b0110,     ///<  5 Hz
            LIS3MDL_DATARATE_10_HZ = 0b1000,    ///<  10 Hz
            LIS3MDL_DATARATE_20_HZ = 0b1010,    ///<  20 Hz
            LIS3MDL_DATARATE_40_HZ = 0b1100,    ///<  40 Hz
            LIS3MDL_DATARATE_80_HZ = 0b1110,    ///<  80 Hz
            LIS3MDL_DATARATE_155_HZ = 0b0001,   ///<  155 Hz (FAST_ODR + UHP)
            LIS3MDL_DATARATE_300_HZ = 0b0011,   ///<  300 Hz (FAST_ODR + HP)
            LIS3MDL_DATARATE_560_HZ = 0b0101,   ///<  560 Hz (FAST_ODR + MP)
            LIS3MDL_DATARATE_1000_HZ = 0b0111,  ///<  1000 Hz (FAST_ODR + LP)
        } lis3mdl_dataRate_t;

        /** The magnetometer performance mode */
        typedef enum {
            LIS3MDL_LOWPOWERMODE = 0b00,  ///< Low power mode
            LIS3MDL_MEDIUMMODE = 0b01,    ///< Medium performance mode
            LIS3MDL_HIGHMODE = 0b10,      ///< High performance mode
            LIS3MDL_ULTRAHIGHMODE = 0b11, ///< Ultra-high performance mode
        } lis3mdl_performancemode_t;

        /** The magnetometer operation mode */
        typedef enum {
            LIS3MDL_CONTINUOUSMODE = 0b00, ///< Continuous conversion
            LIS3MDL_SINGLEMODE = 0b01,     ///< Single-shot conversion
            LIS3MDL_POWERDOWNMODE = 0b11,  ///< Powered-down mode
        } lis3mdl_operationmode_t;

        /**! 3D floating point vector with X Y Z components */
        typedef struct vector_s {
            float x; ///< X component
            float y; ///< Y component
            float z; ///< Z component
        } lsm9ds1Vector_t;

        lsm9ds1Vector_t accel_data; ///< Last read accelerometer data will be available here
        lsm9ds1Vector_t gyro_data; ///< Last read gyroscope data will be available here
        lsm9ds1Vector_t mag_data; ///< Last read magnetometer data will be available here
        int16_t temperature; ///< Last read temperature data will be available here

        bool begin(void);
        void read(void);
        void read_accel(void);
        void read_gyro(void);
        void read_mag(void);
        void read_temp(void);

        void setup_accel(lsm9ds1AccelRange_t range);
        void setup_mag(lsm9ds1MagGain_t gain);
        void setup_gyro(lsm9ds1GyroScale_t scale);

        // STDOUT functions
        void print_accel_data_raw();
        void print_accel_data();
        void print_gyro_data_raw();
        void print_gyro_data();
        void print_mag_data_raw();
        void print_mag_data();

        // Temperature sensing is meant to keep track of Gyro Die temp
        // not necessarily used as a enviromental temperature sensor
        void print_temp_data_raw();
        void print_temp_data();

    private:
        void write8(bool type, uint8_t reg, uint8_t value);
        uint8_t read8(bool type, uint8_t reg);
        uint8_t readBuffer(bool type, uint8_t reg, uint8_t len, uint8_t *buffer);

        // Used for data readings
        float _accel_mg_lsb;
        float _gyro_dps_digit;
        float _mag_gain;

        // TODO: probably can remove _i2c_scl_pin and _i2c_sda_pin 
        // They are unused. Need to keep around _i2c_port
        uint8_t _i2c_sda_pin;
        uint8_t _i2c_scl_pin;
        i2c_inst_t* _i2c_port;
};

#endif
extern "C"
{
    #include "pico/stdlib.h"
    #include "hardware/i2c.h"
    #include <stdio.h>
}

#define I2C_SDA_PIN 20
#define I2C_SCL_PIN 21
#define I2C_PORT i2c0;

#define BMP388_ADDR 0x76
#define BMX160_ADDR 0x68
#define MCP9808_ADDR 0x18

int main()
{
    stdio_init_all();
    printf("Hello\n");
    // Configure I2C pins
    i2c_init(i2c0, 400 * 1000);
    printf("I2C port inited\n");
    gpio_init(I2C_SCL_PIN);
    gpio_init(I2C_SDA_PIN);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    sleep_ms(1000);
    printf("Starting to ping i2c bus");

    while(1)
    {
        int ret;
        uint8_t rxdata;
        ret = i2c_read_blocking(i2c0, BMP388_ADDR, &rxdata, 1, false);
        if( ret < 0 )
            printf( "Did not find BMP388 on I2C bus\n" );
        else
            printf( "FOUND BMP388!!!\n" );

        ret = i2c_read_blocking(i2c0, BMP388_ADDR + 1, &rxdata, 1, false);
        if( ret < 0 )
            printf( "Did not find alternate BMP388 address\n");
        else
            printf( "Found BMP388 alternate address\n" );

        sleep_ms(5000); // Chill for 5 seconds

        ret = i2c_read_blocking(i2c0, BMX160_ADDR, &rxdata, 1, false);
        if( ret < 0 )
            printf( "Did not find BMX160\n" );
        else
            printf( "FOUND BMX160!!!!!!\n" );

        ret = i2c_read_blocking(i2c0, BMX160_ADDR + 1, &rxdata, 1, false);
        if( ret < 0 )
            printf( "Did not find alternate BMX160 address\n");
        else
            printf( "Found BMX160 alternate address\n" );

        sleep_ms(5000);

        ret = i2c_read_blocking(i2c0, MCP9808_ADDR, &rxdata, 1, false);
        if( ret < 0)
            printf( "Did not find MXP9808\n" );
        else
            printf( "FOUND MCP9808!!!\n" );

        ret = i2c_read_blocking(i2c0, MCP9808_ADDR + 1, &rxdata, 1, false);
        if( ret < 0)
            printf( "Did not find alternate MXP9808 address\n" );
        else
            printf( "Found MCP9808 alternate address\n" );

        sleep_ms(5000);
    }

    return 0;
}


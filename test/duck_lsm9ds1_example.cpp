extern "C"
{
    #include "pico/stdio.h"
    #include "pico/stdlib.h"
    #include "hardware/gpio.h"
}

#include <stdio.h>
#include "LSM9DS1.h"

#define LED_PIN 25

#define SCL_PIN 21
#define SDA_PIN 20

int main()
{
    stdio_init_all();
    printf("LED ON\n");
    printf("STDIO init\n");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    uint8_t led_val = 1;
    gpio_put(LED_PIN, led_val);
    sleep_ms(10 * 1000);

    Pico_LSM9DS1 lsm = Pico_LSM9DS1(0, SDA_PIN, SCL_PIN, 1);

    printf("Out of lsm\n");
    sleep_ms(10 * 1000);
    if(!lsm.begin())
    {
        printf("Oops, no LSM9DS1 detected ... (check your wiring!");
    }
    else
    {
        printf("Successfully found the device LSM9DS1");
    }

    while(true)
    {
        lsm.read_accel();
        lsm.read_gyro();
        lsm.read_mag();
        lsm.read_temp();
        //lsm.print_accel_data();
        //lsm.print_gyro_data();
        //lsm.print_temp_data();
        lsm.print_mag_data();
        sleep_ms(50);
    }
}
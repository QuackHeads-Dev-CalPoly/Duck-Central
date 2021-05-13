#include "bmx_160.h"
#include "pico/stdlib.h"

#define INT_1_PIN 6
#define LED_PIN 25
#define BMX160_PIN_SCK 21
#define BMX160_PIN_SDA 20

void setup_led(void);

void popop(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio, events);
    gpio_set_irq_enabled(gpio, events, false);

    gpio_put(LED_PIN, 1);

    printf("pop!!\n");
    fflush(stdout);

    gpio_set_irq_enabled(gpio, events, true);
}

int main() {
    stdio_init_all();

    gpio_init(INT_1_PIN);
    gpio_set_dir(INT_1_PIN, GPIO_IN);

    sleep_ms(5000);

    BMX160 bmx160(i2c0, 4000, BMX160_PIN_SCK, BMX160_PIN_SDA);

    if (bmx160.begin() != true) {
        printf("init was false\n");
    }

    gpio_set_irq_enabled_with_callback(INT_1_PIN, GPIO_IRQ_EDGE_RISE, true, &popop);

    bmx160.enable_low_g_interrupt();

    //bmx160.set_gyro_range(eGyroRange_500DPS);

    //bmx160.set_accel_range(eAccelRange_4G);

    bmx160SensorData magnetometer, gyroscope, accelerometer;

    while(1) {
        /* Get a new sensor event */
        bmx160.get_all_data(&magnetometer, &gyroscope, &accelerometer);
    
        /* Display the magnetometer results (magn is magnetometer in uTesla) */
        printf("Magnetometer (uT):\n");
        printf("\tX: %d\tY: %d\tZ: %d\n", magnetometer.x, magnetometer.y, magnetometer.z);
    
        /* Display the gyroscope results (gyroscope data is in g) */
        printf("Gyroscope (g):\n");
        printf("\tX: %d\tY: %d\tZ: %d\n", gyroscope.x, gyroscope.y, gyroscope.z);
        
        /* Display the accelerometer results (accelerometer data is in m/s^2) */
        printf("Acxcelerometer (m/s/s):\n");
        printf("\tX: %d\tY: %d\tZ: %d\n", accelerometer.x, accelerometer.y, accelerometer.z);

        printf("\n\n");
    
        sleep_ms(1000);
    }
    
    return 0;
}

void setup_led(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // pull low initially
    gpio_put(LED_PIN, 0);
}
#include "bmp_388.h"
#include <stdio.h>
#include "pico/stdlib.h"

#define BMP388_ADDRESS 0x77

int main() {
    stdio_init_all();
    sleep_ms(3000);
    printf("Starting\n");
    fflush(stdout);

    BMP388 bmpsensor = BMP388(BMP388_ADDRESS);

    int i = 0;
    while (i < 50) {
        bmpsensor.perform_reading();
        printf("After reading, pressure is: %lf and Temp is %lf\n", bmpsensor.get_pressure(), bmpsensor.get_temperature());
        printf("Altitude in meters: %lf\n", bmpsensor.get_altitude());
        fflush(stdout);
        sleep_ms(2000);
    }
    

    printf("Finished\n");

    return 0;
}
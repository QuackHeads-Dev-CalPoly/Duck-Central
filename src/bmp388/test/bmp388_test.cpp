#include "bmp_388.h"
#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    sleep_ms(3000);
    printf("Starting\n");
    fflush(stdout);

    BMP bmpsensor = BMP();

    int i = 0;
    while (i < 50) {
        bmpsensor.performReading();
        printf("After reading, pressure is: %lf and Temp is %lf\n", bmpsensor.getPressure(), bmpsensor.getTemperature());
        printf("Altitude in meters: %lf\n", bmpsensor.getAltitude());
        fflush(stdout);
        sleep_ms(2000);
    }
    

    printf("Finished\n");

    return 0;
}
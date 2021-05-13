#include "bmp_388.h"
#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    sleep_ms(3000);
    printf("Starting\n");
    fflush(stdout);

    gpio_init(0);
    gpio_pull_down(0);      // Pull down the pin to keep off
    gpio_set_dir(0, true);  // The pin to be output
    gpio_put(0, 1);         // Enable GPS

    BMP bmpsensor = BMP();

    int i = 0;
    while (i < 50) {
        bmpsensor.performReading();
        printf("After reading, pressure is: %lf and Temp is %lf\n", bmpsensor.getPressure(), bmpsensor.getTemperature());
        fflush(stdout);
        sleep_ms(2000);
    }
    

    printf("Finished\n");

    return 0;
}
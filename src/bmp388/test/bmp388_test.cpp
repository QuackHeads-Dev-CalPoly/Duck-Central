#include "bmp_388.h"
#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    sleep_ms(3000);
    printf("Starting\n");
    fflush(stdout);

    BMP bmpsensor = BMP();

    printf("Finished\n");

    return 0;
}
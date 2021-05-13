extern "C"
{
    #include "pico/stdio.h"
    #include "pico/stdlib.h"
    #include <stdio.h>
}

#include "sd_card.h"

int main()
{
    stdio_init_all();
    sleep_ms(2 * 1000);
    printf("Turned on stdio to stdout\n");
    printf("Sleeping for 10 seconds");
    sleep_ms(10 * 1000);

    sd_card my_sd = sd_card();

    printf("Initing SD card\n");
    my_sd.init_sd_card();
    printf("Done\n");
    return 0;
}
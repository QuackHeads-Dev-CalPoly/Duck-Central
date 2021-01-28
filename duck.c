#include <stdio.h>
#include "pico/stdlib.h"
#include "gps.h"

#define LED_PIN         25

void setup_led(void);
void setup_gps(void);
void greet(void);

int main() {
    stdio_init_all();

    setup_led();

    setup_gps();

    greet();

    while(1) {
        read_gps_full();
        sleep_ms(2000);
    }

    return 0;
}

void setup_led(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // pull low initially
    gpio_put(LED_PIN, 0);
}



void greet(void) {
    // debug pin to know we are up and running
    gpio_put(LED_PIN, 1);
    sleep_ms(1000);

    printf("mama online\n");

    gpio_put(LED_PIN, 0);
    sleep_ms(1000);
}


#include <stdio.h>
#include <string.h>

#include "log.h"
#include "gps.h"
#include "pico/stdlib.h"

#define LED_PIN 25

void setup_led(void);

int main() {
    stdio_init_all();

    setup_led();

    gpio_init(0);
    gpio_pull_down(0);      // Pull down the pin to keep off
    gpio_set_dir(0, true);  // The pin to be output
    gpio_put(0, 1);         // Enable GPS

    sleep_ms(5000);

    GPS gps;

    printf("initialized GPS\n");

    while (1) {
        printf("[%f,%f,%d:%d:%d]\n", 
            gps.get_latitude(),
            gps.get_longitude(), 
            gps.get_hours(), 
            gps.get_minutes(), 
            gps.get_seconds());
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
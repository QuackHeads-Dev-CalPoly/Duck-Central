#include <stdio.h>
#include <string.h>

#include "gps.h"
#include "lora.h"
#include "log.h"
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

    gpio_init(2);
    gpio_pull_down(2);          // Pull down the pin to keep off
    gpio_set_dir(2, GPIO_OUT);  // The pin to be output
    gpio_put(2, 1);             // Enable LoRa

    sleep_ms(5000);

    GPS gps;
    Lora lora;

    printf("initialized GPS\n");

    int sequence_num = 0;
    while (1) {
        uint8_t buffer[255] = {};

        sprintf((char*) buffer, "%d,%f,%f,%d:%d:%d\0", 
            sequence_num,
            gps.get_latitude(),
            gps.get_longitude(), 
            gps.get_hours(), 
            gps.get_minutes(),
            gps.get_seconds());

        gpio_put(LED_PIN, 1);

        printf("sending message: `%s`\n", buffer);
        lora.transmit(buffer, strlen((char*)buffer) + 1);

        sleep_ms(250);

        gpio_put(LED_PIN, 0);
        sequence_num++;

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
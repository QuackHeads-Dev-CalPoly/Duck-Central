#include <stdio.h>
#include <string.h>

#include "lora.h"
#include "pico/stdlib.h"

#define LED_PIN 25

void setup_led(void);

int main() {
    stdio_init_all();

    setup_led();

    sleep_ms(5000);
    gpio_put(LED_PIN, 1);

    printf("\nsetting up lora...\n");
    fflush(stdout);
    sleep_ms(100);

    lora_setup(LOGGING_VERBOSE);

    lora_set_op_mode(OPMODE_RX_CONT);

    while (1) {
    }

    return 0;
}

void setup_led(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // pull low initially
    gpio_put(LED_PIN, 0);
}
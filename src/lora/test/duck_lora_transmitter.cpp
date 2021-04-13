#include <stdio.h>
#include <string.h>

#include "log.h"
#include "lora.h"
#include "pico/stdlib.h"

#define LED_PIN 25

void setup_led(void);
void on_receive(LoraPayload payload);
void on_transmit(void);

static Lora lora;

int main() {
    stdio_init_all();

    setup_led();

    sleep_ms(5000);

    int i = 0;
    while (1) {
        uint8_t buffer[255] = {};
        sprintf((char*) buffer, "message #%d", i);

        sleep_ms(2000);

        gpio_put(LED_PIN, 1);

        printf("sending message: `%s`\n", buffer);
        lora.transmit(buffer, strlen((char*) buffer));

        sleep_ms(250);

        gpio_put(LED_PIN, 0);
        i++;
    }

    return 0;
}

void setup_led(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // pull low initially
    gpio_put(LED_PIN, 0);
}
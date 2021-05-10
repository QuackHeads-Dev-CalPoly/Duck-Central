#include <stdio.h>
#include <string.h>

#include "log.h"
#include "lora.h"
#include "pico/stdlib.h"

#define LED_PIN 25

void setup_led(void);
void on_receive(LoraPayload payload);
void on_transmit(void);


int main() {
    stdio_init_all();

    setup_led();

    gpio_init(2);
    gpio_pull_down(2);      // Pull down the pin to keep off
    gpio_set_dir(2, GPIO_OUT);  // The pin to be output
    gpio_put(2, 1);         // Enable LoRa

    // wait for user to connect to serial port
    sleep_ms(5000);

    // NOTE: MUST NOT BE GLOBAL. SPI TO THE LORA MODULE NEEDS TO BE TURNED ON FIRST.
    Lora lora;

    int i = 0;
    while (1) {
        uint8_t buffer[255] = {};
        sprintf((char*) buffer, "%d\0", i);

        sleep_ms(2000);

        gpio_put(LED_PIN, 1);

        printf("sending message: `%s`\n", buffer);
        lora.transmit(buffer, strlen((char*) buffer)+1);

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
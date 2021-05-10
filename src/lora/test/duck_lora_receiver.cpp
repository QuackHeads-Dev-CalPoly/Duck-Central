#include <stdio.h>
#include <string.h>

#include "lora.h"
#include "pico/stdlib.h"
#include "log.h"

#define LED_PIN 25

void setup_led(void);
void on_receive(LoraPayload payload);
void on_transmit(void);

int main() {
    stdio_init_all();

    setup_led();

    gpio_init(2);
    gpio_pull_down(2);      // Pull down the pin to keep off
    gpio_set_dir(2, true);  // The pin to be output
    gpio_put(2, 1);         // Enable LoRa

    sleep_ms(5000);

    Lora lora;

    printf("SNR, RSSI\n");

    lora.set_receive_callback(on_receive);

    lora.startReceive();

    while (1) {
        tight_loop_contents();
    }

    return 0;
}

void on_receive(LoraPayload payload) {
    gpio_put(LED_PIN, 1);
    printf("%s, %f, %f\n", payload.payload, payload.SNR, payload.RSSI);
    fflush(stdout);
    gpio_put(LED_PIN, 0);
}

void setup_led(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // pull low initially
    gpio_put(LED_PIN, 0);
}
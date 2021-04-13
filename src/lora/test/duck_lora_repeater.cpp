#include <stdio.h>
#include <string.h>

#include "lora.h"
#include "pico/stdlib.h"
#include "log.h"

#define LED_PIN 25

void setup_led(void);
void on_receive(LoraPayload payload);
void on_transmit(void);

static Lora lora;

int main() {
    stdio_init_all();

    setup_led();

    sleep_ms(5000);
    gpio_put(LED_PIN, 1);

    lora.set_receive_callback(on_receive);
    lora.set_transmit_callback(on_transmit);

    lora.startReceive();

    while (1) {
        tight_loop_contents();
    }

    return 0;
}

void on_receive(LoraPayload payload) {
    printf("Received.\n\tSNR: %f\n", payload.SNR);
    printf("\tRSSI: %f\n", payload.RSSI);
    // repeat the payload
    lora.transmit(payload.payload, payload.length);
}

void on_transmit() {
    // set back to RX mode so we can hear more packets once we're done repeating the first.
    lora.startReceive();
}

void setup_led(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // pull low initially
    gpio_put(LED_PIN, 0);
}
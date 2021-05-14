#include <stdio.h>
#include <string.h>

#include "log.h"
#include "lora.h"
#include "pico/stdlib.h"
#include "power_control.h"

#define LED_PIN 25

void setup_led(void);
void on_receive(LoraPayload payload);
void on_transmit(void);

Lora* lora;

int main() {
    stdio_init_all();

    setup_led();

    PowerControl power_control = PowerControl();
    power_control.turn_on_lora();

    sleep_ms(5000);

    lora = new Lora();

    printf("SNR, RSSI\n");

    lora->set_receive_callback(on_receive);
    lora->set_transmit_callback(on_transmit);

    lora->startReceive();

    while (1) {
        tight_loop_contents();
    }

    return 0;
}

void on_receive(LoraPayload payload) {
    gpio_put(LED_PIN, 1);
    for (int i = 0; i < payload.length; i++) {
        printf("%c", payload.payload[i]);
    }
    printf("  {{{%f, %f}}}\n", payload.payload, payload.SNR, payload.RSSI);
    fflush(stdout);

    lora->transmit(payload.payload, payload.length);

    gpio_put(LED_PIN, 0);
}

void on_transmit() {
    printf("repeated succesfully.\n");
    gpio_put(LED_PIN, 0);
    lora->startReceive();
}

void setup_led(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // pull low initially
    gpio_put(LED_PIN, 0);
}
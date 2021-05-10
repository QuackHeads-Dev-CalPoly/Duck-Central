extern "C"
{
    #include <stdio.h>
    #include "pico/stdlib.h"
}

#include "pwr_controller.h"

int main() {
    stdio_init_all();
    sleep_ms(4000);
    printf("Hello world!\n");

    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    
    Pwr_Cntrl power_controller = Pwr_Cntrl();
    printf("Turning on 5V in 5 secs...");
    sleep_ms(1000);
    printf("4...");
    sleep_ms(1000);
    printf("3...");
    sleep_ms(1000);
    printf("2...");
    sleep_ms(1000);
    printf("1...");
    sleep_ms(1000);
    power_controller.turn_on_5v_pwr();
    printf("5V is now on\n");
    printf("Turning on LoRa in 5 secs...");
    sleep_ms(1000);
    printf("4...");
    sleep_ms(1000);
    printf("3...");
    sleep_ms(1000);
    printf("2...");
    sleep_ms(1000);
    printf("1...");
    sleep_ms(1000);
    power_controller.turn_on_lora();
    printf("LORA is now on\n");

    while (true) {
        sleep_ms(5000);
    }
}
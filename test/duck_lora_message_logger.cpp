extern "C"
{
    #include <string.h>
    #include <stdio.h>

    #include "lora.h"
    #include "pico/stdlib.h"
}

#define LED_PIN 25

void setup_led(void);

int main()
{
    stdio_init_all();

    setup_led();

    sleep_ms(5000);
    gpio_put(LED_PIN, 1);

    printf("Setting up LoRa Reciever\n");
    fflush(stdout);
    sleep_ms(100);

    lora_setup(LOGGING_LIGHT);

    //lora_set_op_mode(OPMODE_RX_CONT);

    char msg_setup[] =
        "LoRa repeater radio setup";

    lora_send_packet((uint8_t*) msg_setup, strlen(msg_setup));
    sleep_ms(500);
    lora_set_op_mode(OPMODE_RX_CONT);

    while(1)
        tight_loop_contents();
        
    return 0;
}

void setup_led(void)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 0);
}
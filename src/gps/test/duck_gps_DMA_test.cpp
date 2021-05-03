extern "C" {
    #include <stdio.h>
    #include "pico.h"
    #include "pico/stdlib.h"
    #include "hardware/dma.h"
    #include "hardware/irq.h"
    #include "hardware/uart.h"
    #include "pico/time.h"
    #include "gps.h"
}

#define LED_PIN 25

// DMA memory location for GPS transfers
//char gps_buff[(MAX_SENTENCES + 1) * MAX_NMEA_FRAME_SIZE] = {};
char gps_buff[1024] = {};

int gps_chan;
dma_channel_config c;

int main()
{
    int led_val = 0;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);
    stdio_init_all();

    sleep_ms(10 * 1000);

    gps_chan = setup_gps(gps_buff);

    while( true )
    {
        (led_val > 0) ? led_val = 0 : led_val = 1;
        gpio_put(LED_PIN, led_val);
        
        //while(dma_channel_is_busy(gps_chan))
        //    ;

        dma_channel_set_irq0_enabled(gps_chan, false); // Temporarily disable restart
        dma_channel_wait_for_finish_blocking(gps_chan); // Wait for transfer to finish
        printf("%s\n", gps_buff); // Read GPS data
        dma_channel_set_irq0_enabled(gps_chan, true); // Enable the IRQ again

        /*
        printf("BUFFER[0]|BUFFER[1]|2|3|4|5|6: %c|%c|%c|%c|%c|%c|%c\n", 
                gps_buff[0], gps_buff[1], gps_buff[2], gps_buff[3], 
                gps_buff[4], gps_buff[5], gps_buff[6]);
        printf("UART FIFO: %ld\n", uart0_hw->dr);
        */

        sleep_ms(10 * 1000); // Sleep for a sec
        tight_loop_contents();
    }
    puts("DMA finished");
}
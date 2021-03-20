extern "C" {
    #include <stdio.h>
    #include "pico/stdlib.h"
    #include "hardware/dma.h"
    #include "hardware/irq.h"
    #include "hardware/uart.h"
    #include "pico/time.h"
    #include "gps.h"
}

#define LED_PIN 25

// GPS parametersTemp
#define GPS_UART_PORT uart0
#define GPS_UART_PIN 17
#define GPS_UART_BAUD_RATE 9600

#define MAX_SENTENCES 6
#define MAX_NMEA_FRAME_SIZE 81

// DMA memory location for GPS transfers
//char gps_buff[(MAX_SENTENCES + 1) * MAX_NMEA_FRAME_SIZE] = {};
char gps_buff[1024] = {};

int gps_chan;
dma_channel_config c;

void dma_handler()
{
    //printf("DMA handler %ld\n", to_ms_since_boot(get_absolute_time()));
    dma_hw->ints0 = 1u << gps_chan;
    dma_channel_set_write_addr(gps_chan, gps_buff, true);    
}

int main()
{
    int led_val = 0;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);
    stdio_init_all();

    sleep_ms(10 * 1000);

    // Enable DMA for UART RX FIFO. Not needed as UART FIFOs are by default enabled
    //uart0_hw->dmacr |= UART_UAchannel_config_set_enableRTDMACR_RXDMAE_BITS;

    // Setup DMA
    gps_chan = dma_claim_unused_channel(true);

    c = dma_channel_get_default_config(gps_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8); // Not multiple of 32 or 16 so have 8
    channel_config_set_dreq(&c, DREQ_UART0_RX);
    channel_config_set_read_increment(&c, false); // Always read from UART FIFO and don't move on that
    channel_config_set_write_increment(&c, true); // Always increment as you read to go through the buffer
    
    dma_channel_configure(
        gps_chan,
        &c,
        gps_buff, // Write into the GPS buffer
        &uart0_hw->dr, // Read from the GPS RX FIFO
        7 * 81, // Not sure about this due to the fact that the GPS may not send empty bytes
        false // Don't start transfers immed.    // Setup GPS
    );

    // Tell the DMA to raise IRQ line 0 whne the channel finishes a block
    dma_channel_set_irq0_enabled(gps_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Setup GPS
    uart_init(GPS_UART_PORT, GPS_UART_BAUD_RATE);
    gpio_set_function(GPS_UART_PIN, GPIO_FUNC_UART);
    dma_channel_set_write_addr(gps_chan, gps_buff, true);    

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
extern "C" {
    #include <stdio.h>
    #include "pico/stdlib.h"
    #include "hardware/dma.h"
    #include "hardware/irq.h"
    #include "hardware/uart.h"
}

#define LED_PIN 25

// GPS parameters
#define GPS_UART_PORT uart0
#define GPS_UART_PIN 17
#define GPS_UART_BAUD_RATE 9600

#define MAX_SENTENCES 6
#define MAX_NMEA_FRAME_SIZE 81

// DMA memory location for GPS transfers
char gps_buff[(MAX_SENTENCES + 1) * MAX_NMEA_FRAME_SIZE] = {};

void dma_handler()
{
    printf("DMA Handler\n");
}

int main()
{
    int led_val = 0;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);
    stdio_init_all();
    sleep_ms(10 * 1000);

    // Setup GPS
    uart_init(GPS_UART_PORT, GPS_UART_BAUD_RATE);
    gpio_set_function(GPS_UART_PIN, GPIO_FUNC_UART);

    // Enable DMA for UART RX FIFO (maybe not needed)
    uart0_hw->dmacr |= UART_UARTDMACR_RXDMAE_BITS;

    // Setup DMA
    int gps_chan = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(gps_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8); // Not multiple of 32 or 16 so have 8
    channel_config_set_dreq(&c, DREQ_UART0_RX);
    channel_config_set_read_increment(&c, false); // Always read from UART FIFO and don't move on that
    channel_config_set_write_increment(&c, true); // Always increment as you read to go through the buffer
    channel_config_set_ring(&c, true, (MAX_SENTENCES + 1) * MAX_NMEA_FRAME_SIZE); // Wrap on the buffer size

    dma_channel_set_irq0_enabled(gps_chan, true);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_configure(
        gps_chan,
        &c,
        gps_buff, // Write into the GPS buffer
        &uart0_hw->dr, // Read from the GPS RX FIFO
        (MAX_SENTENCES + 1) * MAX_NMEA_FRAME_SIZE, // Not sure about this due to the fact that the GPS may not send empty bytes
        true // Don't start transfers immed.
    );

    while( true )
    {
        //(led_val > 0) ? led_val = 0 : led_val = 1;
        //gpio_put(LED_PIN, led_val);
        tight_loop_contents();
    }
    puts("DMA finished");
}
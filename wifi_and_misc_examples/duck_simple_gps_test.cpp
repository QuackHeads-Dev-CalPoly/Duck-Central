
extern "C" {
    #include <stdio.h>
    #include "pico/stdlib.h"
    #include "hardware/uart.h"
    #include "hardware/dma.h"
    #include "hardware/irq.h"
    #include "gps.h"
}

#define LED_PIN 25

#define MAX_SENTENCES       6
#define MAX_NMEA_FRAME_SIZE 81

const char GPS_DMA_BUFFER[(MAX_SENTENCES + 1) * MAX_NMEA_FRAME_SIZE] = {};
char GPS_SINGLE_CHAR;

int main()
{
    stdio_init_all();

    /*
    THIS CODE IS NOT FUNCTIONAL AT ALL. 
    IF YOU SEE THIS MESSAGE DO NOT RUN OR TRY TO USE THIS CODE
    */

    sleep_ms(1000 * 15);

    setup_gps_temp();

    GPS_SINGLE_CHAR = 0;

    int gps_chan = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(gps_chan);
    c = dma_channel_get_default_config(gps_chan);
    channel_config_set_write_increment(&c, false);
    channel_config_set_read_increment(&c, false);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);

    channel_config_set_dreq(&c, DREQ_UART0_RX + 2 * PICO_DEFAULT_UART);

    dma_channel_configure(
        gps_chan,
        &c,
        &GPS_SINGLE_CHAR,
        &uart0_hw->dr,
        sizeof(GPS_SINGLE_CHAR),
        false
    );

    while( 1 )
    {
        sleep_ms(1000);
        printf("GPS_SINGLE_CHAR: %c, %d, %x\n", GPS_SINGLE_CHAR, GPS_SINGLE_CHAR, GPS_SINGLE_CHAR);
    }

   while( 1 )
   {
       sleep_ms(2000);
       read_gps_full();
   }

}
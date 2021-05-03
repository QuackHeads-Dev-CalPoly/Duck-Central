#include "gps.h"

#include <stdio.h>
#include <string.h>

#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pico/time.h"

// GPS parametersTemp
#define GPS_UART_PIN 17
#define GPS_UART_BAUD_RATE 9600

#define UART_GPS uart0
#define MAX_SENTENCES 6
#define MAX_NMEA_FRAME_SIZE 81

#define INDENT_SPACES "  "

int gps_chan;
dma_channel_config c;

// save buffer location for handler
char *gps_buffer_ptr;

void dma_handler()
{
    //printf("DMA handler %ld\n", to_ms_since_boot(get_absolute_time()));
    dma_hw->ints0 = 1u << gps_chan;
    dma_channel_set_write_addr(gps_chan, gps_buffer_ptr, true);    
}

int setup_gps(char gps_buff[]) {

    gps_buffer_ptr = gps_buff;

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
    uart_init(UART_GPS, GPS_UART_BAUD_RATE);
    gpio_set_function(GPS_UART_PIN, GPIO_FUNC_UART);
    dma_channel_set_write_addr(gps_chan, gps_buff, true);  

    return gps_chan;  
}

int get_latitude(GPSData gps_data) {
    if (gps_data.rmc_valid == false) {
        return -1;
    } else {
        return minmea_rescale(&(gps_data.rmc).latitude, 1000);
    }
}

int get_longitude(GPSData gps_data) {
    if (gps_data.rmc_valid == false) {
        return -1;
    } else {
        return minmea_rescale(&(gps_data.rmc).longitude, 1000);
    }
}

int get_hours(GPSData gps_data) {
    if (gps_data.rmc_valid == false) {
        return -1;
    } else {
        return gps_data.rmc.time.hours;
    }
}

int get_minutes(GPSData gps_data) {
    if (gps_data.rmc_valid == false) {
        return -1;
    } else {
        return gps_data.rmc.time.minutes;
    }
}

int get_seconds(GPSData gps_data) {
    if (gps_data.rmc_valid == false) {
        return -1;
    } else {
        return gps_data.rmc.time.seconds;
    }
}

/**
 * Return UTC time in nanoseconds or -1 if invalid/insufficient data
 **/
long get_time(GPSData gps_data) {
    if (gps_data.rmc_valid == false) {
        return -1;
    } else {
        struct timespec time;
        if (-1 == minmea_gettime(&time, &(gps_data.rmc.date), &(gps_data.rmc.time))) {
            return -1;
        } else {
            return time.tv_nsec;
        }
    }
}

void read_gps_blocking(GPSData* gps_data) {
    char buff[(MAX_SENTENCES + 1) * MAX_NMEA_FRAME_SIZE] = {};

    uart_read_blocking(UART_GPS, buff, sizeof(buff));

    while (!gps_data->rmc_valid && !gps_data->gga_valid) {
        char* sentence = buff;
        char* end = buff;
        while (NULL != (sentence = strsep(&end, "\r\n"))) {
            if (sentence[0] != '$') {
                // strsep does funky splitting with multi-char delimeters this limits noise
                continue;
            }

            switch (minmea_sentence_id(sentence, false)) {
                case MINMEA_SENTENCE_RMC: {
                    if (minmea_parse_rmc(&(gps_data->rmc), sentence)) {
                        gps_data->rmc_valid = true;
                        printf(INDENT_SPACES
                               "$xxRMC time:%d:%d:%d fixed-point coordinates, "
                               "speed "
                               "scaled to three decimal places: (%d,%d) %d\n",
                               gps_data->rmc.time.hours,
                               gps_data->rmc.time.minutes,
                               gps_data->rmc.time.seconds,
                               minmea_rescale(&(gps_data->rmc).latitude, 1000),
                               minmea_rescale(&(gps_data->rmc).longitude, 1000),
                               minmea_rescale(&(gps_data->rmc).speed, 1000));
                    } else {
                        printf(INDENT_SPACES "$xxRMC sentence is not parsed\n");
                    }
                } break;

                case MINMEA_SENTENCE_GGA: {
                    if (minmea_parse_gga(&(gps_data->gga), sentence)) {
                        gps_data->gga_valid = true;
                        printf(INDENT_SPACES
                               "$xxGGA: sats used: %d fix quality: %d\n",
                               gps_data->gga.satellites_tracked,
                               gps_data->gga.fix_quality);
                    } else {
                        printf(INDENT_SPACES "$xxGGA sentence is not parsed\n");
                    }
                } break;

                case MINMEA_SENTENCE_GSV:
                case MINMEA_SENTENCE_VTG:
                case MINMEA_SENTENCE_GLL:
                    // ignore
                    break;

                case MINMEA_INVALID: {
                    // printf(INDENT_SPACES "$xxxxx sentence is not valid
                    // (%02x%02x)\n", sentence[0], sentence[1]);
                } break;

                default: {
                    // printf(INDENT_SPACES "$xxxxx sentence is not
                    // parsed(%02x%02x)\n", sentence[0], sentence[1]);
                } break;
            }
        }
    }
}

void read_gps_full(void) {
    char buff[(MAX_SENTENCES + 1) * MAX_NMEA_FRAME_SIZE] = {};

    while (1) {
        printf("\n\n\n\n");
        uart_read_blocking(UART_GPS, buff, sizeof(buff));
        printf(":::::::::::::::::::::::::::::::::::\n{\n %s \n}\n:::::::::::::::::::::::::::::::::::", buff);
        char* sentence = buff;
        char* end = buff;
        while (NULL != (sentence = strsep(&end, "\r\n"))) {
            if (sentence[0] != '$') {
                // strsep does funky splitting with multi-char delimeters this limits noise
                continue;
            }
            switch (minmea_sentence_id(sentence, false)) {
                case MINMEA_SENTENCE_RMC: {
                    struct minmea_sentence_rmc frame;
                    if (minmea_parse_rmc(&frame, sentence)) {
                        printf(
                            INDENT_SPACES
                            "$xxRMC time:%d:%d:%d fixed-point coordinates, speed "
                            "scaled to three decimal places: (%d,%d) %d\n",
                            frame.time.hours, frame.time.minutes, frame.time.seconds,
                            minmea_rescale(&frame.latitude, 1000),
                            minmea_rescale(&frame.longitude, 1000),
                            minmea_rescale(&frame.speed, 1000));
                    } else {
                        printf(
                            INDENT_SPACES
                            "$xxRMC sentence is not parsed\n");
                    }
                } break;
                case MINMEA_SENTENCE_GGA: {
                    struct minmea_sentence_gga frame;
                    if (minmea_parse_gga(&frame, sentence)) {
                        printf(
                            INDENT_SPACES 
                            "$xxGGA: sats used: %d fix quality: %d\n",
                            frame.satellites_tracked,
                            frame.fix_quality);
                    } else {
                        printf(INDENT_SPACES "$xxGGA sentence is not parsed\n");
                    }
                } break;
                case MINMEA_SENTENCE_GSV: {
                    // ignore
                } break;
                case MINMEA_SENTENCE_VTG: {
                    // ignore
                } break;
                case MINMEA_SENTENCE_GLL: {
                    // ignore
                } break;
                case MINMEA_INVALID: {
                    //printf(INDENT_SPACES "$xxxxx sentence is not valid (%02x%02x)\n", sentence[0], sentence[1]);
                } break;
                default: {
                    //printf(INDENT_SPACES "$xxxxx sentence is not parsed(%02x%02x)\n", sentence[0], sentence[1]);
                } break;
            }
        }
    }
}

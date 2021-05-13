#include "gps.h"

#include <stdio.h>
#include <string.h>

#include "hardware/uart.h"
#include "pico/stdlib.h"

#define INDENT_SPACES "  "
#define NO_ACQUIRED_SEMAPHORES 1
#define MAX_SEMAPHORE_ACCESSORS 1

int dma_channel;
char gps_buffer_ptr[MAX_GPS_SIZE];
static struct semaphore gps_buffer_semaphore;

GPS::GPS() {
    uart_init(GPS_UART, GPS_BAUD_RATE);
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);
    sem_init(&gps_buffer_semaphore, NO_ACQUIRED_SEMAPHORES, MAX_SEMAPHORE_ACCESSORS);
    init_dma();
}

static void dma_handler() {
    // block until we get the semaphore
    //printf("waiting for dma semaphore\n");
    sem_acquire_blocking(&gps_buffer_semaphore);

    // continue once we get it
    dma_hw->ints0 = 1u << dma_channel;
    dma_channel_set_write_addr(dma_channel, gps_buffer_ptr, true);
    //printf("did dma channel set write addr\n");

    // let the semaphore go
    sem_release(&gps_buffer_semaphore);
}

void GPS::init_dma() {
    // Setup DMA
    dma_channel = dma_claim_unused_channel(true);

    dma_channel_config dma_config = dma_channel_get_default_config(dma_channel);

    // Not multiple of 32 or 16 so have 8
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);

    // Set the dreq
    channel_config_set_dreq(&dma_config, DREQ_UART0_RX);

    // Always read from UART FIFO and don't move on that
    channel_config_set_read_increment(&dma_config, false);

    // Always increment as you read to go through the buffer
    channel_config_set_write_increment(&dma_config, true);

    dma_channel_configure(
        dma_channel, 
        &dma_config,
        gps_buffer_ptr,  // Write into the GPS buffer
        &uart0_hw->dr,     // Read from the GPS RX FIFO
        MAX_GPS_SIZE,      // Size of DMA buffer needed
        false              // Don't start transfers immed.
    );

    // Tell the DMA to raise IRQ line 0 whne the channel finishes a block
    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Setup GPS
    uart_init(GPS_UART, GPS_BAUD_RATE);
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);
    dma_channel_set_write_addr(dma_channel, gps_buffer_ptr, true);
}

float GPS::get_latitude() {
    this->parse_gps();

    if (this->gps_data.rmc_valid == false) {
        return -1;
    } else {
        return minmea_tocoord(&(this->gps_data.rmc).latitude);
    }
}

float GPS::get_longitude() {
    this->parse_gps();

    if (this->gps_data.rmc_valid == false) {
        return -1;
    } else {
        return minmea_tocoord(&(this->gps_data.rmc).longitude);
    }
}

int GPS::get_hours() {
    this->parse_gps();

    if (this->gps_data.rmc_valid == false) {
        return -1;
    } else {
        return this->gps_data.rmc.time.hours;
    }
}

int GPS::get_minutes() {
    this->parse_gps();

    if (this->gps_data.rmc_valid == false) {
        return -1;
    } else {
        return this->gps_data.rmc.time.minutes;
    }
}

int GPS::get_seconds() {
    this->parse_gps();

    if (this->gps_data.rmc_valid == false) {
        return -1;
    } else {
        return this->gps_data.rmc.time.seconds;
    }
}

/**
 * Return UTC time in milliseconds or -1 if invalid/insufficient data
 **/
long GPS::get_time_millis() {
    this->parse_gps();

    if (this->gps_data.rmc_valid == false) {
        return -1;
    } else {
        struct timespec time;
        if (-1 == minmea_gettime(&time, &(this->gps_data.rmc.date),
                                 &(this->gps_data.rmc.time))) {
            return -1;
        } else {
            return (time.tv_sec * 1000) + (time.tv_nsec / 1.0e6);
        }
    }
}

void GPS::parse_gps() {
    // block until we get the semaphore
    sem_acquire_blocking(&gps_buffer_semaphore);

    char* sentence = gps_buffer_ptr;
    char* end = gps_buffer_ptr;
    while (NULL != (sentence = strsep(&end, "\r\n"))) {
        if (sentence[0] != '$') {
            // strsep does funky splitting with multi-char delimeters this limits noise
            continue;
        }

        switch (minmea_sentence_id(sentence, false)) {
            case MINMEA_SENTENCE_RMC: {
                if (minmea_parse_rmc(&(this->gps_data.rmc), sentence)) {
                    this->gps_data.rmc_valid = true;
                    /*printf(
                        INDENT_SPACES
                        "$xxRMC time:%d:%d:%d fixed-point coordinates, "
                        "speed "
                        "scaled to three decimal places: (%d,%d) %d\n",
                        this->gps_data.rmc.time.hours,
                        this->gps_data.rmc.time.minutes,
                        this->gps_data.rmc.time.seconds,
                        minmea_rescale(&(this->gps_data.rmc).latitude,
                                        1000),
                        minmea_rescale(&(this->gps_data.rmc).longitude,
                                        1000),
                        minmea_rescale(&(this->gps_data.rmc).speed, 1000));*/
                } else {
                    printf(INDENT_SPACES "$xxRMC sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_GGA: {
                if (minmea_parse_gga(&(this->gps_data.gga), sentence)) {
                    this->gps_data.gga_valid = true;
                    /*printf(INDENT_SPACES
                            "$xxGGA: sats used: %d fix quality: %d\n",
                            this->gps_data.gga.satellites_tracked,
                            this->gps_data.gga.fix_quality);*/
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

    // let the semaphore go
    sem_release(&gps_buffer_semaphore);
}
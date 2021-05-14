#ifndef GPS_H
#define GPS_H

#include <stdbool.h>
#include <stdint.h>

#include "hardware/dma.h"
#include "hardware/irq.h"
#include "minmea.h"
#include "pico/sync.h"

// if using the GP_20U7 module
#define MAX_SENTENCES 6

#define MAX_NMEA_FRAME_SIZE 81
#define NUM_NMEA_SENTENCES 7
#define MAX_GPS_SIZE ((NUM_NMEA_SENTENCES) * (MAX_NMEA_FRAME_SIZE))

#define GPS_UART uart0
#define GPS_BAUD_RATE 9600
#define GPS_RX_PIN 1

typedef struct {
    struct minmea_sentence_rmc rmc;
    bool rmc_valid;
    struct minmea_sentence_gga gga;
    bool gga_valid;
    struct minmea_sentence_vtg vtg;
    bool vtg_valid;
} GPSData;

class GPS {
   public:
    GPS();
    float get_latitude(void);
    float get_longitude(void);
    int get_hours(void);
    int get_minutes(void);
    int get_seconds(void);
    long get_time_millis(void);
    void parse_gps(void);

   private:
    void init_dma(void);

    GPSData gps_data;
    dma_channel_config c;
};

static void dma_handler();

#endif
#ifndef GPS_H
#define GPS_H

#include <stdbool.h>
#include <stdint.h>
#include "minmea.h"

typedef struct {
    struct minmea_sentence_rmc rmc;
    bool rmc_valid;
    struct minmea_sentence_gga gga;
    bool gga_valid;
} GPSData;

void setup_gps(void);
void setup_gps_temp(void);
void read_gps_full(void);
void read_gps_blocking(GPSData* gps_data);
int get_latitude(GPSData gps_data);
int get_longitude(GPSData gps_data);
int get_hours(GPSData gps_data);
int get_minutes(GPSData gps_data);
int get_seconds(GPSData gps_data);
long get_time(GPSData gps_data);

#endif
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

/**********************************************************
 * BEGIN: GGA definition
 **********************************************************/
#define NUM_GGA_FIELDS 16

#define MESSAGE_ID_SIZE 6
#define UTC_POSITION_SIZE   10
#define LATITUDE_SIZE       9
#define LONGITUDE_SIZE      10
#define STATION_ID_SIZE     4
#define CHECKSUM_SIZE       3

typedef struct gga {
    char message_id[MESSAGE_ID_SIZE];
    char utc_position[UTC_POSITION_SIZE];
    char latitude[LATITUDE_SIZE];
    char n_s;
    char longitude[LONGITUDE_SIZE];
    char e_w;
    uint8_t position_fix_indicator;
    uint8_t satellites_used;
    float HDOP;
    float altitude;
    char altitude_units;
    float undulation;
    char undulation_units;
    uint8_t age;
    uint8_t station_id;
    char checksum[CHECKSUM_SIZE];
} GGA;
/**********************************************************
 * END: GGA definition
 **********************************************************/

/**********************************************************
 * BEGIN: GLL definition
 **********************************************************/
#define NUM_GLL_FIELDS 8

/**********************************************************
 * END: GLL definition
 **********************************************************/


/**********************************************************
 * BEGIN: GSA definition
 **********************************************************/
#define NUM_GSA_FIELDS 19

/**********************************************************
 * END: GSA definition
 **********************************************************/


/**********************************************************
 * BEGIN: GSV definition
 **********************************************************/
#define NUM_GSV_FIELDS 13

/**********************************************************
 * END: GSV definition
 **********************************************************/


/**********************************************************
 * BEGIN: RMC definition
 **********************************************************/
#define NUM_RMC_FIELDS 13

/**********************************************************
 * END: RMC definition
 **********************************************************/


/**********************************************************
 * BEGIN: VTG definition
 **********************************************************/
#define NUM_VTG_FIELDS 13

/**********************************************************
 * END: VTG definition
 **********************************************************/
#endif    //char buff[(MAX_SENTENCES + 1) * MAX_NMEA_FRAME_SIZE] = {};
void read_gps_full(void);
void read_gps_blocking(GPSData* gps_data);
int get_latitude(GPSData gps_data);
int get_longitude(GPSData gps_data);
int get_hours(GPSData gps_data);
int get_minutes(GPSData gps_data);
int get_seconds(GPSData gps_data);
long get_time(GPSData gps_data);

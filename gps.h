#ifndef GPS_H
#define GPS_H

#include <stdint.h>

void setup_gps(void);
void read_gps_full(void);

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
#endif
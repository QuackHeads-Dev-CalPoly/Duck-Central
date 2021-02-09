#include "gps.h"

#include <stdio.h>
#include <string.h>

#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "minmea.h"

// if using the GP_20U7 module
#define MAX_SENTENCES       6
#define GPS_BAUD_RATE       9600

#define MAX_NMEA_FRAME_SIZE 81

#define UART_GPS uart1
#define UART_RX_PIN 5

#define INDENT_SPACES "  "


void get_fields(char** fields, char* head);
void print_gga(char** fields);

void setup_gps(void) {
    uart_init(UART_GPS, GPS_BAUD_RATE);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
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

void get_fields(char** fields, char* head) {
    char* string;
    int i = 0;
    while (NULL != (string = strsep(&head, ","))) {
        fields[i++] = string;
    }
}

void print_gga(char** fields) {
    printf("Header:.......... %s\n", fields[0]);
    printf("UTC time:........ %s\n", fields[1]);
    printf("Latitude:........ %s\n", fields[2]);
    printf("Latitude dir:.... %s\n", fields[3]);
    printf("Longitude:....... %s\n", fields[4]);
    printf("Longitude dir:... %s\n", fields[5]);
    printf("Quality:......... %s\n", fields[6]);
    printf("# Satellites:.... %s\n", fields[7]);
    printf("HDOP:............ %s\n", fields[8]);
    printf("Altitude:........ %s\n", fields[9]);
    printf("Altitude units:.. %s\n", fields[10]);
    printf("Undulation:...... %s\n", fields[11]);
    printf("Undulation units: %s\n", fields[12]);
    printf("Age:............. %s\n", fields[13]);
    printf("Station ID:...... %s\n", fields[14]);
    printf("Checksum:........ %s\n", fields[15]);
}

void print_gll(char** fields) {
    printf("Header:.......... %s\n", fields[0]);
    printf("Latitude:........ %s\n", fields[1]);
    printf("Latitude dir:.... %s\n", fields[2]);
    printf("Longitude:....... %s\n", fields[3]);
    printf("Longitude dir:... %s\n", fields[4]);
    printf("UTC Time......... %s\n", fields[5]);
    printf("Status:.......... %s\n", fields[6]);
    printf("Checksum:........ %s\n", fields[7]);
}

void print_gsa(char** fields) {
    printf("Header:.......... %s\n", fields[0]);
    printf("Mode 1:.......... %s\n", fields[1]);
    printf("Mode 2:.......... %s\n", fields[2]);
    printf("Satellite Cn. 1:. %s\n", fields[3]);
    printf("Satellite Cn. 2:. %s\n", fields[4]);
    printf("Satellite Cn. 3:. %s\n", fields[5]);
    printf("Satellite Cn. 4:. %s\n", fields[6]);
    printf("Satellite Cn. 5:. %s\n", fields[7]);
    printf("Satellite Cn. 6:. %s\n", fields[8]);
    printf("Satellite Cn. 7:. %s\n", fields[9]);
    printf("Satellite Cn. 8:. %s\n", fields[10]);
    printf("Satellite Cn. 9:. %s\n", fields[11]);
    printf("Satellite Cn. 10: %s\n", fields[12]);
    printf("Satellite Cn. 11: %s\n", fields[13]);
    printf("Satellite Cn. 12: %s\n", fields[14]);
    printf("PDOP:............ %s\n", fields[15]);
    printf("HDOP............. %s\n", fields[16]);
    printf("VDOP:............ %s\n", fields[17]);
    printf("Checksum:........ %s\n", fields[18]);
}
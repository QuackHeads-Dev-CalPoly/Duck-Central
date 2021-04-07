#include <stdio.h>
#include <string.h>

#include "lora.h"
#include "pico/stdlib.h"
#include "gps.h"

#define LED_PIN 25

void setup_led(void);

int main() {
    stdio_init_all();

    setup_led();

    sleep_ms(5000);
    gpio_put(LED_PIN, 1);

    printf("\nsetting up lora...\n");
    fflush(stdout);
    sleep_ms(100);

    lora_setup(LOGGING_VERBOSE);

    lora_set_op_mode(OPMODE_TX);

    setup_gps();

    GPSData gps_data;

    int latitude, longitude;
    long timestamp;

    uint8_t packet_length = sizeof(latitude) + sizeof(',') + sizeof(longitude) +
                            sizeof('\n') + sizeof(timestamp) + sizeof('\n') +
                            sizeof('\n');

    uint8_t packet_buff[packet_length];

    while (1) {
        gps_data.gga_valid = false;
        gps_data.rmc_valid = false;

        printf("memsetting\n");
        fflush(stdout);

        memset(packet_buff, 0, packet_length);

        printf("reading\n");
        fflush(stdout);

        read_gps_blocking(&gps_data);

        printf("read\n");
        fflush(stdout);

        int latitude = get_latitude(gps_data);
        int longitude = get_longitude(gps_data);
        long timestamp = get_time(gps_data);
        printf("siZE long = %d\n", sizeof(long));

        printf("lat=%d, long=%d, time=%ld\n", latitude, longitude, timestamp);

        int offset = 0;
        memcpy(packet_buff + offset, &latitude, sizeof(latitude));
        offset += sizeof(latitude);

        packet_buff[offset] = ',';
        offset++;

        memcpy(packet_buff + offset, &longitude, sizeof(longitude));
        offset += sizeof(longitude);

        packet_buff[offset] = '\n';
        offset++;

        memcpy(packet_buff + offset, &timestamp, sizeof(timestamp));
        offset += sizeof(timestamp);

        packet_buff[offset] = '\n';
        offset++;

        printf("sending\n");
        fflush(stdout);

        lora_send_packet(packet_buff, offset);
    }

    return 0;
}

void setup_led(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // pull low initially
    gpio_put(LED_PIN, 0);
}
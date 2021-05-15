#include "gps.h"
#include "lora.h"
#include "bmp_388.h"
#include "bmx_160.h"
#include "IridiumSBD.h"
#include "power_control.h"
#include "board_config.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <vector>


#define SAT_UART_ID uart1
#define SAT_BAUD_RATE 19200

#define MESSAGE_ID_LENGTH 4

#define TWO_MINUTES 120000
#define FIVE_SECONDS 5000
#define ONE_SECOND 1000

#define INT_1_PIN 6

volatile bool sat_pop;
volatile bool lora_pop;

PowerControl* power_control;
GPS* gps;
Lora* lora;
BMP388* bmp388_external;
BMP388* bmp388_internal;
BMX160* bmx160;
IridiumSBD* iridium;


void enable_all_modules(void);
void print_art(void);
void setup_iridium(void);
void setup_bmx(void);
void on_transmit(void);
void create_payload(char* buffer, int sequence_num);
void send_satellite_payload(char* payload, uint8_t payload_length);
void send_lora_payload(uint8_t* payload, uint8_t payload_length);
void create_uuid(char msg[4]);
void pop_topic(uint gpio, uint32_t events);


int main() {
    stdio_init_all();

    sleep_ms(2000);

    for (int i = 0; i < 75; i++) {
        printf("\n");
    }

    srand(time(NULL));  // Initialization, should only be called once.
    sat_pop = false;
    lora_pop = false;

    printf("enabling power...\n");
    fflush(stdout);

    power_control = new PowerControl();
    enable_all_modules();

    sleep_ms(5000);

    gps = new GPS();
    printf("GPS initialized successfully.\n");
    fflush(stdout);

    sleep_ms(1000);

    lora = new Lora();
    lora->set_transmit_callback(on_transmit);
    printf("LoRa initialized successfully.\n");
    fflush(stdout);

    sleep_ms(1000);

    bmp388_external = new BMP388(0x77);
    printf("BMP388 External initialized successfully.\n");
    fflush(stdout);

    sleep_ms(1000);

    bmp388_internal = new BMP388(0x76);
    printf("BMP388 Internal initialized successfully.\n");
    fflush(stdout);

    sleep_ms(1000);

    bmx160 = new BMX160(i2c0, 4000, BMX160_SCLK_PIN, BMX160_SDA_PIN);
    setup_bmx();
    printf("BMX160 initialized successfully.\n");
    fflush(stdout);

    sleep_ms(1000);

    iridium = new IridiumSBD(SAT_UART_ID, SAT_BAUD_RATE, ROCKBLOCK_TX, ROCKBLOCK_RX);
    setup_iridium();
    printf("RockBlock initialized successfully.\n");
    fflush(stdout);

    sleep_ms(1000);

    printf("===============================\n");
    printf("All modules initialized successfully.\n");
    printf("===============================\n\n\n");
    fflush(stdout);

    sleep_ms(100);
    printf("SpaceDuck II online.\n\n");
    print_art();
    fflush(stdout);
    sleep_ms(100);

    int sequence_num = 0;
    while (1) {
        char payload[255] = {};
        uint8_t payload_length = 0;

        // burst LoRa packets
        for (int i = 0; i < 10; i++) {
            create_payload(payload, sequence_num);
            payload_length = strlen((char*)payload);
            printf("payload length is %d\n", payload_length);
            printf("sending payload over lora...\n");
            send_lora_payload((uint8_t*)payload, payload_length);
            sleep_ms(FIVE_SECONDS);
        }
        // un-set the LoRa pop topic
        lora_pop = false;

        sleep_ms(1000);

        send_satellite_payload(payload, payload_length);

        // increment the sequence number
        sequence_num++;

        sleep_ms(TWO_MINUTES);
    }

    fflush(stdout);
    return 0;
}

void create_uuid(char* msg) {
    int i;
    for (i = 0; i < MESSAGE_ID_LENGTH; i++) {
        uint8_t randomValue = rand() % 37;

        if (randomValue < 26) {
            msg[i] = msg[i] + char(randomValue + 'a');
        } else {
            msg[i] = msg[i] + char((randomValue - 26) + '0');
        }
    }
}

/*
duck-id/message-id/payload/path/topic/papa-id
*/
void send_satellite_payload(char* payload, uint8_t payload_length) {
    int sig_qual;
    int err = iridium->getSignalQuality(sig_qual);
    if (err != ISBD_SUCCESS) {
        printf("getSignalQuality() failed: error %d\n", err);
        fflush(stdout);
    }

    printf("RockBlock signal quality is %d.\n", sig_qual);

    char message[255] = {};

    char* duck_id = "PHOENIX1";

    char message_id[MESSAGE_ID_LENGTH+1] = {};
    create_uuid(message_id);

    char* path = duck_id;

    uint8_t topic = sat_pop ? 0x16 : 0x10;
    sat_pop = false;

    int index = 0;
    sprintf(message, "%s/%s/", duck_id, message_id);
    index += strlen(message);
    printf("index is %d\n", index);

    memcpy(message + index, payload, payload_length);
    index += payload_length;
    printf("index is %d\n", index);

    sprintf(message + index, "/%s/%d/%s\0", path, topic, duck_id);

    printf("made message for iridium: [%s]\n", message);
    fflush(stdout);

    sleep_ms(100);
    printf("made message for iridium: [%s]\n", message);
    fflush(stdout);

    err = iridium->sendSBDText(message);
    if (err != ISBD_SUCCESS) {
        printf("sendSBDText failed: error %d\n", err);
        if (err == ISBD_SENDRECEIVE_TIMEOUT) {
            printf("Try again with a better view of the sky.\n");
            fflush(stdout);
        }
    } else {
        printf("RockBlock message sent successfully.\n");
        fflush(stdout);
    }
}

void send_lora_payload(uint8_t* payload, uint8_t payload_length) {
    std::vector<uint8_t> buffer;

    printf("building lora payload\n");

    // duck id
    std::string duck_id("PHOENIX1");
    buffer.insert(buffer.end(), duck_id.begin(), duck_id.end());

    // target device
    std::string target_device("00000000");
    buffer.insert(buffer.end(), target_device.begin(), target_device.end());

    // message uuid
    char message_id[MESSAGE_ID_LENGTH + 1] = {};
    create_uuid(message_id);
    buffer.insert(buffer.end(), message_id, message_id + MESSAGE_ID_LENGTH);

    // topic
    buffer.insert(buffer.end(), lora_pop ? 0x16 : 0x10);

    // path offset
    buffer.insert(buffer.end(), 28 + payload_length);

    // duck type
    buffer.insert(buffer.end(), 0x00);

    // hop count
    buffer.insert(buffer.end(), 0x00);

    // CRC
    buffer.insert(buffer.end(), 0x00);
    buffer.insert(buffer.end(), 0x11);
    buffer.insert(buffer.end(), 0x22);
    buffer.insert(buffer.end(), 0x33);

    // payload
    buffer.insert(buffer.end(), payload, payload + payload_length);

    // path
    buffer.insert(buffer.end(), duck_id.begin(), duck_id.end());

    uint8_t lora_packet[255];
    std::copy(buffer.begin(), buffer.end(), lora_packet);

    printf("transmitting lora payload\n");

    lora->transmit(lora_packet, buffer.size());
}

/*
[sequence number, temp (C), pressure (Pa), altitude (m), mag (uT), gyro (g), accel (m/s/s), lat, long, temp (C), pressure (Pa), altitude (m)]
*/
void create_payload(char* buffer, int sequence_num) {
    bmx160SensorData magnetometer, gyroscope, accelerometer;

    bmp388_external->perform_reading();
    bmp388_internal->perform_reading();

    bmx160->get_all_data(&magnetometer, &gyroscope, &accelerometer);

    sprintf((char*)buffer,
            "%d,%.4lf,%.4lf,%.4lf,%d:%d:%d,%d:%d:%d,%d:%d:%d,%f,%f,%.4lf,%.4lf,%.4lf",
            sequence_num, bmp388_external->get_temperature(),
            bmp388_external->get_pressure(), bmp388_external->get_altitude(),
            magnetometer.x, magnetometer.y, magnetometer.z, gyroscope.x,
            gyroscope.y, gyroscope.z, accelerometer.x, accelerometer.y,
            accelerometer.z, gps->get_latitude(), gps->get_longitude(),
            bmp388_internal->get_temperature(), bmp388_internal->get_pressure(),
            bmp388_internal->get_altitude());

    printf("Message built: [%s]\n", buffer);
}

void on_transmit(void) {
    printf("LoRa packet transmitted successfully.\n");
    fflush(stdout);
}

void print_art(void) {
    printf(
        "     _______..______      ___       ______  _______  _______   __    "
        "__    ______  __  ___     __   __\n");
    printf(
        "    /       ||   _  \\    /   \\     /      ||   ____||       \\ |  | "
        " | "
        " |  /      ||  |/  /    |  | |  | \n");
    printf(
        "   |   (----`|  |_)  |  /  ^  \\   |  ,----'|  |__   |  .--.  ||  |  "
        "| "
        " | |  ,----'|  '  /     |  | |  |\n");
    printf(
        "    \\   \\    |   ___/  /  /_\\  \\  |  |     |   __|  |  |  |  ||  "
        "|  | "
        " | |  |     |    <      |  | |  |\n");
    printf(
        ".----)   |   |  |     /  _____  \\ |  `----.|  |____ |  '--'  ||  "
        "`--' "
        " | |  `----.|  .  \\     |  | |  |\n");
    printf(
        "|_______/    | _|    /__/     \\__\\ \\______||_______||_______/  "
        "\\______/   \\______||__|\\__\\    |__| |__| \n\n");
    printf("                                   ___\n");
    printf("                               ,-""   `.\n");
    printf("                             ,'  _   e )`-._\n");
    printf("                            /  ,' `-._<.===-'\n");
    printf("                           /  /\n");
    printf("                          /  ;\n");
    printf("              _.--.__    /   ;\n");
    printf(" (`._    _.-\"\"       \"--'    |\n");
    printf(" <_  `-\"\"                     \\\n");
    printf("  <`-                          :\n");
    printf("   (__   <__.                  ;\n");
    printf("     `-.   '-.__.      _.'    /\n");
    printf("        \\      `-.__,-'    _,'\n");
    printf("         `._    ,    /__,-'\n");
    printf("            \"\"._\\__,'< <____\n");
    printf("                 | |  `----.`.\n");
    printf("                 | |        \\ `.\n");
    printf("                 ; |___      \\-``\n");
    printf("                 \\   --<\n");
    printf("                  `.`.<\n");
    printf("                    `-'\n");
}

void setup_bmx() {
    if (bmx160->begin() != true) {
        printf("BMX160 initialization failed.\n");
        fflush(stdout);
        return;
    }

    //gpio_init(INT_1_PIN);
    //gpio_set_dir(INT_1_PIN, GPIO_IN);
    //gpio_set_irq_enabled_with_callback(INT_1_PIN, GPIO_IRQ_EDGE_RISE, true, &pop_topic);
    //bmx160->enable_low_g_interrupt();
}

void setup_iridium() {
    printf("Beginning RockBlock initiation.\n");
    int err = iridium->begin();
    if (err != ISBD_SUCCESS) {
        printf("RockBlock begin failed: error %d\n", err);

        if (err == ISBD_NO_MODEM_DETECTED) {
            printf("No modem detected: check wiring.\n");
        }

        fflush(stdout);
        return;
    }

    int sig_qual;
    err = iridium->getSignalQuality(sig_qual);
    if (err != ISBD_SUCCESS) {
        printf("getSignalQuality() failed: error %d\n", err);
        fflush(stdout);
        return;
    }

    printf("RockBlock signal quality is %d.\n", sig_qual);
}

void enable_all_modules() {
    power_control->turn_on_gps();
    printf("GPS module enabled.\n");

    power_control->turn_on_lora();
    printf("LoRa module enabled.\n");

    power_control->turn_on_5v_pwr();
    printf("5V rail enabled.\n");

    printf("===============================\n");
    printf("All modules powered successfully.\n");
    printf("===============================\n\n\n");
}

void pop_topic(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio, events);
    gpio_set_irq_enabled(gpio, events, false);

    // let satellite and lora know to send a pop topic
    sat_pop = true;
    lora_pop = true;

    printf("balloon has popped\n");

    gpio_set_irq_enabled(gpio, events, true);
}

void ISBDConsolePrintCallback(IridiumSBD* device, char c) { printf("%c", c); }

void ISBDDebugPrintCallback(IridiumSBD* device, char c) { printf("%c", c); }

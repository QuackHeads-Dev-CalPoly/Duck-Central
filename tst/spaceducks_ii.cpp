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

volatile bool did_pop;

void enable_all_modules(PowerControl* power_control);
void print_art(void);
void setup_iridium(IridiumSBD* iridium);
void setup_bmx(BMX160* bmx160);
void on_transmit(void);
void create_payload(char* buffer, BMP388 bmp388_external, bmx160SensorData magnetometer,
                    bmx160SensorData gyroscope, bmx160SensorData accelerometer,
                    int sequence_num, GPS* gps);
void send_satellite_payload(IridiumSBD* iridium, char* payload, uint8_t payload_length);
void send_lora_payload(Lora lora, uint8_t* payload, uint8_t payload_length);
void create_uuid(char msg[4]);


int main() {
    stdio_init_all();

    sleep_ms(2000);

    for (int i = 0; i < 75; i++) {
        printf("\n");
    }

    srand(time(NULL));  // Initialization, should only be called once.
    did_pop = false;

    printf("enabling power...\n");
    fflush(stdout);

    PowerControl power_control = PowerControl();
    enable_all_modules(&power_control);

    GPS gps = GPS();
    printf("GPS initialized successfully.\n");
    fflush(stdout);

    Lora lora = Lora();
    lora.set_transmit_callback(on_transmit);
    printf("LoRa initialized successfully.\n");
    fflush(stdout);

    BMP388 bmp388_external = BMP388(0x77);
    printf("BMP388 External initialized successfully.\n");
    fflush(stdout);

    BMP388 bmp388_internal = BMP388(0x76);
    printf("BMP388 Internal initialized successfully.\n");
    fflush(stdout);

    BMX160 bmx160 = BMX160(i2c0, 4000, BMX160_SCLK_PIN, BMX160_SDA_PIN);
    setup_bmx(&bmx160);
    printf("BMX160 initialized successfully.\n");
    fflush(stdout);
    bmx160SensorData magnetometer, gyroscope, accelerometer;

    IridiumSBD iridium = IridiumSBD(SAT_UART_ID, SAT_BAUD_RATE, ROCKBLOCK_TX, ROCKBLOCK_RX);
    setup_iridium(&iridium);
    printf("RockBlock initialized successfully.\n");
    fflush(stdout);

    printf("===============================\n");
    printf("All modules initialized successfully.\n");
    printf("===============================\n\n\n");
    fflush(stdout);

    sleep_ms(100);
    printf("SpaceDucks II online.\n\n");
    print_art();
    fflush(stdout);
    sleep_ms(100);

    int sequence_num = 0;
    while (1) {
        bmp388_external.perform_reading();
        bmx160.get_all_data(&magnetometer, &gyroscope, &accelerometer);

        char payload[255] = {};
        create_payload(payload, bmp388_external, magnetometer, gyroscope, accelerometer,
                       sequence_num, &gps);
        uint8_t payload_length = strlen((char*)payload);
        printf("payload length is %d\n", payload_length);

        send_satellite_payload(&iridium, payload, payload_length);

        send_lora_payload(lora, (uint8_t*) payload, payload_length);

        sequence_num++;

        sleep_ms(1000);
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
void send_satellite_payload(IridiumSBD* iridium, char* payload, uint8_t payload_length) {
    char message[255] = {};

    char* duck_id = "AQUILA01";

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

    int err = iridium->sendSBDText(message);
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

void send_lora_payload(Lora lora, uint8_t* payload, uint8_t payload_length) {
    std::vector<uint8_t> buffer;

    // duck id
    std::string duck_id("AQUILA01");
    buffer.insert(buffer.end(), duck_id.begin(), duck_id.end());

    // target device
    std::string target_device("00000000");
    buffer.insert(buffer.end(), target_device.begin(), target_device.end());

    // message uuid
    char message_id[MESSAGE_ID_LENGTH + 1] = {};
    create_uuid(message_id);
    buffer.insert(buffer.end(), message_id, message_id + MESSAGE_ID_LENGTH);

    // topic
    buffer.insert(buffer.end(), 0x10);

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

    lora.transmit(lora_packet, buffer.size());
}

/*
[sequence number, temp (C), pressure (Pa), altitude (m), mag (uT), gyro (g), accel (m/s/s), lat, long]
*/
void create_payload(char* buffer, BMP388 bmp388_external, bmx160SensorData magnetometer,
                      bmx160SensorData gyroscope,
                      bmx160SensorData accelerometer, int sequence_num, GPS* gps) {

    sprintf((char*)buffer, "%d,%lf,%lf,%lf,%d:%d:%d,%d:%d:%d,%d:%d:%d,%f,%f",
            sequence_num, bmp388_external.get_temperature(), bmp388_external.get_pressure(),
            bmp388_external.get_altitude(), magnetometer.x, magnetometer.y,
            magnetometer.z, gyroscope.x, gyroscope.y, gyroscope.z,
            accelerometer.x, accelerometer.y, accelerometer.z,
            gps->get_latitude(), gps->get_longitude());

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

void setup_bmx(BMX160* bmx160) {
    if (bmx160->begin() != true) {
        printf("BMX160 initialization failed.\n");
        fflush(stdout);
        return;
    }
}

void setup_iridium(IridiumSBD* iridium) {
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

void enable_all_modules(PowerControl* power_control) {
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

    sat_pop = true;

    gpio_set_irq_enabled(gpio, events, true);
}

void ISBDConsolePrintCallback(IridiumSBD* device, char c) { printf("%c", c); }

void ISBDDebugPrintCallback(IridiumSBD* device, char c) { printf("%c", c); }

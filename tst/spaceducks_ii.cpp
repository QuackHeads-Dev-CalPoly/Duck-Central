#include "gps.h"
#include "lora.h"
#include "bmp_388.h"
#include "bmx160.h"
#include "IridiumSBD.h"
#include "power_controller.h"

#include <stdio.h>
#include <string.h>


#define BMX160_PIN_SCLK 21
#define BMX160_PIN_SDA 20

#define SAT_UART_ID uart1
#define SAT_BAUD_RATE 19200
#define SAT_UART_TX_PIN 4
#define SAT_UART_RX_PIN 5


void enable_all_modules(Pwr_Cntrl power_control);
void print_art(void);
void setup_iridium(IridiumSBD* iridium);
void setup_bmx(BMX160 bmx160);
void on_transmit(void);


int main() {
    stdio_init_all();

    sleep_ms(5000);
    for (int i = 0; i < 75; i++) {
        printf("\n");
    }
    printf("enabling power...\n");
    fflush(stdout);

    Pwr_Cntrl power_control = Pwr_Cntrl();
    enable_all_modules(power_control);

    GPS gps = GPS();
    printf("GPS initialized successfully.\n");
    fflush(stdout);

    Lora lora = Lora();
    lora.set_transmit_callback(on_transmit);
    printf("LoRa initialized successfully.\n");
    fflush(stdout);

    BMP bmp388 = BMP();
    printf("BMP388 initialized successfully.\n");
    fflush(stdout);

    BMX160 bmx160 = BMX160(i2c0, 4000, BMX160_PIN_SCLK, BMX160_PIN_SDA);
    setup_bmx(bmx160);
    printf("BMX160 initialized successfully.\n");
    fflush(stdout);
    bmx160SensorData magnetometer, gyroscope, accelerometer;

    IridiumSBD iridium = IridiumSBD(SAT_UART_ID, SAT_BAUD_RATE, SAT_UART_TX_PIN, SAT_UART_RX_PIN);
    setup_iridium(&iridium);
    printf("RockBlock initialized successfully.\n");
    fflush(stdout);

    printf("===============================\n");
    printf("All modules initialized successfully.\n");
    printf("===============================\n\n\n");
    fflush(stdout);

    sleep_ms(1000);
    printf("SpaceDucks II online.\n\n");
    print_art();
    fflush(stdout);
    sleep_ms(1000);

    int sequence_num = 0;
    while (1) {
        uint8_t buffer[255] = {};

        bmp388.performReading();
        bmx160.getAllData(&magnetometer, &gyroscope, &accelerometer);

        /*
        [sequence, temp (C), pressure (Pa), altitude (m), mag (uT), gyro (g), accel (m/s/s), lat, long]
        */
        sprintf((char*)buffer, "%d,%lf,%lf,%lf,%d:%d:%d,%d:%d:%d,%d:%d:%d,%f,%f\0", 
                sequence_num,
                bmp388.getTemperature(), bmp388.getPressure(), bmp388.getAltitude(),
                magnetometer.x, magnetometer.y, magnetometer.z,
                gyroscope.x, gyroscope.y, gyroscope.z,
                accelerometer.x, accelerometer.y, accelerometer.z,
                gps.get_latitude(), gps.get_longitude());

        printf("Message built: [%s]\n", buffer);
        fflush(stdout);
        lora.transmit(buffer, strlen((char*)buffer) + 1);
        int err = iridium.sendSBDText((char*)buffer);
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

        sequence_num++;

        sleep_ms(1000);
    }

    fflush(stdout);
    return 0;
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
}

void setup_bmx(BMX160 bmx160) {
    if (bmx160.begin() != true) {
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

void enable_all_modules(Pwr_Cntrl power_control) {
    power_control.turn_on_gps();
    printf("GPS module enabled.\n");

    power_control.turn_on_lora();
    printf("LoRa module enabled.\n");

    power_control.turn_on_5v_pwr();
    printf("5V rail enabled.\n");

    printf("===============================\n");
    printf("All modules powered successfully.\n");
    printf("===============================\n\n\n");
}

void ISBDConsolePrintCallback(IridiumSBD* device, char c) { printf("%c", c); }

void ISBDDebugPrintCallback(IridiumSBD* device, char c) { printf("%c", c); }

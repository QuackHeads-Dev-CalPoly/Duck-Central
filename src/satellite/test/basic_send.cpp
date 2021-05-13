#include "IridiumSBD.h"
#include "log.h"

#include <stdio.h>

/*
 * BasicSend
 *
 * This sends a "Hello, world!" message from the satellite modem.
 * If you have activated your account and have credits, this message
 * should arrive at the endpoints you have configured (email address or
 * HTTP POST).
 */

#define UART_ID uart1
#define BAUD_RATE 19200
#define UART_TX_PIN 4
#define UART_RX_PIN 5

int main() {
    int signalQuality = -1;
    int err;

    stdio_init_all();
    gpio_init(28);
    gpio_set_dir(28, 1);
    gpio_put(28, 1);
    printf("initializing\n");
    fflush(stdout);
    sleep_ms(4000);

    // Declare the IridiumSBD object
    IridiumSBD modem(UART_ID, BAUD_RATE, UART_TX_PIN, UART_RX_PIN);
    printf("initialized\n");

    // Begin satellite modem operation
    printf("Starting modem...\n");
    fflush(stdout);
    err = modem.begin();
    if (err != ISBD_SUCCESS) {
        printf("Begin failed: error %d\n", err);

        if (err == ISBD_NO_MODEM_DETECTED) {
            printf("No modem detected: check wiring.\n");
        }

        fflush(stdout);
        return -1;
    }

    // Example: Print the firmware revision
    char version[12];
    err = modem.getFirmwareVersion(version, sizeof(version));
    if (err != ISBD_SUCCESS) {
        printf("FirmwareVersion failed: error  %d\n", err);
        fflush(stdout);
        return -1;
    }
    printf("Firmware Version is %d.\n", version);

    // Example: Test the signal quality.
    // This returns a number between 0 and 5.
    // 2 or better is preferred.
    err = modem.getSignalQuality(signalQuality);
    if (err != ISBD_SUCCESS) {
        printf("SignalQuality failed: error %d\n", err);
        fflush(stdout);
        return -1;
    }

    printf("On a scale of 0 to 5, signal quality is currently %d.\n", signalQuality);
    int count = 1;
    absolute_time_t curr_time;
    uint32_t sec_since_boot;
    while (true) {
        // Send the message
        curr_time = get_absolute_time();
        sec_since_boot = to_ms_since_boot(curr_time) / 1000;
        printf("Trying to send the message.  This might take several minutes.\n");
        char buffer[100];
        sprintf(buffer, "Hello, world! Test Message #%d sent at %d seconds since boot", count, sec_since_boot);
        err = modem.sendSBDText(buffer);
        if (err != ISBD_SUCCESS) {
            printf("sendSBDText failed: error %d\n", err);
            if (err == ISBD_SENDRECEIVE_TIMEOUT) {
                printf("Try again with a better view of the sky.\n");
                fflush(stdout);
            }
        }

        else {
            printf("Hey, it worked!\n");
            fflush(stdout);
        }

        fflush(stdout);
        count++;
        sleep_ms(60000); //Send every minute
    }
    return 0;
}

void ISBDConsolePrintCallback(IridiumSBD *device, char c) { 
    printf("%c", c);
}

void ISBDDebugPrintCallback(IridiumSBD *device, char c) { 
    printf("%c", c);
}

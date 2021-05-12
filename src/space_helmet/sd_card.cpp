
extern "C"
{
    #include "hardware/spi.h"
    #include "hardware/gpio.h"
    #include "pico/time.h"
    #include <stdio.h>
}

#include "sd_card.h"

sd_card::sd_card()
{
    // Do nothing for the moment
    return;
}

uint32_t millis()
{
    absolute_time_t time_stamp = get_absolute_time();
    return to_ms_since_boot( time_stamp );
}

uint8_t sd_card::wait_not_busy(uint32_t timeout_millis)
{
    unsigned int t0 = millis();
    unsigned int d;
    uint8_t dummy = 0xFF;
    uint8_t res = 0x00;
    uint32_t count = 0;
    do
    {
        spi_write_read_blocking(spi0, &dummy, &res, 1);
        if(res == 0xFF)
            return res;
        d = millis() - t0;
    } while (d < timeout_millis);
    return res;
} 

uint8_t sd_card::init_sd_card()
{
    // Setup sd card
    spi_init(spi0, 250000); // Set the SCK to 250 kHz
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // All SPI pins
    gpio_set_function(SD_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_SCLK_PIN, GPIO_FUNC_SPI);

    // Chip select
    gpio_init(SD_CS_PIN);
    gpio_set_dir(SD_CS_PIN, GPIO_OUT);
    gpio_put(SD_CS_PIN, 1); // Set high

    // Reset SD card
    uint8_t reset_cmd[6] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    uint8_t response = 0x00;
    uint8_t dummy = 0xFF;
    uint8_t empty = 0x00;

    printf("Sending CMD 0\n");
    gpio_put(SD_CS_PIN, 1); // Set high explicitly
    int i;
    for( i = 0; i < 10; i++ )
        spi_write_blocking(spi0, &dummy, 1);
    
    gpio_put(SD_CS_PIN, 0);

    wait_not_busy(300);
    spi_write_blocking(spi0, reset_cmd, 6);
    while(response != 0x01) // Wait till we are in the IDLE state for the card.
    {
        spi_write_read_blocking(spi0, &dummy, &response, 1);
        printf("Response 1: 0x%02x\n", response);
    }

    gpio_put(SD_CS_PIN, 1); // Set CS high

    // Send response 8 and return the response
    return response;
}

uint8_t sd_card::send_cmd8()
{
    uint8_t cmd_8[6] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    uint8_t response = 0x00;
    uint8_t extra_response = 0x00;
    uint8_t dummy = 0xFF;

    printf("Sending CMD 8\n");
    gpio_put(SD_CS_PIN, 0); // Drive CS low
    wait_not_busy(300);
    spi_write_blocking(spi0, cmd_8, 6);
    spi_write_read_blocking(spi0, &dummy, &response, 1);
    printf("Res imm: 0x%02x\n", response);
    while((response != 0x01 || response != 0x05) && (0x80 & response))
    {
        spi_write_read_blocking(spi0, &dummy, &response, 1);
        printf("Res imm: 0x%02x\n", response);
    }
    printf("Response 2: 0x%02x\n", response);
    int i;
    /* Command 8 is followed by 00 00 01 AA which was the command sent */
    for ( i = 0; i < 4; i++ )
    {
        spi_write_read_blocking(spi0, &dummy, &extra_response, 1);
        printf("Res following: 0x%02x\n", extra_response);
    }
    gpio_put(SD_CS_PIN, 1); // Set CS high

    return response;
}

uint8_t sd_card::send_cmd58()
{
    uint8_t cmd_58[6] = {0x7A, 0x00, 0x00, 0x00, 0x00, 0xFF};
    uint8_t response = 0xFF;
    uint8_t dummy = 0xFF;
    uint8_t state_response[4] = {0x00, 0x00, 0x00, 0x00};

    printf("Sending CMD 58\n");
    gpio_put(SD_CS_PIN, 0); // Set CS low
    wait_not_busy(300);
    spi_write_blocking(spi0, cmd_58, 6);
    spi_write_read_blocking(spi0, &dummy, &response, 1);
    printf("First resp: 0x%02x\n", response);
    while((0x80 & response))
    {
        spi_write_read_blocking(spi0, &dummy, &response, 1);
        printf("Res imm: 0x%02x\n", response);
    }

    spi_write_read_blocking(spi0, &dummy, state_response, 1);
    spi_write_read_blocking(spi0, &dummy, state_response + 1, 1);
    spi_write_read_blocking(spi0, &dummy, state_response + 2, 1);
    spi_write_read_blocking(spi0, &dummy, state_response + 3, 1);

    printf("State response\n");
    int i;
    for( i = 0; i < 4; i++ )
    {
        printf("0x%02x\n", state_response[i]);
    }
    gpio_put(SD_CS_PIN, 1); // Set CS high

    return response;
}

uint8_t sd_card::send_acmd41()
{
    uint8_t cmd_55[6] = {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
    uint8_t acmd_41[6] = {0x69, 0x40, 0x00, 0x00, 0x00, 0xFF};
    uint8_t response = 0xFF;
    uint8_t dummy = 0xFF;

    // printf("Sending CMD 55\n");
    // gpio_put(SD_CS_PIN, 0); // Bring CS low
    // wait_not_busy();
    // spi_write_blocking(spi0, cmd_55, 6);
    // spi_write_read_blocking(spi0, &dummy, &response, 1);
    // while((response != 0x01 || response != 0x05) && (0x80 & response))
    // {
    //     spi_write_read_blocking(spi0, &dummy, &response, 1);
    //     printf("Res imm: 0x%02x\n", response);
    // }
    // printf("Response: 0x%02x\n", response);
    // gpio_put(SD_CS_PIN, 1);

    while(response != 0x00)
    {
        gpio_put(SD_CS_PIN, 0);
        wait_not_busy(300);
        printf("Sending CMD 55 and 48");
        spi_write_blocking(spi0, cmd_55, 6);
        spi_write_read_blocking(spi0, &dummy, &response, 1);
        while((0x80 & response))
        {
            spi_write_read_blocking(spi0, &dummy, &response, 1);
            printf("Res imm 1 : 0x%02x\n", response);
        }
        printf("Response 3: 0x%02x\n", response);

        printf("next\n");
        wait_not_busy(300);
        spi_write_blocking(spi0, acmd_41, 6);
        spi_write_read_blocking(spi0, &dummy, &response, 1);
        while((0x80 & response)) // While not response
        {
            spi_write_read_blocking(spi0, &dummy, &response, 1);
            printf("Res imm 2 : 0x%02x\n", response);
        }

        //gpio_put(SD_CS_PIN, 1);
    }

    printf("Response 4: 0x%02x\n", response);
    return response;
}
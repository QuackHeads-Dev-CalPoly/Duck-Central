extern "C"
{
    #include "hardware/spi.h"
    #include "hardware/gpio.h"
    #include "pico/time.h"
    #include <stdio.h>
    #include <string.h>
}

#include "sd_logger.h"
#include "sd_def.h"

uint32_t millis()
{
    absolute_time_t time_stamp = get_absolute_time();
    return to_ms_since_boot( time_stamp );
}

/* Interface functions for SPI */
static void spi_send(uint8_t data)
{
    //printf("Send %02x\n", data);
    spi_write_blocking(spi0, &data, 1);
}

static uint8_t spi_read()
{
    uint8_t dummy = 0xFF;
    uint8_t response = 0x00;
    spi_write_read_blocking(spi0, &dummy, &response, 1);
    //printf("read resp 0x%02x\n", response); 
    return response;
}

uint8_t wait_not_busy(uint32_t timeout_millis)
{
    unsigned int t0 = millis();
    unsigned int d;
    uint8_t dummy = 0xFF;
    uint8_t res = 0x00;
    do
    {
        spi_write_read_blocking(spi0, &dummy, &res, 1);
        if(res == 0xFF)
            return res;
        d = millis() - t0;
    } while (d < timeout_millis);
    //printf("Timeout\n");
    return false;
}

uint8_t card_command(uint8_t cmd, uint32_t arg)
{
    gpio_put(SD_CS_PIN, 0); // Drive the CS pin low to begin

    wait_not_busy(300);

    spi_send(cmd | 0x40);

    for(int8_t s = 24; s >= 0; s -= 8)
    {
        //printf("send arg\n");
        spi_send(arg >> s);
    }

    uint8_t crc = 0xFF;
    if(cmd == CMD0)
        crc = 0x95;
    else if(cmd == CMD8)
        crc = 0x87;

    spi_send(crc);

    //wait for response
    uint8_t resp;
    for( uint8_t i = 0; ((resp = spi_read()) & 0x80) && i != 0xFF; i++)
        ;

    return resp;
}

uint8_t sd_logger::init_sd_card()
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

    for(uint8_t i = 0; i < 10; i++)
        spi_send(0xFF);

    uint8_t resp;
    printf("CMD0\n");
    while ( (resp = card_command(CMD0, 0)) != R1_IDLE_STATE)
        ;
        //printf("hey\n"); 

    printf("CMD8\n");
    if( (card_command(CMD8, 0x1AA) & R1_ILLEGAL_COMMAND) )
        type = 0x00;
    else
    {
        for(uint8_t i = 0; i < 4; i++)
            resp = spi_read();
        
        if(resp != 0xAA)
        {
            printf("SD_CARD_ERROR_CMD8\n");
            return -1;
        }
        type = 0x01;
    }

    resp = 0xff;
    while(resp != R1_READY_STATE)
    {
        printf("CMD55\n");
        card_command(CMD55, 0);
        printf("ACMD41\n");
        resp = card_command(ACMD41, 0x40000000);
    }

    printf("SD type: %d\n", type);
    if(type == 0x01)
    {
        printf("CMD58\n");
        resp = card_command(CMD58, 0);

        //printf("Resp: 0x%02x\n", resp);
        if( resp )
            printf("SD_CARD_ERROR_CMD58\n");
        if( (spi_read() & 0xC0) == 0xC0 )
            type = 0x02;
    }

    for (uint8_t i = 0; i < 3; i++)
        spi_read();

    gpio_put(SD_CS_PIN, 1);
}

uint8_t wait_start_block()
{
    uint8_t resp;
    unsigned int t0 = millis();
    while((resp = spi_read()) == 0xFF)
    {
        unsigned int d = millis() - t0;
        if( d > SD_READ_TIMEOUT)
        {    
            printf("SD_CARD_ERROR_READ_TIMEOUT\n");
            gpio_put(SD_CS_PIN, 1);
            return false;
        }
    }
    
    if( resp != DATA_START_BLOCK )
    {
        printf("SD_CARD_ERROR_READ\n");
        gpio_put(SD_CS_PIN, 1);
        return false;
    }

    return true;
}

uint8_t sd_logger::read_block(uint32_t addr, uint8_t* buffer)
{
    printf("CMD17");
    if(card_command(CMD17, addr))
    {
        printf("SD_CARD_ERROR_CMD17");
        return true;
    }

    if( !wait_start_block() )
        return false;

    for(uint16_t i = 0; i < 512; i++)
    {
        buffer[i] = spi_read();
    }
    spi_read();
    spi_read();
    gpio_put(SD_CS_PIN, 1);
}

uint8_t write_data(uint32_t token, uint8_t* buffer)
{
    spi_send(token); // Put start token down on the wire
    for(uint16_t i = 0; i < 512; i++)
    {
        spi_send(buffer[i]);
    }

    spi_send(0xff); // send dummy crc
    spi_send(0xff); // send dummy crc

    uint8_t resp = spi_read();
    if( (resp & DATA_RES_MASK) != DATA_RES_ACCEPTED )
    {
        printf("SD_CARD_ERROR_WRITE\n");
        gpio_put(SD_CS_PIN, 1);
        return false;
    }

    return true;
}

uint8_t sd_logger::write_block(uint32_t addr, uint8_t* buffer)
{
    if(card_command(CMD24, addr))
    {
        printf("SD_CARD_ERROR_CMD24\n");
        gpio_put(SD_CS_PIN, 1); // Ensure to stop transaction
        return false;
    }

    if(!write_data(DATA_START_BLOCK, buffer))
    {
        printf("SD_WRITE_DATA_ERROR\n");
        gpio_put(SD_CS_PIN, 1); // Ensure to stop transaction
        return false;
    }

    if(!wait_not_busy(SD_WRITE_TIMEOUT))
    {
        printf("SD_CARD_ERROR_WRITE_TIMEOUT\n");
        gpio_put(SD_CS_PIN, 1); // Ensure to stop transaction
        return false;
    }

    //r2 response so ensure that the two bytes are nonzero
    if( card_command(CMD13, 0) || spi_read() )
    {
        printf("SD_CARD_ERROR_WRITE_PROGRAMMING\n");
        gpio_put(SD_CS_PIN, 1);
        return false;
    }

    gpio_put(SD_CS_PIN, 1); // End transaction
}

uint8_t sd_logger::start_logger(uint32_t start_blk_addr)
{
    curr_blk_addr = start_blk_addr;

    memset(active_buffer, 0x00, 512); // Clear out the buffer on init
    buff_offset = 0;

    if(!init_sd_card())
    {
        printf("Failed to init SD card");
        return false;
    }
    return true;
}

uint8_t sd_logger::log(uint8_t* buffer, uint16_t len)
{
    memset(active_buffer, 0x00, 512); // Clear out the buffer
    memcpy(active_buffer, buffer, len);
    
    uint32_t checksum = 0;
    for(int i = 0; i < len; i++)
        checksum += active_buffer[i];

    memcpy(active_buffer, (uint8_t*)&checksum, 4);

    if(!write_block(curr_blk_addr, active_buffer))
    {
        printf("Failed to write block");
        return false;
    }

    return true;
}
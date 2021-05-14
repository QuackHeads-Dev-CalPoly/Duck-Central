#ifndef __SD_LOGGER_H__
#define __SD_LOGGER_H__

extern "C"
{
    #include <stdint.h>
}

#define SD_SCLK_PIN 18
#define SD_MOSI_PIN 19
#define SD_MISO_PIN 16
#define SD_CS_PIN 17
#define SD_SCLK_SPEED

class sd_logger
{

    private:
        uint32_t curr_blk_addr = 0;
        uint16_t data_size = 0;
        uint8_t type = 0x00; // SD1 default

        /* Buffer size of a block */
        uint8_t active_buffer[512];
        uint16_t buff_offset;

        uint8_t init_sd_card();
        uint8_t read_block(uint32_t addr, uint8_t* buffer);
        uint8_t write_block(uint32_t addr, uint8_t* buffer);

    public:

        /* 
            size: size of log item
            start_blk_addr: block address to start at
        */
        uint8_t start_logger(uint32_t start_blk_addr);

        /* Structure of data */
        uint8_t log(uint8_t* buffer, uint16_t len);
};

#endif /* */
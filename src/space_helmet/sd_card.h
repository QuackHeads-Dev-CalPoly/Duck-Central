#ifndef __SD_CARD_H__
#define __SD_CARD_H__

extern "C"
{
    #include <stdint.h>
}

#define SD_SCLK_PIN 18
#define SD_MOSI_PIN 19
#define SD_MISO_PIN 16
#define SD_CS_PIN 17
#define SD_SCLK_SPEED

class sd_card
{

    private:
        uint8_t type = 0x00; // SD1 default

    public:
        sd_card();

        // Initializes the SD card and returns the byte response
        uint8_t init_sd_card();
        uint8_t read_block(uint32_t, uint8_t*);
        uint8_t write_block(uint32_t, uint8_t*);
};

#endif /* */
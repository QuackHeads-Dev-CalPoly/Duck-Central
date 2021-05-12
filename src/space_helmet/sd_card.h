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

    public:
        sd_card();

        // Initializes the SD card and returns the byte response
        uint8_t init_sd_card();
        uint8_t send_cmd8();
        uint8_t send_cmd58();
        uint8_t send_acmd41();
        uint8_t wait_not_busy(uint32_t);
};

#endif /* */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

/* Quacker Board Revisions

    To ensure that the board and modules are setup correctly
        use the key below to set what BOARD_REVISION you are 
        building for

    quacker board rev0.1 : 1
    quacker board rev0.2 : 2
    quacker board rev0.3 : 3
*/

#define BOARD_REVISION 3

#if BOARD_REVISION <= 2

#endif

#if BOARD_REVISION >= 3
    #define GPS_RX_PIN 2

    #define LORA_CS_PIN 9
    #define LORA_MOSI_PIN 11
    #define LORA_MISO_PIN 8
    #define LORA_SCLK_PIN 10
    #define LORA_DIO_0_PIN 3

    #define WIFI_MOSI_PIN 11
    #define WIFI_MISO_PIN 8
    #define WIFI_SCLK_PIN 10
    #define WIFI_CS_PIN 13
    #define WIFI_IRQ_PIN 14

    #define BMX160_SDA_PIN 20
    #define BMX160_SCLK_PIN 21
    #define BMX160_INT1_PIN 6
    #define BMX160_INT2_PIN 26

    #define BMP388_SDA_PIN 20
    #define BMP388_SCLK_PIN 21

    #define HAT_LED_PIN 22

    #define SD_CARD_CS 17
    #define SD_CARD_MISO 16
    #define SD_CARD_MOSI 19
    #define SD_CARD_SCLK 18

    #define ROCKBLOCK_RX 5
    #define ROCKBLOCK_TX 4
#endif

#endif /* */
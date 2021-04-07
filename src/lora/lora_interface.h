#ifndef __LORA_INTERFACE_H__
#define __LORA_INTERFACE_H__

#include "lora.h"
#include "board_config.h"

/* 
    LoRa SPI interface and Radio config definitions/paramters
    for the HopeRF95/96W modules
*/

#define SPI_DATA_BITS 8

// LoRa SPI Configuration
#if BOARD_REVISION <= 2
    #define LORA_SPI_PORT    spi0
    #define LORA_PIN_MISO    4
    #define LORA_PIN_MOSI    7
    #define LORA_PIN_CS      5
    #define LORA_PIN_SCK     6
    #define LORA_PIN_DIO0    3  // used for TX/RX complete
    // 0.5 MHz
    #define LORA_SPI_CLK_SPEED 500*1000

#elif BOARD_REVISION >= 3
    #define LORA_SPI_PORT    spi1
    #define LORA_PIN_MISO    8
    #define LORA_PIN_MOSI    11
    #define LORA_PIN_CS      9
    #define LORA_PIN_SCK     10
    #define LORA_PIN_DIO0    3  // used for TX/RX complete
    // 8 MHz
    #define LORA_SPI_CLK_SPEED 8000000
#endif

// LoRa Radio Configuration
#define LORA_FREQUENCY  FREQ_915
#define LORA_BW         BANDWIDTH_125K
#define LORA_ECR        ERROR_CODING_4_5
#define LORA_HEADER     EXPLICIT_MODE
#define LORA_SF         SPREADING_7
#define LORA_CRC        CRC_ON

// based on the frequency of the crystal on the LoRa module.
#define FREQ_RESOLUTION 61.035

#endif

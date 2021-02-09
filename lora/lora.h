#ifndef LORA_H
#define LORA_H

#include "pico/stdlib.h"

void lora_setup(void);
void lora_set_op_mode(uint8_t mode);
void lora_send_packet(uint8_t* buffer, uint8_t length);


// registers are on page 102 of the datasheet:
// https://cdn.sparkfun.com/assets/learn_tutorials/8/0/4/RFM95_96_97_98W.pdf

/******************************************************************************
 * begin: Operating Mode register
 *****************************************************************************/
#define REG_OPMODE 0x01
#define __LORA_MODE         (0x80)
#define OPMODE_SLEEP        (0b000 | __LORA_MODE)
#define OPMODE_STDBY        (0b001 | __LORA_MODE)
#define OPMODE_FSTX         (0b010 | __LORA_MODE)
#define OPMODE_TX           (0b011 | __LORA_MODE)
#define OPMODE_FSRX         (0b100 | __LORA_MODE)
#define OPMODE_RX_CONT      (0b101 | __LORA_MODE)
#define OPMODE_RX_SINGLE    (0b110 | __LORA_MODE)
#define OPMODE_CAD          (0b111 | __LORA_MODE)
/******************************************************************************
 * end: Operating Mode register
 *****************************************************************************/


/******************************************************************************
 * begin: Modem config registers
 *****************************************************************************/
#define REG_MODEM_CONFIG_1 0x1D
// bits 7-4
#define BANDWIDTH_7K8 0x00
#define BANDWIDTH_10K4 0x10
#define BANDWIDTH_15K6 0x20
#define BANDWIDTH_20K8 0x30
#define BANDWIDTH_31K25 0x40
#define BANDWIDTH_41K7 0x50
#define BANDWIDTH_62K5 0x60
#define BANDWIDTH_125K 0x70
#define BANDWIDTH_250K 0x80
#define BANDWIDTH_500K 0x90
// bits 3-1
#define ERROR_CODING_4_5 0x02
#define ERROR_CODING_4_6 0x04
#define ERROR_CODING_4_7 0x06
#define ERROR_CODING_4_8 0x08
// bit 0
#define EXPLICIT_MODE 0x00
#define IMPLICIT_MODE 0x01

#define REG_MODEM_CONFIG_2 0x1E
// bits 7-4
#define SPREADING_6 0x60
#define SPREADING_7 0x70
#define SPREADING_8 0x80
#define SPREADING_9 0x90
#define SPREADING_10 0xA0
#define SPREADING_11 0xB0
#define SPREADING_12 0xC0
// bit 3
#define TX_CONTINUOUS_MODE_ON 0x0
#define TX_CONTINUOUS_MODE_OFF 0x0
// bit 2
#define CRC_OFF 0x00
#define CRC_ON 0x04

#define REG_MODEM_CONFIG_3 0x26
#define LNA_SET_BY_AGC 0x04

/******************************************************************************
 * end: Modem config registers
 *****************************************************************************/

// the fifo buffer control registers
#define REG_FIFO 0x00
#define REG_FIFO_ADDR_PTR 0x0D
#define REG_FIFO_TX_BASE_ADDR 0x0E
#define DEFAULT_FIFO_TX_BASE_ADDR 0x80
#define REG_FIFO_RX_BASE_ADDR 0x0F
#define DEFAULT_FIFO_RX_BASE_ADDR 0x00

#define REG_IRQ_FLAGS 0x10
#define REG_IRQ_FLAGS_MASK 0x11

// power amplifier config
#define REG_PA_CONFIG 0x09
#define PA_MAX_BOOST 0x8F  // 100mW (max 869.4 - 869.65)
#define PA_LOW_BOOST 0x81
#define PA_MED_BOOST 0x8A
#define PA_MAX_UK 0x88  // 10mW (max 434)
#define PA_OFF_BOOST 0x00
#define RFO_MIN 0x00

// 20DBm
#define REG_PA_DAC 0x4D
#define PA_DAC_20 0x87

// low noise amplifier
#define REG_LNA 0x0C
#define LNA_MAX_GAIN 0b00100011
#define LNA_OFF_GAIN 0x00

#define REG_PAYLOAD_LENGTH 0x22

/******************************************************************************
 * begin: DIO config registers
 *****************************************************************************/
#define REG_DIO_MAPPING_1 0x40
#define DIO_0_RX_COMPLETE 0b00000000
#define DIO_0_TX_COMPLETE 0b01000000
#define DIO_0_CAD_COMPLETE 0b10000000
// others aren't used for our purposes
/******************************************************************************
 * end: DIO config registers
 *****************************************************************************/

// pre-computed frequency values
// computed by: [frequency in Hz] / 61.035Hz
enum frequency {
    Freq_915 = 0xE4C026
};

#endif
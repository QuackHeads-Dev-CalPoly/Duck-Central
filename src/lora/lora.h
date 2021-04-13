#ifndef LORA_H
#define LORA_H

#include "pico/stdlib.h"

#define LOG_DEBUG 1     // errors and info and debug
#define LOG_INFO 2      // errors and info
#define LOG_ERROR 3     // only errors
#define LOG_NONE 4      // no logging

typedef struct {
    uint8_t payload[255];
    uint8_t length;
    float RSSI;
    float SNR;
} LoraPayload;

// pre-computed frequency values
// computed by: [frequency in Hz] / 61.035Hz
enum frequency { Freq_915 = 0xE4C026 };

// a function pointer for a receive callback
typedef void (*callback_rx_fn)(LoraPayload);

// a function pointer for a transmit callback
typedef void (*callback_tx_fn)(void);

class Lora {
public:
    Lora();
    void set_receive_callback(callback_rx_fn);
    void set_transmit_callback(callback_tx_fn);
    void transmit(uint8_t*, uint8_t);
    void sleep(void);
    void startReceive(void);
    void standby(void);

private:
    void set_op_mode(uint8_t);
    void set_frequency(enum frequency);
    void init_io(void);
    void init_modem(void);
    void modify_packet(uint8_t*, uint8_t*);
};

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

#define REG_FIFO_RX_CURRENT_ADDR 0x10

#define REG_RX_NB_BYTES 0x13

/******************************************************************************
 * begin: IRQ register flags
 *****************************************************************************/
#define REG_IRQ_FLAGS 0x12
#define IRQ_FLAG_RX_TIMEOUT (1<<7)
#define IRQ_FLAG_RX_DONE (1<<6)
#define IRQ_FLAG_PAYLOAD_CRC_ERROR (1 << 5)
#define IRQ_FLAG_VALID_HEADER (1 << 4)
#define IRQ_FLAG_TX_DONE (1 << 3)
#define IRQ_FLAG_CAD_DONE (1 << 2)
#define IRQ_FLAG_FHSS_CHANGE_CHANNEL (1 << 1)
#define IRQ_FLAG_CAD_DETECTED 0x01

#define REG_IRQ_FLAGS_MASK 0x11
// mask values are the same as the flag values, no need to repeat them for this register.
/******************************************************************************
 * end: IRQ register masks
 *****************************************************************************/

// power amplifier config
#define REG_PA_CONFIG 0x09
#define PA_MAX_BOOST 0x8F  // 100mW (max 869.4 - 869.65)
#define PA_LOW_BOOST 0x81
#define PA_MED_BOOST 0x8A
#define PA_OFF_BOOST 0x00
#define RFO_MIN 0x00

// 20DBm
#define REG_PA_DAC 0x4D
#define PA_DAC_20DB 0x87  // upper bit is reserved, it should be a 1
#define PA_DAC_DEFAULT 0x84  // upper bit is reserved, it should be a 1

// low noise amplifier
#define REG_LNA 0x0C
#define LNA_MAX_GAIN 0b00100011
#define LNA_OFF_GAIN 0x00

#define REG_PAYLOAD_LENGTH 0x22

#define REG_HOP_CHANNEL 0x1C
#define RX_PAYLOAD_CRC_MASK 0b01000000
#define RX_PAYLOAD_CRC_ON (1 << 6)

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

#define REG_PKT_SNR_VAL 0x19

#define REG_PKT_RSSI_VAL 0x1A

#define REG_OCP 0x0B
#define OCP_MAX_CURRENT 0b00111111

#endif
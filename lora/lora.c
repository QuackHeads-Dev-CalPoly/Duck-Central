// For the HopeRF RFM95CW Adafruit breakout board

#include "lora.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include <stdio.h>

#define SPI_DATA_BITS 8

#define SPI_PORT    spi0
#define PIN_MISO    4
#define PIN_MOSI    7
#define PIN_CS      5
#define PIN_SCK     6
#define PIN_DIO0    3  // used for TX/RX complete

#define LORA_FREQUENCY  FREQ_915
#define LORA_BW         BANDWIDTH_125K
#define LORA_ECR        ERROR_CODING_4_5
#define LORA_HEADER     EXPLICIT_MODE
#define LORA_SF         SPREADING_7
#define LORA_CRC        CRC_ON

// based on the frequency of the crystal on the LoRa module.
#define FREQ_RESOLUTION 61.035


void set_frequency(enum frequency freq);
void init_io(void);
void init_modem(void);
void irq_rx_complete(uint gpio, uint32_t events);
void modify_packet(uint8_t* payload[], uint8_t* payload_size);
void write_register(uint8_t reg, uint8_t data);
uint8_t read_register(uint8_t addr);
static inline void cs_select(void);
static inline void cs_deselect(void);


int logging_set = LOGGING_VERBOSE;

void lora_setup(int logging) {
    printf("\nInitializing LoRa...\n");

    if(logging == LOGGING_VERBOSE)
        printf("LoRa Logging set at: VERBOSE\n");
    else if(logging == LOGGING_LIGHT)
        printf("LoRa Logging set at: LIGHT\n");
    else
        printf("LoRa Logging set at: NONE\n");

    logging_set = logging;

    init_io();

    init_modem();

    write_register(REG_OPMODE, OPMODE_STDBY);
    uint8_t mode = read_register(REG_OPMODE);

    printf("LoRa initialization complete.\n");
}

void init_io(void) {
    // SPI0 at 0.5MHz.
    spi_init(SPI_PORT, 500 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // set up pin for RX complete
    gpio_init(PIN_DIO0);
    gpio_set_dir(PIN_DIO0, GPIO_IN);
}

void init_modem(void) {
    lora_set_op_mode(OPMODE_SLEEP);

    set_frequency(Freq_915);

    write_register(REG_MODEM_CONFIG_1, LORA_BW | LORA_ECR | LORA_HEADER);
    write_register(REG_MODEM_CONFIG_2, LORA_SF | LORA_CRC);

    // enable 'rx complete' signal
    write_register(REG_DIO_MAPPING_1, DIO_0_RX_COMPLETE);
    // set interrupt when RX is complete
    gpio_set_irq_enabled_with_callback(PIN_DIO0, GPIO_IRQ_EDGE_RISE, true, &irq_rx_complete);
}

void set_frequency(enum frequency freq) { 
    // frequency can only be set in sleep mode
    uint8_t original_op_mode = read_register(REG_OPMODE);

    lora_set_op_mode(OPMODE_SLEEP);

    write_register(0x06, (freq >> 16) & 0xFF);
    write_register(0x07, (freq >> 8) & 0xFF);
    write_register(0x08, freq & 0xFF);

    // reset to original operation mode 
    lora_set_op_mode(original_op_mode);
}

void lora_set_op_mode(uint8_t mode) {
    switch (mode) {
        case OPMODE_TX: {
            if(logging_set == LOGGING_VERBOSE)
                printf("setting to TX single\n");
            // Low noise amplifier off for transmissions to save power
            write_register(REG_LNA, LNA_OFF_GAIN);
            write_register(REG_PA_CONFIG, PA_MAX_BOOST);
        } break;

        case OPMODE_RX_CONT: {
            if(logging_set == LOGGING_VERBOSE)
                printf("setting to RX Continuous\n");
            write_register(REG_LNA, LNA_MAX_GAIN);
            write_register(REG_PA_CONFIG, PA_OFF_BOOST);
        } break;

        default:
            if(logging_set == LOGGING_VERBOSE)
            printf("setting to mode=%u\n", mode);
            break;
    }

    write_register(REG_OPMODE, mode);
}

/**
 * See: datasheet page 75
 **/
void lora_send_packet(uint8_t* buffer, uint8_t length) {
    if(logging_set == LOGGING_VERBOSE)
        printf("sending packet.\nbuffer is (size=%u):\n", length);
    uint8_t i = 0;
    for (i = 0; i < length; i++) {
        if(logging_set == LOGGING_VERBOSE)
            printf("%02x", buffer[i]);
    }
    printf("\n");

    // sleep to write registers
    lora_set_op_mode(OPMODE_STDBY);

    // the full buffer is used for Tx
    write_register(REG_FIFO_TX_BASE_ADDR, 0x00);
    write_register(REG_FIFO_ADDR_PTR, 0x00);
    write_register(REG_PAYLOAD_LENGTH, length);

    // cs_select();
    // address of the FIFO (tx)
    uint8_t addr_byte = REG_FIFO | 0x80;

    // write
    cs_select();
    spi_write_blocking(SPI_PORT, &addr_byte, 1);
    spi_write_blocking(SPI_PORT, buffer, length);
    cs_deselect();

    // put it into tx mode
    lora_set_op_mode(OPMODE_TX);
}

/**
 * See: datasheet page 36
 * In order to retrieve received data from the FIFO, we must ensure
 * ValidHeader, PayloadCrcError, RxDone, and RxTimeout interrupts
 * in the status register RegIrqFlags are not asserted to ensure that
 * packet reception has terminated successfully.
 * In case of errors, the packet shall be discarded.
 **/
void lora_repeat_rx(void) {
    uint8_t irq_flags = read_register(REG_IRQ_FLAGS);

    uint8_t irq_flag_masks = IRQ_FLAG_VALID_HEADER ||
                             IRQ_FLAG_PAYLOAD_CRC_ERROR || 
                             IRQ_FLAG_RX_DONE ||
                             IRQ_FLAG_RX_TIMEOUT;

    if (irq_flags & irq_flag_masks) {
        // clear all interrupts
        write_register(REG_IRQ_FLAGS, 0xFF);
        
        // tell somebody
        printf("error in flags: %02x\n", irq_flags);
        fflush(stdout);

        // run away
        return;
    }
    else {
        // mask interrupts while we process
        write_register(REG_IRQ_FLAGS_MASK, irq_flag_masks);
    }

    // FifoNbRxBytes indicates the number of bytes received thus far
    // RegRxDataAddr is a pointer that indicates precisely where the LoRa
    // modem received data has been written up to

    // Set the FIFO pointer to the location of the last packet received in
    // the FIFO
    write_register(REG_FIFO_ADDR_PTR, read_register(REG_FIFO_RX_CURRENT_ADDR));

    uint8_t payload_size = read_register(REG_RX_NB_BYTES);

    uint8_t payload[255];

    uint8_t i = 0;
    for (i = 0; i < payload_size; i++) {
        // FifoAddrPtr is incremented automatically (per datasheet page 30)
        uint8_t val = read_register(REG_FIFO);
        payload[i] = val;
    }

    if(logging_set == LOGGING_VERBOSE)
    {
        printf("\n\n=====\nNEW PAYLOAD RECEIVED:\n");
        for (i = 0; i < payload_size; i++) {
            printf("%02x", payload[i]);
        }
        printf("\n=====\n\n");
        for (i = 0; i < payload_size; i++) {
            printf("%c", payload[i]);
        }
        printf("\n=========\n");
        fflush(stdout);
    }
    else if(logging_set == LOGGING_LIGHT)
    {
        printf("===========================\n");
        printf("***New packet received***\n");
        printf("Content: ");
        for(i = 0; i < payload_size; i++)
            printf("%c", payload[i]);
        printf("\n===========================\n");
    }

    payload[payload_size] = ' ';
    payload[payload_size + 1] = 'R';
    payload[payload_size + 2] = 'e';

    // repeat the packet
    init_modem();
    write_register(REG_OPMODE, OPMODE_STDBY);
    uint8_t mode = read_register(REG_OPMODE);
    
    lora_send_packet(payload, payload_size + 3);

    // unmask interrupts
    write_register(REG_IRQ_FLAGS_MASK, 0x00);

    // set back to RX mode since when the TX is complete, it will go to STDBY mode
    lora_set_op_mode(OPMODE_RX_CONT);
}


void irq_rx_complete(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio, events);
    
    // check for a valid header and clear the flag if ok.
    if (read_register(REG_IRQ_FLAGS) & IRQ_FLAG_VALID_HEADER) {
        // we have received a valid header. clear flag and continue
        write_register(REG_IRQ_FLAGS, IRQ_FLAG_VALID_HEADER);
    } else {
        printf("invalid header. discarding packet\n");
        fflush(stdout);
        return;
    }

    // IRQ register is "set-to-clear" so writing a 1 actually clears it.
    write_register(REG_IRQ_FLAGS, IRQ_FLAG_RX_DONE);

    // disable interrupt, repeat, reset mode, re-enable interrupt
    gpio_set_irq_enabled(gpio, events, false);
    lora_repeat_rx();
    gpio_set_irq_enabled(gpio, events, true);
}

void write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2];

    // OR it with a 1 in the MSbit because the first byte tx'd is the address 
    // with 1 as the MSbit if you're writing, or 0 if reading.
    buf[0] = reg | (1<<7);

    // second MOSI is the actual data you are writing
    buf[1] = data;

    cs_select();
    spi_write_blocking(SPI_PORT, buf, 2);
    cs_deselect();
}

uint8_t read_register(uint8_t addr) {
    uint8_t buf;

    // clear the MSbit because it must be 0 for reading.
    // this is ok because valid addresses are 6 bits anyways.
    addr &= 0x7F;

    cs_select();
    spi_write_blocking(SPI_PORT, &addr, 1);
    spi_read_blocking(SPI_PORT, 0, &buf, 1);
    cs_deselect();

    //printf("READ [%02x] = %02X (ascii %c)\n", addr, buf, buf);
    fflush(stdout);

    return buf;
}

static inline void cs_select(void) {
    // nops are delays
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(void) {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}
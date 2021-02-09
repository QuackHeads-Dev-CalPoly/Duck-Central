// For the HopeRF RFM95CW Adafruit breakout board

#include "lora.h"
#include "hardware/spi.h"
#include <stdio.h>

#define SPI_DATA_BITS 8

#define SPI_PORT    spi1
#define PIN_MISO    12
#define PIN_MOSI    11
#define PIN_CS      13
#define PIN_SCK     10
#define PIN_DIO0    22  // used for TX/RX complete

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
void irq_tx_complete(uint gpio, uint32_t events);
void write_register(uint8_t reg, uint8_t data);
uint8_t read_register(uint8_t addr);
static inline void cs_select(void);
static inline void cs_deselect(void);


void lora_setup(void) {
    printf("\nInitializing LoRa...\n");

    init_io();

    init_modem();

    printf("LoRa initialization complete.\n");
}

void init_io(void) {
    // SPI0 at 0.5MHz.
    spi_init(SPI_PORT, 500 * 1000);
    //spi_set_format(SPI_PORT, SPI_DATA_BITS, 0, 0, SPI_MSB_FIRST);
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

    // enable 'tx complete' signal
    write_register(REG_DIO_MAPPING_1, DIO_0_TX_COMPLETE);
    // set interrupt when TX is complete
    gpio_set_irq_enabled_with_callback(PIN_DIO0, GPIO_IRQ_EDGE_FALL, true, &irq_tx_complete);
}

void set_frequency(enum frequency freq) { 
    // frequency can only be set in sleep mode
    uint8_t original_op_mode = read_register(REG_OPMODE);
    printf("original opmode %u\n", original_op_mode);
    fflush(stdout);

    lora_set_op_mode(OPMODE_SLEEP);

    write_register(0x06, (freq >> 16) & 0xFF);
    write_register(0x07, (freq >> 8) & 0xFF);
    write_register(0x08, freq & 0xFF);

    // reset to original operation mode 
    lora_set_op_mode(original_op_mode);
}

void lora_set_op_mode(uint8_t mode) { 
    write_register(REG_OPMODE, mode);
}

/**
 * See: datasheet page 75
 **/
void lora_send_packet(uint8_t* buffer, uint8_t length) {
    printf("\n\n-----------BEGIN lora_send_packet()------------\n");
    printf("buffer is '%s' (size=%u)\n", buffer, length);

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
    printf("beginning TX\n");
    lora_set_op_mode(OPMODE_TX);
    printf("-----------END lora_send_packet()------------\n\n\n");
}

void irq_tx_complete(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio, events);

    printf("Interrupt handled for gpio%u\n", gpio);

    lora_set_op_mode(OPMODE_STDBY);
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

    printf("READ %02X\n", buf);

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
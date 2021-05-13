#include "lora.h"
#include "lora_interface.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include "log.h"

static void write_register(uint8_t, uint8_t);
static uint8_t read_register(uint8_t);
static inline void cs_select(void);
static inline void cs_deselect(void);
static void irq_rx_complete(uint, uint32_t);
static void irq_tx_complete(uint, uint32_t);
static uint8_t receive_packet(LoraPayload* payload);
static float get_rssi(void);
static float get_snr(void);

// a function pointer for a user-provided callback function to be called on
// receive interrupt
static callback_rx_fn on_receive = NULL;

// a function pointer for a user-provided callback function to be called on
// transmit-complete interrupt
static callback_tx_fn on_transmit = NULL;

Lora::Lora() {
    init_io();
    init_modem();
}

void Lora::sleep() { set_op_mode(OPMODE_SLEEP); }

void Lora::startReceive() { set_op_mode(OPMODE_RX_CONT); }

void Lora::standby() { set_op_mode(OPMODE_STDBY); }

void Lora::set_op_mode(uint8_t mode) {
    switch (mode) {
        case OPMODE_TX: {
            // Low noise amplifier off for transmissions to save power
            write_register(REG_LNA, LNA_OFF_GAIN);
            write_register(REG_PA_CONFIG, PA_MAX_BOOST);
            write_register(REG_PA_DAC, PA_DAC_20DB);
            // enable 'tx complete' signal
            write_register(REG_DIO_MAPPING_1, DIO_0_TX_COMPLETE);
            // set interrupt when TX is complete
            gpio_set_irq_enabled_with_callback(
                LORA_PIN_DIO0, GPIO_IRQ_EDGE_RISE, true, &irq_tx_complete);
            fflush(stdout);
        } break;

        case OPMODE_RX_CONT: {
            write_register(REG_LNA, LNA_MAX_GAIN);
            write_register(REG_PA_CONFIG, PA_OFF_BOOST);
            write_register(REG_PA_DAC, PA_DAC_DEFAULT);
            // enable 'rx complete' signal
            write_register(REG_DIO_MAPPING_1, DIO_0_RX_COMPLETE);
            // set interrupt when RX is complete
            gpio_set_irq_enabled_with_callback(
                LORA_PIN_DIO0, GPIO_IRQ_EDGE_RISE, true, &irq_rx_complete);
        } break;

        case OPMODE_STDBY: {
            write_register(REG_LNA, LNA_OFF_GAIN);
            write_register(REG_PA_CONFIG, PA_OFF_BOOST);
            write_register(REG_PA_DAC, PA_DAC_DEFAULT);
        } break;

        case OPMODE_SLEEP: {
            write_register(REG_LNA, LNA_OFF_GAIN);
            write_register(REG_PA_CONFIG, PA_OFF_BOOST);
            write_register(REG_PA_DAC, PA_DAC_DEFAULT);
        } break;

        default:
            logerror("mode 0x%02x does not exist\n", mode);
            return;
    }

    write_register(REG_OPMODE, mode);
}

/**
 * See: datasheet page 75
 **/
void Lora::transmit(uint8_t* buffer, uint8_t length) {
    // sleep to write registers
    set_op_mode(OPMODE_STDBY);
    fflush(stdout);

    // the full buffer is used for Tx
    write_register(REG_FIFO_TX_BASE_ADDR, 0x00);
    write_register(REG_FIFO_ADDR_PTR, 0x00);
    write_register(REG_PAYLOAD_LENGTH, length);

    // cs_select();
    // address of the FIFO (tx)
    uint8_t addr_byte = REG_FIFO | 0x80;

    // write
    cs_select();
    spi_write_blocking(LORA_SPI_PORT, &addr_byte, 1);
    spi_write_blocking(LORA_SPI_PORT, buffer, length);
    cs_deselect();

    // put it into tx mode to initiate transmission
    set_op_mode(OPMODE_TX);
    fflush(stdout);
}

void Lora::set_receive_callback(callback_rx_fn fn) { on_receive = fn; }

void Lora::set_transmit_callback(callback_tx_fn fn) { on_transmit = fn; }

void Lora::init_io(void) {
    spi_init(LORA_SPI_PORT, LORA_SPI_CLK_SPEED);

    gpio_set_function(LORA_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(LORA_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(LORA_PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialize it to a driven-high state
    gpio_init(LORA_PIN_CS);
    gpio_set_dir(LORA_PIN_CS, GPIO_OUT);
    gpio_put(LORA_PIN_CS, 1);

    // set up pin for RX complete
    gpio_init(LORA_PIN_DIO0);
    gpio_set_dir(LORA_PIN_DIO0, GPIO_IN);
}

void Lora::init_modem(void) {
    printf("sleep pre\n");
    set_op_mode(OPMODE_SLEEP);
    printf("sleep post\n");

    set_frequency(Freq_915);

    write_register(REG_MODEM_CONFIG_1, LORA_BW | LORA_ECR | LORA_HEADER);
    write_register(REG_MODEM_CONFIG_2, LORA_SF | LORA_CRC);

    // set OCP to max value (240mA)
    write_register(REG_OCP, OCP_MAX_CURRENT);

    // enable 'rx complete' signal
    write_register(REG_DIO_MAPPING_1, DIO_0_RX_COMPLETE);
    // set interrupt when RX is complete
    gpio_set_irq_enabled_with_callback(LORA_PIN_DIO0, GPIO_IRQ_EDGE_RISE, true, &irq_rx_complete);
}

void Lora::set_frequency(enum frequency freq) {
    // frequency can only be set in sleep mode
    uint8_t original_op_mode = read_register(REG_OPMODE);

    set_op_mode(OPMODE_SLEEP);

    write_register(0x06, (freq >> 16) & 0xFF);
    write_register(0x07, (freq >> 8) & 0xFF);
    write_register(0x08, freq & 0xFF);

    // reset to original operation mode 
    set_op_mode(original_op_mode);
}

static float get_snr() {
    int8_t rawSNR = (int8_t) read_register(REG_PKT_SNR_VAL);
    return (rawSNR / 4.0);
}

static float get_rssi() {
    // for LoRa, get RSSI of the last packet
    float lastPacketRSSI = -157 + read_register(REG_PKT_RSSI_VAL);

    // spread-spectrum modulation signal can be received below noise floor
    // check last packet SNR and if it's less than 0, add it to reported RSSI to
    // get the correct value
    float lastPacketSNR = get_snr();
    if (lastPacketSNR < 0.0) {
        lastPacketRSSI += lastPacketSNR;
    }

    return lastPacketRSSI;
}

/**
 * See: datasheet page 36
 * In order to retrieve received data from the FIFO, we must ensure
 * ValidHeader, PayloadCrcError, RxDone, and RxTimeout interrupts
 * in the status register RegIrqFlags are not asserted to ensure that
 * packet reception has terminated successfully.
 * In case of errors, the packet shall be discarded.
 **/
static uint8_t receive_packet(LoraPayload* payload) {
    uint8_t irq_flags = read_register(REG_IRQ_FLAGS);

    uint8_t irq_flag_masks = IRQ_FLAG_VALID_HEADER ||
                             IRQ_FLAG_PAYLOAD_CRC_ERROR || IRQ_FLAG_RX_DONE ||
                             IRQ_FLAG_RX_TIMEOUT;

    // should equate to zero if there are no errors.
    if (irq_flags & irq_flag_masks) {  // handle error
        // clear all interrupts
        write_register(REG_IRQ_FLAGS, 0xFF);

        // tell somebody
        logwarn("error in flags: %02x\n", irq_flags);
        fflush(stdout);

        // run away
        return 0;
    } else {  // hardware has detected no errors, continue
        // mask interrupts while we process
        write_register(REG_IRQ_FLAGS_MASK, irq_flag_masks);
    }

    // FifoNbRxBytes indicates the number of bytes received thus far
    // RegRxDataAddr is a pointer that indicates precisely where the LoRa
    // modem received data has been written up to

    // Set the FIFO pointer to the location of the last packet received in
    // the FIFO
    write_register(REG_FIFO_ADDR_PTR, read_register(REG_FIFO_RX_CURRENT_ADDR));

    payload->length = read_register(REG_RX_NB_BYTES);
    payload->RSSI = get_rssi();
    payload->SNR = get_snr();

    uint8_t i = 0;
    for (i = 0; i < payload->length; i++) {
        // FifoAddrPtr is incremented automatically (per datasheet page 30)
        uint8_t val = read_register(REG_FIFO);
        payload->payload[i] = val;
    }

    return payload->length;
}

static void irq_rx_complete(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio, events);
    gpio_set_irq_enabled(gpio, events, false);

    // check for a valid header and clear the flag if ok.
    if (read_register(REG_IRQ_FLAGS) & IRQ_FLAG_VALID_HEADER) {
        // we have received a valid header. clear flag and continue
        write_register(REG_IRQ_FLAGS, IRQ_FLAG_VALID_HEADER);
    } else {
        logwarn("invalid header. discarding packet\n");
        fflush(stdout);
        return;
    }

    LoraPayload payload;
    uint8_t payload_length = receive_packet(&payload);

    if (on_receive != NULL) {
        on_receive(payload);
    } else {
        loginfo("callback function was NULL\n");
    }

    // IRQ register is "set-to-clear" so writing a 1 actually clears it.
    write_register(REG_IRQ_FLAGS, IRQ_FLAG_RX_DONE);

    gpio_set_irq_enabled(gpio, events, true);
}

static void irq_tx_complete(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio, events);
    gpio_set_irq_enabled(gpio, events, false);

    if (on_transmit != NULL) {
        on_transmit();
    } else {
        loginfo("callback function was NULL\n");
    }

    // IRQ register is "set-to-clear" so writing a 1 actually clears it.
    write_register(REG_IRQ_FLAGS, IRQ_FLAG_TX_DONE);

    gpio_set_irq_enabled(gpio, events, true);
}

static void write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2];

    // OR it with a 1 in the MSbit because the first byte tx'd is the address 
    // with 1 as the MSbit if you're writing, or 0 if reading.
    buf[0] = reg | (1<<7);

    // second MOSI is the actual data you are writing
    buf[1] = data;

    cs_select();
    spi_write_blocking(LORA_SPI_PORT, buf, 2);
    cs_deselect();
}

static uint8_t read_register(uint8_t addr) {
    uint8_t buf;

    // clear the MSbit because it must be 0 for reading.
    // this is ok because valid addresses are 6 bits anyways.
    addr &= 0x7F;

    cs_select();
    spi_write_blocking(LORA_SPI_PORT, &addr, 1);
    spi_read_blocking(LORA_SPI_PORT, 0, &buf, 1);
    cs_deselect();

    // logdebug("READ [%02x] = %02X (ascii %c)\n", addr, buf, buf);
    // fflush(stdout);

    return buf;
}

static inline void cs_select(void) {
    // nops are delays
    asm volatile("nop \n nop \n nop");
    gpio_put(LORA_PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(void) {
    asm volatile("nop \n nop \n nop");
    gpio_put(LORA_PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}
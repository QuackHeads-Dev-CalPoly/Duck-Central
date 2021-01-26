/**
 *
 * \file
 *
 * \brief This module contains RP2040 BSP APIs implementation.
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#include "bsp/include/nm_bsp.h"
#include "bsp/include/nm_bsp_rp2040.h"
#include "common/include/nm_common.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define HIGH 1
#define LOW 0

static tpfNmBspIsr gpfIsr;

static void chip_isr(void)
{
	if (gpfIsr) {
		gpfIsr();
	}
}

static void init_chip_pins(void)
{
    /* Setup reset pin */
    if( WINC1500_RESET_PIN > -1 )
    {
        gpio_init(WINC1500_RESET_PIN);
        gpio_set_dir(WINC1500_RESET_PIN, GPIO_OUT);
        gpio_put(WINC1500_RESET_PIN, HIGH);
    }

    /* Setup interrupt pin */
    /* TODO: Using gpio IRQ, maybe switch to hardware IRQ at some pt */
    gpio_init(WINC1500_INTN_PIN);

    /* Setup enable pin (Probably will remain unused) */
    if( WINC1500_CHIP_EN_PIN > -1 )
    {
        gpio_init(WINC1500_CHIP_EN_PIN);
        gpio_set_dir(WINC1500_CHIP_EN_PIN, GPIO_OUT);
        gpio_pull_up(WINC1500_CHIP_EN_PIN);
    }
}

static void deinit_chip_pins(void)
{
    /* Set reset pin to init state */
    if(WINC1500_RESET_PIN > -1)
    {
        gpio_put(WINC1500_RESET_PIN, LOW);
        gpio_set_dir(WINC1500_RESET_PIN, GPIO_IN);
    }

    /* Set en pin to init state */
    if(WINC1500_RESET_PIN > -1)
    {
        gpio_set_dir(WINC1500_CHIP_EN_PIN, GPIO_IN);
    }
}

sint8 nm_bsp_init(void)
{
    gpfIsr = NULL;
    
    init_chip_pins();

    nm_bsp_reset();

    return M2M_SUCCESS;
}

sint8 nm_bsp_deinit(void)
{
    deinit_chip_pins();

    return M2M_SUCCESS;
}

void nm_bsp_reset(void)
{
    if(WINC1500_RESET_PIN > -1)
    {
        gpio_put(WINC1500_RESET_PIN, LOW);
        nm_bsp_sleep(100);
        gpio_put(WINC1500_RESET_PIN, HIGH);
        nm_bsp_sleep(100);
    }
}

void nm_bsp_sleep(uint32 u32TimeMsec)
{
        sleep_ms(u32TimeMsec);
}

void nm_bsp_register_isr(tpfNmBspIsr pfIsr)
{
    gpfIsr = pfIsr;
    
    /* Attach interrupt to interrupt pin */
    /* TODO: Inititally over gpio instead of hardware nvic module */
    gpio_set_irq_enabled_with_callback(WINC1500_INTN_PIN, 
                                        GPIO_IRQ_EDGE_FALL, true, 
                                        &chip_isr);
}

void nm_bsp_interrupt_ctrl(uint8 u8Enable)
{
    if(u8Enable) 
    {
        // Enable interrupt
        gpio_set_irq_enabled_with_callback(WINC1500_INTN_PIN,
                                            GPIO_IRQ_EDGE_FALL, true,
                                            &chip_isr);
    } 
    else
    {
        // Disable interrupt
        gpio_set_irq_enabled_with_callback(WINC1500_INTN_PIN,
                                            GPIO_IRQ_EDGE_FALL, false,
                                            &chip_isr);
    }
}
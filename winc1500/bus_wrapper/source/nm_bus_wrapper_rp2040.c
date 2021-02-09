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

#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "bsp/include/nm_bsp.h"
#include "bsp/include/nm_bsp_rp2040.h"
#include "common/include/nm_common.h"
#include "bus_wrapper/include/nm_bus_wrapper.h"

#define NM_BUS_MAX_TRX_SZ 256

#define HIGH 1
#define LOW 0

tstrNmBusCapabilities egstrNmBusCapabilities =
{
    NM_BUS_MAX_TRX_SZ
};

static sint8 spi_rw(uint8* pu8Mosi, uint8* pu8Miso, uint16 u16Sz)
{
    /* TODO: Fix some of this dummy code for the new variation on pico */
    uint8 u8Dummy = 0;
    uint8 u8SkipMosi = 0, u8SkipMiso = 0;

    if (!pu8Mosi)
    {
        pu8Mosi = &u8Dummy;
        u8SkipMosi = 1;
    }
    else if(!pu8Miso)
    {
        pu8Miso = &u8Dummy;
        u8SkipMiso = 1;
    }
    else
    {
        return M2M_ERR_BUS_FAIL;
    }

    /* Set the CS PIN Low to begin transmission */
    gpio_put(WINC1500_SPI_CS_PIN, LOW);

    while( u16Sz )
    {
        spi_write_read_blocking(WINC1500_SPI_PORT, pu8Mosi, pu8Miso, 1);

        u16Sz--;
        if(!u8SkipMiso)
            pu8Miso++;
        if(!u8SkipMosi)
            pu8Mosi++;
    }

    /* Set the CS PIN High to begin transmission */
    gpio_put(WINC1500_SPI_CS_PIN, HIGH);
    
    return M2M_SUCCESS;
}

sint8 nm_bus_init(void *pvinit)
{
    sint8 result = M2M_SUCCESS;

    /* Set up SPI on port and pins at 12 MHz for WINC1500 */
    spi_init(WINC1500_SPI_PORT, WINC1500_SPI_CLK_SPEED);
    
    /* TODO: Default to 8 bits for transfer b.c. that is what Arduino does */
    spi_set_format(WINC1500_SPI_PORT, 8, 
                    SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    /* Set up MISO, MOSI, and SCK PINs */
    gpio_set_function(WINC1500_SPI_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(WINC1500_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(WINC1500_SPI_MOSI_PIN, GPIO_FUNC_SPI);

    /* Configure the CS PIN */
    gpio_init(WINC1500_SPI_CS_PIN);
    gpio_set_dir(WINC1500_SPI_CS_PIN, GPIO_OUT);
    gpio_put(WINC1500_SPI_CS_PIN, HIGH);

    /* Reset WINC1500 */
    nm_bsp_reset();
    nm_bsp_sleep(1);

    return result;
}

sint8 nm_bus_ioctl(uint8 u8Cmd, void* pvParameter)
{
    sint8 s8Ret = 0;
    switch(u8Cmd)
    {
        case NM_BUS_IOCTL_RW: {
            tstrNmSpiRw *pstrParam = (tstrNmSpiRw *)pvParameter;
            s8Ret = spi_rw(pstrParam->pu8InBuf, pstrParam->pu8OutBuf, pstrParam->u16Sz);
        }
        break;
        default:
            s8Ret = -1;
            M2M_ERR("invalide ioclt cmd\n");
            break;
    }

    return s8Ret;
}

sint8 nm_bus_deinit(void)
{
    /* De-init spi module */
    spi_deinit(WINC1500_SPI_PORT);
    return 0;
}

sint8 nm_bus_reinit(void *config)
{
    return M2M_SUCCESS;
}
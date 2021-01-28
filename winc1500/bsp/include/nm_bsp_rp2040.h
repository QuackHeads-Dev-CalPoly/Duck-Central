/**
 *
 * \file
 *
 * \brief This module contains NMC1500 BSP APIs definitions.
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef _NM_BSP_RP_2040_H_
#define _NM_BSP_RP_2040_H_

#include <stdint.h>

/*
 * Pico Board pin defs for WINC1500 WiFi co-processor module:
 *  
 *  RESET, INTERRUPT, and ENABLE PINS
 *      WINC1500_RESET_PIN   - pin 2 (GP1)
 *      WINC1500_INTN_PIN    - pin 9 (GP6)
 *      WINC1500_CHIP_EN_PIN - not connected (tied to VCC)
 *  
 *  SPI PINS:
 *      WINC1500_SPI_SCK    - pin 4 (GP2)
 *      WINC1500_SPI_MOSI   - pin 5 (GP3)
 *      WINC1500_SPI_MISO   - pin 6 (GP4)
 *      WINC1500_SPI_CS     - pin 7 (GP5)
 *  
 */

/* RESET, INTERRUPT, and ENABLE GPIO Pin Macros */
#if !defined(WINC1500_RESET_PIN)
  #define WINC1500_RESET_PIN  1
#endif
#if !defined(WINC1500_INTN_PIN)
  #define WINC1500_INTN_PIN   6
#endif
#if !defined(WINC1501_CHIP_EN_PIN)
  #define WINC1500_CHIP_EN_PIN -1
#endif

/* SCK, MOSI, MISO, CS Pin Macros */
#if !defined(WINC1500_SPI_SCK_PIN)
  #define WINC1500_SPI_SCK_PIN 2
#endif
#if !defined(WINC1500_SP0_MOSI_PIN)
  #define WINC1500_SPI_MOSI_PIN 3
#endif
#if !defined(WINC1500_SPI_MISO_PIN)
  #define WINC1500_SPI_MISO_PIN 4
#endif
#if !defined(WINC1500_SPI_CS_PIN)
  #define WINC1500_SPI_CS_PIN 5
#endif

/* SPI Port we are on */
#if !defined(WINC1500_SPI_PORT)
  #define WINC1500_SPI_PORT spi0
#endif

/* SPI CLK speed */
#if !defined(WINC1500_SPI_CLK_SPEED)
  #define WINC1500_SPI_CLK_SPEED 12000000
#endif

#define NM_EDGE_INTERRUPT 1

#endif /* _NM_BSP_ARDUINO_H_ */
/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** ============================================================================
 *  @file       CC2640R2DK_CXS.h
 *
 *  @brief      CC2640R2DK_CXS Board Specific header file.
 *
 *  The CC2640R2DK_CXS header file should be included in an application as
 *  follows:
 *  @code
 *  #include "CC2640R2DK_CXS.h"
 *  @endcode
 *
 *  This board file is made for the 2.7x2.7mm WCSP package, to convert this
 *  board file to use for other smaller device packages please refer to the table
 *  below which lists the max IOID values supported by each package. All other
 *  unused pins should be set to IOID_UNUSED.
 *
 *  Furthermore the board file is also used
 *  to define a symbol that configures the RF front end and bias.
 *  See the comments below for more information.
 *  For an in depth tutorial on how to create a custom board file, please refer
 *  to the section "Running the SDK on Custom Boards" with in the Software
 *  Developer's Guide..
 *
 *  Refer to the datasheet for all the package options and IO descriptions:
 *  http://www.ti.com/lit/ds/symlink/cc2640r2f.pdf
 *
 *  +-----------------------+------------------+-----------------------+
 *  |     Package Option    |  Total GPIO Pins |   MAX IOID            |
 *  +=======================+==================+=======================+
 *  |     7x7 mm QFN        |     31           |   IOID_30             |
 *  +-----------------------+------------------+-----------------------+
 *  |     5x5 mm QFN        |     15           |   IOID_14             |
 *  +-----------------------+------------------+-----------------------+
 *  |     4x4 mm QFN        |     10           |   IOID_9              |
 *  +-----------------------+------------------+-----------------------+
 *  |     2.7 x 2.7 mm WCSP |     14           |   IOID_13             |
 *  +-----------------------+------------------+-----------------------+
 *  ============================================================================
 */
#ifndef __CC2640R2DK_CXS_BOARD_H__
#define __CC2640R2DK_CXS_BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include <ti/drivers/PIN.h>
#include <ti/devices/cc26x0r2/driverlib/ioc.h>

/* Externs */
extern const PIN_Config BoardGpioInitTable[];

/* Defines */
#ifndef CC2640R2DK_CXS
  #define CC2640R2DK_CXS
#endif /* CC2640R2DK_CXS */

/*
 *  ============================================================================
 *  RF Front End and Bias configuration symbols for TI reference designs and
 *  kits. This symbol sets the RF Front End configuration in ble_user_config.h
 *  and selects the appropriate PA table in ble_user_config.c.
 *  Other configurations can be used by editing these files.
 *
 *  Define only one symbol:
 *  CC2650EM_7ID    - Differential RF and internal biasing
                      (default for CC2640R2 LaunchPad)
 *  CC2650EM_5XD    – Differential RF and external biasing
 *  CC2650EM_4XS    – Single-ended RF on RF-P and external biasing
 *  CC2640R2DK_CXS  - WCSP: Single-ended RF on RF-N and external biasing
 *                    (Note that the WCSP is only tested and characterized for
 *                     single ended configuration, and it has a WCSP-specific
 *                     PA table)
 *
 *  Note: CC2650EM_xxx reference designs apply to all CC26xx devices.
 *  ==========================================================================
 */
#define CC2640R2EM_CXS

/* Mapping of pins to board signals using general board aliases
 *      <board signal alias>                <pin mapping>
 */

/* Accelerometer */
#define CC2640R2DK_CXS_ACC_PWR                       PIN_UNASSIGNED         /* P2.8 */
#define CC2640R2DK_CXS_ACC_CSN                       PIN_UNASSIGNED         /* P2.10 */

/* Ambient Light Sensor */
#define CC2640R2DK_CXS_ALS_OUT                       PIN_UNASSIGNED         /* P2.5 */
#define CC2640R2DK_CXS_ALS_PWR                       PIN_UNASSIGNED         /* P2.6 */

/* Analog Capable DIO's */
#define CC2640R2DK_CXS_DIO7_ANALOG                  IOID_7
#define CC2640R2DK_CXS_DIO8_ANALOG                  IOID_8
#define CC2640R2DK_CXS_DIO9_ANALOG                  IOID_9
#define CC2640R2DK_CXS_DIO10_ANALOG                  IOID_10
#define CC2640R2DK_CXS_DIO11_ANALOG                  IOID_11
#define CC2640R2DK_CXS_DIO12_ANALOG                  IOID_12
#define CC2640R2DK_CXS_DIO13_ANALOG                  IOID_13

/* Button Board */
#define CC2640R2DK_CXS_KEY_SELECT                    IOID_9         /* P1.14 */
#define CC2640R2DK_CXS_KEY_UP                        IOID_6         /* P1.10 */
#define CC2640R2DK_CXS_KEY_DOWN                      IOID_4         /* P1.12 */
#define CC2640R2DK_CXS_KEY_LEFT                      IOID_5         /* P1.6  */
#define CC2640R2DK_CXS_KEY_RIGHT                     IOID_13         /* P1.8  */

/* GPIO */
#define CC2640R2DK_CXS_GPIO_LED_ON                   1
#define CC2640R2DK_CXS_GPIO_LED_OFF                  0

/* LEDs */
#define CC2640R2DK_CXS_PIN_LED_ON                    1
#define CC2640R2DK_CXS_PIN_LED_OFF                   0
#define CC2640R2DK_CXS_PIN_LED1                      PIN_UNASSIGNED         /* P2.11 */
#define CC2640R2DK_CXS_PIN_LED2                      PIN_UNASSIGNED         /* P2.13 */
#define CC2640R2DK_CXS_PIN_LED3                      IOID_2          /* P1.2  */
#define CC2640R2DK_CXS_PIN_LED4                      IOID_3          /* P1.4  */

/* LCD  Board */
#define CC2640R2DK_CXS_LCD_MODE                      IOID_7          /* P1.11 */
#define CC2640R2DK_CXS_LCD_RST                       PIN_UNASSIGNED          /* P1.13 */
#define CC2640R2DK_CXS_LCD_CSN                       PIN_UNASSIGNED         /* P1.17 */

/* SD Card */
#define CC2640R2DK_CXS_SDCARD_CSN                    PIN_UNASSIGNED         /* P2.12 */

/* SPI Board */
#define CC2640R2DK_CXS_SPI0_MISO                     IOID_12          /* P1.20 */
#define CC2640R2DK_CXS_SPI0_MOSI                     IOID_11          /* P1.18 */
#define CC2640R2DK_CXS_SPI0_CLK                      IOID_10         /* P1.16 */
#define CC2640R2DK_CXS_SPI0_CSN                      PIN_UNASSIGNED  /* P1.14, separate CSn for LCD, SDCARD, and ACC */
#define CC2640R2DK_CXS_SPI1_MISO                     PIN_UNASSIGNED         /* RF2.10 for testing only */
#define CC2640R2DK_CXS_SPI1_MOSI                     PIN_UNASSIGNED         /* RF2.5  for testing only */
#define CC2640R2DK_CXS_SPI1_CLK                      PIN_UNASSIGNED         /* RF2.12 for testing only */
#define CC2640R2DK_CXS_SPI1_CSN                      PIN_UNASSIGNED  /* RF2.6  for testing only */

/* Power Board */
#define CC2640R2DK_CXS_3V3_EN                        IOID_8         /* P1.15 */

/* PWM Outputs */
#define CC2640R2DK_CXS_PWMPIN0                       CC2640R2DK_CXS_PIN_LED1
#define CC2640R2DK_CXS_PWMPIN1                       CC2640R2DK_CXS_PIN_LED2
#define CC2640R2DK_CXS_PWMPIN2                       PIN_UNASSIGNED
#define CC2640R2DK_CXS_PWMPIN3                       PIN_UNASSIGNED
#define CC2640R2DK_CXS_PWMPIN4                       PIN_UNASSIGNED
#define CC2640R2DK_CXS_PWMPIN5                       PIN_UNASSIGNED
#define CC2640R2DK_CXS_PWMPIN6                       PIN_UNASSIGNED
#define CC2640R2DK_CXS_PWMPIN7                       PIN_UNASSIGNED

/* UART Board */
#define CC2640R2DK_CXS_UART_RX                       IOID_1          /* P1.7 */
#define CC2640R2DK_CXS_UART_TX                       IOID_0          /* P1.9 */
#define CC2640R2DK_CXS_UART_CTS                      PIN_UNASSIGNED          /* P1.3 */
#define CC2640R2DK_CXS_UART_RTS                      PIN_UNASSIGNED         /* P2.18 */

/*!
 *  @brief  Initialize the general board specific settings
 *
 *  This function initializes the general board specific settings.
 */
void CC2640R2DK_CXS_initGeneral(void);

/*!
 *  @def    CC2640R2DK_CXS_ADCBufName
 *  @brief  Enum of ADCBufs
 */
typedef enum CC2640R2DK_CXS_ADCBufName {
    CC2640R2DK_CXS_ADCBUF0 = 0,

    CC2640R2DK_CXS_ADCBUFCOUNT
} CC2640R2DK_CXS_ADCBufName;

/*!
 *  @def    CC2640R2_LAUNCHXL_ADCBuf0SourceName
 *  @brief  Enum of ADCBuf channels
 */
typedef enum CC2640R2DK_CXS_ADCBuf0ChannelName {
    CC2640R2DK_CXS_ADCBUF0CHANNEL1,
    CC2640R2DK_CXS_ADCBUF0CHANNEL2,
    CC2640R2DK_CXS_ADCBUF0CHANNEL3,
    CC2640R2DK_CXS_ADCBUF0CHANNEL4,
    CC2640R2DK_CXS_ADCBUF0CHANNEL5,
    CC2640R2DK_CXS_ADCBUF0CHANNEL6,
    CC2640R2DK_CXS_ADCBUF0CHANNEL7,
    CC2640R2DK_CXS_ADCBUF0CHANNELVDDS,
    CC2640R2DK_CXS_ADCBUF0CHANNELDCOUPL,
    CC2640R2DK_CXS_ADCBUF0CHANNELVSS,

    CC2640R2DK_CXS_ADCBUF0CHANNELCOUNT
} CC2640R2DK_CXS_ADCBuf0ChannelName;

/*!
 *  @def    CC2640R2DK_CXS_ADCName
 *  @brief  Enum of ADCs
 */
typedef enum CC2640R2DK_CXS_ADCName {
    CC2640R2DK_CXS_ADC1,
    CC2640R2DK_CXS_ADC2,
    CC2640R2DK_CXS_ADC3,
    CC2640R2DK_CXS_ADC4,
    CC2640R2DK_CXS_ADC5,
    CC2640R2DK_CXS_ADC6,
    CC2640R2DK_CXS_ADC7,
    CC2640R2DK_CXS_ADCDCOUPL,
    CC2640R2DK_CXS_ADCVSS,
    CC2640R2DK_CXS_ADCVDDS,

    CC2640R2DK_CXS_ADCCOUNT
} CC2640R2DK_CXS_ADCName;

/*!
 *  @def    CC2640R2DK_CXS_CryptoName
 *  @brief  Enum of Crypto names
 */
typedef enum CC2640R2DK_CXS_CryptoName {
    CC2640R2DK_CXS_CRYPTO0 = 0,

    CC2640R2DK_CXS_CRYPTOCOUNT
} CC2640R2DK_CXS_CryptoName;

/*!
 *  @def    CC2640R2DK_CXS_GPIOName
 *  @brief  Enum of GPIO names
 */
typedef enum CC2640R2DK_CXS_GPIOName {
    CC2640R2DK_CXS_GPIO_S1 = 0,
    CC2640R2DK_CXS_GPIO_S2,
    CC2640R2DK_CXS_GPIO_LED1,
    CC2640R2DK_CXS_GPIO_LED2,
    CC2640R2DK_CXS_GPIO_LED3,
    CC2640R2DK_CXS_GPIO_LED4,

    CC2640R2DK_CXS_GPIOCOUNT
} CC2640R2DK_CXS_GPIOName;

/*!
 *  @def    CC2640R2DK_CXS_GPTimerName
 *  @brief  Enum of GPTimer parts
 */
typedef enum CC2640R2DK_CXS_GPTimerName {
    CC2640R2DK_CXS_GPTIMER0A = 0,
    CC2640R2DK_CXS_GPTIMER0B,
    CC2640R2DK_CXS_GPTIMER1A,
    CC2640R2DK_CXS_GPTIMER1B,
    CC2640R2DK_CXS_GPTIMER2A,
    CC2640R2DK_CXS_GPTIMER2B,
    CC2640R2DK_CXS_GPTIMER3A,
    CC2640R2DK_CXS_GPTIMER3B,

    CC2640R2DK_CXS_GPTIMERPARTSCOUNT
} CC2640R2DK_CXS_GPTimerName;

/*!
 *  @def    CC2640R2DK_CXS_GPTimers
 *  @brief  Enum of GPTimers
 */
typedef enum CC2640R2DK_CXS_GPTimers {
    CC2640R2DK_CXS_GPTIMER0 = 0,
    CC2640R2DK_CXS_GPTIMER1,
    CC2640R2DK_CXS_GPTIMER2,
    CC2640R2DK_CXS_GPTIMER3,

    CC2640R2DK_CXS_GPTIMERCOUNT
} CC2640R2DK_CXS_GPTimers;

/*!
 *  @def    CC2640R2DK_CXS_PWM
 *  @brief  Enum of PWM outputs
 */
typedef enum CC2640R2DK_CXS_PWMName {
    CC2640R2DK_CXS_PWM0 = 0,
    CC2640R2DK_CXS_PWM1,
    CC2640R2DK_CXS_PWM2,
    CC2640R2DK_CXS_PWM3,
    CC2640R2DK_CXS_PWM4,
    CC2640R2DK_CXS_PWM5,
    CC2640R2DK_CXS_PWM6,
    CC2640R2DK_CXS_PWM7,

    CC2640R2DK_CXS_PWMCOUNT
} CC2640R2DK_CXS_PWMName;

/*!
 *  @def    CC2640R2DK_CXS_SPIName
 *  @brief  Enum of SPI names
 */
typedef enum CC2640R2DK_CXS_SPIName {
    CC2640R2DK_CXS_SPI0 = 0,
    CC2640R2DK_CXS_SPI1,

    CC2640R2DK_CXS_SPICOUNT
} CC2640R2DK_CXS_SPIName;

/*!
 *  @def    CC2640R2DK_CXS_UARTName
 *  @brief  Enum of UARTs
 */
typedef enum CC2640R2DK_CXS_UARTName {
    CC2640R2DK_CXS_UART0 = 0,

    CC2640R2DK_CXS_UARTCOUNT
} CC2640R2DK_CXS_UARTName;

/*!
 *  @def    CC2640R2DK_CXS_UDMAName
 *  @brief  Enum of DMA buffers
 */
typedef enum CC2640R2DK_CXS_UDMAName {
    CC2640R2DK_CXS_UDMA0 = 0,

    CC2640R2DK_CXS_UDMACOUNT
} CC2640R2DK_CXS_UDMAName;

/*!
 *  @def    CC2640R2DK_CXS_WatchdogName
 *  @brief  Enum of Watchdogs
 */
typedef enum CC2640R2DK_CXS_WatchdogName {
    CC2640R2DK_CXS_WATCHDOG0 = 0,

    CC2640R2DK_CXS_WATCHDOGCOUNT
} CC2640R2DK_CXS_WatchdogName;

/*!
 *  @def    CC2650DK_CXS_TRNGName
 *  @brief  Enum of TRNG names on the board
 */
typedef enum CC2640R2DK_CXS_TRNGName {
    CC2640R2DK_CXS_TRNG0 = 0,
    CC2640R2DK_CXS_TRNGCOUNT
} CC2640R2DK_CXS_TRNGName;

#ifdef __cplusplus
}
#endif

#endif /* __CC2640R2DK_CXS_BOARD_H__ */

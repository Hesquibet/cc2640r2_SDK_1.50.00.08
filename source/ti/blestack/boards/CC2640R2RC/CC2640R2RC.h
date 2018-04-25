/*
 * Copyright (c) 2017, Texas Instruments Incorporated
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
 *  @file       CC2640R2RC.h
 *
 *  @brief      CC2640R2RC Board Specific header file.
 *              The project options should point to this file if this is the
 *              CC2640R2RC you are developing code for.
 *
 *  The CC2640R2RC header file should be included in an application as follows:
 *  @code
 *  #include <CC2640R2RC.h>
 *  @endcode
 *
 *  This board file is made for the 7x7 mm QFN package, to convert this board
 *  file to use for other smaller device packages please refer to the table
 *  below which lists the max IOID values supported by each package. All other
 *  unused pins should be set to IOID_UNUSED.
 *
 *  Furthermore the board file is also used
 *  to define a symbol that configures the RF front end and bias.
 *  See the comments below for more information.
 *  For an in depth tutorial on how to create a custom board file, please refer
 *  to the section "Running the SDK on Custom Boards" with in the Software
 *  Developer's Guide.
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
#ifndef __CC2640R2RC_H__
#define __CC2640R2RC_H__

#ifdef __cplusplus
extern "C" {
#endif


/** ============================================================================
 *  Includes
 *  ==========================================================================*/
#include <ti/drivers/PIN.h>
#include <driverlib/ioc.h>

/** ============================================================================
 *  Externs
 *  ==========================================================================*/
extern const PIN_Config BoardGpioInitTable[];

/** ============================================================================
 *  Number of peripherals and their names
 *  ==========================================================================*/
/*!
 *  @def    CC2640R2RC_WatchdogName
 *  @brief  Enum of Watchdogs on the CC2650RC dev board
 */
typedef enum CC2640R2RC_WatchdogName {
    CC2640R2RC_WATCHDOG0 = 0,
    CC2640R2RC_WATCHDOGCOUNT
} CC2640R2RC_WatchdogName;

/*!
 *  @def    CC2640R2RC_ADCBufName
 *  @brief  Enum of ADCBufs
 */
typedef enum CC2640R2RC_ADCBufName {
    CC2640R2RC_ADCBUF0 = 0,

    CC2640R2RC_ADCBUFCOUNT
} CC2640R2RC_ADCBufName;

/*!
 *  @def    CC2640R2RC_ADCBuf0SourceName
 *  @brief  Enum of ADCBuf channels
 */
typedef enum CC2640R2RC_ADCBuf0ChannelName {
    CC2640R2RC_ADCBUF0CHANNEL0 = 0,
    CC2640R2RC_ADCBUF0CHANNEL1,
    CC2640R2RC_ADCBUF0CHANNEL2,
    CC2640R2RC_ADCBUF0CHANNEL3,
    CC2640R2RC_ADCBUF0CHANNEL4,
    CC2640R2RC_ADCBUF0CHANNEL5,
    CC2640R2RC_ADCBUF0CHANNEL6,
    CC2640R2RC_ADCBUF0CHANNEL7,
    CC2640R2RC_ADCBUF0CHANNELVDDS,
    CC2640R2RC_ADCBUF0CHANNELDCOUPL,
    CC2640R2RC_ADCBUF0CHANNELVSS,

    CC2640R2RC_ADCBUF0CHANNELCOUNT
} CC2640R2RC_ADCBuf0ChannelName;

/*!
 *  @def    CC2640R2RC_ADCName
 *  @brief  Enum of ADCs
 */
typedef enum CC2640R2RC_ADCName {
    CC2640R2RC_ADC0 = 0,
    CC2640R2RC_ADC1,
    CC2640R2RC_ADC2,
    CC2640R2RC_ADC3,
    CC2640R2RC_ADC4,
    CC2640R2RC_ADC5,
    CC2640R2RC_ADC6,
    CC2640R2RC_ADC7,
    CC2640R2RC_ADCDCOUPL,
    CC2640R2RC_ADCVSS,
    CC2640R2RC_ADCVDDS,

    CC2640R2RC_ADCCOUNT
} CC2640R2RC_ADCName;

/*!
 *  @def    CC2640R2RC_CryptoName
 *  @brief  Enum of Crypto names on the CC2650 dev board
 */
typedef enum CC2640R2RC_CryptoName {
    CC2640R2RC_CRYPTO0 = 0,
    CC2640R2RC_CRYPTOCOUNT
} CC2640R2RC_CryptoName;

/*!
 *  @def    CC2640R2RC_I2CName
 *  @brief  Enum of I2C names on the CC2650 dev board
 */
typedef enum CC2640R2RC_I2CName {
    CC2640R2RC_I2C0 = 0,
    CC2640R2RC_I2CCOUNT
} CC2640R2RC_I2CName;

/*!
 *  @def    CC2640R2RC_UARTName
 *  @brief  Enum of UARTs on the CC2650 dev board
 */
typedef enum CC2640R2RC_UARTName {
    CC2640R2RC_UART0 = 0,
    CC2640R2RC_UARTCOUNT
} CC2640R2RC_UARTName;

/*!
 *  @def    CC2640R2RC_UdmaName
 *  @brief  Enum of DMA buffers
 */
typedef enum CC2640R2RC_UdmaName {
    CC2640R2RC_UDMA0 = 0,
    CC2640R2RC_UDMACOUNT
} CC2640R2RC_UdmaName;

/*!
 *  @def    CC2640R2RC_SPIName
 *  @brief  Enum of SPI names on the CC2650 dev board
 */
typedef enum CC2640R2RC_SPIName {
    CC2640R2RC_SPI0 = 0,
    CC2640R2RC_SPICOUNT
} CC2640R2RC_SPIName;

/*!
 *  @def    CC2650_TRNGName
 *  @brief  Enum of TRNG names on the board
 */
typedef enum CC2650_TRNGName {
    CC2650_TRNG0 = 0,
    CC2650_TRNGCOUNT
} CC2650_TRNGName;

/*!
 *  @def    CC2640R2RC_GPTimerName
 *  @brief  Enum of GPTimer parts
 */
typedef enum CC2640R2RC_GPTimerName
{
    CC2640R2RC_GPTIMER0A = 0,
    CC2640R2RC_GPTIMER0B,
    CC2640R2RC_GPTIMER1A,
    CC2640R2RC_GPTIMER1B,
    CC2640R2RC_GPTIMER2A,
    CC2640R2RC_GPTIMER2B,
    CC2640R2RC_GPTIMER3A,
    CC2640R2RC_GPTIMER3B,
    CC2640R2RC_GPTIMERPARTSCOUNT
} CC2640R2RC_GPTimerName;

/*!
 *  @def    CC2640R2RC_GPTimers
 *  @brief  Enum of GPTimers
 */
typedef enum CC2640R2RC_GPTimers
{
    CC2640R2RC_GPTIMER0 = 0,
    CC2640R2RC_GPTIMER1,
    CC2640R2RC_GPTIMER2,
    CC2640R2RC_GPTIMER3,
    CC2640R2RC_GPTIMERCOUNT
} CC2640R2RC_GPTimers;

/*!
 *  @def    CC2640R2RC_PWM
 *  @brief  Enum of PWM outputs on the board
 */
typedef enum CC2640R2RC_PWM
{
    CC2640R2RC_PWM0 = 0,
    CC2640R2RC_PWM1,
    CC2640R2RC_PWM2,
    CC2640R2RC_PWM3,
    CC2640R2RC_PWM4,
    CC2640R2RC_PWM5,
    CC2640R2RC_PWM6,
    CC2640R2RC_PWM7,
    CC2640R2RC_PWMCOUNT
} CC2640R2RC_PWM;


/** ============================================================================
 *  Defines
 *  ==========================================================================*/
/* External flash manufacturer and device ID */
/* the following defines are for the Macronix flash */
#define EXT_FLASH_MAN_ID            0xC2
#define EXT_FLASH_DEV_ID            0x14

/* Same RF Configuration as 7x7 EM */
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
#define CC2650EM_7ID

/* Mapping of pins to board signals using general board aliases
 *      <board signal alias>                <pin mapping>
 */
/* I2C (SDA1 and SCL1 names are used to match sensor driver) */
#define Board_I2C0_SDA1             IOID_0
#define Board_I2C0_SCL1             IOID_1

/* Sensor Power control */
#define Board_MPU_POWER             IOID_2
#define Board_MPU_POWER_ON          1
#define Board_MPU_POWER_OFF         0

/* Sensor input */
#define Board_MPU_INT               IOID_3

/* Leds */
#define Board_BUZZER                IOID_4
#define Board_LED_G                 IOID_5
#define Board_LED_R                 IOID_6
#define Board_LED_IR                IOID_7
#define Board_LED_ON                1 /* LEDs on CC2650 are active high */
#define Board_LED_OFF               0

/* Audio */
#define Board_I2S_ADI               IOID_8
#define Board_I2S_ADO               (PIN_Id)IOID_UNUSED
#define Board_MIC_POWER             IOID_9

/* Key Scan */
#define Board_KEY_PWR               IOID_10
#define Board_KEY_CLK               IOID_11
#define Board_KEY_SCAN              IOID_12
#define Board_KEY_COL1              IOID_13
#define Board_KEY_COL2              IOID_14
#define Board_KEY_COL3              IOID_15

/* DevPack common */
#define Board_AUDIOFS_TDO           IOID_16
#define Board_AUDIODO               IOID_22
#define Board_DP2                   IOID_23
#define Board_DP1                   IOID_24
#define Board_DP0                   IOID_25
#define Board_DP3                   IOID_26
#define Board_DP4_UARTRX            IOID_27
#define Board_DP5_UARTTX            IOID_28
#define Board_DEVPK_ID              IOID_29
#define Board_SPI_DEVPK_CS          IOID_20

/* Analog Capable DIO's */
#define CC2640R2RC_DIO23_ANALOG                  IOID_23
#define CC2640R2RC_DIO24_ANALOG                  IOID_24
#define CC2640R2RC_DIO25_ANALOG                  IOID_25
#define CC2640R2RC_DIO26_ANALOG                  IOID_26
#define CC2640R2RC_DIO27_ANALOG                  IOID_27
#define CC2640R2RC_DIO28_ANALOG                  IOID_28
#define CC2640R2RC_DIO29_ANALOG                  IOID_29
#define CC2640R2RC_DIO30_ANALOG                  IOID_30

/* SPI0 */
#define Board_SPI0_CLK              IOID_17
#define Board_SPI0_MISO             IOID_18
#define Board_SPI0_MOSI             IOID_19
#define Board_SPI0_CSN              PIN_UNASSIGNED

/* SPI EXT FLASH CS (drived by middleware) */
#define Board_SPI_FLASH_CS          IOID_30
#define Board_FLASH_CS_ON           0
#define Board_FLASH_CS_OFF          1

/* I2S */
#define Board_I2S_BCLK              IOID_21
#define Board_I2S_MCLK              (PIN_Id)IOID_UNUSED
#define Board_I2S_WCLK              (PIN_Id)IOID_UNUSED

/* UART */
#define Board_UART_RX               Board_DP4_UARTRX
#define Board_UART_TX               Board_DP5_UARTTX

/* Unit Testing */
#define Board_TEST_TP11             Board_DP5_UARTTX
#define Board_TEST_TP12             Board_DP4_UARTRX
#define Board_TEST_TP14             Board_I2C0_SDA1
#define Board_Debug_0               Board_DP0
#define Board_Debug_1               Board_DP1
#define Board_Debug_2               Board_DP2
#define Board_Debug_3               Board_DP3

//For debugging IR signal.
#ifdef IR_TO_DRIVE_BUZZER
#undef Board_LED_IR
#define Board_LED_IR                Board_BUZZER
#endif //IR_TO_DRIVE_BUZZER
#ifdef  IRGENCC26XX_DEBUG
#define Board_IR_OUTPUT_DEBUG               Board_DP0  //DP0
#define Board_IR_DATA_CH_DEBUG              Board_DP1  //DP1
#define Board_IR_SHADOW_CH_DEBUG            Board_DP2  //DP2
#endif //IRGENCC26XX_DEBUG

// For debugging application
#ifdef APP_DEBUG_PINS
#define Board_DEBUG_APP             Board_DP3  //DP3
#endif //APP_DEBUG_PINS

/* PWM outputs */
#define Board_PWMPIN0                       PIN_UNASSIGNED
#define Board_PWMPIN1                       PIN_UNASSIGNED
#define Board_PWMPIN2                       PIN_UNASSIGNED
#define Board_PWMPIN3                       PIN_UNASSIGNED
#define Board_PWMPIN4                       PIN_UNASSIGNED
#define Board_PWMPIN5                       PIN_UNASSIGNED
#define Board_PWMPIN6                       PIN_UNASSIGNED
#define Board_PWMPIN7                       PIN_UNASSIGNED

/** ============================================================================
 *  Instance identifiers
 *  ==========================================================================*/
/* Generic Crypto instance identifiers */
#define Board_CRYPTO                CC2640R2RC_CRYPTO0

/* Generic I2C instance identifiers */
#define Board_I2C                   CC2640R2RC_I2C0

/* Generic UART instance identifiers */
#define Board_UART                  CC2640R2RC_UART0

/* Generic SPI instance identifiers */
#define Board_SPI0                  CC2640R2RC_SPI0

/* Generic TRNG instance identiifer */
#define Board_TRNG                  CC2650_TRNG0

/* Generic GPTimer instance identifiers */
#define Board_GPTIMER0A             CC2640R2RC_GPTIMER0A
#define Board_GPTIMER0B             CC2640R2RC_GPTIMER0B
#define Board_GPTIMER1A             CC2640R2RC_GPTIMER1A
#define Board_GPTIMER1B             CC2640R2RC_GPTIMER1B
#define Board_GPTIMER2A             CC2640R2RC_GPTIMER2A
#define Board_GPTIMER2B             CC2640R2RC_GPTIMER2B
#define Board_GPTIMER3A             CC2640R2RC_GPTIMER3A
#define Board_GPTIMER3B             CC2640R2RC_GPTIMER3B

/* Generic PWM instance identifiers */
#define Board_PWM0                  CC2640R2RC_PWM0
#define Board_PWM1                  CC2640R2RC_PWM1
#define Board_PWM2                  CC2640R2RC_PWM2
#define Board_PWM3                  CC2640R2RC_PWM3
#define Board_PWM4                  CC2640R2RC_PWM4
#define Board_PWM5                  CC2640R2RC_PWM5
#define Board_PWM6                  CC2640R2RC_PWM6
#define Board_PWM7                  CC2640R2RC_PWM7

#ifdef __cplusplus
}
#endif

#endif /* __CC2640R2RC_H__ */

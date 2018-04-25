/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
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

/**
 *  @file       SensorMpu9250.h
 *
 *  @brief      Driver for the InvenSense MPU9250 Motion Processing Unit.
 *
 *  # Driver include #
 *  This header file should be included in an application as follows:
 *  @code
 *  #include <ti/mw/sensors/SensorMpu9250.h>
 *  @endcode
 */
#ifndef SENSOR_MPU9250_H
#define SENSOR_MPU9250_H

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------------
 *                                          Includes
 * -----------------------------------------------------------------------------
 */
#include "stdint.h"
#include "stdbool.h"

/* -----------------------------------------------------------------------------
 *                                          Constants
 * -----------------------------------------------------------------------------
 */
// Accelerometer ranges
#define ACC_RANGE_2G      0       /**< Accelerometer range +- 2G */
#define ACC_RANGE_4G      1       /**< Accelerometer range +- 4G */
#define ACC_RANGE_8G      2       /**< Accelerometer range +- 8G */
#define ACC_RANGE_16G     3       /**< Accelerometer range +- 16G */
#define ACC_RANGE_INVALID 0xFF    /**< Accelerometer range: not valid*/

// Axis bitmaps
#define MPU_AX_GYR        0x07    /**< Bitmap of gyro axes (X=0, Y=1, Z=2) */
#define MPU_AX_ACC        0x38    /**< Bitmap of acc. axes (X=4, Y=8, Z=16) */
#define MPU_AX_MAG        0x40    /**< Bitmap of all mag. axes (32) */
#define MPU_AX_ALL        0x7F    /**< Bitmap of all MPU axes */

// Interrupt status bit
#define MPU_DATA_READY    0x01    /**< Movement data ready */
#define MPU_MOVEMENT      0x40    /**< Movement detected (WOM mode) */

// Magnetometer status
#define MAG_STATUS_OK     0x00    /**< Nor magnetometer error */
#define MAG_READ_ST_ERR   0x01    /**< Magnetometer error */
#define MAG_DATA_NOT_RDY  0x02    /**< Magnetometer data not ready */
#define MAG_OVERFLOW      0x03    /**< Magnetometer data overflow */
#define MAG_READ_DATA_ERR 0x04    /**< Error when reading data */
#define MAG_BYPASS_FAIL   0x05    /**< Magnetometer bypass enable failed */
#define MAG_NO_POWER      0x06    /**< No magnetometer power */

/* ----------------------------------------------------------------------------
 *                                           Typedefs
 * -----------------------------------------------------------------------------
*/
/** Signature of callback function for handling of MPU interrupts */
typedef void (*SensorMpu9250CallbackFn_t)(void);

/* -----------------------------------------------------------------------------
 *                                          Functions
 * -----------------------------------------------------------------------------
 */

 /**
* @brief       This function initializes the MPU abstraction layer.
*
* This function must be called before any access to the MMPU9250 can take place.
* It configures the IO lines connected to the, primarily the power signal. It also
* make sure that I2C lines are in a safe state also when power to the device is
* switched off.
*
* @return      true
*/ 
bool SensorMpu9250_init(void);

/**
* @brief       This function resets the MPU
*
* @return      true if success
*/
bool SensorMpu9250_reset(void);

/**
* @brief       Register an application defined call-back for interrupt processing.
*
* @param       pCallback - the function to be called on interrupt
*/
void SensorMpu9250_registerCallback(SensorMpu9250CallbackFn_t pCallback);

/**
 * @brief       Run a self-test of the gyro/accelerometer component.
 *
 * @return      true if passed
 */
bool SensorMpu9250_test(void);

/**
* @brief       Applies power to the MPU9250
*
*/
void SensorMpu9250_powerOn(void);

/**
* @brief       Remove power to the MPU9250
*
*/
void SensorMpu9250_powerOff(void);

/**
* @brief       get the state of the power supply to the MPU9250
*
* @return      true if the device is powered
*/
bool SensorMpu9250_powerIsOn(void);

/**
* @brief       Enable data collection
*
* @param       config - bitmap of axes to be enabled
*
* Enable individual axes of the movement sensor:<br>
*              Gyro bits [0..2], X = 1, Y = 2, Z = 4. 0 = gyro off<br>
*              Acc  bits [3..5], X = 8, Y = 16, Z = 32. 0 = accelerometer off<br>
*              MPU  bit [6], all axes<br>
*/
void SensorMpu9250_enable(uint16_t config);

/**
* @brief       Enable Wake On Motion functionality
*
* @param       threshold - wake-up trigger threshold (unit: 4 mg, max 1020mg)
*
* In this mode the gyro is disabled and the accelerometer is running in low power mode.
* The wake-on-motion interrupt is enabled and the selected wake-up trigger threshold 
* is applied.
*
* @return      true if success
*/
bool SensorMpu9250_womEnable(uint8_t threshold);

/**
* @brief       Read interrupt status register
*
* The interrupts used by the application are Data Ready(bit 0) and Movement Detected(bit 6).
*
* @return      Interrupt status
*/
uint8_t SensorMpu9250_irqStatus(void);

/**
* @brief       Set the accelerometer range
*
* @param       range: ACC_RANGE_2G, ACC_RANGE_4G, ACC_RANGE_8G, ACC_RANGE_16G
*
* @return      true if write succeeded
*/
bool SensorMpu9250_accSetRange(uint8_t range);

/**
* @brief       Read the accelerometer range
*
* @return      range: ACC_RANGE_2G, ACC_RANGE_4G, ACC_RANGE_8G, ACC_RANGE_16G
*/
uint8_t SensorMpu9250_accReadRange(void);

/**
* @brief       Read data from the accelerometer
*
* Read accelerometer data in the order z, y,z-axis (3 x 16 bits)
*
* @param       pData - buffer for raw data from accelerometer
*
* @return      true if valid data
*/
bool SensorMpu9250_accRead(uint16_t *pData);

/**
 * @brief      Convert raw data to G units
 *
 * @param      data - raw data from sensor
 *
 * @return     Converted value
 */
float SensorMpu9250_accConvert(int16_t data);

/**
* @brief       Read data from the gyroscope
*
* Read gyroscope data in the order z, y,z-axis (3 x 16 bits)
*
* @param       pData - buffer for raw data from gyroscope
*
* @return      true if valid data
*/
bool SensorMpu9250_gyroRead(uint16_t *pData);

/**
 * @brief      Convert raw data to deg/sec units
 *
 * @param      data - raw data from sensor
 *
 * @return     Converted value
 */
float SensorMpu9250_gyroConvert(int16_t data);

/**
 * @brief      Run a magnetometer self test
 *
 * @return     true if passed
 */
bool SensorMpu9250_magTest(void);

/**
* @brief       Read data from the compass - X, Y, Z - 3 words
*
* @param       pData - buffer for raw data from magnetometer
*
* @return      Magnetometer status
*/
uint8_t SensorMpu9250_magRead(int16_t *pData);

/**
* @brief       Return the magnetometer status
*
* @return      magnetometer status
*/
uint8_t SensorMpu9250_magStatus(void);

/**
 * @brief      Reset the magnetometer
 */
void SensorMpu9250_magReset(void);

/*******************************************************************************
*/

#ifdef __cplusplus
};
#endif

#endif

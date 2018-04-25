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
 *  @file       SensorI2C.h
 *
 *  @brief      Interface to the TI-RTOS I2C driver. 
 *
 *  Simplified access to the I2C driver and switching between the two I2C buses.
 *
 *  # Driver include #
 *  This header file should be included in an application as follows:
 *  @code
 *  #include <ti/mw/sensors/SensorI2C.h>
 *  @endcode
 */
#ifndef SENSOR_I2C_H
#define SENSOR_I2C_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "stdbool.h"

/*********************************************************************
 * CONSTANTS
 */
/** I2C interface 0: humidity, temperature, pressure, light, humidity */
#define SENSOR_I2C_0      0
/** I2C interface 1: movement sensor */
#define SENSOR_I2C_1      1
/** I2C interface not selected */
#define SENSOR_I2C_NONE  -1

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * FUNCTIONS
 */
 
/**
* @brief       Initialize the RTOS I2C driver (must be called only once)
*
* @return      true if I2C_open succeeds
*/
bool SensorI2C_open(void);

/**
* @brief       Select an I2C interface and I2C device
*
* @param       i2cInterface - selected interface (1 for MPU9250, 0 for other sensors)
* @param       i2cAddr - I2C slave address of device
*
* @return      true if success
*/
bool SensorI2C_select(uint8_t i2cInterface, uint8_t i2cAddr);

/**
* @brief       Read from an I2C device starting at \a regAddr
*
* @param       regAddr - which register to read
* @param       pData - pointer to data buffer
* @param       nBytes - number of bytes to read
*
* @return      true if the required number of bytes are received
*
* @pre         A sensor must be selected before this routine is called.
*/
bool SensorI2C_readReg(uint8_t regAddr, uint8_t *pData, uint8_t nBytes);

/**
* @brief       Write to an I2C device starting at \a regAddr
*
* @param       regAddr - which register to write
* @param       pData - pointer to data buffer
* @param       nBytes - number of bytes to write
*
* @return      true if successful write
*
* @pre         A sensor must be selected before this routine is called.
*/
bool SensorI2C_writeReg(uint8_t regAddr, uint8_t *pData, uint8_t nBytes);

/**
* @brief       Burst read from an I2C device
*
* @param       pData - pointer to data buffer
* @param       nBytes - number of bytes to write
*
* @return      true if success
*
* @pre         A sensor and register must be selected before this routine is called.
*/
bool SensorI2C_read(uint8_t *pData, uint8_t nBytes);

/**
* @brief       Burst write to an I2C device
*
* @param       pData - pointer to data buffer
* @param       nBytes - number of bytes to write
*
* @return      true if success
*
* @pre         A sensor and register must be selected before this routine is called.
*/
bool SensorI2C_write(uint8_t *pData, uint8_t nBytes);

/**
* @brief       Allow other tasks to access the I2C driver
*/
void SensorI2C_deselect(void);

/**
* @brief       Close the I2C interface and release the data lines
*
* @return      true if I2C_open succeeds
*/
void SensorI2C_close(void);

////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif

#endif /* SENSOR_I2C_H */

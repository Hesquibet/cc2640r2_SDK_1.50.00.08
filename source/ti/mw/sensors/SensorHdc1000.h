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
 *  @file       SensorHdc1000.h
 *
 *  @brief      Driver for Texas Instruments HCD1000 humidity sensor.
 *
 *  # Driver include #
 *  This header file should be included in an application as follows:
 *  @code
 *  #include <ti/mw/sensors/SensorHdc1000.h>
 *  @endcode
 */
#ifndef SENSOR_HDC1000_H
#define SENSOR_HDC1000_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "stdbool.h"
#include "stdint.h"

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * FUNCTIONS
 */
 
 /**
 * @brief       Initialize the sensor
 *
 * Configure reading of temperature and humidity as one read cycle,
 * and use 14 bit data size.
 *
 * @return      true if writing to the sensor succeeds
 */
bool SensorHdc1000_init(void);

/**
* @brief       Start measurement
*
* @pre         SensorHdc1000_init must be called (and succeed first).
*/
void SensorHdc1000_start(void);

/**
* @brief          Read humidity sensor data
*
* @param pRawTemp - buffer for raw temperature
*
* @param pRawHum  - buffer for raw humidity
*
* @return         true if I2C read operation succeeds
*/
bool SensorHdc1000_read(uint16_t *pRawTemp, uint16_t *pRawHum);

/**
* @brief          Convert raw data to temperature and humidity
*
* Calculate relative humidity and temperature based on provided
* raw data.
*
* @param rawTemp  - raw temperature value
*
* @param rawHum   - raw humidity value
*
* @param pTemp    - converted temperature [deg. C]
*
* @param pHum     - converted humidity
*/
void SensorHdc1000_convert(uint16_t rawTemp, uint16_t rawHum,
                           float *pTemp, float *pHum);
                           
/**
* @brief       Humidity sensor self test
*
* @return      true if test passes
*/
bool SensorHdc1000_test(void);
                           

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_HDC1000_H */

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
 *  @file       SensorBmp280.h
 *
 *  @brief      Driver for the Bosch BMP280 Pressure Sensor.
 *
 *  # Driver include #
 *  This header file should be included in an application as follows:
 *  @code
 *  #include <ti/mw/sensors/SensorBmp280.h>
 *  @endcode
 */
#ifndef SENSOR_BMP280_H
#define SENSOR_BMP280_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "stdint.h"
#include "stdbool.h"

/*********************************************************************
 * CONSTANTS
 */
/** Size of the data read from the sensor */
#define SENSOR_BMP280_DATASIZE           6 

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * FUNCTIONS
 */
 
/**
 * @brief       Initialize the sensor
 *
 * Reads and stores the calibration data for later use, then resets the sensor.
 *
 * @return      true if success
 */ 
bool SensorBmp280_init(void);

/**
 * @brief       Enable/disable measurement cycle
 *
 * This function enables or disables measurement according to the value of
 * \a enable. When enabled the sensor uses forced mode, 4x pressure oversampling,
 * and 1x temperature oversampling. When disabled the sensor is put in low power mode.
 *
 * @param       enable - flag to turn the sensor on/off.
 */
void SensorBmp280_enable(bool enable);

/**
 * @brief       Read temperature and pressure data
 *
 * Reads 6 bytes of data into the provided buffer (3 bytes of temperature 
 * followed by 3 bytes of pressure data. The data need to be process further
 * using the calibration data to calculate the atmospheric pressure.
 * 
 * @param       pData - buffer for raw data, temperature and pressure (6 bytes)
 *
 * @return      true if read operation succeeds and the data are valid
 */
bool SensorBmp280_read(uint8_t *pData);

/**
 * @brief       Convert raw data to object and ambiance temperature
 *
 * Calculate pressure and temperature using provided raw data and the 
 * stored calibration data.
 *
 * @param       pData - buffer for raw data from sensor
 *
 * @param       pTemp - converted temperature
 *
 * @param       pPress - converted pressure
 */
void SensorBmp280_convert(uint8_t *pData, int32_t *pTemp, uint32_t *pPress);

/**
 * @brief       Run a sensor self-test
 *
 * @return      true if passed
 */
bool SensorBmp280_test(void);


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_BMP280_H */

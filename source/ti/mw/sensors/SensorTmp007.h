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
 *  @file       SensorTmp007.h
 *
 *  @brief      Driver for the Texas Instruments TMP007 infra-red thermopile sensor.
 *
 *  # Driver include #
 *  This header file should be included in an application as follows:
 *  @code
 *  #include <ti/mw/sensors/SensorTmp007.h>
 *  @endcode
 */
#ifndef SENSOR_TMP007_H
#define SENSOR_TMP007_H

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


/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * FUNCTIONS
 */

 /**
 * @brief       Initialize the temperature sensor driver
 *
 * @return      true if success
 */
bool SensorTmp007_init(void);

/**
 * @brief       Enable/disable measurement cycle
 *
 * @param       enable - flag to turn the sensor on/off
 *
 * @return      true if success
 */
bool SensorTmp007_enable(bool enable);

/**
 * @brief       Run a sensor self-test
 *
 * @return      true if passed
 */
bool SensorTmp007_test(void);

/**
 * @brief       Read the sensor temperature registers
 *
 * @param       rawLocalTemp - local temperature in 16 bit format
 *
 * @param       rawObjTemp - object temperature in 16 bit format
 *
 * @return      true if valid data
 */
bool SensorTmp007_read(uint16_t *rawLocalTemp, uint16_t *rawObjTemp);

/**
 * @brief       Convert raw data to temperature [C]
 *
 * @param       rawLocalTemp - raw temperature from sensor
 *
 * @param       rawObjTemp - raw temperature from sensor
 *
 * @param       tObj - converted object temperature
 *
 * @param       tLocal - converted die temperature
 */
void SensorTmp007_convert(uint16_t rawLocalTemp, uint16_t rawObjTemp, float *tObj, float *tLocal);


#ifdef __cplusplus
}
#endif

#endif /* SENSOR_TMP007_H */

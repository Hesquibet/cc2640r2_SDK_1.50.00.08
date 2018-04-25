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
 *  @file       SensorUtil.h
 *
 *  @brief      Utilities for sensor drivers
 *
 *  # Driver include #
 *  This header file should be included in an application as follows:
 *  @code
 *  #include <ti/mw/sensors/SensorUtil.h>
 *  @endcode
 */
#ifndef SENSOR_UTIL_H
#define SENSOR_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/*********************************************************************
 * CONSTANTS and MACROS
 */

/** bool assertion; return 'false' and release I2C if condition is not met */
#define ST_ASSERT(cond)   ST( if (!(cond)) {SensorI2C_deselect(); return false;})

/** void assertion; return and release I2C if condition is not met */
#define ST_ASSERT_V(cond) ST( if (!(cond)) {SensorI2C_deselect(); return;} )

/** Loop enclosure for macros */
#define ST(x)             do { x } while (__LINE__ == -1)

/* Conversion macros */
/** Extract MSB from a 16 bit word */
#define HI_UINT16(a)     (((a) >> 8) & 0xFF)
/** Extract LSB from a 16 bit word */
#define LO_UINT16(a)     ((a) & 0xFF)
/** Swap bytes within a 16 bit word */
#define SWAP(v)          ((LO_UINT16(v) << 8) | HI_UINT16(v))

/* Delay */
/** Delay task execution by (n) milliseconds */
#define DELAY_MS(i)      (Task_sleep(((i) * 1000) / Clock_tickPeriod))
/** Delay task execution by (n) microseconds */
#define DELAY_US(i)      (Task_sleep(((i) * 1) / Clock_tickPeriod))
/** Convert milliseconds to RTOS system ticks */
#define MS_2_TICKS(ms)   (((ms) * 1000) / Clock_tickPeriod)

/*********************************************************************
* FUNCTIONS
*/

/**
* @brief   Convert 16-bit words to/from big-endian to little-endian
*
* @param   pData - buffer to the data to be converted
*
* @param   nWords - number of 16-bit words to be swapped
*/
void     SensorUtil_convertToLe(uint8_t *pData, uint8_t nWords);

/**
* @brief   Convert a float to a short float
*
* @param   data - floating point number to convert
*
* @return  converted value
*/
uint16_t SensorUtil_floatToSfloat(float data);


/**
* @brief   Convert a short float to a float
*
* @param   data - short float number to convert
*
* @return  converted value
*/
float    SensorUtil_sfloatToFloat(uint16_t data);


/**
* @brief   Convert an integer to a short float
*
* @param   data - integer to convert
*
* @return  converted value
*/
uint16_t SensorUtil_intToSfloat(int data);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_UTIL_H */

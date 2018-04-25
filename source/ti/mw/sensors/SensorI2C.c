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

/* -----------------------------------------------------------------------------
*  Includes
* ------------------------------------------------------------------------------
*/
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/i2c/I2CCC26XX.h>

#include "Board.h"
#include "SensorUtil.h"
#include "SensorI2C.h"

/* -----------------------------------------------------------------------------
*  Constants
* ------------------------------------------------------------------------------
*/
#define I2C_TIMEOUT 500

/* -----------------------------------------------------------------------------
*  Public Variables
* ------------------------------------------------------------------------------
*/

/* -----------------------------------------------------------------------------
*  Private Variables
* ------------------------------------------------------------------------------
*/

/* I2C driver interface */
static I2C_Handle i2cHandle;
static I2C_Params i2cParams;
static Semaphore_Struct mutex;
static const I2CCC26XX_I2CPinCfg pinCfg1 =
{
    // Pin configuration for I2C interface 1
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

/* Module state */
static volatile uint8_t interface;
static volatile uint8_t slaveAddr;
static uint8_t buffer[32];

/* -----------------------------------------------------------------------------
*  Public Functions
* ------------------------------------------------------------------------------
*/

/*
 *  ======== SensorI2C_write ========
 */
bool SensorI2C_write(uint8_t *data, uint8_t len)
{
    I2C_Transaction masterTransaction;

    masterTransaction.writeCount   = len;
    masterTransaction.writeBuf     = data;
    masterTransaction.readCount    = 0;
    masterTransaction.readBuf      = NULL;
    masterTransaction.slaveAddress = slaveAddr;

    return I2C_transfer(i2cHandle, &masterTransaction) == TRUE;
}

/*
 *  ======== SensorI2C_writeSingle ========
 */
 bool SensorI2C_writeSingle(uint8_t data)
{
    uint8_t d;

    d = data;

    return SensorI2C_write(&d, 1);
}

/*
 *  ======== SensorI2C_read ========
 */
bool SensorI2C_read(uint8_t *data, uint8_t len)
{
    I2C_Transaction masterTransaction;

    masterTransaction.writeCount = 0;
    masterTransaction.writeBuf = NULL;
    masterTransaction.readCount = len;
    masterTransaction.readBuf = data;
    masterTransaction.slaveAddress = slaveAddr;

    return I2C_transfer(i2cHandle, &masterTransaction) == TRUE;
}

/*
 *  ======== SensorI2C_writeRead ========
 */
bool SensorI2C_writeRead(uint8_t *wdata, uint8_t wlen, uint8_t *rdata, uint8_t rlen)
{
    I2C_Transaction masterTransaction;

    masterTransaction.writeCount = wlen;
    masterTransaction.writeBuf = wdata;
    masterTransaction.readCount = rlen;
    masterTransaction.readBuf = rdata;
    masterTransaction.slaveAddress = slaveAddr;

    return I2C_transfer(i2cHandle, &masterTransaction) == TRUE;
}


/*
 *  ======== SensorI2C_readReg ========
 */
bool SensorI2C_readReg(uint8_t addr, uint8_t *pBuf, uint8_t nBytes)
{
    return SensorI2C_writeRead(&addr,1,pBuf,nBytes);
}

/*
 *  ======== SensorI2C_writeReg ========
 */
bool SensorI2C_writeReg(uint8_t addr, uint8_t *pBuf, uint8_t nBytes)
{
    uint8_t i;
    uint8_t *p = buffer;

    /* Copy address and data to local buffer for burst write */
    *p++ = addr;
    for (i = 0; i < nBytes; i++)
    {
        *p++ = *pBuf++;
    }
    nBytes++;

    /* Send data */
    return SensorI2C_write(buffer,nBytes);
}

/*
 *  ======== SensorI2C_select ========
 */
bool SensorI2C_select(uint8_t newInterface, uint8_t address)
{
    // Acquire I2C resource
    if (!Semaphore_pend(Semaphore_handle(&mutex),MS_2_TICKS(I2C_TIMEOUT)))
    {
        return false;
    }

    // Store new slave address
    slaveAddr = address;

    // Interface changed ?
    if (newInterface != interface)
    {
        // Store new interface
        interface = newInterface;

        // Shut down RTOS driver
        I2C_close(i2cHandle);

        // Sets custom to NULL, selects I2C interface 0
        I2C_Params_init(&i2cParams);

        // Assign I2C data/clock pins according to selected I2C interface 1
        if (interface == SENSOR_I2C_1)
        {
            i2cParams.custom = (void *)&pinCfg1;
        }

        // Re-open RTOS driver with new bus pin assignment
        i2cHandle = I2C_open(Board_I2C0, &i2cParams);
    }

    return i2cHandle != NULL;
}

/*
 *  ======== SensorI2C_deselect ========
 */
void SensorI2C_deselect(void)
{
    // Release I2C resource
    Semaphore_post(Semaphore_handle(&mutex));
}

/*
 *  ======== SensorI2C_open ========
 */
bool SensorI2C_open(void)
{
    Semaphore_Params semParamsMutex;

    // Create protection semaphore
    Semaphore_Params_init(&semParamsMutex);
    semParamsMutex.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&mutex, 1, &semParamsMutex);

    // Initialize I2C bus
    I2C_init();
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2cHandle = I2C_open(Board_I2C0, &i2cParams);

    // Initialize local variables
    slaveAddr = 0xFF;
    interface = SENSOR_I2C_0;

    return i2cHandle != NULL;
}

/*
 *  ======== SensorI2C_close ========
 */
void SensorI2C_close(void)
{
    if (i2cHandle != NULL)
    {
        I2C_close(i2cHandle);
    }
}

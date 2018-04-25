/******************************************************************************

 @file       bim_util.c

 @brief This module contains the definitions for the main functionality of a
        Boot  Image Manager.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2012-2017, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 Release Name: simplelink_cc2640r2_sdk_01_50_00_58
 Release Date: 2017-10-17 18:09:51
 *****************************************************************************/

/*******************************************************************************
 *                                          Includes
 */

#include <inc/hw_device.h>
#include <driverlib/sys_ctrl.h>
#include <driverlib/aon_event.h>
#include <driverlib/osc.h>
#include <driverlib/setup.h>
#include <driverlib/pwr_ctrl.h>

#include "hal_types.h"
#include "oad_image_header.h"
#include "crc32.h"
#include "flash_interface.h"

/*******************************************************************************
 * LOCAL VARIABLES
 */

const uint8_t OAD_IMG_ID[OAD_IMG_ID_LEN] = OAD_IMG_ID_VAL;
const uint8_t OAD_EXTFL_ID[OAD_IMG_ID_LEN] = OAD_EXTFL_ID_VAL;

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * @fn          halSleepExec
 *
 * @brief       This function puts the device to sleep.
 *
 * @param       None.
 *
 * @return      None.
 */
void halSleepExec(void)
{
    AONIOCFreezeEnable();
    SysCtrlAonSync();
    PRCMPowerDomainOff(PRCM_DOMAIN_CPU);

    //Propogate changes to AON.
    PRCMLoadSet();

    VIMSModeSet(VIMS_BASE, VIMS_MODE_OFF);

    PRCMCacheRetentionDisable();

    SysCtrlAonSync();

    // Sleep.
    PRCMDeepSleep();

    // Reset
    HAL_SYSTEM_RESET();
}

/*******************************************************************************
 * @fn          jumpToPrgEntry
 *
 * @brief       This function jumps the execution to program entry to execute
 *              application
 *
 * @param       prgEntry - address of application entry.
 *
 * @return      None.
 */
void jumpToPrgEntry(uint32_t prgEntry)
{
#ifdef __IAR_SYSTEMS_ICC__

    prgEntry +=4;
    uint32_t *entry = (uint32_t *)&prgEntry;
    __asm volatile( "LDR R2, [%0]\n\t"    :: "r" (entry));
    asm(" LDR.W R2, [R2] ");

    // Reset the stack pointer,
    asm(" LDR SP, [R0, #0x0] ");
    asm(" BX R2 ");

#elif defined(__TI_COMPILER_VERSION__)

    // Reset the stack pointer,
    prgEntry +=4;
    asm(" LDR SP, [R0, #0x0] ");
    ((void (*)(void))(*((uint32_t*)prgEntry)))();

#endif
}

/*******************************************************************************
 * @fn          imgIDCheck
 *
 * @brief       This function check if the buffer contains OAD image
 *              identification value.
 *
 * @param       imgHdr - pointer to image identification header buffer.
 *
 * @return      None.
 */
bool imgIDCheck(imgHdr_t *imgHdr)
{
    uint8_t* pBuff = imgHdr->imgID;

    int8 idLen = OAD_IMG_ID_LEN - 1;
    while( idLen >= 0)
    {
        if (pBuff[idLen] != OAD_IMG_ID[idLen])
        {
            return(FALSE);
        }
        idLen--;
    }
    return(TRUE);
}

/*******************************************************************************
 * @fn          metadataIDCheck
 *
 * @brief       This function check if the buffer contains external flash OAD
 *              metadata identification value.
 *
 * @param       imgHdr - pointer to flash metadata header buffer.
 *
 * @return      None.
 */
bool metadataIDCheck(imgHdr_t *imgHdr)
{
    uint8_t* pBuff = imgHdr->imgID;

    int8 idLen = OAD_IMG_ID_LEN - 1;
    while( idLen >= 0)
    {
        if (pBuff[idLen] != OAD_EXTFL_ID[idLen])
        {
            return(FALSE);
        }
        idLen--;
    }
    return(TRUE);
}

/*******************************************************************************
*/

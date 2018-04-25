/******************************************************************************

 @file       bim_onchip.c

 @brief This module contains the definitions for the supporting functionality of
        a Boot Image Manager.

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

/* -----------------------------------------------------------------------------
 *                                          Includes
 * -----------------------------------------------------------------------------
 */
#include "hal_flash.h"
#include "hal_types.h"
#include "oad_image_header.h"
#include "flash_interface.h"
#include "bim_util.h"

/*******************************************************************************
 *                                          Constants
 */

#define DEFAULT_RAM             0
#define OAD_IMG_ID_LEN          8
#define BIM_VER                0x1     /* Supported BIM version */
#define META_VER               0x1     /* Supported metadata version */

/*******************************************************************************
 * @fn
 *
 * @brief       This function sets the value of local variable based on the
 *              content of RAM varaible bimVar
 *
 * @param       bimVar - RAM varaible.
 * @param       FN - pointer to local variable flash page number
 * @param       IT - pointer to local variable image type
 *
 * @return      None.
 */
void setSearchVar(uint16_t bimVar, uint8_t *FN, uint8_t *IT)
{
    if((bimVar == DEFAULT_RAM) || ((bimVar & 0xFF) > 1))
    {
        *FN = 0;
        *IT = 1;
    }
    else
    {
        if((bimVar & 0xFF) == 0)    /* Valid flash page number */
        {
            *FN = (bimVar & 0xFF00) >> 8;
            *IT = 0xFF;
        }
        else /* Valid Image Type */
        {
            *IT = (bimVar & 0xFF00) >> 8;
            *FN = 0xFF; /* start with flash page 0 */
        }
    }
}

/*******************************************************************************
 * @fn     copyFlashImage
 *
 * @brief  Copies firmware image into the executable flash area.
 *
 * @param  imgStart - starting address of image in external flash.
 * @param  imgLen   - size of image in bytes
 * @param  dstAddr  - destination address within internal flash.
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
int8_t copyFlashImage(uint32_t imgStart, uint32_t imgLen, uint32_t dstAddr)
{
    if(!imgLen)
    {
        return(-1);
    }

    uint8_t page = (imgStart)?(FLASH_PAGE(imgStart)):0;
    uint8_t pageEnd = (imgLen)?(FLASH_PAGE(imgStart + imgLen - 1)):0;
    pageEnd += page;

    uint8_t dstPageStart = FLASH_PAGE(dstAddr);

    /* Change image size into word unit */
    imgLen += 3;
    imgLen >>= 2;

    if (dstAddr & 3)
    {
        /* Not an aligned address */
        return(-1);
    }

    if (page > pageEnd || pageEnd > 30)
    {
        return(-1);
    }

    for (; page <= pageEnd; page++)
    {
        uint32_t buf;
        uint16_t count = (HAL_FLASH_PAGE_SIZE / 4);

        if (page == pageEnd)
        {
            /* Count could be shorter */
            count = imgLen;
        }

        /* Erase destination flash page */
        eraseFlash(dstPageStart++);

        /* Erase the page */
        while (count-- > 0)
        {
            /* Read word from flash */
            readFlash(imgStart, (uint8_t *)&buf, 4);

            /* Write word to flash */
            writeFlash(dstAddr, (uint8_t *)&buf, 4);
            imgStart += 4;
            dstAddr += 4;
            imgLen--;
        }
    } /* for (; page <= pageEnd; page++) */
    return(0);
}

/**************************************************************************************************
*/

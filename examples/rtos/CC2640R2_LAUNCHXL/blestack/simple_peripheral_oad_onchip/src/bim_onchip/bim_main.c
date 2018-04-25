/******************************************************************************

 @file       bim_main.c

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

#include "hal_flash.h"
#include "hal_types.h"
#include "oad_image_header.h"
#include "led_debug.h"
#include "bim_onchip.h"
#include "bim_util.h"
#include "crc32.h"
#include "flash_interface.h"

/*******************************************************************************
 *                                          Constants
 */

#define SUCCESS                0

static uint8_t FN = 0;   /* Flash page number */
static uint8_t IT = 0;   /* Image Type */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// BIM variable
#ifdef __TI_COMPILER_VERSION__
  #pragma location = BIM_VAR_ADDR
  #pragma NOINIT(_bim_var)
  uint32_t _bim_var;
#elif __IAR_SYSTEMS_ICC__
  __no_init uint32_t _bim_var @ BIM_VAR_ADDR;
#endif

/*******************************************************************************
 * @fn     checkPgBoundary
 *
 * @brief  Checks if the last page of the external flash page is reached if not
 *         increments the page and returns the page number. If it reached at the
 *         last flash page then reset the flash page number and switch the image
 *         type if needed and increments the serch iteration number.
 *
 * @param  flashPageNum - flash page number
 *         imgType      - image type to be searched
 *         searchItrNum - search iteration number
 *
 * @return None.
 */
static void checkPgBoundary(uint8_t *flashPageNum, uint8_t *imgType, uint8_t *searchItrNum)
{
    *flashPageNum += 1;
    if (*flashPageNum >= MAX_ONCHIP_FLASH_PAGES) /* End of flash reached */
    {
        /* If the serach was for user application, and uanble to find that switch to
           persistent image or vice versa
         */
        *flashPageNum = 0;
        if (*imgType == OAD_IMG_TYPE_APP)
        {
            *imgType = OAD_IMG_TYPE_PERSISTENT_APP;
        }
        else
        {
            *imgType = OAD_IMG_TYPE_APP;
        }

        *searchItrNum += 1;
    }
}

/*******************************************************************************
 * @fn     findImage
 *
 * @brief  Finds the image type specified in FN and IT paramaters, starting with
 *         the page number flashPageNum and executes the image it finds the valid
 *         image. If not, sets the imageType, flashPageNum, searchItr to restart
 *         the search for alternative image.
 *
 * @param  FN - Flash page number as per RAM variable
 *         IT - Image Type as per RAM variable
 *         flashPageNum - flash page number, serves both i/p a o/p parameter
 *         imgType      - image type to be searched
 *         searchItrNum - search iteration number
 *
 * @return None.
 */
static uint8_t findImage(uint8_t FN, uint8_t IT, uint8_t *flashPageNum, uint8_t *imgType, uint8_t *searchItrNum)
{
    imgHdr_t imgHdr;

    /*
    If FN is valid the jump to flash page number and read image header
    Read flash to find OAD image identification value
    */
    readFlashPg(*flashPageNum, 0, &imgHdr.imgID[0], OAD_IMG_ID_LEN);

    /* Check imageID bytes */
    if (imgIDCheck(&imgHdr) != TRUE)
    {
        if (FN != 0xFF) /* go to next flash page */
        {
            checkPgBoundary(flashPageNum, imgType, searchItrNum);

        } /* set FN=0 and try to find valid user application */
        else  /* Searching for image type */
        {
            /* continue on next flash page */
            *flashPageNum += 1;
            if (*flashPageNum >= MAX_ONCHIP_FLASH_PAGES) /* End of flash reached */
            {
                /*
                If the serach was for user application, and uanble to find then
                switch to persistent image or vice versa
                */
                *flashPageNum = 0;
                if (*imgType == OAD_IMG_TYPE_APP)
                {
                    *imgType = OAD_IMG_TYPE_PERSISTENT_APP;  /* User application */
                }
                else
                {
                    *imgType = OAD_IMG_TYPE_APP;
                }

                *searchItrNum += 1;
            }
        }
        return(*flashPageNum);
    }
    else
    {
        /* Read whole image header */
        readFlashPg(*flashPageNum, 0, (uint8_t *)&imgHdr, OAD_IMG_HDR_LEN);

        if( ((*imgType != 0xFF) && (*imgType != imgHdr.imgType) && (imgHdr.imgCpStat != NEED_COPY))
              ||
            ((imgHdr.imgCpStat == NEED_COPY) && (imgHdr.imgType != OAD_IMG_TYPE_STACK)) )
        {
            checkPgBoundary(flashPageNum, imgType, searchItrNum);

            /* and return so that same process can be repeated on next flash page */
            return(*flashPageNum);
        }

        /* check BIM and Metadata version */
        if( (imgHdr.bimVer != BIM_VER  || imgHdr.metaVer != META_VER) /* Invalid metadata version */
              ||
            (imgHdr.crcStat == 0xFC) ) /* Invalid CRC */
        {
            /*
            We found incompatible image, most probably download stack or user application,
            switch back to persistent image so that new compatible image can be downloaded
            */
            if( ((imgHdr.imgType == IT) || (*flashPageNum == FN))// found the intended image)
                   &&
                (*imgType != OAD_IMG_TYPE_PERSISTENT_APP) )
            {
                *imgType = OAD_IMG_TYPE_PERSISTENT_APP;
                *flashPageNum += 1; /* increment flash page number  */
                /* and return so that same process can be repeated on next flash page */
                return(*flashPageNum);
            }
            else /* it is persistent image currpted, mean we got problem */
            {
                /* should we end the search or search for user application */
                *searchItrNum += 1;
                *imgType = OAD_IMG_TYPE_APP;
                *flashPageNum += 1; /* increment flash page number */
            }
        }
        else if(imgHdr.crcStat == 0xFF && imgHdr.imgType != OAD_IMG_TYPE_PERSISTENT_APP) /* CRC not calculated */
        {
            /* Calculate the CRC over the data buffer and update status */
            uint32_t crc32 = 0;
            uint8_t  crcstat = CRC_VALID;
            crc32 = crcCalc(*flashPageNum, 0, false);

            /* Check if calculated CRC matched with the image header */
            if (crc32 != imgHdr.crc32)
            {
                /* Update CRC status */
                crcstat = CRC_INVALID;
                writeFlashPg(*flashPageNum, CRC_STAT_OFFSET, (uint8_t *)&crcstat, 1);

                /*
                If it is stack only or user application only image, searh for
                valid target application to switch to
                */
                if ((imgHdr.imgType == OAD_IMG_TYPE_STACK) || (imgHdr.imgType == OAD_IMG_TYPE_APP))
                {
                    *imgType = OAD_IMG_TYPE_PERSISTENT_APP;
                }

                /*
                We found persistent application with invalid CRC, we should not suppose to
                meet this condition optionally we should try to find valid use application if possible
                */
                else if ((imgHdr.imgType == *imgType) && (imgHdr.imgType == OAD_IMG_TYPE_PERSISTENT_APP))
                {
                    *imgType = OAD_IMG_TYPE_PERSISTENT_APP;
                }
                *searchItrNum += 1;

                *flashPageNum = 0;

                /* Return so that same process can be repeated on next flash page */
                return(*flashPageNum);
            } /* if (crc32 != imgHdr.crc32) */

            /*
             Image don't need to be copied, and CRC matched else this would upset the
             crc calculation of the image to be copied
             */
            if(imgHdr.imgCpStat != NEED_COPY)
            {
                /* Update CRC status */
                writeFlashPg(*flashPageNum, CRC_STAT_OFFSET, (uint8_t *)&crcstat, 1);
            }

        } /* else if(imgHdr.crcStat == 0xFF) */

        /*
         We get here, we must have valid CRC
         Check image copy status,  On CC26XXR2 platform only stack application needes
         to be copied
         */
        if (imgHdr.imgCpStat == NEED_COPY)
        {
            if (imgHdr.imgType != OAD_IMG_TYPE_STACK)
            {
                /*
                 Shouldn't have come here, as only 'stack only' image should need copy,
                 set image type to target app type to find target application and
                 Flash Page Number=0
                 */
                *imgType = OAD_IMG_TYPE_PERSISTENT_APP;
                *flashPageNum = 0;

                /* Return so that same process can be repeated on next flash page */
                return(*flashPageNum);
            }

            /*
             On CC26XXR2 platform only stack application needes to be copied
             Do the image copy
             */
            uint32_t imgStart = (*flashPageNum)?FLASH_ADDRESS(*flashPageNum, 0):0;
            uint8_t status = COPY_DONE;

            /* If image copy is successful */
            int8_t retVal = copyFlashImage(imgStart, imgHdr.len, imgHdr.startAddr);

            if(retVal == SUCCESS)
            {

                /*
                 * The crcStat and cpStat fields have changed, the crc is
                 * different, recalculate it
                 */
                uint32_t preCopyCrc = crcCalc(*flashPageNum, 0, false);

                uint32_t crc32 = crcCalc(FLASH_PAGE(imgHdr.startAddr), 0, false);
                status = CRC_VALID;

                if (crc32 == preCopyCrc) /* if crc matched then update its status in the copied image */
                {
                    writeFlashPg(FLASH_PAGE(imgHdr.startAddr), CRC_STAT_OFFSET, &status, 1);
                }
                else
                {
                    status = CRC_INVALID;
                    writeFlashPg(FLASH_PAGE(imgHdr.startAddr), CRC_STAT_OFFSET, &status, 1);
                }

                /*
                 *  We need to write the copy done field to both the copied stack
                 * and the one in the scratch space so the copy doesn't continuously loop
                 */
                status = COPY_DONE;
                writeFlashPg(FLASH_PAGE(imgHdr.startAddr), IMG_COPY_STAT_OFFSET, (uint8_t *)&status, 1);
                writeFlashPg(*flashPageNum, IMG_COPY_STAT_OFFSET, (uint8_t *)&status, 1);

                *imgType = OAD_IMG_TYPE_PERSISTENT_APP;
                *flashPageNum = 0;
                return(*flashPageNum);
            }
            else
            {
                writeFlashPg(*flashPageNum, IMG_COPY_STAT_OFFSET, (uint8_t *)&status, 1);
            }
        } /* if (imgHdr.imgCpStat == NEED_COPY)  */

        if ((imgHdr.imgType == OAD_IMG_TYPE_APP) || (imgHdr.imgType == OAD_IMG_TYPE_PERSISTENT_APP))
        {
            jumpToPrgEntry(imgHdr.prgEntry);
        }
        else
        {
           /* Unable to find valid executable image, try to find valid user application */
           *flashPageNum += 1;
           *imgType = 0x1;
        }
    }
    return(0);
}
/*******************************************************************************
 * @fn          main
 *
 * @brief       C-code main function.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
void main(void)
{
    /* For LED feedback */
#ifdef LED_DEBUG
    powerUpGpio();
    blinkLed(IOID_6, 20, 100);
    blinkLed(IOID_7, 15, 100);
    powerDownGpio();
#endif

    setSearchVar(_bim_var, &FN, &IT);

    uint8_t searchItr = 0;
    uint8_t flashPgNum = FN;
    uint8_t imgType = 0;

    flashPgNum = (FN != 0xFF) ? FN : 0;
    imgType = IT;

    while (searchItr < 3)
    {
        findImage(FN, IT, &flashPgNum, &imgType, &searchItr);
    }

#ifdef BIM_DEBUG
#ifdef LED_DEBUG
       powerUpGpio();
    while(1)
    {
        lightRedLed();
    }
    powerDownGpio(); /* This code doesn't execute added for completeness */
#else
      while(1);
#endif /* ifdef LED_DEBUG */
#endif /* ifdef BIM_DEBUG */

    /* Neither image is ready to run.  Go to sleep. */
    halSleepExec(); /* Should not get here. */
}

/**************************************************************************************************
*/

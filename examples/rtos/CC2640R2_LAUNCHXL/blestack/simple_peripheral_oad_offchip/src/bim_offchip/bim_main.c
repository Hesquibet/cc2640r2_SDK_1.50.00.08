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
#include <stdint.h>
#include <string.h>
#include <driverlib/flash.h>
#include <driverlib/watchdog.h>
#include <inc/hw_prcm.h>

#include "hal_flash.h"
#include "ext_flash_layout.h"
#include "crc32.h"
#include "ext_flash.h"
#include "flash_interface.h"
#include "bim_util.h"
#include "oad_image_header.h"
#include "bim_util.h"

/*******************************************************************************
 *                                          Constants
 */

#define IMG_HDR_FOUND                  -1
#define EMPTY_METADATA                 -2

#define SUCCESS                         0

#if defined(CREATE_FACT_IMG_INT_TO_EXT_FLSH) || defined(TEST_IMG_COPY)

#define INTFLASH_BLE_APP_ADDR           0x0
#define EFL_SIZE_FACT_IMAGE             0x1C000
#define EFL_ADDR_META_FACT_IMG_BLE      0x0    /* BLE factory image metadata address */

#endif /* if defined(CREATE_FACT_IMG_INT_TO_EXT_FLSH) || defined(TEST_IMG_COPY) */

#define EFL_METADATA_LEN                52    /* Metadata header length in bytes */
#define EFL_IMG_STR_ADDR_OFFSET         (44/sizeof(uint32_t)) /* External flash storage address offset in words */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * @fn     Bim_intErasePage
 *
 * @brief  Erase an internal flash page.
 *
 * @param  page - page number
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
static int8_t Bim_intErasePage(uint_least16_t page)
{
    /* Note that normally flash */
    if(FlashSectorErase(page * HAL_FLASH_PAGE_SIZE) == FAPI_STATUS_SUCCESS)
    {
        return(0);
    }

    return(-1);
}

/*******************************************************************************
 * @fn     Bim_intWriteWord
 *
 * @brief  Write a word to internal flash.
 *
 * @param  addr - address
 * @param  word - value to write
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
static int8_t Bim_intWriteWord(uint_least32_t addr, uint32_t word)
{
    if(FlashProgram((uint8_t *) &word, addr, 4) == FAPI_STATUS_SUCCESS)
    {
        return(0);
    }

    return(-1);
}

/*******************************************************************************
 * @fn     Bim_copyImage
 *
 * @brief  Copies firmware image into the executable flash area.
 *
 * @param  imgStart - starting address of image in external flash.
 * @param  imgLen   - size of image in 4 byte blocks.
 * @param  dstAddr  - destination address within internal flash.
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
static int8_t Bim_copyImage(uint32_t imgStart, uint32_t imgLen,
                                uint32_t dstAddr)
{
    uint_fast16_t page = dstAddr/HAL_FLASH_PAGE_SIZE;
    uint_fast16_t lastPage;

    lastPage = (uint_fast16_t) ((dstAddr + imgLen - 1) / HAL_FLASH_PAGE_SIZE);

    /* Change image size into word unit */
    imgLen += 3;
    imgLen /= 4;

    if(dstAddr & 3)
    {
        /* Not an aligned address */
        return(-1);
    }

    if(page > lastPage || lastPage > 30)
    {
        return(-1);
    }

    for (; page <= lastPage; page++)
    {
        uint32_t buf;
        uint_least16_t count = (HAL_FLASH_PAGE_SIZE / 4);

        if(page == lastPage)
        {
            /* count could be shorter */
            count = imgLen;
        }

      /* Erase the page */
        if(Bim_intErasePage(page))
        {
            /* Page erase failed */
            return(-1);
        }
        while (count-- > 0)
        {
            /* Read word from external flash */
            if(!extFlashRead(imgStart, 4, (uint8_t *)&buf))
            {
                /* read failed */
                return(-1);
            }
            /* Write word to internal flash */
            if(Bim_intWriteWord(dstAddr, buf))
            {
                /* Program failed */
                return(-1);
            }

            imgStart += 4;
            dstAddr += 4;
            imgLen--;
        }
    }
    /* Do not close external flash driver here just return */
    return(0);
}

#ifdef CREATE_FACT_IMG_INT_TO_EXT_FLSH
/*******************************************************************************
 * @fn     Bim_createFactoryImageBackup
 *
 * @brief  Copies the internal application + stack image to the external flash
 *         to serve as the permanant resident factory image in external flash.
 *
 * @param  imgStart - starting address of image in external flash.
 * @param  imgLen   - size of image in 4 byte blocks
 * @param  dstAddr  - destination address within internal flash.
 *
 * @return none
 */
static uint8_t Bim_createFactoryImageBackup(uint32_t imgStart, uint32_t imgLen,
                                            uint32_t dstAddr)
{
    uint32_t *srcAddr;
    uint32_t writeBuf;
    uint32_t tempReadBuf;

    bool rtn = true;
    uint32_t dstAddrStart = dstAddr;

    /* initialize external flash driver */
    if(extFlashOpen() != 0)
    {
        /* Erase - external portion to be written*/
        if(extFlashErase(dstAddr, imgLen))
        {
            /* COPY - image from internal to external */
            if(extFlashWrite(dstAddr, imgLen, (uint8_t *)imgStart))
            {
                /* VERIFY - if image copied correctly */
                for(srcAddr = (uint32_t *)imgStart, dstAddr = dstAddrStart;
                        ((uint32_t)srcAddr) < imgStart + imgLen;
                         srcAddr++, dstAddr+=4)
                {
                    /* Read internal flash */
                    writeBuf = *((uint32_t*)srcAddr);

                    /* clear read buffer */
                    tempReadBuf = 0;

                    /* read word from external flash */
                    if(extFlashRead(dstAddr, 4, (uint8_t *)&tempReadBuf) == false)
                    {
                        /* Program failed */
                        rtn = false;
                        break;
                    }

                    /* verify word in external flash */
                    if(writeBuf != tempReadBuf)
                    {
                        /* Program failed */
                        rtn = false;
                        break;
                    }

                }
                if(rtn != false)
                {
                    imgHdr_t imgHdr = { .imgID = OAD_EXTFL_ID_VAL }; /* Write OAD flash metadata identification */

                    /* Copy Image header from internal flash image, skip ID values */
                    memCpy((uint8_t *)&imgHdr.crc32, ((uint8_t *)imgStart + 8) , EFL_METADATA_LEN);

                    /*
                     * Calculate the CRC32 value and update that in image header as CRC32
                     * wouldn't be available for running image.
                     */
                    imgHdr.crc32 = crcCalc(imgHdr.startAddr, 0, false);

                    /* Update CRC status */
                    imgHdr.crcStat = CRC_VALID;

                    /* update external flash storage address */
                    *((uint32_t *)&imgHdr.segTypeBd) = dstAddrStart;

                    /* Allow application or some other place in BIM to mark factory image as
                    pending copy (OAD_IMG_COPY_PEND). Should not be done here, as
                    what is in flash at this time will already be the factory
                    image. */
                    imgHdr.imgCpStat = COPY_DONE;

                    /* WRITE METADATA */
                    /* Erase - external portion to be written*/
                    if(extFlashErase(EFL_ADDR_META_FACT_IMG_BLE, EFL_METADATA_LEN))
                    {
                        extFlashWrite(EFL_ADDR_META_FACT_IMG_BLE, EFL_METADATA_LEN,
                                      (uint8_t *)&imgHdr);
                    }
                    else
                    {
                        rtn = false;
                    }
                }
            }
            else
            {
                rtn = false;
            }
        }
        else
        {
            rtn = false;
        }

        /* close driver */
        extFlashClose();
    }
    else
    {
        rtn = false;
    }

#ifdef LED_DEBUG
    /* For LED feedback */
    Bim_PowerUpGpio();

    /* If the copy of the internal image to external flash failed,
       turn on RED LED indefinitely */
    if(rtn != true)
    {
        /* Turn on RED LED */
        GPIO_setDio(IOID_6);
    }
    else
    {
        /* No Error, Blink Green Led */
        Bim_blinkLed(IOID_5, 10, 50);

        /* Turn on GREEN LED */
        GPIO_setDio(IOID_5);
    }
#endif /* #ifdef LED_DEBUG */
    return(rtn);
}
#endif /* CREATE_FACT_IMG_INT_TO_EXT_FLSH */

#ifdef TEST_IMG_COPY
/*******************************************************************************
 * @fn     testImgCopy
 *
 * @brief  This is a debug function to test the image copy from external flash
 *         to onchip flash and CRC32 validation and excuetion of copied image.
 *
 * @return status - True -if test pass otherwise
 *                  False - if test fail
 */
static uint8_t testImgCopy(void)
{
    imgHdr_t metadataHdr;

    /* read metadata a mark it to be copied and copy image and jump to copied
     application */
    bool rtn = true;

    /* initialize external flash driver */
    if(!extFlashOpen())
    {
        return(false);
    }

    extFlashRead(EFL_ADDR_META_FACT_IMG_BLE,  EFL_METADATA_LEN, (uint8_t *)&metadataHdr);

    /* mark header to copy needed */
    metadataHdr.imgCpStat = NEED_COPY;

    /* Update METADATA */

    /* Erase - external portion to be written*/
    if(extFlashErase(EFL_ADDR_META_FACT_IMG_BLE, EFL_METADATA_LEN))
    {
        rtn = extFlashWrite(EFL_ADDR_META_FACT_IMG_BLE, EFL_METADATA_LEN, (uint8_t *)&metadataHdr);
    }

    extFlashClose();
    return(rtn);
}
#endif /* TEST_IMG_COPY */

/*******************************************************************************
 * @fn     isLastMetaData
 *
 * @brief  Copies the internal application + stack image to the external flash
 *         to serve as the permanent resident image in external flash.
 *
 * @param  flashPageNum - Flash page number to start search for external flash
 *         metadata
 *
 * @return flashPageNum - Valid flash page number if metadata is found.
 *         IMG_HDR_FOUND - if metadat starting form specified flash page not found
 *         EMPTY_METADATA - if it is empty flashif metadata not found.
 */
static int8_t isLastMetaData(uint8_t flashPageNum)
{
    /* Read flash to find OAD image identification value */
    imgHdr_t imgHdr;
    uint8_t readNext = TRUE;
    do
    {
        extFlashRead(FLASH_ADDRESS(flashPageNum, 0),  OAD_IMG_ID_LEN, (uint8_t *)&imgHdr.imgID[0]);

        /* Check imageID bytes */
        if(metadataIDCheck(&imgHdr) == TRUE)  /* External flash metadata found */
        {
            return(flashPageNum);
        }
        else if(imgIDCheck(&imgHdr) == TRUE)   /* Image metadata found which indicate end of external flash metadata pages*/
        {
           return(IMG_HDR_FOUND);
        }
        flashPageNum +=1;
        if(flashPageNum >= MAX_ONCHIP_FLASH_PAGES) /* End of flash reached */
        {
            readNext = FALSE;
        }

    }while(readNext);

    return(EMPTY_METADATA);
}

/*******************************************************************************
 * @fn     checkImagesExtFlash
 *
 * @brief  Checks for stored images on external flash. If valid image is found
 * to be copied, it copies the image and if the image is executable it will jump
 * to execute.
 *
 * @param  None
 *
 * @return 1 - If flash open fails or no image found on external flash
 *             number if metadata is found.
 *         EMPTY_METADATA - if metadata not found.
 */
static int8_t checkImagesExtFlash(void)
{
    imgHdr_t metadataHdr;

    uint8_t flashPageNum = 0;

    /* Initialize external flash driver. */
    if(!extFlashOpen())
    {
        return(-1);
    }

    /* Read flash to find OAD external flash metadata identification value  and check for external
      flash bytes */
    while((isLastMetaData(flashPageNum)) > -1)
    {
        /* Read whole metadata header */
        extFlashRead(FLASH_ADDRESS(flashPageNum, 0), EFL_METADATA_LEN, (uint8_t *)&metadataHdr);

        /* check BIM and Metadata version */
        if((metadataHdr.imgCpStat != NEED_COPY) ||
           (metadataHdr.bimVer != BIM_VER  || metadataHdr.metaVer != META_VER) ||
           (metadataHdr.crcStat == 0xFC))  /* Invalid CRC */
        {
            flashPageNum += 1; /* increment flash page number */
            continue;          /* Continue search on next flash page */
        }

        /* check image CRC status */
        if(metadataHdr.crcStat == 0xFF) /* CRC not calculated */
        {
            /*
             * Calculate the CRC over the data buffer and update status
             * @T.B.D. Skipping this section, BIM shouldn't calculate the CRC,
             * the application should calculate and update it.
             */
#ifdef BIM_CALC_CRC
            uint32_t crc32 = 0;
            uint8_t  crcstat = CRC_VALID;
            crc32 = crcCalc(imgHdr.startAddr, 0, false);

            /* Check if calculated CRC matched with the image header */
            if(crc32 != imgHdr.crc32)
            {
                /* Update CRC status */
                crcstat = CRC_INVALID;
                extFlashWrite(FLASH_ADDRESS(flashPageNum, CRC_STAT_OFFSET), 1, (uint8_t *)&crcstat);

                /* Continue looking for next metadata */
                flashPageNum += 1; /* increment flash page number */
                continue;          /* Continue serach on next flash page */
            }

            /*
             * Image don't need to be copied, and CRC matched else this would upset the
             * crc calculation of the image to be copied
             */
            if(imgHdr.imgCpStat != NEED_COPY)
            {
                /* Update CRC status */
                extFlashWrite(FLASH_ADDRESS(flashPageNum, CRC_STAT_OFFSET), 1, (uint8_t *)&crcstat);
            }
#endif /* BIM_CALC_CRC */
        } /* if(metadataHdr.crcStat == 0xFF) */

        /*
         * We get here, we must have valid CRC
         * Check image copy status,  On CC26XXR2 platform only stack application
         * needes to be copied.
         */
        if(metadataHdr.imgCpStat == NEED_COPY)
        {
            /* On CC26XXR2 platform only stack application needes to be copied
               Do the image copy */
            uint32_t eFlStrAddr = ((uint32_t *)&metadataHdr)[EFL_IMG_STR_ADDR_OFFSET];
            uint8_t status = COPY_DONE;

           /* Now read image's start address from image stored on external flash */
           imgHdr_t imgHdr;

           /* Read whole image header, to find the image start address */
           extFlashRead(eFlStrAddr, OAD_IMG_HDR_LEN, (uint8_t *)&imgHdr);

           /* Copy image on internal flash */
           /*
            * NOTE:@TBD during debugging image length wouldn't available, as it is updated
            * by python script during post build process, for now this implementation
            * catering only for contiguous image's, so image length will be calculate
            * by subtracting the image end adress by image start address.
            */
            imgHdr.len = imgHdr.imgEndAddr - imgHdr.startAddr;

            uint8_t retVal = Bim_copyImage(eFlStrAddr, imgHdr.len, imgHdr.startAddr);
            extFlashWrite(FLASH_ADDRESS(flashPageNum, IMG_COPY_STAT_OFFSET), 1, (uint8_t *)&status);

           /* If image copy is successful */
           if(retVal == SUCCESS)
           {
                /* update image copy status and calculate the
                the CRC of the copied
                and update it's CRC status. CRC_STAT_OFFSET
                */
                uint32_t crc32 = crcCalc(FLASH_PAGE(imgHdr.startAddr), 0, false);
                status = CRC_VALID;
                if(crc32 == imgHdr.crc32) // if crc matched then update its status in the copied image
                {
                    extFlashWrite(FLASH_ADDRESS(flashPageNum, CRC_STAT_OFFSET),
                                    sizeof(status), &status);

                    writeFlashPg(FLASH_PAGE(imgHdr.startAddr), CRC_STAT_OFFSET,
                                    &status, sizeof(status));

                    /* If it is executable Image jump to execute it */
                    if((imgHdr.imgType == OAD_IMG_TYPE_APP) ||
                        (imgHdr.imgType == OAD_IMG_TYPE_PERSISTENT_APP) ||
                        (imgHdr.imgType == OAD_IMG_TYPE_APP_STACK))
                    {
                        jumpToPrgEntry(imgHdr.prgEntry);
                    }
                }
                else
                {
                    status = CRC_INVALID;
                    extFlashWrite((imgHdr.startAddr + CRC_STAT_OFFSET), 1, &status);
                }
                status = COPY_DONE;
                extFlashWrite((imgHdr.startAddr + IMG_COPY_STAT_OFFSET), 1, (uint8_t *)&status);
            } /* if(retVal == SUCCESS) */
        }

        /* Unable to find valid executable image, try to find valid user application */
        flashPageNum += 1;

    }  /* end of  while((retVal = isLastMetaData(flashPageNum)) > -1) */

    extFlashClose();
    return(0);
}

/*******************************************************************************
 * @fn     checkPgBoundary
 *
 * @brief  Checks if the last page of the external flash page is reached if not
 *         increments the page and returns the page number.
 *
 * @param  flashPageNum - page number of the ext flash.
 *
 * @return None.
 */
uint8_t checkPgBoundary(uint8 *flashPageNum)
{
    *flashPageNum += 1;
    return(*flashPageNum >= MAX_ONCHIP_FLASH_PAGES)? TRUE: FALSE;
}

/*******************************************************************************
 * @fn     checkImagesIntFlash
 *
 * @brief  Checks for stored images on the on-chip flash. If valid image is
 * found to be copied, it executable it.
 *
 * @param  flashPageNum - Flash page number to start searching for imageType.
 * @param  imgType - Image type to look for.
 *
 * @return - No return if image is found else
 *           0 - If indended image is not found.
 */
uint8_t checkImagesIntFlash(uint8 flashPageNum)
{
    imgHdr_t imgHdr;

    do
    {
        /* Read flash to find OAD image identification value */
        readFlash((uint32_t)FLASH_ADDRESS(flashPageNum, 0), &imgHdr.imgID[0], OAD_IMG_ID_LEN);

        /* Check imageID bytes */
        if(imgIDCheck(&imgHdr) == TRUE)
        {
            /* Read whole image header */
            readFlash((uint32_t)FLASH_ADDRESS(flashPageNum, 0), (uint8 *)&imgHdr, OAD_IMG_HDR_LEN);

            /* If application is neither executable user application or merged app_stack */
            if(!(OAD_IMG_TYPE_APP == imgHdr.imgType ||
                OAD_IMG_TYPE_APP_STACK == imgHdr.imgType) )
            {
                continue;
            }
            /* Invalid metadata version */
            if((imgHdr.bimVer != BIM_VER  || imgHdr.metaVer != META_VER) ||
               /* Invalid CRC */
               (imgHdr.crcStat == 0xFC))
            {
                continue;
            }
            else if(imgHdr.crcStat == DEFAULT_STATE) /* CRC not calculated */
            {
                uint8_t  crcstat = CRC_VALID;

                /* Calculate the CRC over the data buffer and update status */
                uint32_t crc32 = crcCalc(flashPageNum, 0, false);

                /* Check if calculated CRC matched with the image header */
                if(crc32 != imgHdr.crc32)
                {
                    /* Update CRC status */
                    crcstat = CRC_INVALID;
                }
#ifndef JTAG_DEBUG  /* If JTAG debug, skip the crc checking and updating the crc
                     as crc wouldn't have been calculated status */
                writeFlash((uint32_t)FLASH_ADDRESS(flashPageNum, CRC_STAT_OFFSET), (uint8_t *)&crcstat, 1);
                if(crc32 == imgHdr.crc32)
                {
#endif
                    jumpToPrgEntry(imgHdr.prgEntry);  /* No return from here */
#ifndef JTAG_DEBUG
                }
#endif
                /* If it reached here nothing can be done other than try to find
                   good another image whicn wouldn't be found since it extrenal
                   flash OAD */
            }
            else if(imgHdr.crcStat == CRC_VALID)
            {
                jumpToPrgEntry(imgHdr.prgEntry);  /* No return from here */
            }

        } /* if (imgIDCheck(&imgHdr) == TRUE) */

    } while(flashPageNum++ < (MAX_ONCHIP_FLASH_PAGES -1));  /* last flash page contains CCFG */
    return(0);
}

/*******************************************************************************
 * @fn     getExtFlashSize
 *
 * @brief  Checks for stored images on the on-chip flash. If valid image is
 * found to be copied, it executable it.
 *
 * @param  None.
 *
 * @return None.
 */
void getExtFlashSize(void)
{
    const ExtFlashInfo_t  *pExtFlashInfo = extFlashInfo();
}

/*******************************************************************************
 * @fn     Bim_checkImages
 *
 * @brief  Check for stored images on external flash needed to be copied and
 *         execute. If there is no image to be copied, execute on-chip image.
 *
 * @param  none
 *
 * @return none
 */
void Bim_checkImages(void)
{
#ifndef DEBUG
    /* In case an imaged is flashed via debugger and no external image check should
     * be made, define NO_COPY.
     */
    #ifndef NO_COPY
        checkImagesExtFlash();
    #endif

    /*  Find executable on onchip flash and execute */
    checkImagesIntFlash(0);

#else /* ifdef DEBUG */
    uint8_t retVal = 0;

    if( !(retVal = checkImagesExtFlash() ) /* if it traversed through the metadata */
    {
       checkImagesIntFlash(); /* Find application image on internal flash page and jump */
    }
    if( retVal == -1)
    {
#ifdef LED_DEBUG
        // Lit LED
#endif
        while(1);
    }
#endif
    /* if it reached there is a problem */
    /* TBD put device to sleep and wait for button press interrupt */
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
#ifdef CREATE_FACT_IMG_INT_TO_EXT_FLSH

    extFlashOpen();
    const ExtFlashInfo_t *pExtFlashInfo = extFlashInfo();
    extFlashClose();
    uint32_t dstAddr = FLASH_PAGE(pExtFlashInfo->deviceSize - EFL_SIZE_FACT_IMAGE)*HAL_FLASH_PAGE_SIZE;
    Bim_createFactoryImageBackup(INTFLASH_BLE_APP_ADDR,
                                 EFL_SIZE_FACT_IMAGE,
                                 dstAddr);
#endif /* CREATE_FACT_IMG_INT_TO_EXT_FLSH */

#ifdef TEST_IMG_COPY
    uint8_t rtn = testImgCopy();
    if(rtn)
    {
        Bim_checkImages();
    }
#endif /* TEST_IMG_COPY */

     Bim_checkImages();
}

/**************************************************************************************************
*/

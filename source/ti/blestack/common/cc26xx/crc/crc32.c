/******************************************************************************

 @file       crc32.c

 @brief This module contains crc32 calculation api.

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

#include "hal_flash.h"
#include "hal_types.h"
#include "flash_interface.h"
#include "oad_image_header.h"

/*******************************************************************************
 *                                          Constants
 */

#define CRC32_POLYNOMIAL      ((uint32_t)0xEDB88320)  /* reveresed version or 802.3 polynomial 0x04C11DB7 */

/* Warning! this must be a power of 2 less than 1024 */
#define CRC32_BUF_SZ          256

/*******************************************************************************
 *                                       Local Variables
 */

#if defined(__IAR_SYSTEMS_ICC__)
__no_init uint8_t crcBuf[CRC32_BUF_SZ];
#elif defined(__TI_COMPILER_VERSION__)
uint8_t crcBuf[CRC32_BUF_SZ];
#endif

/*******************************************************************************
 * @fn          memCpy
 *
 * @brief       Copies source buffer to destination buffer.
 *
 * @param       dest  - Destination buffer.
 * @param       src   - Destination buffer.
 * @param       len   - Destination buffer.
 *
 * @return      pointer to destination buffer.
 */
void *memCpy(void *dest, const void *src, uint16_t len)
{
    if((dest == NULL))
    {
        return(NULL);
    }
    while(len--)
    {
        ((uint8_t *)dest)[len] = ((uint8_t *)src)[len];
    }
    return(dest);
}

/*******************************************************************************
 * @fn          CRC32Value
 *
 * @brief       Copies source buffer to destination buffer.
 *
 * @param       inCRC  - input word.
 *
 * @return      calculated crc32 word.
 */
uint32_t CRC32Value(uint32_t inCRC)
{
    uint8_t  j;
    uint32_t ulCRC = inCRC;

    /* for this byte (inCRC).. */
    for (j = 8; j; j--)
    {
        /* lsb on? yes -- shift right and XOR with poly. else just shift */
        if (ulCRC & 1)
        {
            ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
        }
        else
        {
            ulCRC >>= 1;
        }
    }

    return(ulCRC);
}

/*******************************************************************************
 * @fn          crcCalc
 *
 * @brief       Run the CRC32 Polynomial calculation over the image specified.
 *
 * @param       page   - Flash page on which to beginning the CRC calculation.
 *
 * @param       offset - offset of first byte of image within first flash page
 *                       of the image.
 *              useExtFl - calculate crc on external or internal flash
 *
 * @return      The CRC32 calculated.
 */
uint32_t crcCalc(uint8_t page, uint16_t offset, bool useExtFl)
{
    imgHdr_t *pImgHdr;

    /* Read first page of the image into the buffer. */
    if(!useExtFl)
    {
        memCpy(crcBuf, (uint8_t *)(page * HAL_FLASH_PAGE_SIZE), CRC32_BUF_SZ);
    }
    else
    {
        readFlashPg(page, 0, crcBuf, CRC32_BUF_SZ);
    }

    pImgHdr = (imgHdr_t *)(crcBuf);

    pImgHdr->len = pImgHdr->imgEndAddr - pImgHdr->startAddr +1;
    uint8_t pageBeg = page;
    uint8_t pageEnd = (pImgHdr->len - 1) / (HAL_FLASH_PAGE_SIZE);

    uint16_t osetEnd = pageEnd += pageBeg;
    /* Determine the number of bytes in the last page */
    uint16_t numBytesInLastPg = ((pImgHdr->len - 1) % HAL_FLASH_PAGE_SIZE ) + 1;

    uint32_t temp1 = HAL_FLASH_PAGE_SIZE, temp2, crc;
    crc = 0xFFFFFFFF;
    uint16_t oset = 0;

    /* Read over image pages. */
    for (uint8_t pageIdx = pageBeg; pageIdx <= pageEnd; pageIdx++)
    {
        uint8_t numBufInCurPg;

        /* Find the number of buffers within this page */
        if(pageIdx == pageEnd)
        {
            /* Number of bytes divided by buf_sz is the number of buffers */
            numBufInCurPg = numBytesInLastPg / CRC32_BUF_SZ;

            /* Round up a buffer if a partial buffer must be used */
            if(numBytesInLastPg % CRC32_BUF_SZ != 0)
            {
                numBufInCurPg++;
            }
        }
        else
        {
            /* Note this requires that HAL_FLASH_PAGE_SIZE is an integer multiple of
              CRC32_BUF_SZ */
            numBufInCurPg = HAL_FLASH_PAGE_SIZE / CRC32_BUF_SZ;
        }
        /* Read over buffers within each page */
        for(uint8_t bufNum = 0; bufNum < numBufInCurPg; bufNum++)
        {
            /* Find ending offset in bytes last buffer. */
            uint16_t osetEnd;
            /* Calculate the ending offset for this buffer */
            if(bufNum == (numBufInCurPg - 1) && pageIdx == pageEnd)
            {
                if(numBytesInLastPg % CRC32_BUF_SZ != 0)
                {
                    osetEnd = (numBytesInLastPg % CRC32_BUF_SZ );
                }
                else
                {
                    osetEnd = CRC32_BUF_SZ;
                }
            }
            else
            {
                osetEnd = CRC32_BUF_SZ;
            }

            /* Read over all flash words in a buffer, excluding the CRC section
             * of the first page and all bytes after the remainder bytes in the
             * last buffer
             */
            for (oset = ((pageIdx == pageBeg && bufNum == 0) ? offset + IMG_DATA_OFFSET : 0);
                     oset < osetEnd;
                     oset++)
            {
                temp1 = (crc >> 8) & 0x00FFFFFFL;
                temp2 = CRC32Value(((uint32_t)crc ^ crcBuf[oset]) & 0xFF);
                crc = temp1 ^ temp2;
            }

            /* Read data into the next buffer */
            if(!useExtFl)
            {
                memCpy(crcBuf, (uint8_t *)((pageIdx*HAL_FLASH_PAGE_SIZE) + ((bufNum + 1)*CRC32_BUF_SZ)),
                        CRC32_BUF_SZ);
            }
            else
            {
                /* Check to see    if the next buffer is on the next page */
                if(bufNum    == (numBufInCurPg - 1))
                {
                    readFlashPg((pageIdx + 1), 0, crcBuf, CRC32_BUF_SZ);
                }
                else
                {
                    readFlashPg(pageIdx, ((bufNum + 1)*CRC32_BUF_SZ), crcBuf,
                                      CRC32_BUF_SZ);
                }
            }
        } /* for(uint8_t bufNum = 0; bufNum < numBufInCurPg; bufNum++) */
    } /* for (uint8_t pageIdx = pageBeg; pageIdx <= pageEnd; pageIdx++) */

    /* XOR CRC with all bits on */
    crc = crc ^ 0xFFFFFFFF;
    return(crc);
}

/**************************************************************************************************
*/

/******************************************************************************

 @file       flash_interface.h

 @brief Interface API for flash access on the CC26xx. Header can be compiled
        with a varation of implementations:

        flash_interface_interal.c - Driverlib access of CC26xx internal flash
        flash_interface_ext_rtos.c - TI-RTOS SPI access of external SPI flash
        flash_interface_ext_rtos.c - Driverlib SPI access of external SPI flash

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2017-2017, Texas Instruments Incorporated
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
#ifndef FLASH_INTERFACE
#define FLASH_INTERFACE

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include "hal_board.h"
#include <stdlib.h>
#include <stdbool.h>


/*********************************************************************
 * CONSTANTS
 */

#define FLASH_SUCCESS   0x00
#define FLASH_FAILURE   0xFF

#define SPI_MAX_READ_SZ 1024

/*********************************************************************
 * MACROS
 */
#define FLASH_ADDRESS(page, offset) (((page) << 12) + (offset))
#define FLASH_PAGE(addr)            (addr >> 12)

/*********************************************************************
 * @fn      flash_open
 *
 * @brief   Open access to flash
 *
 * @param   None.
 *
 * @return  TRUE if interface successfully opened
 */
extern bool flash_open(void);

/*********************************************************************
 * @fn      flash_close
 *
 * @brief   Close/cleanup access to flash
 *
 * @param   None.
 *
 * @return  None.
 */
extern void flash_close(void);

/*********************************************************************
 * @fn      hasExternalFlash
 *
 * @brief   Check if the interface is built for external flash
 *
 * @param   None.
 *
 * @return  TRUE if the target has external flash
 */
extern bool hasExternalFlash(void);

/*********************************************************************
 * @fn      readFlash
 *
 * @brief   Read data from flash.
 *
 * @param   addr   - address to read from
 * @param   pBuf   - pointer to buffer into which data is read.
 * @param   len    - length of data to read in bytes.
 *
 * @return  None.
 */
extern uint8_t readFlash(uint_least32_t addr, uint8_t *pBuf, size_t len);

/*********************************************************************
 * @fn      readFlashPg
 *
 * @brief   Read data from flash.
 *
 * @param   page   - page to read from in flash
 * @param   offset - offset into flash page to begin reading
 * @param   pBuf   - pointer to buffer into which data is read.
 * @param   len    - length of data to read in bytes.
 *
 * @return  None.
 */
extern uint8_t readFlashPg(uint8_t page, uint32_t offset, uint8_t *pBuf,
                            uint16_t len);

/*********************************************************************
 * @fn      writeFlash
 *
 * @brief   Write data to flash.
 *
 * @param   addr   - address to write to in flash
 * @param   pBuf   - pointer to buffer of data to write
 * @param   len    - length of data to write in bytes
 *
 * @return  status - FLASH_SUCCESS if programmed successfully or
 *                   FLASH_FAILURE if programming failed
 */
extern uint8_t writeFlash(uint_least32_t addr, uint8_t *pBuf, size_t len);

/*********************************************************************
 * @fn      writeFlashPg
 *
 * @brief   Write data to flash.
 *
 * @param   page   - page to write to in flash
 * @param   offset - offset into flash page to begin writing
 * @param   pBuf   - pointer to buffer of data to write
 * @param   len    - length of data to write in bytes
 *
 * @return  status - FLASH_SUCCESS if programmed successfully or
 *                   FLASH_FAILURE if programming failed
 */
extern uint8_t writeFlashPg(uint8_t page, uint32_t offset, uint8_t *pBuf,
                            uint16_t len);

/*********************************************************************
 * @fn      eraseFlash
 *
 * @brief   Erase selected flash page.
 *
 * @param   page - the page to erase.
 *
 * @return  None.
 */
extern uint8_t eraseFlash(uint8_t page);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* FLASH_INTERFACE */

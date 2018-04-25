/******************************************************************************

 @file       oad_image_header_app.c

 @brief OAD image header definition file.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2014-2017, Texas Instruments Incorporated
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
 * INCLUDES
 */

#include <stddef.h>
#include "hal_types.h"
#include "hal_board_cfg.h"
#include "icall.h"
#include "oad_image_header.h"

/*******************************************************************************
 * EXTERNS
 */

/*******************************************************************************
 * PROTOTYPES
 */

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#define CRC16_POLYNOMIAL  0x1021
#define VECTOR_TB_SIZE    0x40 //!< Interrupt vector table entry size

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

extern const uint32_t  RAM_END;

#if defined HAL_IMAGE_A
extern const uint8_t  ENTRY_END;
extern const uint8_t  ENTRY_START;
#endif

#ifdef __IAR_SYSTEMS_ICC__
#pragma section = "ROSTK"
#pragma section = "RWDATA"
#pragma section = "ENTRY_FLASH"
#endif

#ifdef __TI_COMPILER_VERSION__
/* This symbol is create by the linker file */
extern uint8_t ramStartHere;
extern uint8_t prgEntryAddr;
extern uint8_t ramStartHere;
extern uint8_t flashEndAddr;
extern uint32_t heapEnd;
extern uint32_t FLASH_END;
#endif /* __TI_COMPILER_VERSION__ */

#include "icall_addrs.h"
#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION(_imgHdr, ".image_header")
#pragma RETAIN(_imgHdr)
const imgHdr_t _imgHdr =
#elif  defined(__IAR_SYSTEMS_ICC__)
#pragma location=".img_hdr"
const imgHdr_t _imgHdr @ ".img_hdr" =
#endif
{
  .imgID = OAD_IMG_ID_VAL,
  .crc32 = 0xFFFFFFFF,
  .bimVer = 0x01,
  .metaVer = 0x01,                       //!< Metadata version */
  .techType = 0xFE,                       //!< Wireless protocol type BLE/TI-MAC/ZIGBEE etc. */
  .imgCpStat = 0xFF,                     //!< Image copy status bytes */
  .crcStat = 0xFF,                       //!< CRC status */
  .imgNo = 0x1,                          //!< Image number of 'image type' */
  .imgVld = 0xFFFFFFFF,                  //!< In indicates if the current image in valid 0xff - valid, 0x00 invalid image */
  .len = 0xFFFFFFFF,                     //!< Image length in bytes. */
  .softVer = SOFTWARE_VER,               //!< Software version of the image */
  .hdrLen = offsetof(imgHdr_t, rfu2) + sizeof(((imgHdr_t){0}).rfu2),   //!< Total length of the image header */
  .rfu2 = 0xFFFF,                        //!< reserved bytes */
  .segTypeBd = 0x0,
  .wirelessTech1 = 0xFE,
  .rfu3 = 0xFF,
  .boundarySegLen = 0x18,
#if defined HAL_IMAGE_A                  // target image
  .imgType =  0x00,                      //!< Target image Type */
  .stackStartAddr = 0xFFFFFFFF,          //!< Stack start adddress */
  .stackEntryAddr = 0xFFFFFFFF,
#else // User application image
  .imgType =  0x1,                      //!< Application image Type */
  .stackEntryAddr = ICALL_STACK0_ADDR,
  .stackStartAddr = ICALL_STACK0_START,
#endif /* defined HAL_IMAGE_A */

#ifdef __IAR_SYSTEMS_ICC__
  .prgEntry = (uint32_t)(__section_begin("ENTRY_FLASH")), //!< Program entry address */
#if defined HAL_IMAGE_A
  .prgEntry = (uint32_t)&ENTRY_START,
  .imgEndAddr = (uint32_t)&ENTRY_END,                     //!< Program entry address */
#else
  .imgEndAddr = (uint32)(__section_end("ROSTK")),
#endif
  .ram0StartAddr = (uint32)(__section_begin("RWDATA")),  //!< RAM entry start address */
  .ram0EndAddr = (uint32_t)&RAM_END,
#else  /* #ifdef __IAR_SYSTEMS_ICC__ */
  .ram0StartAddr = (uint32_t)&ramStartHere,             //!< RAM entry start address */
  .prgEntry = (uint32_t)&prgEntryAddr,
#if defined HAL_IMAGE_A
  .imgEndAddr = (uint32)&prgEntryAddr + VECTOR_TB_SIZE -1,
#else
  .imgEndAddr = (uint32_t)&flashEndAddr,
#endif /*defined HAL_IMAGE_A */
  .ram0EndAddr = (uint32_t)&heapEnd,
#endif /* __IAR_SYSTEMS_ICC__ */
  .segTypeImg = 1,
  .wirelessTech2  = 0xFE,
  .rfu4 =0xFF,                                 //!< Image payload and length */
  .startAddr = (uint32_t)&(_imgHdr.imgID)
 };

/******************************************************************************

 @file       ext_flash_layout.h

 @brief Contains a layout plan for the external flash of the SensorTag

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2015-2017, Texas Instruments Incorporated
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
 *****************************************************************************/\

#ifndef EXT_FLASH_LAYOUT_H
#define EXT_FLASH_LAYOUT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_types.h"

#define OAD_EFL_MAGIC               {'O', 'A', 'D', ' ', 'N', 'V', 'M', '1'}
#define OAD_EFL_MAGIC_SZ            8

#define EFL_NUM_FACT_IMAGES         2

// Page size
#define EFL_PAGE_SIZE               0x1000
#define EFL_FLASH_SIZE              0x80000

#define EFL_MAX_IMG_SZ              0x20000
#define EFL_APP_IMG_SZ              0x10000

// Image information (meta-data)
#define EFL_ADDR_META               0x00000
#define EFL_SIZE_META               EFL_PAGE_SIZE
#define EFL_MAX_META                4

#define EFL_FACT_IMG_0_ADDR         EFL_FLASH_SIZE - EFL_MAX_IMG_SZ
#define EFL_FACT_IMG_1_ADDR         EFL_FACT_IMG_0_ADDR - EFL_MAX_IMG_SZ

#define EFL_USR_AREA_ADDR           EFL_SIZE_META*EFL_MAX_META
#define EFL_USR_AREA_SZ             EFL_IMG_SPACE_START - EFL_USR_AREA_ADDR

#define EFL_IMG_SPACE_START         EFL_APP_IMG_SZ

#define EFL_IMG_SPACE_END           (EFL_FACT_IMG_1_ADDR - 1)


// Address/length s
#define EFL_META_COUNTER_OFFSET     46

#define EFL_META_COPY_SZ            42

PACKED_TYPEDEF_STRUCT
{
    uint8_t     imgID[8];       //!< User-defined Image Identification bytes. */
    uint32_t    crc32;
    uint8_t     bimVer;         //!< BIM version */
    uint8_t     metaVer;        //!< Metadata version */
    uint16_t    techType;       //!< Wireless protocol type BLE/TI-MAC/ZIGBEE etc. */
    uint8_t     imgCpStat;      //!< Image copy status bytes */
    uint8_t     crcStat;        //!< CRC status */
    uint8_t     imgType;        //!< Image Type */
    uint8_t     imgNo;          //!< Image number of 'image type' */
    uint32_t    imgVld;         //!< In indicates if the current image in valid 0xff - valid, 0x00 invalid image */
    uint32_t    len;            //!< Image length in 4-byte blocks (i.e. HAL_FLASH_WORD_SIZE blocks). */
    uint32_t    prgEntry;       //!< Program entry address */
    uint8_t     softVer[4];     //!< Software version of the image */
    uint32_t    imgEndAddr;
    uint16_t    hdrLen;         //!< Total length of the image header */
    uint16_t    rfu;
    uint32_t    extFlAddr;
    uint32_t    counter;
} ExtImageInfo_t;

#ifdef __cplusplus
}
#endif


#endif /* EXT_FLASH_LAYOUT_H */

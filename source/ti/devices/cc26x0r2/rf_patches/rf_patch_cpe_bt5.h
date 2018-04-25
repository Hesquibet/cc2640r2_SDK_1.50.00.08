/******************************************************************************
*  Filename:       rf_patch_cpe_bt5.h
*  Revised:        $Date: 2017-09-21 19:13:50 +0200 (Thu, 21 Sep 2017) $
*  Revision:       $Revision: 17970 $
*
*  Description: RF core patch for Bluetooth 5 support in CC2640R2F
*
*  Copyright (c) 2015-2017, Texas Instruments Incorporated
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
*  3) Neither the name of the ORGANIZATION nor the names of its contributors may
*     be used to endorse or promote products derived from this software without
*     specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/
#ifndef _RF_PATCH_CPE_BT5_H
#define _RF_PATCH_CPE_BT5_H

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <string.h>

#ifndef CPE_PATCH_TYPE
#define CPE_PATCH_TYPE static const uint32_t
#endif

#ifndef SYS_PATCH_TYPE
#define SYS_PATCH_TYPE static const uint32_t
#endif

#ifndef PATCH_FUN_SPEC
#define PATCH_FUN_SPEC static inline
#endif

#ifndef _APPLY_PATCH_TAB
#define _APPLY_PATCH_TAB
#endif


CPE_PATCH_TYPE patchImageBt5[] = {
   0x21000659,
   0x210006b1,
   0x21000469,
   0x21000475,
   0x21000789,
   0x461db570,
   0x47204c00,
   0x0000b287,
   0x0a004670,
   0x28c44a05,
   0x4710d004,
   0x32e44a03,
   0x90042001,
   0x20004902,
   0x47107008,
   0x0000679d,
   0x210001dc,
   0x4801b510,
   0x00004700,
   0x000009d5,
   0x4ca6b5f8,
   0x462548a6,
   0x7b2f3560,
   0x732e2600,
   0x732f4780,
   0x43017b69,
   0x2f00d117,
   0x7e61d015,
   0xd00e2900,
   0x31204621,
   0x08537c0a,
   0xd10007db,
   0x6a237626,
   0x2b007b49,
   0x2304d101,
   0x42114319,
   0x7d21d002,
   0xd40006c9,
   0xbdf875e6,
   0xb4304893,
   0x31404601,
   0x2a027c0a,
   0x6802d116,
   0x754a79d2,
   0x68936802,
   0x32804602,
   0x7d486093,
   0xd00b2802,
   0xd0092800,
   0x2c061ec4,
   0x498ad809,
   0x18400080,
   0x6b803840,
   0x60901818,
   0xbc304887,
   0x20034700,
   0x80c802c0,
   0x72082002,
   0x2003bc30,
   0xb5704770,
   0x4604487e,
   0x7da53440,
   0xd0122d01,
   0x06497d01,
   0x21800fca,
   0x7c21540a,
   0xd10a2900,
   0x78403060,
   0x07c00880,
   0x7ce0d002,
   0xd5020700,
   0x75a02001,
   0x2000e000,
   0x487573a0,
   0x75a54780,
   0xb570bd70,
   0x48734c6e,
   0x35504625,
   0x28024780,
   0x3440d109,
   0x4a7088e1,
   0xd1044291,
   0x06897ce9,
   0x1d91d401,
   0xbd7080e1,
   0x4965b570,
   0x46084a6b,
   0x78033050,
   0xd0102b00,
   0x2b017983,
   0x7c03d10d,
   0x07db095b,
   0x7d09d009,
   0x74c14d65,
   0x20207f6c,
   0x77684320,
   0x776c4790,
   0x21ffbd70,
   0x479074c1,
   0xb510bd70,
   0x4c56485f,
   0x28024780,
   0x4621d10e,
   0x88ca3140,
   0x429a4b58,
   0x7ccad108,
   0x07d20952,
   0x7d22d004,
   0xd4010692,
   0x80ca1d9a,
   0xb570bd10,
   0x4955484b,
   0x7cc03040,
   0x07c00940,
   0x4d50d007,
   0x8b2c4852,
   0x83284320,
   0x832c4788,
   0x4788bd70,
   0xb570bd70,
   0x494e4d42,
   0x792c3540,
   0x184000a0,
   0x478068c0,
   0x2c0a493e,
   0x3154d00a,
   0x0993780a,
   0xd00407db,
   0x780a73aa,
   0x431a2303,
   0xbd70700a,
   0xe7f33153,
   0x31404936,
   0x281e7108,
   0xdc08d022,
   0xd0132808,
   0xd013280a,
   0xd013280b,
   0xd1082818,
   0x2825e012,
   0x282ad012,
   0x283cd012,
   0x283dd008,
   0x4937d012,
   0x18400080,
   0x477068c0,
   0x47704835,
   0x47704835,
   0x47704835,
   0x47704835,
   0x47704835,
   0x47704835,
   0x47704835,
   0x47704835,
   0x4607b5f8,
   0x204e4c1f,
   0x09855d00,
   0x25fb07ee,
   0xd0172e00,
   0x07347d26,
   0x2c050f24,
   0x2401d003,
   0xd0024220,
   0x2402e012,
   0x09c0e7fa,
   0x0670d005,
   0x6848d503,
   0x28010f80,
   0x085bd008,
   0x0852005b,
   0xe0030052,
   0xd0012800,
   0x402a402b,
   0xd0032b06,
   0x2b022010,
   0xe010d010,
   0x402a2302,
   0xf7ff4638,
   0x2800fea5,
   0x00c2da07,
   0x39201ab9,
   0x22147e49,
   0xd1004211,
   0xbdf82000,
   0x43034302,
   0xf7ff4638,
   0xbdf8fe95,
   0x21000144,
   0x0000a767,
   0x210000ec,
   0x0000d575,
   0x0000bcf3,
   0x0000cb37,
   0x00001404,
   0x0000c8eb,
   0x210000a8,
   0x0000b4f5,
   0x0000ad53,
   0x00002020,
   0x0000df80,
   0x21000499,
   0x21000627,
   0x21000603,
   0x210005d7,
   0x2100059d,
   0x21000577,
   0x21000537,
   0x210004e5,
   0x4c03b510,
   0xfe7ef7ff,
   0x28006820,
   0xbd10d1fa,
   0x40041100,
};
#define _NWORD_PATCHIMAGE_BT5 213

#define _NWORD_PATCHSYS_BT5 0



#ifndef _BT5_SYSRAM_START
#define _BT5_SYSRAM_START 0x20000000
#endif

#ifndef _BT5_CPERAM_START
#define _BT5_CPERAM_START 0x21000000
#endif

#define _BT5_SYS_PATCH_FIXED_ADDR 0x20000000

#define _BT5_PARSER_PATCH_TAB_OFFSET 0x0350
#define _BT5_PATCH_TAB_OFFSET 0x0358
#define _BT5_IRQPATCH_OFFSET 0x03E8
#define _BT5_PATCH_VEC_OFFSET 0x0448

PATCH_FUN_SPEC void enterBt5CpePatch(void)
{
   uint32_t *pPatchVec = (uint32_t *) (_BT5_CPERAM_START + _BT5_PATCH_VEC_OFFSET);

#if (_NWORD_PATCHIMAGE_BT5 > 0)
   memcpy(pPatchVec, patchImageBt5, sizeof(patchImageBt5));
#endif
}

PATCH_FUN_SPEC void enterBt5SysPatch(void)
{
}

PATCH_FUN_SPEC void configureBt5Patch(void)
{
   uint8_t *pPatchTab = (uint8_t *) (_BT5_CPERAM_START + _BT5_PATCH_TAB_OFFSET);


   pPatchTab[1] = 0;
   pPatchTab[18] = 1;
   pPatchTab[105] = 2;
   pPatchTab[108] = 3;
   pPatchTab[28] = 4;
}

PATCH_FUN_SPEC void applyBt5Patch(void)
{
   enterBt5SysPatch();
   enterBt5CpePatch();
   configureBt5Patch();
}

PATCH_FUN_SPEC void refreshBt5Patch(void)
{
   enterBt5CpePatch();
   configureBt5Patch();
}

PATCH_FUN_SPEC void rf_patch_cpe_bt5(void)
{
   applyBt5Patch();
}


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  _RF_PATCH_CPE_BT5_H


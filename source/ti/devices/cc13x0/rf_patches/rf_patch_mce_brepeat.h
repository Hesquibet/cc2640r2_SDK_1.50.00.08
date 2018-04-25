/******************************************************************************
*  Filename:       rf_patch_mce_brepeat.h
*  Revised:        $Date: 2017-08-24 11:43:33 +0200 (Thu, 24 Aug 2017) $
*  Revision:       $Revision: 17889 $
*
*  Description: RF core patch for CC13x0 for 1.2kbps and 2.4kbps Generic FSK
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

#ifndef _RF_PATCH_MCE_BREPEAT_H
#define _RF_PATCH_MCE_BREPEAT_H

#include <stdint.h>
#include "../inc/hw_types.h"

#ifndef MCE_PATCH_TYPE
#define MCE_PATCH_TYPE static const uint32_t
#endif

#ifndef PATCH_FUN_SPEC
#define PATCH_FUN_SPEC static inline
#endif

#ifndef RFC_MCERAM_BASE
#define RFC_MCERAM_BASE 0x21008000
#endif

#ifndef MCE_PATCH_MODE
#define MCE_PATCH_MODE 0
#endif

MCE_PATCH_TYPE patchBrepeatMce[301] = { 
   0x2fcf604e,
   0x030c3f9d,
   0x070c680a,
   0x00068080,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0xf80007c0,
   0x1f0000f8,
   0xe007003f,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00070003,
   0x00003d1f,
   0x04000000,
   0x0000000f,
   0x000b0387,
   0x004348f4,
   0x80078000,
   0x00000670,
   0x0510091e,
   0x00070054,
   0x1f080100,
   0x00000031,
   0x3030002f,
   0x0000027f,
   0x00000000,
   0x0000aa00,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x72487220,
   0x7303a32d,
   0x72037305,
   0x73067304,
   0x73767204,
   0xc7c07276,
   0x00018001,
   0x90109001,
   0x90010801,
   0x720d720c,
   0xb0c0720e,
   0xb0f07100,
   0x7218a0c0,
   0x10208132,
   0x06703952,
   0x16300020,
   0x14011101,
   0x60836c01,
   0x60a06084,
   0x60836175,
   0x60836083,
   0x12106083,
   0x730f7220,
   0x73117310,
   0x00108181,
   0xb0709180,
   0x60796063,
   0x6654c030,
   0xc282c1e1,
   0x1820c470,
   0x6e236f13,
   0x16121611,
   0x7830688a,
   0x78409ab0,
   0x78509ac0,
   0xc4829ad0,
   0x1820c5a0,
   0x1203409d,
   0x16126e23,
   0x7870689a,
   0x607997e0,
   0xb1087276,
   0xb0f3b100,
   0xb0d8b0f1,
   0xb1087100,
   0xb200a0d8,
   0x87e0b760,
   0xb0f19780,
   0x7100b0c1,
   0xb0f17276,
   0xb012a0c1,
   0xb013b002,
   0xb484b003,
   0xb101b0d1,
   0x7100722c,
   0x8140b101,
   0x06f03940,
   0x06f38143,
   0x65ae8161,
   0x1a133911,
   0x68c04cc3,
   0x39808140,
   0x12031610,
   0x140385e1,
   0x1e011a11,
   0xc21144cd,
   0x48f71c13,
   0x1c13c311,
   0x181348e8,
   0x84511030,
   0x40e21ef0,
   0x1830c100,
   0x39111a10,
   0x103068df,
   0x391165ae,
   0x846168e2,
   0x60f412f0,
   0x1813c211,
   0x84611030,
   0x40f41ef0,
   0x1830c100,
   0x39111a10,
   0x103068f1,
   0x391165ae,
   0x843168f4,
   0x65ae12f0,
   0x68f93911,
   0x12f08441,
   0x391165ae,
   0x721c68fe,
   0x65a181a1,
   0x222081e0,
   0xa0d14102,
   0x9780c040,
   0x9760c030,
   0xb0f1b0c1,
   0xb0f17100,
   0x7276a0c1,
   0xa0037248,
   0x7248a002,
   0x73067305,
   0x72767376,
   0xc7c0a200,
   0x60799010,
   0xb016b006,
   0xb014b004,
   0xb012b002,
   0x90307810,
   0x78209050,
   0x90609040,
   0x8ae2b205,
   0x93028303,
   0x857f65b7,
   0xc00cb064,
   0xb011c00b,
   0xa0c1b072,
   0xb116a0c0,
   0x8ab1b0e6,
   0x413e22d1,
   0xb0c2b0f2,
   0xb0737100,
   0x7378a760,
   0x80b0b760,
   0x45482260,
   0x613e661a,
   0x8960b88f,
   0x955018f0,
   0x720e100f,
   0xb201a0c2,
   0xb107b0d7,
   0xb0417100,
   0x22e08ad0,
   0x8210415a,
   0x45332220,
   0xb06db04d,
   0x9070c300,
   0xc040b201,
   0xa0446654,
   0xb88c7000,
   0x89a48180,
   0x31843924,
   0x91840004,
   0x6654c050,
   0x72767376,
   0x72067248,
   0x72047202,
   0x73067305,
   0x721b6079,
   0xc060b32d,
   0xb0f86654,
   0xb0d7651e,
   0x7100b107,
   0x7100b107,
   0x7100b107,
   0x7100b107,
   0x120ab107,
   0x22008090,
   0x66034463,
   0x815b161a,
   0x41861e0b,
   0xd0708552,
   0x66549862,
   0x4d9f1cba,
   0x1efb18ab,
   0x10b0499b,
   0x66031af0,
   0xa2056999,
   0x6603c0f0,
   0xa0d7699d,
   0x22016163,
   0x13f245a5,
   0x121261a6,
   0x85e292c2,
   0xb1017100,
   0x1e021a12,
   0x700045a8,
   0x45b22201,
   0x61b313f2,
   0x92c21212,
   0xb1017100,
   0x81437000,
   0x2a733983,
   0x85e09473,
   0x16131a10,
   0x143f120f,
   0x120b69bf,
   0x1c0fc200,
   0x61c841c6,
   0x61cd791b,
   0x10fbc400,
   0x318b180b,
   0x8400312b,
   0x04107941,
   0x940000b0,
   0xa405b404,
   0x1e4185e1,
   0x1e3141e7,
   0x841041df,
   0x31103180,
   0x94203980,
   0x841061ec,
   0x39803180,
   0x31101001,
   0x94201410,
   0x841061ec,
   0x39803180,
   0x94203120,
   0x84301201,
   0x87d097c0,
   0x84401401,
   0x87d097c0,
   0xc1001401,
   0x31111801,
   0x70009571,
   0x22011202,
   0x3a3241ff,
   0x38326200,
   0x69fb3911,
   0x85e37000,
   0x71001202,
   0xb107b88d,
   0x31818b11,
   0x14123d81,
   0x31818b21,
   0x14123d81,
   0x1e031a13,
   0x22f24605,
   0x12124617,
   0x12026218,
   0x70009192,
   0x22b08ab0,
   0x1e3b4620,
   0x62224652,
   0x46521e7b,
   0xb889c00b,
   0x31808940,
   0x16103d80,
   0x140c3d30,
   0x226080b0,
   0x7000422e,
   0x39838ab3,
   0x8ab106f3,
   0x0401cff0,
   0x1c1c3031,
   0x12004e4a,
   0x1c0c1810,
   0x80b04a4c,
   0x423f2260,
   0x10c27000,
   0x3c321612,
   0x83208ae1,
   0x424e2210,
   0x93016250,
   0x7000b0f2,
   0x623b101c,
   0x623b100c,
   0x62471821,
   0x62471421,
   0x6248161b,
   0x88409850,
   0x46552200,
   0x7000b830
};

PATCH_FUN_SPEC void rf_patch_mce_brepeat(void)
{
#ifdef __PATCH_NO_UNROLLING
   uint32_t i;
   for (i = 0; i < 301; i++) {
      HWREG(RFC_MCERAM_BASE + 4 * i) = patchBrepeatMce[i];
   }
#else
   const uint32_t *pS = patchBrepeatMce;
   volatile unsigned long *pD = &HWREG(RFC_MCERAM_BASE);
   uint32_t t1, t2, t3, t4, t5, t6, t7, t8;
   uint32_t nIterations = 37;

   do {
      t1 = *pS++;
      t2 = *pS++;
      t3 = *pS++;
      t4 = *pS++;
      t5 = *pS++;
      t6 = *pS++;
      t7 = *pS++;
      t8 = *pS++;
      *pD++ = t1;
      *pD++ = t2;
      *pD++ = t3;
      *pD++ = t4;
      *pD++ = t5;
      *pD++ = t6;
      *pD++ = t7;
      *pD++ = t8;
   } while (--nIterations);

   t1 = *pS++;
   t2 = *pS++;
   t3 = *pS++;
   t4 = *pS++;
   t5 = *pS++;
   *pD++ = t1;
   *pD++ = t2;
   *pD++ = t3;
   *pD++ = t4;
   *pD++ = t5;
#endif
}

#endif

/******************************************************************************

 @file       crypto_api.h

 @brief Header for Crypto proxy for stack's interface to the crypto driver.

 Group: WCS, LPC/BTS
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
 *****************************************************************************/

#ifndef CRYPTO_API_H
#define CRYPTO_API_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */

#include <ti/drivers/crypto/CryptoCC26XX.h>

extern uint32_t *cryptoDrvTblPtr;

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

// Crypto proxy index for crypto driver API
#define CRYPTOCC26XX_CLOSE              0
#define CRYPTOCC26XX_INIT               1
#define CRYPTOCC26XX_OPEN               2
#define CRYPTOCC26XX_PARAMS_INIT        3
#define CRYPTOCC26XX_TRANSAC_INIT       4
#define CRYPTOCC26XX_ALLOCATEKEY        5
#define CRYPTOCC26XX_RELEASEKEY         6
#define CRYPTOCC26XX_TRANSACT           7
#define CRYPTOCC26XX_TRANSACTPOLLING    8
#define CRYPTOCC26XX_TRANSACTCALLBACK   9
#define CRYPTOCC26XX_LOADKEY            10

/*
** Crypto API Proxy
*/

#define CRYPTO_TABLE(index)           (*((uint32_t *)((uint32_t)cryptoDrvTblPtr + (uint32_t)((index)*4))))

#define CryptoCC26XX_close            ((int                 (*)(CryptoCC26XX_Handle))                                             \
                                       CRYPTO_TABLE(CRYPTOCC26XX_CLOSE))
#define CryptoCC26XX_init             ((void                (*)(void))                                                            \
                                       CRYPTO_TABLE(CRYPTOCC26XX_INIT))
#define CryptoCC26XX_open             ((CryptoCC26XX_Handle (*)(unsigned int, bool, CryptoCC26XX_Params*))                        \
                                       CRYPTO_TABLE(CRYPTOCC26XX_OPEN))
#define CryptoCC26XX_Params_init      ((void                (*)(CryptoCC26XX_Params *))                                           \
                                       CRYPTO_TABLE(CRYPTOCC26XX_PARAMS_INIT))
#define CryptoCC26XX_Transac_init     ((void                (*)(CryptoCC26XX_Transaction *, CryptoCC26XX_Operation))              \
                                       CRYPTO_TABLE(CRYPTOCC26XX_TRANSAC_INIT))
#define CryptoCC26XX_allocateKey      ((int                 (*)(CryptoCC26XX_Handle, CryptoCC26XX_KeyLocation, const uint32_t *)) \
                                       CRYPTO_TABLE(CRYPTOCC26XX_ALLOCATEKEY))
#define CryptoCC26XX_releaseKey       ((int                 (*)(CryptoCC26XX_Handle, int *))                                      \
                                       CRYPTO_TABLE(CRYPTOCC26XX_RELEASEKEY))
#define CryptoCC26XX_transact         ((int                 (*)(CryptoCC26XX_Handle, CryptoCC26XX_Transaction *))                 \
                                       CRYPTO_TABLE(CRYPTOCC26XX_TRANSACT))
#define CryptoCC26XX_transactPolling  ((int                 (*)(CryptoCC26XX_Handle, CryptoCC26XX_Transaction *))                 \
                                       CRYPTO_TABLE(CRYPTOCC26XX_TRANSACTPOLLING))
#define CryptoCC26XX_transactCallback ((int                 (*)(CryptoCC26XX_Handle, CryptoCC26XX_Transaction *))                 \
                                       CRYPTO_TABLE(CRYPTOCC26XX_TRANSACTCALLBACK))
#define CryptoCC26XX_loadKey          ((int                 (*)(CryptoCC26XX_Handle, int, const uint32_t *))                      \
                                       CRYPTO_TABLE(CRYPTOCC26XX_LOADKEY))

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_API_H */

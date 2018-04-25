/*
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/DebugP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/AESCCM.h>
#include <ti/drivers/aesccm/AESCCMCC26XX.h>
#include <ti/drivers/cryptoutils/sharedresources/CryptoResourceCC26XX.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_crypto.h)
#include DeviceFamily_constructPath(driverlib/aes.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/smph.h)


/* Forward declarations */
static void AESCCM_hwiFxn (uintptr_t arg0);
static void AESCCM_swiFxn (uintptr_t arg0, uintptr_t arg1);
static void AESCCM_internalCallbackFxn (AESCCM_Handle handle, CryptoKey *processedKey);
static int_fast16_t AESCCM_waitForAccess(AESCCM_Handle handle);
static int_fast16_t AESCCM_startOperation(AESCCM_Handle handle,
                                          CryptoKey *key,
                                          uint8_t *nonce,
                                          uint8_t nonceLength,
                                          uint8_t *aad,
                                          size_t aadLength,
                                          uint8_t *input,
                                          size_t inputLength,
                                          uint8_t *mac,
                                          uint8_t macLength,
                                          uint8_t *output,
                                          AESCCM_Mode mode);
static void AESCCM_waitForResult(AESCCM_Handle handle);

/* Extern globals */
extern const AESCCM_Config AESCCM_config[];
extern const uint_least8_t AESCCM_count;

/* Static globals */
static bool isInitialized = false;

/*****************************************************************************
 *
 * CCM SWI function. Reads/verifies MAC, cleans up, and calls callback function
 * registered with the driver.
 *
 *****************************************************************************/
static void AESCCM_swiFxn (uintptr_t arg0, uintptr_t arg1) {
    AESCCMCC26XX_Object *object = ((AESCCM_Handle)arg0)->object;

    /* We need to copy / verify the MAC now so that it is not clobbered when we
     * release the CryptoResourceCC26XX_accessSemaphore semaphore.
     */
    if (object->mode == AESCCM_MODE_ENCRYPT) {
        /* If we are encrypting and authenticating a message, we only want to
         * copy the MAC to the target buffer
         */
        AESReadTag(object->mac, object->macLength);
    }
    else {
        /* If we are decrypting and verifying a message, we must now verify that the provided
         * MAC matches the one calculated in the decryption operation.
         */
        uint32_t verifyResult = AESVerifyTag(object->mac, object->macLength);

        object->macVerifyResult = (verifyResult == AES_SUCCESS) ? AESCCM_STATUS_SUCCESS : AESCCM_STATUS_MAC_INVALID;
    }

    /* Since plaintext keys use two reserved (by convention) slots in the keystore,
     * the slots must be invalidated to prevent its re-use without reloading
     * the key material again.
     */
    AESInvalidateKey(AES_KEY_AREA_6);
    AESInvalidateKey(AES_KEY_AREA_7);

    /*  This powers down all sub-modules of the crypto module until needed.
     *  It does not power down the crypto module at PRCM level and provides small
     *  power savings.
     */
    AESSelectAlgorithm(0x00);

    /*  Grant access for other threads to use the crypto module.
     *  The semaphore must be posted before the callbackFxn to allow the chaining
     *  of operations.
     */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

    Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);

    object->callbackFxn((AESCCM_Handle)arg0, object->key);
}

/******************************************************************************
 *
 * CCM HWI function. Only clears the interrupt and then posts the SWI.
 *
 *****************************************************************************/
static void AESCCM_hwiFxn (uintptr_t arg0) {
    AESCCMCC26XX_Object *object = ((AESCCM_Handle)arg0)->object;

    AESIntClear(AES_RESULT_RDY);

    SwiP_post(&(object->callbackSwi));
}

/******************************************************************************
 *
 * CCM callback function registered in AESCCM_RETURN_BEHAVIOR_BLOCKING and
 * AESCCM_RETURN_BEHAVIOR_POLLING.
 *
 *****************************************************************************/
static void AESCCM_internalCallbackFxn (AESCCM_Handle handle, CryptoKey *processedKey) {
    AESCCMCC26XX_Object *object = handle->object;

    /* This function is only ever registered when in AESCCM_RETURN_BEHAVIOR_BLOCKING
     * or AESCCM_RETURN_BEHAVIOR_POLLING.
     */
    if (object->returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
    else {
        CryptoResourceCC26XX_pollingFlag = 1;
    }
}

/******************************************************************************
 *
 * Initializes all AESCCMCC26XX_Objects in the board file.
 *
 *****************************************************************************/
void AESCCM_init(void) {
    uint_least8_t i;
    uint_fast8_t key;

    key = HwiP_disable();

    if (!isInitialized) {
        /* Call each instances' driver init function */
        for (i = 0; i < AESCCM_count; i++) {
            AESCCM_Handle handle = (AESCCM_Handle)&(AESCCM_config[i]);
            AESCCMCC26XX_Object *object = (AESCCMCC26XX_Object *)handle->object;
            object->isOpen = false;
        }

        isInitialized = true;
    }

    HwiP_restore(key);
}

/******************************************************************************
 *
 * Set the provided AESCCM_Params to their default values.
 *
 *****************************************************************************/
void AESCCM_Params_init(AESCCM_Params *params){
    *params = AESCCM_defaultParams;
}

/******************************************************************************
 *
 * Open the driver and initialize all RTOS objects.
 *
 *****************************************************************************/
AESCCM_Handle AESCCM_open(uint_least8_t index, AESCCM_Params *params) {
    SwiP_Params                 swiParams;
    AESCCM_Handle               handle;
    AESCCMCC26XX_Object        *object;
    AESCCMCC26XX_HWAttrs const *hwAttrs;
    uint_fast8_t                key;

    handle = (AESCCM_Handle)&(AESCCM_config[index]);
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    DebugP_assert(index >= AESCCM_count);

    key = HwiP_disable();

    if (!isInitialized ||  object->isOpen) {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    /* If params are NULL, use defaults */
    if (params == NULL) {
        params = (AESCCM_Params *)&AESCCM_defaultParams;
    }

    /* This is currently not supported. Eventually it will make the TRNG generate the nonce */
    DebugP_assert(!params->nonceInternallyGenerated);
    DebugP_assert(params->returnBehavior == AESCCM_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    object->nonceInternallyGenerated = params->nonceInternallyGenerated;
    object->returnBehavior = params->returnBehavior;
    object->callbackFxn = params->returnBehavior == AESCCM_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : AESCCM_internalCallbackFxn;
    object->semaphoreTimeout = params->timeout;

    /* Create Swi object for this AESCCM peripheral */
    SwiP_Params_init(&swiParams);
    swiParams.arg0 = (uintptr_t)handle;
    swiParams.priority = hwAttrs->swiPriority;
    SwiP_construct(&(object->callbackSwi), AESCCM_swiFxn, &swiParams);

    CryptoResourceCC26XX_constructRTOSObjects();

    /* Set power dependency - i.e. power up and enable clock for Crypto (CryptoResourceCC26XX) module. */
    Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);

    return handle;
}

/******************************************************************************
 *
 * Destruct all RTOS objects and close the driver.
 *
 *****************************************************************************/
void AESCCM_close(AESCCM_Handle handle) {
    AESCCMCC26XX_Object         *object;

    DebugP_assert(handle);

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;

    CryptoResourceCC26XX_destructRTOSObjects();

    /* Destroy the Swi */
    SwiP_destruct(&(object->callbackSwi));

    /* Release power dependency on Crypto Module. */
    Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);

    /* Mark the module as available */
    object->isOpen = false;
}

/******************************************************************************
 *
 * Verify the MAC calculated in the last operation against the provided one
 *
 *****************************************************************************/
int_fast16_t AESCCM_verifyMAC(AESCCM_Handle handle, uint8_t *mac) {
    AESCCMCC26XX_Object *object = handle->object;

    DebugP_assert(handle);

    /* In order to support operation chaining and prevent a race-condition, the MAC is verified
     * within the SWI and the result is stored in the object.
     */
    int_fast16_t verificationResult = object->macVerifyResult;

    object->macVerifyResult = AESCCM_STATUS_ERROR;

    return verificationResult;
}

/******************************************************************************
 *
 * Start a CCM operation.
 *
 *****************************************************************************/
static int_fast16_t AESCCM_startOperation(AESCCM_Handle handle,
                                          CryptoKey *key,
                                          uint8_t *nonce,
                                          uint8_t nonceLength,
                                          uint8_t *aad,
                                          size_t aadLength,
                                          uint8_t *input,
                                          size_t inputLength,
                                          uint8_t *mac,
                                          uint8_t macLength,
                                          uint8_t *output,
                                          AESCCM_Mode mode) {
    AESCCMCC26XX_Object *object = handle->object;
    AESCCMCC26XX_HWAttrs const *hwAttrs = handle->hwAttrs;
    /* Only plaintext CryptoKeys are supported for now */
    uint16_t keyLength = key->u.plaintext.keyLength;
    uint8_t *keyingMaterial = key->u.plaintext.keyMaterial;

    DebugP_assert(handle);
    DebugP_assert(key);
    DebugP_assert(nonce && (nonceLength >= 7 && nonceLength <= 13));
    DebugP_assert((aad && aadLength) || (input && inputLength));
    DebugP_assert(mac && (macLength <= 16));
    DebugP_assert(key->encoding == CryptoKey_PLAINTEXT);

    /* Try and obtain access to the crypto module */
    if (AESCCM_waitForAccess(handle) != SemaphoreP_OK) {
        return AESCCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    object->key = key;
    object->macLength = macLength;
    object->mac = mac;
    object->mode = mode;

    Power_setConstraint(PowerCC26XX_SB_DISALLOW);

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&CryptoResourceCC26XX_hwi, AESCCM_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_CRYPTO_RESULT_AVAIL_IRQ, hwAttrs->intPriority);

    /* Load the key from RAM or flash into the key store at a hardcoded and reserved location */
    if (AESWriteToKeyStore(keyingMaterial, keyLength, AES_KEY_AREA_6) != AES_SUCCESS) {
        return AESCCM_STATUS_ERROR;
    }

    /* Power the AES sub-module of the crypto module */
    AESSelectAlgorithm(AES_ALGSEL_AES);

    /* Load the key from the key store into the internal register banks of the AES sub-module */
    AESReadFromKeyStore(AES_KEY_AREA_6);

    AESWriteCCMInitializationVector(nonce, nonceLength);

    AESConfigureCCMCtrl(nonceLength, macLength, mode == AESCCM_MODE_ENCRYPT);

    AESSetDataLength(inputLength);
    AESSetAuthLength(aadLength);

    if (aadLength) {
        /* If aadLength were 0, AESWaitForIRQFlags() would never return as the AES_DMA_IN_DONE flag
         * would never trigger.
         */
        AESStartDMAOperation(aad, aadLength,  NULL, 0);
        AESWaitForIRQFlags(AES_DMA_IN_DONE | AES_DMA_BUS_ERR | AES_KEY_ST_WR_ERR | AES_KEY_ST_RD_ERR);
    }


    /* If inputLength is 0, AESStartDMAOperation() does nothing but the AAD DMA transaction will cause
     * the interrupt to fire and the handler will be called when IntEnable enables the vector.
     * The flag must be cleared before the interrupts are enabled. Otherwise there is a race condition
     * where the flag is cleared after the interrupt sets it causing the driver to wait on the flag forever.
     */
    CryptoResourceCC26XX_pollingFlag = 0;
    IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
    IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    AESStartDMAOperation(input, inputLength, output, inputLength);

    AESCCM_waitForResult(handle);

    if (mode == AESCCM_MODE_ENCRYPT || object->returnBehavior == AESCCM_RETURN_BEHAVIOR_CALLBACK) {
        return AESCCM_STATUS_SUCCESS;
    }
    else {
        return AESCCM_verifyMAC(handle, NULL);
    }
}

/******************************************************************************
 *
 * Wait for the peripheral to become available.
 *
 *****************************************************************************/
static int_fast16_t AESCCM_waitForAccess(AESCCM_Handle handle) {
    AESCCMCC26XX_Object *object = handle->object;
    uint32_t timeout;

    /* Set to SemaphoreP_NO_WAIT to start operations from SWI or HWI context */
    timeout = object->returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING ? object->semaphoreTimeout : SemaphoreP_NO_WAIT;

    return SemaphoreP_pend(&CryptoResourceCC26XX_accessSemaphore, timeout);
}

/******************************************************************************
 *
 * Wait until the CCM operation completes.
 *
 *****************************************************************************/
static void AESCCM_waitForResult(AESCCM_Handle handle){
    AESCCMCC26XX_Object *object = handle->object;

    if (object->returnBehavior == AESCCM_RETURN_BEHAVIOR_POLLING) {
        while(!CryptoResourceCC26XX_pollingFlag);
    }
    else if (object->returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_pend(&CryptoResourceCC26XX_operationSemaphore, SemaphoreP_WAIT_FOREVER);
    }
}

/******************************************************************************
 *
 * Start a CCM encrypt operation.
 *
 *****************************************************************************/
int_fast16_t AESCCM_oneStepEncrypt(AESCCM_Handle handle,
                                   CryptoKey *key,
                                   uint8_t *aad, size_t aadLength,
                                   uint8_t *plaintext, size_t plaintextLength,
                                   uint8_t *nonce, uint8_t nonceLength,
                                   uint8_t *mac, uint8_t macLength,
                                   uint8_t *ciphertext) {

    return AESCCM_startOperation(handle,
                                         key,
                                         nonce,
                                         nonceLength,
                                         aad,
                                         aadLength,
                                         plaintext,
                                         plaintextLength,
                                         mac,
                                         macLength,
                                         ciphertext,
                                         AESCCM_MODE_ENCRYPT);
}

/******************************************************************************
 *
 * Start a CCM decrypt operation.
 *
 *****************************************************************************/
int_fast16_t AESCCM_oneStepDecrypt(AESCCM_Handle handle,
                                   CryptoKey *key,
                                   uint8_t *aad, size_t aadLength,
                                   uint8_t *ciphertext, size_t ciphertextLength,
                                   uint8_t *nonce, uint8_t nonceLength,
                                   uint8_t *mac, uint8_t macLength,
                                   uint8_t *plaintext) {

    return AESCCM_startOperation(handle,
                                         key,
                                         nonce,
                                         nonceLength,
                                         aad,
                                         aadLength,
                                         ciphertext,
                                         ciphertextLength,
                                         mac,
                                         macLength,
                                         plaintext,
                                         AESCCM_MODE_DECRYPT);
}

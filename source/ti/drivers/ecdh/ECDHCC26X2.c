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
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/ecdh/ECDHCC26X2.h>
#include <ti/drivers/cryptoutils/sharedresources/PKAResourceCC26XX.h>

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
static void ECDH_hwiFxn (uintptr_t arg0);
static void ECDH_swiFxn (uintptr_t arg0, uintptr_t arg1);
static void ECDH_internalCallbackFxn (ECDH_Handle handle, int_fast16_t operationStatus);
static int_fast16_t ECDH_waitForAccess(ECDH_Handle handle);
static void ECDH_waitForResult(ECDH_Handle handle);

/* Extern globals */
extern const ECDH_Config ECDH_config[];
extern const uint_least8_t ECDH_count;

/* Static globals */
static bool isInitialized = false;

//*****************************************************************************
//
// CCM SWI function. Reads/verifies MAC, cleans up, and calls callback function
// registered with the driver.
//
//*****************************************************************************
static void ECDH_swiFxn (uintptr_t arg0, uintptr_t arg1) {
    ECDHCC26X2_Object *object = ((ECDH_Handle)arg0)->object;
    uint32_t operationResult;
    int_fast16_t operationStatus;

    operationResult = PKAEccMultiplyGetResult(object->generatedKeyingMaterial, object->generatedKeyingMaterial + object->curve->length, object->resultAddress, object->curve->length);

    switch (operationResult) {
        case PKA_STATUS_SUCCESS:
            operationStatus = ECDH_STATUS_SUCCESS;
            break;
        case PKA_STATUS_RESULT_0:
            operationStatus = ECDH_STATUS_RESULT_POINT_AT_INFINITY;
            break;
        case PKA_STATUS_FAILURE:
            operationStatus = ECDH_STATUS_ERROR;
            break;
        default:
            operationStatus = ECDH_STATUS_ERROR;
            break;
    }

    /*  Grant access for other threads to use the crypto module.
     *  The semaphore must be posted before the callbackFxn to allow the chaining
     *  of operations.
     */
    SemaphoreP_post(&PKAResourceCC26XX_accessSemaphore);

    Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);

    object->callbackFxn((ECDH_Handle)arg0, operationStatus);
}

//*****************************************************************************
//
// CCM HWI function. Only clears the interrupt and then posts the SWI.
//
//*****************************************************************************
static void ECDH_hwiFxn (uintptr_t arg0) {
    ECDHCC26X2_Object *object = ((ECDH_Handle)arg0)->object;

    // Disable interrupt again
    IntDisable(INT_PKA_IRQ);

    SwiP_post(&(object->callbackSwi));
}

//*****************************************************************************
//
// CCM callback function registered in ECDH_RETURN_BEHAVIOR_BLOCKING and
// ECDH_RETURN_BEHAVIOR_POLLING.
//
//*****************************************************************************
static void ECDH_internalCallbackFxn (ECDH_Handle handle, int_fast16_t operationStatus) {
    ECDHCC26X2_Object *object = handle->object;

    /* This function is only ever registered when in ECDH_RETURN_BEHAVIOR_BLOCKING
     * or ECDH_RETURN_BEHAVIOR_POLLING.
     */
    if (object->returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_post(&PKAResourceCC26XX_operationSemaphore);
    }
    else {
        PKAResourceCC26XX_pollingFlag = 1;
    }
}

//*****************************************************************************
//
// Initializes all ECDHCC26X2_Objects in the board file.
//
//*****************************************************************************
void ECDH_init(void) {
    uint_least8_t i;
    uint_fast8_t key;

    key = HwiP_disable();

    if (!isInitialized) {
        // Call each instances' driver init function
        for (i = 0; i < ECDH_count; i++) {
            ECDH_Handle handle = (ECDH_Handle)&(ECDH_config[i]);
            ECDHCC26X2_Object *object = (ECDHCC26X2_Object *)handle->object;
            object->isOpen = false;
        }

        isInitialized = true;
    }

    HwiP_restore(key);
}

//*****************************************************************************
//
// Set the provided ECDH_Params to their default values.
//
//*****************************************************************************
void ECDH_Params_init(ECDH_Params *params){
    *params = ECDH_defaultParams;
}

//*****************************************************************************
//
// Open the driver and initialize all RTOS objects.
//
//*****************************************************************************
ECDH_Handle ECDH_open(uint_least8_t index, ECDH_Params *params) {
    SwiP_Params                 swiParams;
    ECDH_Handle                  handle;
    ECDHCC26X2_Object           *object;
    ECDHCC26X2_HWAttrs const    *hwAttrs;
    uint_fast8_t                key;

    handle = (ECDH_Handle)&(ECDH_config[index]);
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    DebugP_assert(index < ECDH_count);

    key = HwiP_disable();

    if (!isInitialized ||  object->isOpen) {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    // If params are NULL, use defaults
    if (params == NULL) {
        params = (ECDH_Params *)&ECDH_defaultParams;
    }

    DebugP_assert((params->returnBehavior == ECDH_RETURN_BEHAVIOR_CALLBACK) ? params->callbackFxn : true);

    object->returnBehavior = params->returnBehavior;
    object->callbackFxn = params->returnBehavior == ECDH_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : ECDH_internalCallbackFxn;
    object->semaphoreTimeout = params->timeout;

    // Create Swi object for this ECC peripheral
    SwiP_Params_init(&swiParams);
    swiParams.arg0 = (uintptr_t)handle;
    swiParams.priority = hwAttrs->swiPriority;
    SwiP_construct(&(object->callbackSwi), ECDH_swiFxn, &swiParams);

    PKAResourceCC26XX_constructRTOSObjects();

    // Set power dependency - i.e. power up and enable clock for PKA (PKAResourceCC26XX) module.
    Power_setDependency(PowerCC26X2_PERIPH_PKA);

    return handle;
}

//*****************************************************************************
//
// Destruct all RTOS objects and close the driver.
//
//*****************************************************************************
void ECDH_close(ECDH_Handle handle) {
    ECDHCC26X2_Object         *object;

    DebugP_assert(handle);

    // Get the pointer to the object
    object = handle->object;

    // Release power dependency on PKA Module.
    Power_releaseDependency(PowerCC26X2_PERIPH_PKA);

    // Destroy the Swi
    SwiP_destruct(&(object->callbackSwi));

    PKAResourceCC26XX_destructRTOSObjects();

    // Mark the module as available
    object->isOpen = false;
}



//*****************************************************************************
//
// Wait for the peripheral to become available.
//
//*****************************************************************************
static int_fast16_t ECDH_waitForAccess(ECDH_Handle handle) {
    ECDHCC26X2_Object *object = handle->object;
    uint32_t timeout;

    // Set to SemaphoreP_NO_WAIT to start operations from SWI or HWI context
    timeout = object->returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING ? object->semaphoreTimeout : SemaphoreP_NO_WAIT;

    return SemaphoreP_pend(&PKAResourceCC26XX_accessSemaphore, timeout);
}

//*****************************************************************************
//
// Wait until the ECDH operation completes.
//
//*****************************************************************************
static void ECDH_waitForResult(ECDH_Handle handle){
    ECDHCC26X2_Object *object = handle->object;

    if (object->returnBehavior == ECDH_RETURN_BEHAVIOR_POLLING) {
        while(!PKAResourceCC26XX_pollingFlag);
    }
    else if (object->returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_pend(&PKAResourceCC26XX_operationSemaphore, SemaphoreP_WAIT_FOREVER);
    }
}

//*****************************************************************************
//
// Perform a synchronous ECC multiplication of a point by a scalar.
//
//*****************************************************************************
static int_fast16_t ECDH_multiplyCurvePoint(ECDH_Handle handle, const ECCParams_CurveParams *eccParams, const uint8_t *scalar, const uint8_t *inputPointX, const uint8_t *inputPointY, uint8_t *outputPoint) {
    ECDHCC26X2_Object *object            = handle->object;
    ECDHCC26X2_HWAttrs const *hwAttrs    = handle->hwAttrs;

    uint32_t operationResult;

    if (ECDH_waitForAccess(handle) != SemaphoreP_OK) {
        return ECDH_STATUS_RESOURCE_UNAVAILABLE;
    }

    object->generatedKeyingMaterial = outputPoint;
    object->curve = eccParams;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECDH_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_SB_DISALLOW);

    operationResult = PKAEccMultiplyStart(scalar,
                                          inputPointX,
                                          inputPointY,
                                          eccParams->prime,
                                          eccParams->a,
                                          eccParams->b,
                                          eccParams->length,
                                          &object->resultAddress);

    IntPendClear(INT_PKA_IRQ);
    IntEnable(INT_PKA_IRQ);

    if (operationResult != PKA_STATUS_SUCCESS) {
        return ECDH_STATUS_ERROR;
    }

    ECDH_waitForResult(handle);

    return ECDH_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Generate a pulic key from a private key and an ECC curve.
//
//*****************************************************************************
int_fast16_t ECDH_generatePublicKey(ECDH_Handle handle, const ECCParams_CurveParams *eccParams, const CryptoKey *myPrivateKey, CryptoKey *myPublicKey) {
    uint8_t *privateKeyingMaterial      = myPrivateKey->u.plaintext.keyMaterial;
    uint8_t *publicKeyingMaterial       = myPublicKey->u.plaintext.keyMaterial;

    DebugP_assert(eccParams->length == myPrivateKey->u.plaintext.keyLength);
    DebugP_assert(eccParams->length == myPublicKey->u.plaintext.keyLength / 2);
    DebugP_assert(myPublicKey->encoding == CryptoKey_BLANK_PLAINTEXT);
    DebugP_assert(myPrivateKey->encoding == CryptoKey_PLAINTEXT);

    return ECDH_multiplyCurvePoint(handle, eccParams, privateKeyingMaterial, eccParams->generatorX, eccParams->generatorY, publicKeyingMaterial);
}

//*****************************************************************************
//
// Compute a shared secret between a forein party's public key and your private key.
//
//*****************************************************************************
int_fast16_t ECDH_computeSharedSecret(ECDH_Handle handle, const ECCParams_CurveParams *eccParams, const CryptoKey *myPrivateKey, const CryptoKey *theirPublicKey, CryptoKey *sharedSecret) {
    uint8_t *privateKeyingMaterial          = myPrivateKey->u.plaintext.keyMaterial;
    uint8_t *publicKeyingMaterialX          = theirPublicKey->u.plaintext.keyMaterial;
    uint8_t *publicKeyingMaterialY          = theirPublicKey->u.plaintext.keyMaterial + eccParams->length;
    uint8_t *sharedSecretKeyingMaterial     = sharedSecret->u.plaintext.keyMaterial;

    DebugP_assert(eccParams->length == myPrivateKey->u.plaintext.keyLength);
    DebugP_assert(eccParams->length == theirPublicKey->u.plaintext.keyLength / 2);
    DebugP_assert(eccParams->length == sharedSecret->u.plaintext.keyLength / 2);
    DebugP_assert(sharedSecret->encoding == CryptoKey_BLANK_PLAINTEXT);
    DebugP_assert(myPrivateKey->encoding == CryptoKey_PLAINTEXT);
    DebugP_assert(theirPublicKey->encoding == CryptoKey_PLAINTEXT);

    return ECDH_multiplyCurvePoint(handle, eccParams, privateKeyingMaterial, publicKeyingMaterialX, publicKeyingMaterialY, sharedSecretKeyingMaterial);
}

//*****************************************************************************
//
// Generate a set of seeded keys derived from a shared secret.
//
//*****************************************************************************
int_fast16_t ECDH_calculateSharedEntropy(ECDH_Handle handle, const CryptoKey *sharedSecret, const ECDH_KDFFxn kdf, void *kdfPrimitiveDriverHandle, CryptoKey derivedKeys[], uint32_t derivedKeyCount) {
    uint8_t *seedEntropy = sharedSecret->u.plaintext.keyMaterial;
    size_t seedEntropyLength = sharedSecret->u.plaintext.keyLength;

    DebugP_assert(sharedSecret->encoding == CryptoKey_PLAINTEXT);

    return kdf(kdfPrimitiveDriverHandle, seedEntropy, seedEntropyLength, derivedKeys, derivedKeyCount);
}

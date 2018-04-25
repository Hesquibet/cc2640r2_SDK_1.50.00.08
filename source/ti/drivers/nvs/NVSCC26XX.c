/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
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

/*
 *  ======== NVSCC26XX.c ========
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/drivers/NVS.h>
#include <ti/drivers/nvs/NVSCC26XX.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/flash.h)
#include DeviceFamily_constructPath(driverlib/vims.h)
#include DeviceFamily_constructPath(driverlib/aon_batmon.h)

/* max number of bytes to write at a time to minimize interrupt latency */
#define MAX_WRITE_INCREMENT 8

static int_fast16_t checkEraseRange(NVS_Handle handle, size_t offset,
                        size_t size);
static int_fast16_t doErase(NVS_Handle handle, size_t offset, size_t size);
static uint8_t disableFlashCache(void);
static void restoreFlashCache(uint8_t mode);
static bool voltageOk(void);

extern NVS_Config NVS_config[];
extern const uint8_t NVS_count;

/* NVS function table for NVSCC26XX implementation */
const NVS_FxnTable NVSCC26XX_fxnTable = {
    NVSCC26XX_close,
    NVSCC26XX_control,
    NVSCC26XX_erase,
    NVSCC26XX_getAttrs,
    NVSCC26XX_init,
    NVSCC26XX_lock,
    NVSCC26XX_open,
    NVSCC26XX_read,
    NVSCC26XX_unlock,
    NVSCC26XX_write
};

/*
 *  Semaphore to synchronize access to flash region.
 */
static SemaphoreP_Handle  writeSem;

static size_t sectorSize;         /* fetched during init() */
static size_t sectorBaseMask;     /* for efficient argument checking */

/*
 *  ======== NVSCC26XX_close ========
 */
void NVSCC26XX_close(NVS_Handle handle)
{
    NVSCC26XX_Object *object;

    object = handle->object;
    object->opened = false;
}

/*
 *  ======== NVSCC26XX_control ========
 */
int_fast16_t NVSCC26XX_control(NVS_Handle handle, uint_fast16_t cmd,
                 uintptr_t arg)
{
    return (NVS_STATUS_UNDEFINEDCMD);
}

/*
 *  ======== NVSCC26XX_erase ========
 */
int_fast16_t NVSCC26XX_erase(NVS_Handle handle, size_t offset, size_t size)
{
    int_fast16_t status;

    SemaphoreP_pend(writeSem, SemaphoreP_WAIT_FOREVER);

    status = doErase(handle, offset, size);

    SemaphoreP_post(writeSem);

    return (status);
}

/*
 *  ======== NVSCC26XX_getAttrs ========
 */
void NVSCC26XX_getAttrs(NVS_Handle handle, NVS_Attrs *attrs)
{
    NVSCC26XX_HWAttrs const *hwAttrs;

    hwAttrs = handle->hwAttrs;

    /* FlashSectorSizeGet() returns the size of a flash sector in bytes. */
    attrs->regionBase  = hwAttrs->regionBase;
    attrs->regionSize  = hwAttrs->regionSize;
    attrs->sectorSize  = FlashSectorSizeGet();
}

/*
 *  ======== NVSCC26XX_init ========
 */
void NVSCC26XX_init()
{
    unsigned int key;
    SemaphoreP_Handle sem;

    /* initialize energy saving variables */
    sectorSize = FlashSectorSizeGet();
    sectorBaseMask = ~(sectorSize - 1);

    /* speculatively create a binary semaphore for thread safety */
    sem = SemaphoreP_createBinary(1);
    /* sem == NULL will be detected in 'open' */

    key = HwiP_disable();

    if (writeSem == NULL) {
        /* use the binary sem created above */
        writeSem = sem;
        HwiP_restore(key);
    }
    else {
        /* init already called */
        HwiP_restore(key);
        /* delete unused Semaphore */
        if (sem) SemaphoreP_delete(sem);
    }
}

/*
 *  ======== NVSCC26XX_lock =======
 */
int_fast16_t NVSCC26XX_lock(NVS_Handle handle, uint32_t timeout)
{
    switch (SemaphoreP_pend(writeSem, timeout)) {
        case SemaphoreP_OK:
            return (NVS_STATUS_SUCCESS);

        case SemaphoreP_TIMEOUT:
            return (NVS_STATUS_TIMEOUT);

        case SemaphoreP_FAILURE:
        default:
            return (NVS_STATUS_ERROR);
    }
}

/*
 *  ======== NVSCC26XX_open =======
 */
NVS_Handle NVSCC26XX_open(uint_least8_t index, NVS_Params *params)
{
    NVSCC26XX_Object *object;
    NVSCC26XX_HWAttrs const *hwAttrs;
    NVS_Handle handle;

    /* Confirm that 'init' has successfully completed */
    if (writeSem == NULL) {
        NVSCC26XX_init();
        if (writeSem == NULL) {
            return (NULL);
        }
    }

    /* verify NVS region index */
    if (index >= NVS_count) {
        return (NULL);
    }

    handle = &NVS_config[index];
    object = NVS_config[index].object;
    hwAttrs = NVS_config[index].hwAttrs;

    SemaphoreP_pend(writeSem, SemaphoreP_WAIT_FOREVER);

    if (object->opened == true) {
        SemaphoreP_post(writeSem);
        return (NULL);
    }

    /* The regionBase must be aligned on a flaah page boundary */
    if ((size_t)(hwAttrs->regionBase) & (sectorSize - 1)) {
        SemaphoreP_post(writeSem);
        return (NULL);
    }

    /* The region cannot be smaller than a sector size */
    if (hwAttrs->regionSize < sectorSize) {
        SemaphoreP_post(writeSem);
        return (NULL);
    }

    /* The region size must be a multiple of sector size */
    if (hwAttrs->regionSize != (hwAttrs->regionSize & sectorBaseMask)) {
        SemaphoreP_post(writeSem);
        return (NULL);
    }

    object->opened = true;

    SemaphoreP_post(writeSem);

    return (handle);
}

/*
 *  ======== NVSCC26XX_read =======
 */
int_fast16_t NVSCC26XX_read(NVS_Handle handle, size_t offset, void *buffer,
                 size_t bufferSize)
{
    NVSCC26XX_HWAttrs const *hwAttrs;

    hwAttrs = handle->hwAttrs;

    /* Validate offset and bufferSize */
    if (offset + bufferSize > hwAttrs->regionSize) {
        return (NVS_STATUS_INV_OFFSET);
    }

    /*
     *  Get exclusive access to the region.  We don't want someone
     *  else to erase the region while we are reading it.
     */
    SemaphoreP_pend(writeSem, SemaphoreP_WAIT_FOREVER);

    memcpy(buffer, (char *)(hwAttrs->regionBase) + offset, bufferSize);

    SemaphoreP_post(writeSem);

    return (NVS_STATUS_SUCCESS);
}

/*
 *  ======== NVSCC26XX_unlock =======
 */
void NVSCC26XX_unlock(NVS_Handle handle)
{
    SemaphoreP_post(writeSem);
}

/*
 *  ======== NVSCC26XX_write =======
 */
int_fast16_t NVSCC26XX_write(NVS_Handle handle, size_t offset, void *buffer,
                 size_t bufferSize, uint_fast16_t flags)
{
    NVSCC26XX_HWAttrs const *hwAttrs;
    unsigned int key;
    unsigned int size;
    uint32_t status = 0;
    int i;
    uint8_t mode;
    uint8_t *srcBuf, *dstBuf;
    size_t writeIncrement;
    int retval = NVS_STATUS_SUCCESS;

    /* confirm sufficient voltage */
    if (!voltageOk()) {
        return (NVSCC26XX_STATUS_LOW_VOLTAGE);
    }

    hwAttrs = handle->hwAttrs;

    /* Validate offset and bufferSize */
    if (offset + bufferSize > hwAttrs->regionSize) {
        return (NVS_STATUS_INV_OFFSET);
    }

    /* Get exclusive access to the Flash region */
    SemaphoreP_pend(writeSem, SemaphoreP_WAIT_FOREVER);

    /* If erase is set, erase destination sector(s) first */
    if (flags & NVS_WRITE_ERASE) {
        retval = doErase(handle, offset & sectorBaseMask,
                     (bufferSize + sectorSize) & sectorBaseMask);
        if (retval != NVS_STATUS_SUCCESS) {
            SemaphoreP_post(writeSem);
            return (retval);
        }
    }
    else if (flags & NVS_WRITE_PRE_VERIFY) {
        /*
         *  If pre-verify, each destination byte must be able to be changed to the
         *  source byte (1s to 0s, not 0s to 1s).
         *  this is satisfied by the following test:
         *     src == (src & dst)
         */
        dstBuf = (uint8_t *)((uint32_t)(hwAttrs->regionBase) + offset);
        srcBuf = buffer;
        for (i = 0; i < bufferSize; i++) {
            if (srcBuf[i] != (srcBuf[i] & dstBuf[i])) {
                SemaphoreP_post(writeSem);
                return (NVS_STATUS_INV_WRITE);
            }
        }
    }

    srcBuf = buffer;
    size   = bufferSize;
    dstBuf = (uint8_t *)((uint32_t)(hwAttrs->regionBase) + offset);

    mode = disableFlashCache();

    while (size) {
        if (size > MAX_WRITE_INCREMENT) {
            writeIncrement = MAX_WRITE_INCREMENT;
        }
        else {
            writeIncrement = size;
        }
        key = HwiP_disable();
        status = FlashProgram((uint8_t*)srcBuf, (uint32_t)dstBuf,
                     writeIncrement);
        HwiP_restore(key);

        if (status != 0) {
            break;
        }
        else {
            size -= writeIncrement;
            srcBuf += writeIncrement;
            dstBuf += writeIncrement;
        }
    }

    restoreFlashCache(mode);

    if (status != 0) {
        retval = NVS_STATUS_ERROR;
    }
    else if (flags & NVS_WRITE_POST_VERIFY) {
        /*
         *  Note: This validates the entire region even on erase mode.
         */
        for (i = 0; i < size; i++) {
            if (srcBuf[i] != dstBuf[i]) {
                retval = NVS_STATUS_ERROR;
                break;
            }
        }
    }

    SemaphoreP_post(writeSem);

    return (retval);
}

/*
 *  ======== checkEraseRange ========
 */
static int_fast16_t checkEraseRange(NVS_Handle handle, size_t offset,
                        size_t size)
{
    NVSCC26XX_HWAttrs const *hwAttrs = handle->hwAttrs;

    if (offset != (offset & sectorBaseMask)) {
        return (NVS_STATUS_INV_ALIGNMENT);    /* poorly aligned start */
                                              /* address */
    }

    if (offset >= hwAttrs->regionSize) {
        return (NVS_STATUS_INV_OFFSET);   /* offset is past end of region */
    }

    if (offset + size > hwAttrs->regionSize) {
        return (NVS_STATUS_INV_SIZE);     /* size is too big */
    }

    if (size != (size & sectorBaseMask)) {
        return (NVS_STATUS_INV_SIZE);     /* size is not a multiple of */
                                          /* sector size */
    }

    return (NVS_STATUS_SUCCESS);
}

/*
 *  ======== doErase ========
 */
static int_fast16_t doErase(NVS_Handle handle, size_t offset, size_t size)
{
    NVSCC26XX_HWAttrs const *hwAttrs = handle->hwAttrs;
    unsigned int key;
    uint8_t mode;
    uint32_t status = 0;
    uint32_t sectorBase;
    int_fast16_t rangeStatus;

    /* confirm sufficient voltage */
    if (!voltageOk()) {
        return (NVSCC26XX_STATUS_LOW_VOLTAGE);
    }

    /* sanity test the erase args */
    rangeStatus = checkEraseRange(handle, offset, size);

    if (rangeStatus != NVS_STATUS_SUCCESS) {
        return (rangeStatus);
    }

    sectorBase = (uint32_t)hwAttrs->regionBase + offset;

    mode = disableFlashCache();

    while (size) {
        key = HwiP_disable();
        status = FlashSectorErase(sectorBase);
        HwiP_restore(key);

        if (status != FAPI_STATUS_SUCCESS) {
            break;
        }

        sectorBase += sectorSize;
        size -= sectorSize;
    }

    restoreFlashCache(mode);

    if (status != FAPI_STATUS_SUCCESS) {
        return (NVS_STATUS_ERROR);
    }

    return (NVS_STATUS_SUCCESS);
}

/*
 *  ======== disableFlashCache ========
 *  When updating the Flash, the VIMS (Vesatile Instruction Memory System)
 *  mode must be set to GPRAM or OFF, before programming, and both VIMS
 *  flash line buffers must be set to disabled.
 */
static uint8_t disableFlashCache(void)
{
    uint8_t mode = VIMSModeGet(VIMS_BASE);

    VIMSLineBufDisable(VIMS_BASE);

    if (mode != VIMS_MODE_DISABLED) {
        VIMSModeSet(VIMS_BASE, VIMS_MODE_DISABLED);
        while (VIMSModeGet(VIMS_BASE) != VIMS_MODE_DISABLED);
    }

    return (mode);
}

/*
 *  ======== restoreFlashCache ========
 */
static void restoreFlashCache(uint8_t mode)
{
    if (mode != VIMS_MODE_DISABLED) {
        VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);
    }

    VIMSLineBufEnable(VIMS_BASE);
}

#define BATMON_ON (HWREG(AON_BATMON_BASE + AON_BATMON_O_CTL) ? true : false)
#define MIN_VDD_FLASH  0x180  /* 1.50 volts (0.50=128/256 -> 128=0x80) */

/*
 *  ======== voltageOk =======
 *  returns true if voltage is high enough for safe programming
 */
static bool voltageOk(void)
{
    bool batMonEnabled = BATMON_ON;
    bool batVoltageOk = true;

    /* enable the battery monitor if it isn't already */
    if (!batMonEnabled) AONBatMonEnable();

    if (AONBatMonBatteryVoltageGet() < MIN_VDD_FLASH) {
        /* Measured device voltage is below threshold */
        batVoltageOk = false;
    }

    /* restore battery monitor to its previous state */
    if (!batMonEnabled) AONBatMonDisable();

    return (batVoltageOk);
}

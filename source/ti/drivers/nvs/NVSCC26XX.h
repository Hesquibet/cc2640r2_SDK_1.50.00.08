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
/*!
 *  ============================================================================
 *  @file       NVSCC26XX.h
 *  @brief      Non-Volatile Storage driver for CC13XX/CC26XX devices.
 *
 *  ### Interrupt Latency During Flash Operations #
 *
 *  When writing to or erasing flash, interrupts must be disabled to avoid
 *  executing code in flash while the flash is being reprogrammed. This
 *  constraint is met internally by the driver. User code does not need
 *  to safeguard against this.
 *
 *  Additionally, to avoid extremely large interrupt latencies that would be
 *  incurred if entire blocks were written with interrupts disabled, block
 *  writes to flash are broken into multiple smaller sizes.
 *
 *  Even with this scheme in place, latencies of roughly 64 microseconds will
 *  be incurred while flash is being written to.
 *
 *  A similar caveat applies to flash erase operations. Erasing an entire
 *  flash sector (the minimal amount that can be erased at a time) can take
 *  roughly 8 milliseconds. This entire operation must be performed with
 *  interrupts disabled. Here again, this requirement is met internally
 *  by the driver and flash region erases are performed one sector at a
 *  time to minimize this significant latency.impact.
 *
 *  Care must be taken by the user to not perform flash write or erase
 *  operations during latency critical phases of an application. See the
 *  NVS_lock() and NVS_unlock() API descriptions for more information.
 *
 *  ============================================================================
 */

#ifndef ti_drivers_nvs_NVSCC26XX__include
#define ti_drivers_nvs_NVSCC26XX__include

#include <stdint.h>
#include <stdbool.h>

#if defined (__cplusplus)
extern "C" {
#endif

/*!
 *  @brief   Error status code returned by NVS_erase(), NVS_write().
 *
 *  This error status is returned if the system voltage is too low to safely
 *  perform the flash operation. Voltage must be 1.5V or greater.
 */
#define NVSCC26XX_STATUS_LOW_VOLTAGE    (NVS_STATUS_RESERVED - 1)

/*!
 *  @internal @brief NVS function pointer table
 *
 *  'NVSCC26XX_fxnTable' is a fully populated function pointer table
 *  that can be referenced in the NVS_config[] array entries.
 *
 *  Users can minimize their application code size by providing their
 *  own custom NVS function pointer table that contains only those APIs
 *  used by the application.
 *
 *  An example of a custom NVS function table is shown below:
 *  @code
 *  //
 *  // Since the application does not use the
 *  // NVS_control(), NVS_lock(), and NVS_unlock() APIs,
 *  // these APIs are removed from the function
 *  // pointer table and replaced with NULL
 *  //
 *  const NVS_FxnTable myNVS_fxnTable = {
 *      NVSCC26XX_close,
 *      NULL,     // remove NVSCC26XX_control(),
 *      NVSCC26XX_erase,
 *      NVSCC26XX_getAttrs,
 *      NVSCC26XX_init,
 *      NULL,     // remove NVSCC26XX_lock(),
 *      NVSCC26XX_open,
 *      NVSCC26XX_read,
 *      NULL,     // remove NVSCC26XX_unlock(),
 *      NVSCC26XX_write
 *  };
 *  @endcode
 */
extern const NVS_FxnTable NVSCC26XX_fxnTable;

/*!
 *  @brief      NVSCC26XX attributes
 *
 *  The 'regionBase' field must point to the base address of the region
 *  to be managed.
 *
 *  The regionSize must be an integer multiple of the flash sector size.
 *  For most CC26XX/CC13XX devices, the flash sector size is 4096 bytes.
 *  The NVSCC26XX driver will determine the device's actual sector size by
 *  reading internal system configuration registers.
 *
 *  Care must be taken to ensure that the linker does not unintentionally
 *  place application content (e.g., code/data) in the flash regions.
 *
 *  For CCS and IAR tools, defining and reserving flash memory regions can
 *  be done entirely within the Board.c file. For GCC, additional content is
 *  required in the application's linker command file to achieve the same
 *  result.
 *
 *  The example below defines a char array, 'flashBuf' and uses compiler
 *  CCS and IAR compiler pragmas to place 'flashBuf' at a specific address
 *  within the flash memory.
 *
 *  For GCC, the 'flashBuf' array is placed into a named linker section.
 *  Corresponding linker commands are added to the application's linker
 *  command file to place the section at a specific flash memory address.
 *  The section placement command is carefully chosen to only RESERVE space
 *  for the 'flashBuf' array, and not to actually initialize it during
 *  the application load process, thus preserving the content of flash.
 *
 *  The 'regionBase' fields of the two HWAttrs region instances
 *  are initialized to point to the base address of 'flashBuf' and to some
 *  offset from the base of the char array.
 *
 *  The linker command syntax is carefully chosen to only RESERVE space
 *  for the char array and not to actually initialize it during application
 *  load.
 *
 *  @code
 *  #define SECTORSIZE 0x1000
 *  #define FLASH_REGION_BASE 0x1b000
 *
 *  //
 *  // Reserve flash sectors for NVS driver use
 *  // by placing an uninitialized byte array
 *  // at the desired flash address.
 *  //
 *  #if defined(__TI_COMPILER_VERSION__)
 *
 *  //
 *  //  Place uninitialized array at FLASH_REGION_BASE
 *  //
 *  #pragma LOCATION(flashBuf, FLASH_REGION_BASE);
 *  #pragma NOINIT(flashBuf);
 *  char flashBuf[SECTORSIZE * 4];
 *
 *  #elif defined(__IAR_SYSTEMS_ICC__)
 *
 *  //
 *  //  Place uninitialized array at FLASH_REGION_BASE
 *  //
 *  __no_init char flashBuf[SECTORSIZE * 4] @ FLASH_REGION_BASE;
 *
 *  #elif defined(__GNUC__)
 *
 *
 *  //
 *  //  Place the flash buffers in the .nvs section created in the gcc linker file.
 *  //  The .nvs section enforces alignment on a sector boundary but may
 *  //  be placed anywhere in flash memory.  If desired the .nvs section can be set
 *  //  to a fixed address by changing the following in the gcc linker file:
 *
 * .nvs (FIXED_FLASH_ADDR) (NOLOAD) : AT (FIXED_FLASH_ADDR) {
 *      *(.nvs)
 * } > REGION_TEXT
 *
 *  __attribute__ ((section (".nvs")))
 *  char flashBuf[SECTORSIZE * 4];
 *
 *  #endif
 *
 *  NVSCC26XX_HWAttrs nvsCC26XXHWAttrs[2] = {
 *      //
 *      // region 0 is 1 flash sector in length.
 *      //
 *      {
 *          .regionBase = (void *)flashBuf,
 *          .regionSize = SECTORSIZE,
 *      },
 *      //
 *      // region 1 is 3 flash sectors in length.
 *      //
 *      {
 *          .regionBase = (void *)(flashBuf + SECTORSIZE),
 *          .regionSize = SECTORSIZE*3,
 *      }
 *  };
 *
 *  Example GCC linker command file content reserves flash space
 *  but does not initialize it:
 *
 *  MEMORY
 *  {
 *      FLASH (RX)      : ORIGIN = 0x00000000, LENGTH = 0x0001ffa8
 *      FLASH_CCFG (RX) : ORIGIN = 0x0001ffa8, LENGTH = 0x00000058
 *      SRAM (RWX)      : ORIGIN = 0x20000000, LENGTH = 0x00005000
 *  }
 *
 *  .nvs (0x1b000) (NOLOAD) : AT (0x1b000) {
 *      *(.nvs)
 *  } > NVS
 *
 *
 *  @endcode
 */
typedef struct NVSCC26XX_HWAttrs {
    void        *regionBase;        /*!< Base address of flash region */
    size_t      regionSize;         /*!< The size of the region in bytes */
} NVSCC26XX_HWAttrs;

/*
 *  @brief      NVSCC26XX Object
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct NVSCC26XX_Object {
    bool        opened;             /* Has this region been opened */
} NVSCC26XX_Object;

/*!
 *  @cond NODOC
 *  NVSCC26XX driver public APIs
 */

extern void         NVSCC26XX_close(NVS_Handle handle);
extern int_fast16_t NVSCC26XX_control(NVS_Handle handle, uint_fast16_t cmd,
                        uintptr_t arg);
extern int_fast16_t NVSCC26XX_erase(NVS_Handle handle, size_t offset,
                        size_t size);
extern void         NVSCC26XX_getAttrs(NVS_Handle handle, NVS_Attrs *attrs);
extern void         NVSCC26XX_init();
extern int_fast16_t NVSCC26XX_lock(NVS_Handle handle, uint32_t timeout);
extern NVS_Handle   NVSCC26XX_open(uint_least8_t index, NVS_Params *params);
extern int_fast16_t NVSCC26XX_read(NVS_Handle handle, size_t offset,
                        void *buffer, size_t bufferSize);
extern void         NVSCC26XX_unlock(NVS_Handle handle);
extern int_fast16_t NVSCC26XX_write(NVS_Handle handle, size_t offset,
                        void *buffer, size_t bufferSize, uint_fast16_t flags);
/*! @endcond */

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

/*@}*/
#endif /* ti_drivers_nvs_NVSCC26XX__include */

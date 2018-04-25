/******************************************************************************

 @file       cc26xx_app_oad.cmd

 @brief CC2650F128 linker configuration file for TI-RTOS with Code Composer
        Studio.

        Imported Symbols
        Note: Linker defines are located in the CCS IDE project by placing them
        in
        Properties->Build->Linker->Advanced Options->Command File Preprocessing.

        CACHE_AS_RAM:       Disable system cache to be used as GPRAM for
                            additional volatile memory storage.
        CCxxxxROM:          Device Name (e.g. CC2650). In order to define this
                            symbol, the tool chain requires that it be set to
                            a specific value, but in fact, the actual value does
                            not matter as it is not used in the linker control
                            file. The only way this symbol is used is based on
                            whether it is defined or not, not its actual value.
                            There are other linker symbols that do specifically
                            set their value to 1 to indicate R1, and 2 to
                            indicate R2, and these values are checked and do make
                            a difference. However, it would appear confusing if
                            the device name's value did not correspond to the
                            value set in other linker symbols. In order to avoid
                            this confusion, when the symbol is defined, it should
                            be set to the value of the device's ROM that it
                            corresponds so as to look and feel consistent. Please
                            note that a device name symbol should always be
                            defined to avoid side effects from default values
                            that may not be correct for the device being used.
        ICALL_RAM0_START:   RAM start of BLE stack.
        ICALL_STACK0_START: Flash start of BLE stack.
        PAGE_AlIGN:         Align BLE stack boundary to a page boundary.
                            Aligns to Flash word boundary by default.
                            (Note: for OAD, page alignment must be used)
        OAD_IMG_(A,B,E)     Used to designate OAD images:
                              - A: On-chip OAD persistent application
                              - B: On-chip OAD user application
                              - E: Off-chip OAD application

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2013-2017, Texas Instruments Incorporated
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
 * CCS Linker configuration
 */

/* Retain interrupt vector table variable                                    */
--retain=g_pfnVectors
/* Override default entry point.                                             */
--entry_point ResetISR
/* Suppress warnings and errors:                                             */
/* - 10063: Warning about entry point not being _c_int00                     */
/* - 16011, 16012: 8-byte alignment errors. Observed when linking in object  */
/*   files compiled using Keil (ARM compiler)                                */
--diag_suppress=10063,16011,16012

/* The following command line options are set as part of the CCS project.    */
/* If you are building using the command line, or for some reason want to    */
/* define them here, you can uncomment and modify these lines as needed.     */
/* If you are using CCS for building, it is probably better to make any such */
/* modifications in your CCS project and leave this file alone.              */
/*                                                                           */
/* --heap_size=0                                                             */
/* --stack_size=256                                                          */
/* --library=rtsv7M3_T_le_eabi.lib                                           */

/*******************************************************************************
 * Memory Sizes
 */

#define FLASH_BASE                0x00000000
#define GPRAM_BASE                0x11000000
#define RAM_BASE                  0x20000000
#define ROM_BASE                  0x10000000

#ifdef CC26X0ROM
  #define FLASH_SIZE              0x00020000
  #define GPRAM_SIZE              0x00002000
  #define RAM_SIZE                0x00005000
  #define ROM_SIZE                0x0001C000
#endif /* CC26X0ROM */

/*******************************************************************************
 * Memory Definitions
 ******************************************************************************/

/*******************************************************************************
 * RAM
 */

#ifdef CC26X0ROM
  #if CC26X0ROM == 2
    #define RESERVED_RAM_SIZE     0x00000C00
  #elif CC26X0ROM == 1
    #define RESERVED_RAM_SIZE     0x00000718
  #endif /* CC26X0ROM */
#endif /* CC26X0ROM */

#define RAM_START                 RAM_BASE

#ifdef ICALL_RAM0_START
  #define RAM_END                 (ICALL_RAM0_START - 1)
#else
  #define RAM_END                 (RAM_START + RAM_SIZE - RESERVED_RAM_SIZE - 1)
#endif /* ICALL_RAM0_START */

/*******************************************************************************
 * Flash
 */

#define WORD_SIZE                 4

#ifdef CC26X0ROM
  #define PAGE_SIZE               0x1000
#endif /* CC26X0ROM */

#ifdef PAGE_ALIGN
  #define FLASH_MEM_ALIGN         PAGE_SIZE
#else
  #define FLASH_MEM_ALIGN         WORD_SIZE
#endif /* PAGE_ALIGN */

#ifdef CC26X0ROM
  #define PAGE_MASK               0xFFFFF000
#endif /* CC26X0ROM */

/* The last Flash page is reserved for the BIM. */
#define NUM_RESERVED_FLASH_PAGES  1
#define RESERVED_FLASH_SIZE       (NUM_RESERVED_FLASH_PAGES * PAGE_SIZE)


#ifdef ICALL_STACK0_START
  #ifdef PAGE_ALIGN
    #define ADJ_ICALL_STACK0_START (ICALL_STACK0_START * PAGE_MASK)
  #else
    #define ADJ_ICALL_STACK0_START ICALL_STACK0_START
  #endif /* PAGE_ALIGN */
#endif

#ifdef OAD_IMG_A
  #define FLASH_START             0
  #define ENTRY_SIZE              0x40
  #define ENTRY_END               FLASH_START + FLASH_SIZE - RESERVED_FLASH_SIZE - 1
  #define ENTRY_START             ENTRY_END - ENTRY_SIZE + 1

  #define FLASH_END               ENTRY_START - 1
#else
  #define  OAD_HDR_START          0
  #define  OAD_HDR_SIZE           0x50
  #define  OAD_HDR_END            OAD_HDR_START + OAD_HDR_SIZE - 1

  #define ENTRY_START             OAD_HDR_END + 1
  #define ENTRY_SIZE              0x40
  #define ENTRY_END               ENTRY_START + ENTRY_SIZE - 1

  #define FLASH_START             ENTRY_END + 1
  #define FLASH_END               ICALL_STACK0_START - 1
#endif


/*******************************************************************************
 * Stack
 */

/* Create global constant that points to top of stack */
/* CCS: Change stack size under Project Properties    */
__STACK_TOP = __stack + __STACK_SIZE;

/*******************************************************************************
 * GPRAM
 */

#ifdef CACHE_AS_RAM
  #define GPRAM_START             GPRAM_BASE
  #define GPRAM_END               (GPRAM_START + GPRAM_SIZE - 1)
#endif /* CACHE_AS_RAM */

/*******************************************************************************
 * Main arguments
 */

/* Allow main() to take args */
/* --args 0x8 */

/*******************************************************************************
 * ROV
 * These symbols are used by ROV2 to extend the valid memory regions on device.
 * Without these defines, ROV will encounter a Java exception when using an
 * autosized heap. This is a posted workaround for a known limitation of
 * RTSC/rta. See: https://bugs.eclipse.org/bugs/show_bug.cgi?id=487894
 *
 * Note: these do not affect placement in RAM or FLASH, they are only used
 * by ROV2, see the BLE Stack User's Guide for more info on a workaround
 * for ROV Classic
 *
 */
__UNUSED_SRAM_start__ = 0x20000000;
__UNUSED_SRAM_end__ = 0x20005000;

__UNUSED_FLASH_start__ = 0;
__UNUSED_FLASH_end__ = 0x1FFFF;

/*******************************************************************************
 * System Memory Map
 ******************************************************************************/
MEMORY
{
  /* EDITOR'S NOTE:
   * the FLASH and SRAM lengths can be changed by defining
   * ICALL_STACK0_START or ICALL_RAM0_START in
   * Properties->ARM Linker->Advanced Options->Command File Preprocessing.
   */

  FLASH (RX) : origin = FLASH_START, length = ((FLASH_END - FLASH_START) + 1)

  ENTRY (RX) : origin = ENTRY_START, length = ENTRY_SIZE

#ifndef OAD_IMG_A
  FLASH_IMG_HDR (RX) : origin = OAD_HDR_START, length = OAD_HDR_SIZE
#endif

  #ifdef ICALL_RAM0_START
      SRAM (RWX) : origin = RAM_START, length = ICALL_RAM0_START - RAM_START
  #else
      SRAM (RWX) : origin = RAM_START, length = RAM_END - RAM_START + 1
  #endif
}

/*******************************************************************************
 * Section Allocation in Memory
 ******************************************************************************/
SECTIONS
{

#ifdef OAD_IMG_A
  GROUP >  FLASH(HIGH)
  {
    .image_header align PAGE_SIZE
    .text
    .const
    .constdata
    .rodata
    .emb_text
    .init_array
    .cinit
    .pinit
  }LOAD_END(flashEndAddr)

  GROUP > ENTRY
  {
    .resetVecs
    .intvecs
    EntrySection  LOAD_START(prgEntryAddr)
  }

#else   // OAD_IMG_B || OAD_IMG_E
  GROUP > FLASH_IMG_HDR
  {
    .image_header align PAGE_SIZE
  }

  GROUP > ENTRY
  {
    .resetVecs
    .intvecs
    EntrySection  LOAD_START(prgEntryAddr)
  }

  GROUP >>  FLASH
  {
    .text
    .const
    .constdata
    .rodata
    .emb_text
    .pinit
  }

  .cinit        : > FLASH LOAD_END(flashEndAddr)

#endif // OAD_IMG_A

  GROUP > SRAM
  {
    .data LOAD_START(ramStartHere)
    #ifndef CACHE_AS_RAM
    .bss
    #endif /* CACHE_AS_RAM */
    .vtable
    .vtable_ram
    vtable_ram
    .sysmem
    .nonretenvar
  } LOAD_END(heapStart)


  .stack            :   >  SRAM (HIGH) LOAD_START(heapEnd)

    #ifdef CACHE_AS_RAM
    ll_bss > SRAM
    {
      --library=cc2640_ll_*.a<ll.o> (.bss)
    }LOAD_END(heapStart)

    .bss :
    {
      *(.bss)
    } > GPRAM
    #endif /* CACHE_AS_RAM */
}

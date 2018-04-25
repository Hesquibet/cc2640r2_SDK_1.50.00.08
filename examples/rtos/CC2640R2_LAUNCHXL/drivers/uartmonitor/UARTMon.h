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

/*
 *  ======== UARTMon.h ========
 *
 *  UART Monitor
 *
 *  The UARTMon module enables host communication with a target device using
 *  the target's UART. The target device can respond to requests to read and
 *  write memory at specified addresses. This module is built on top of the
 *  UART Driver. To enter low power states using the Power Driver, this module
 *  must be disabled. CCS includes features such as the Debug View and GUI
 *  Composer that allows one to leverage the UART Monitor's capabilities.
 */

#ifndef UARTMon_H_
#define UARTMon_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UARTMon_CMDSIZE         5
#define UARTMon_ERRORSTATUS     0xFF
#define UARTMon_READCMD         0xC0
#define UARTMon_WRITECMD        0x80

/*
 *  UART baudrate.
 *
 *  Baudrate for the UART peripheral used by the monitor.
 *  Default is 9600.
 */
#define UARTMon_BAUDRATE        9600

/*!
 *  Board UART index.
 *
 *  Consult Board.h to find the index of the UART
 *  peripherals available for your board.
 *  By default Board_UART0 is used (0).
 */
#define UARTMon_INDEX           0

/*
 *  Monitor stack size.
 *
 *  This is the stack size of the monitor task.
 *  Default value is 768.
 */
#define UARTMon_STACKSIZE       768

/*
 *  ======== UARTMon_init ========
 *  Initialize the UART and create a thread to handle the UART communication.
 */
void UARTMon_init();

#ifdef __cplusplus
}
#endif

#endif /* UARTMon_H_ */

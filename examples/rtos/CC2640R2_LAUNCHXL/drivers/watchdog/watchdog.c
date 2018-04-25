/*
 * Copyright (c) 2016, Texas Instruments Incorporated
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
 *  ======== watchdog.c ========
 */
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/Watchdog.h>

/* Example/Board Header files */
#include "Board.h"

/* Global memory storage for a PIN_Config table */
static PIN_State ledPinState;
static PIN_State buttonPinState;

/* Pin driver handles */
static PIN_Handle ledPinHandle;
static PIN_Handle buttonPinHandle;

/*
 * Application LED pin configuration table:
 *   - Board_PIN_LED0 is initially off.
 */
PIN_Config ledPinTable[] = {
    Board_PIN_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

/*
 * Application button pin configuration table:
 */
PIN_Config buttonPinTable[] = {
    Board_PIN_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_POSEDGE,
    PIN_TERMINATE
};

bool serviceFlag = true;
bool watchdogExpired = false;
Watchdog_Handle watchdogHandle;

/*
 *  ======== watchdogCallback ========
 *  Watchdog interrupt callback function.
 */
void watchdogCallback(uintptr_t unused)
{
    /* Clear watchdog interrupt flag */
    Watchdog_clear(watchdogHandle);

    watchdogExpired = true;

    /* Insert timeout handling code here. */
}

/*
 *  ======== pinButtonFxn ========
 *  Callback function for the GPIO interrupt on Board_PIN_BUTTON0.
 */
void pinButtonFxn(PIN_Handle handle, PIN_Id pinId)
{
    /* Clear serviceFlag to stop continuously servicing the watchdog */
    serviceFlag = false;
}

/*
 *  ======== main ========
 */
void *mainThread(void *arg0)
{
    Watchdog_Params params;

    /* Call board init functions */
    Watchdog_init();

    /* Open LED pin */
    ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    if(!ledPinHandle) {
        /* Error initializing board LED pin */
        while (1);
    }

    /* Turn OFF user LED */
    PIN_setOutputValue(ledPinHandle, Board_PIN_LED0, 0);

    /* Setup callback for button pin */
    buttonPinHandle = PIN_open(&buttonPinState, buttonPinTable);
    if(!buttonPinHandle) {
        /* Error initializing button pins */
        while (1);
    }
    if (PIN_registerIntCb(buttonPinHandle, &pinButtonFxn) != 0) {
        /* Error registering button callback function */
        while (1);
    }
    PIN_setInterrupt(buttonPinHandle, Board_PIN_BUTTON0|PIN_IRQ_POSEDGE);

    /* Create and enable a Watchdog with resets disabled */
    Watchdog_Params_init(&params);
    params.callbackFxn = (Watchdog_Callback)watchdogCallback;
    params.resetMode = Watchdog_RESET_OFF;
    watchdogHandle = Watchdog_open(Board_WATCHDOG0, &params);
    if (watchdogHandle == NULL) {
        /* Error opening Watchdog */
        while (1);
    }

    /* Enter continous loop */
    while (true) {

        /* Service watchdog if serviceFlag is true */
        if (serviceFlag) {
            Watchdog_clear(watchdogHandle);
        }

        /* If watchdog expired since last started, turn ON LED */
        if (watchdogExpired) {
            PIN_setOutputValue(ledPinHandle, Board_PIN_LED0, 1);
            sleep(5);
            Watchdog_clear(watchdogHandle);
            serviceFlag = true;
            watchdogExpired = false;
            PIN_setOutputValue(ledPinHandle, Board_PIN_LED0, 0);
        }
    }
}

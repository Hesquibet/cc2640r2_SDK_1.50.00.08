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
 *  ======== display.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/display/Display.h>
#include <ti/display/DisplayExt.h>
#include <ti/display/DisplayUart.h>
#include <ti/display/AnsiColor.h>

/* Board Header files */
#include "Board.h"

/* Example GrLib image */
#include "splash_image.h"

#define TASKSTACKSIZE   768

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/* Pin driver handle */
static PIN_Handle ledPinHandle;
static PIN_State ledPinState;

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
PIN_Config ledPinTable[] = {
    Board_PIN_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

/*
 *  ======== taskFxn ========
 *  Toggle the Board_PIN_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
Void taskFxn(UArg arg0, UArg arg1)
{
    unsigned int ledPinValue;

    /* Initialize display and try to open both UART and LCD types of display. */
    Display_Params params;
    Display_Params_init(&params);
    params.lineClearMode = DISPLAY_CLEAR_BOTH;

    /* Open both an available LCD display and an UART display.
     * Whether the open call is successful depends on what is present in the
     * Display_config[] array of the board file.
     *
     * Note that for SensorTag evaluation boards combined with the SHARP96x96
     * Watch DevPack, there is a pin conflict with UART such that one must be
     * excluded, and UART is preferred by default. To display on the Watch
     * DevPack, add the precompiler define BOARD_DISPLAY_EXCLUDE_UART.
     */
    Display_Handle hLcd = Display_open(Display_Type_LCD, &params);
    Display_Handle hSerial = Display_open(Display_Type_UART, &params);

    /* Check if the selected Display type was found and successfully opened */
    if (hSerial) {
        Display_printf(hSerial, 0, 0, "Hello Serial!");
    }
    else
    {
        /* Print unavail message on LCD. Note it's not necessary to
         * check for NULL handles with Display APIs, so just assume hLcd works.
         */
        Display_printf(hLcd, 4, 0, "Serial display");
        Display_printf(hLcd, 5, 0, "not present");
        Task_sleep(1000 * (1000/Clock_tickPeriod));
    }

    /* Check if the selected Display type was found and successfully opened */
    if (hLcd) {
        Display_printf(hLcd, 5, 3, "Hello LCD!");

        /* Wait a while so text can be viewed. */
        Task_sleep(1000 * (1000/Clock_tickPeriod));

        /*
         * Use the GrLib extension to get the GraphicsLib context object of the
         * LCD, if it is supported by the display type.
         */
        Graphics_Context *context = DisplayExt_getGraphicsContext(hLcd);

        /* It's possible that no compatible display is available. */
        if (context) {
            /* Draw splash */
            Graphics_drawImage(context, &splashImage, 0, 0);
            Graphics_flushBuffer(context);
        }
        else {
            /* Not all displays have a GraphicsLib back-end */
            Display_printf(hLcd, 0, 0, "Display driver");
            Display_printf(hLcd, 1, 0, "is not");
            Display_printf(hLcd, 2, 0, "GrLib capable!");
        }

        /* Wait for a bit, then clear */
        Task_sleep(5000 * (1000/Clock_tickPeriod));
        Display_clear(hLcd);
    }
    else
    {
        Display_printf(hSerial, 1, 0, "LCD display not present");
        Task_sleep(1000 * (1000/Clock_tickPeriod));
    }

    char *serialLedOn = "On";
    char *serialLedOff = "Off";

    /* If serial display can handle ANSI colors, use colored strings instead.
     *
     * You configure DisplayUart to use the ANSI variant by choosing what
     * function pointer list it should use. Ex:
     *
     * const Display_Config Display_config[] = {
     *   {
     *      .fxnTablePtr = &DisplayUartAnsi_fxnTable, // Alt: DisplayUartMin_
     *      ...
     */
    if (Display_getType(hSerial) & Display_Type_ANSI)
    {
        serialLedOn = ANSI_COLOR(FG_GREEN, ATTR_BOLD) "On" ANSI_COLOR(ATTR_RESET);
        serialLedOff = ANSI_COLOR(FG_RED, ATTR_UNDERLINE) "Off" ANSI_COLOR(ATTR_RESET);
    }

    /* Loop forever, alternating LED state and Display output. */
    while (1) {
        ledPinValue = PIN_getOutputValue(Board_PIN_LED0);

        /* Print to LCD and clear alternate lines if the LED is on or not. */
        Display_clearLine(hLcd, ledPinValue ? 0:1);
        Display_printf(hLcd, ledPinValue ? 1:0, 0, "LED: %s", (!ledPinValue) ? "On!":"Off!");

        /* Print to UART */
        Display_clearLine(hSerial, ledPinValue ? 0:1);
        Display_printf(hSerial, ledPinValue ? 1:0, 0, "LED: %s", (!ledPinValue)?serialLedOn:serialLedOff);

        /* If ANSI is supported, print a "log" in the scrolling region */
        if (Display_getType(hSerial) & Display_Type_ANSI)
        {
            float currTime = (float)(Clock_getTicks() * Clock_tickPeriod) / 1e6;
            char *currLedState = (!ledPinValue)?serialLedOn:serialLedOff;
            Display_printf(hSerial, DisplayUart_SCROLLING, 0, "[ %f ] LED: %s", currTime, currLedState);
        }

        /* Toggle LED */
        PIN_setOutputValue(ledPinHandle, Board_PIN_LED0,
                           !PIN_getOutputValue(Board_PIN_LED0));

        Task_sleep((UInt)arg0);
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    /* Call driver init functions */
    Board_initGeneral();
    Display_init();

    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000000 / Clock_tickPeriod;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)taskFxn, &taskParams, NULL);


    /* Open LED pins */
    ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    if(!ledPinHandle) {
        System_abort("Error initializing board LED pins\n");
    }

    PIN_setOutputValue(ledPinHandle, Board_PIN_LED0, 1);

    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");

    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}

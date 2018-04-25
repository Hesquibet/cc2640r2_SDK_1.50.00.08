//*****************************************************************************
//! @file       pinShutdown.c
//! @brief      Pin shutdown example.
//!
//  Copyright (C) 2015-2017 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/

//******************************************************************************
//! The example will cycle the device between active mode and shutdown mode.
//! The cycling is controlled by pin interrupts, i.e. buttons.
//!
//! The mapping is:
//!             Board_PIN_BUTTON0 will wake up the chip to active mode.
//!             Board_PIN_BUTTON1 will put the device in shutdown mode.
//!
//! The example is using a simple debounce logic, so it does not support
//! multiple buttons pushed at the same time.
//!
//! The debounce work like this: When the pin interrupt is triggered, a button
//! clock is configured to trigger 50 ms ahead in time. After 50 ms, when the
//! clock interrupt is triggered, the LED connected to the active/pushed button
//! will be inverted if the button is still pushed.
//*****************************************************************************/

//******************************************************************************
// Includes
//*****************************************************************************/
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Swi.h>
#include "Board.h"
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_prcm.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)


//******************************************************************************
// Globals
//*****************************************************************************/
/* Task and tast stack */
Task_Struct myTask;
Char myTaskStack[512];

/* Semaphore used to gate for shutdown */
Semaphore_Struct shutdownSem;

/* Clock used for debounce logic */
Clock_Struct buttonClock;
Clock_Handle hButtonClock;

/* Pin driver handles */
PIN_Handle hPins;
PIN_Handle hButtons;

/* LED pin state */
PIN_State LedPinState;

/* Button pin state */
PIN_State buttonState;

/* Flag to store whether we woke up from shutdown or not */
bool isWakingFromShutdown;

/* PIN_Id for active button (in debounce period) */
PIN_Id activeButtonPinId;

/* Led pin table used when waking from reset*/
PIN_Config LedPinTable[] = {
    Board_PIN_LED1    | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW  | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* LED initially off */
    Board_PIN_LED0    | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* LED initially on */
    PIN_TERMINATE                                                    /* Terminate list */
};

/* Led pin table used when waking from shutdown */
PIN_Config LedPinTableSd[] = {
    Board_PIN_LED1    | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* LED initially on */
    Board_PIN_LED0    | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW  | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* LED initially off */
    PIN_TERMINATE                                                    /* Terminate list */
};

/* Wake-up Button pin table */
PIN_Config ButtonTableWakeUp[] = {
    Board_PIN_BUTTON0     | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
    PIN_TERMINATE                                 /* Terminate list */
};

/* Shutdown Button pin table */
PIN_Config ButtonTableShutdown[] = {
    Board_PIN_BUTTON1   | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
    PIN_TERMINATE                                 /* Terminate list */
};


/*!*****************************************************************************
 *  @brief      Button clock callback
 *
 *  Called when the debounce periode is over. Stopping the clock, toggling
 *  the device mode based on activeButtonPinId:
 *
 *              Board_PIN_BUTTON1 will put the device in shutdown mode.
 *
 *  Reenabling the interrupts and resetting the activeButtonPinId.
 *
 *  @param      arg  argument (PIN_Handle) connected to the callback
 *
 ******************************************************************************/
static void buttonClockCb(UArg arg) {
    PIN_Handle buttonHandle = (PIN_State *) arg;

    /* Stop the button clock */
    Clock_stop(hButtonClock);

    /* Check that there is active button for debounce logic*/
    if (activeButtonPinId != PIN_TERMINATE) {
        /* Debounce logic, only toggle if the button is still pushed (low) */
        if (!PIN_getInputValue(activeButtonPinId)) {
            /* Toggle LED based on the button pressed */
            switch (activeButtonPinId) {
            case Board_PIN_BUTTON1:
                Semaphore_post(Semaphore_handle(&shutdownSem));
                break;
            default:
                /* Do nothing */
                break;
            }
        }
    }

    /* Re-enable interrupts to detect button release. */
    PIN_setConfig(buttonHandle, PIN_BM_IRQ, activeButtonPinId | PIN_IRQ_NEGEDGE);

    /* Set activeButtonPinId to none... */
    activeButtonPinId = PIN_TERMINATE;
}

/*!*****************************************************************************
 *  @brief      Button callback
 *
 *  Initiates the debounce period by disabling interrupts, setting a timeout
 *  for the button clock callback and starting the button clock.
 *  Sets the activeButtonPinId.
 *
 *  @param      handle PIN_Handle connected to the callback
 *
 *  @param      pinId  PIN_Id of the DIO triggering the callback
 *
 *  @return     none
 ******************************************************************************/
static void buttonCb(PIN_Handle handle, PIN_Id pinId) {
    /* Set current pinId to active */
    activeButtonPinId = pinId;

    /* Disable interrupts during debounce */
    PIN_setConfig(handle, PIN_BM_IRQ, activeButtonPinId | PIN_IRQ_DIS);

    /* Set timeout 50 ms from now and re-start clock */
    Clock_setTimeout(hButtonClock, (50 * (1000 / Clock_tickPeriod)));
    Clock_start(hButtonClock);
}

/*!*****************************************************************************
 *  @brief      Task which runs when the device is active
 *
 *  @param      UArg a0 : User argument that can be passed to the task
 *
 *  @param      UArg a1 : User argument that can be passed to the task
 *
 *  @return     none - Should never return
 ******************************************************************************/
static void taskFxn(UArg a0, UArg a1)
{
    /* If we are waking up from shutdown, we do something extra. */
    if (isWakingFromShutdown) {
        /* In this example we toggle LED1 */
        uint32_t sleepUs = 500000;
        Task_sleep(sleepUs / Clock_tickPeriod);
        PIN_setOutputValue(hPins, Board_PIN_LED1, 0);
        Task_sleep(sleepUs / Clock_tickPeriod);
        PIN_setOutputValue(hPins, Board_PIN_LED1, 1);
        Task_sleep(sleepUs / Clock_tickPeriod);
        PIN_setOutputValue(hPins, Board_PIN_LED1, 0);
        Task_sleep(sleepUs / Clock_tickPeriod);
    }
    else {
        /* Shut down external flash on LaunchPads. It is powered on by default
         * but can be shut down through SPI
         */
        #ifdef Board_shutDownExtFlash
            Board_shutDownExtFlash();
        #endif
    }

    /* Turn on LED0 to indicate active */
    PIN_setOutputValue(hPins, Board_PIN_LED0, 1);

    /* Pend on semaphore before going to shutdown */
    Semaphore_pend(Semaphore_handle(&shutdownSem), BIOS_WAIT_FOREVER);

    /* Turn off LED0 */
    PIN_setOutputValue(hPins, Board_PIN_LED0, 0);

    /* Configure DIO for wake up from shutdown */
    PINCC26XX_setWakeup(ButtonTableWakeUp);

    /* Go to shutdown */
    Power_shutdown(0, 0);

    /* Should never get here, since shutdown will reset. */
    while(1);
}

/*!*****************************************************************************
 *  @brief      Application main entry point
 *
 *  @param      none
 *
 *  @return     int - Should never return
 ******************************************************************************/
int main(void)
{
     /* Locals */
    Task_Params taskParams;
    Semaphore_Params semParams;

    Power_init();

    /* Get the reason for reset */
    uint32_t rSrc = SysCtrlResetSourceGet();

    /* Do pin init before starting BIOS */
    /* If coming from shutdown, use special gpio table.*/
    if(rSrc == RSTSRC_WAKEUP_FROM_SHUTDOWN) {
        /* The shutdown table has LED1 on to ensure no glitch on the
         * output.
         */
        PIN_init(BoardGpioInitTable);
        isWakingFromShutdown = true;
        /* Open LED pins with shutdown table (LED1 on, LED0 off).
         * A separate PIN_config table is used to keep LED1 on.
         */
        hPins = PIN_open(&LedPinState, LedPinTableSd);
    } else {
        /* When not waking from shutdown, use default init table. */
        PIN_init(BoardGpioInitTable);
        isWakingFromShutdown = false;
        /* Open LED pins (LED1 off, LED0 on)*/
        hPins = PIN_open(&LedPinState, LedPinTable);
    }

    /* Setup button pins with ISR */
    hButtons = PIN_open(&buttonState, ButtonTableShutdown);
    PIN_registerIntCb(hButtons, buttonCb);

    /* Construct clock for debounce */
    Clock_Params clockParams;
    Clock_Params_init(&clockParams);
    clockParams.arg = (UArg)hButtons;
    Clock_construct(&buttonClock, buttonClockCb, 0, &clockParams);
    hButtonClock = Clock_handle(&buttonClock);

    /* Configure task. */
    Task_Params_init(&taskParams);
    taskParams.stack = myTaskStack;
    taskParams.stackSize = sizeof(myTaskStack);
    Task_construct(&myTask, taskFxn, &taskParams, NULL);

    /* Configure shutdown semaphore. */
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&shutdownSem, 0, &semParams);

    /* Start kernel. */
    BIOS_start();

    /* Should never get here, keep compiler happy */
    return 0;
}

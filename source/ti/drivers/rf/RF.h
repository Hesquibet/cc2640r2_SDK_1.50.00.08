/*
 * Copyright (c) 2016-2017, Texas Instruments Incorporated
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
/** @file       RF.h

@brief      RF core driver for the CC13xx/CC26xx device family.

To use the RF driver, ensure that the correct driver library for your device
is linked in and include this header file as follows:

@code #include
<ti/drivers/rf/RF.h>
@endcode

The RF driver provides access to the radio core on the CC13xx/CC26xx device
family. It offers a high-level interface for command execution and to the
radio timer (RAT). The RF driver ensures the lowest possible power consumption
by providing automatic power management that is fully transparent for the
application.

This document describes the features and usage of the RF driver API. For a
detailed explanation of the RF core, please refer to the Technical Reference
Manual or the Proprietary RF user's guide. Key features are:

- @ref rf_command_execution "Synchronous execution of direct and immediate radio commands",
- @ref rf_command_execution "Synchronous and asynchronous execution of radio operation commands",
- Various @ref rf_event_callbacks "event hooks" to interact with RF commands and the RF driver,
- Automatic @ref rf_power_management "power management",
- @ref rf_scheduling "Preemptive scheduler for RF operations" of different RF driver instances,
- Convenient @ref rf_convenience_features "access to the radio timer".


@anchor rf_setup_and_configuration
Setup and configuration
=======================

The RF driver can be configured at 4 different places:

1. In the build configuration by chosing either the single-client or
   multi-client driver version.

2. At compile-time by setting hardware and software interrupt priorities
   in the board support file.

3. During run-time initialization by setting #RF_Params when calling
   #RF_open().

4. At run-time via #RF_control().


Build configuration
-------------------

The RF driver comes in two versions: single-client and multi-client. The
single-client version allows only one driver instance to access the RF core at
a time. The multi-client driver version allows concurrent access to the RF
core with different RF settings. The multi-client driver has a slightly larger
footprint and is not needed for many proprietary applications. The driver
version can be selected in the build configuration by linking either against a
RFCC26XX_singleMode or RFCC26XX_multiMode prebuilt library. When using the
single-client driver, `RF_SINGLEMODE` has to be defined globally in the build
configuration. The multi-client driver is the default configuration in the
SimpleLink SDKs.


Board configuration
-------------------

The RF driver handles RF core hardware interrupts and uses software interrupts
for its internal state machine. For managing the interrupt priorities, it
expects the existance of a global #RFCC26XX_HWAttrs object. This is
usually defined in the board support file, for example `CC1310_LAUNCHXL.c`,
but when developing on custom boards, it might be kept anywhere in the
application. By default, the priorities are set to the lowest possible value:

@code
const RFCC26XX_HWAttrs RFCC26XX_hwAttrs = {
    .hwiCpe0Priority = INT_PRI_LEVEL7,         // lowest
    .hwiHwPriority   = INT_PRI_LEVEL1,         // highest
    .swiCpe0Priority =  0,                     // lowest
    .swiHwPriority   =  Swi.numPriorities - 1, // highest
};
@endcode


Initialization
--------------

When initiating an RF driver instance, the function #RF_open() accepts a
pointer to a #RF_Params object which might set several driver parameters. In
addition, it expects an #RF_Mode object and a setup command which is usually
generated by SmartRF Studio:

@code
RF_Params rfParams;
RF_Params_init(&rfParams);
rfParams.nInactivityTimeout = 2000;

RF_Handle rfHandle = RF_open(&rfObject, &RF_prop,
        (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
@endcode

The function #RF_open() returns a driver handle that is used for accessing the
correct driver instance. Please note that the first RF operation command
before an RX or TX operation command must be a `CMD_FS` to set the synthesizer
frequency. The RF driver caches both, the pointer to the setup command and the
physical `CMD_FS` for automatic power management.


Run-time configuration
----------------------

While a driver instance is opened, it can be re-configured with the function
#RF_control(). Various configuration parameters @ref RF_CTRL are available.
Example:

@code
uint32_t timeoutUs = 2000;
RF_control(rfHandle, RF_CTRL_SET_INACTIVITY_TIMEOUT, &timeoutUs);
@endcode


@anchor rf_command_execution
Command execution
=================

The RF core supports 3 different kinds of commands:

1. Direct commands
2. Immediate commands
3. Radio operation commands

Direct and immediate commands are dispatched via #RF_runDirectCmd() and
#RF_runImmediateCmd() respectively. These functions block until the command
has completed and return a status code of the type #RF_Stat when done.

@code
#include <ti/devices/${DEVICE_FAMILY}/driverlib/rf_common_cmd.h>

RF_Stat status = RF_runDirectCmd(rfHandle, CMD_ABORT);
assert(status == RF_StatCmdDoneSuccess);
@endcode

Radio operation commands are potentially long-running commands and support
different triggers as well as conditional execution. Only one command can be
executed at a time, but the RF driver provides an internal queue that stores
commands until the RF core is free. Two interfaces are provided for radio
operation commands:

1. Asynchronous: #RF_postCmd() and #RF_pendCmd()
2. Synchronous: #RF_runCmd()

The asynchronous function #RF_postCmd() posts a radio operation into the
driver's internal command queue and returns a command handle of the type
#RF_CmdHandle which is an index in the command queue. The command is
dispatched as soon as the RF core has completed any previous radio operation
command.

@code
#include <ti/devices/${DEVICE_FAMILY}/driverlib/rf_common_cmd.h>

RF_Callback callback = NULL;
RF_EventMask subscribedEvents = 0;
RF_CmdHandle rxCommandHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdRx,
        RF_PriorityNormal, callback, subscribedEvents);

assert(rxCommandHandle != RF_ALLOC_ERROR); // The command queue is full.
@endcode

Command execution happens in background. The calling task may proceed with
other work or execute direct and immediate commands to interact with the
posted radio operation. But beware that the posted command might not have
started, yet. By calling the function #RF_pendCmd() and subscribing events of
the type #RF_EventMask, it is possible to re-synchronize to a posted command:

@code
// RF_EventRxEntryDone must have been subscribed in RF_postCmd().
RF_EventMask events = RF_pendCmd(rfHandle, rxCommandHandle,
        RF_EventRxEntryDone);

// Program proceeds after RF_EventRxEntryDone or after a termination event.
@endcode

The function #RF_runCmd() is a combination of both, #RF_postCmd() and
#RF_pendCmd() and allows synchronous execution.

A pending or already running command might be aborted at any time by calling
the function #RF_cancelCmd() or #RF_flushCmd(). These functions take command
handles as parameters, but can also just abort anything in the RF driver's
queue:

@code
uint8_t abortGraceful = 1;

// Abort a single command
RF_cancelCmd(rfHandle, rxCommandHandle, abortGraceful);

// Abort anything
RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, abortGraceful);
@endcode

When aborting a command, the return value of #RF_runCmd() or #RF_pendCmd()
will contain the termination reason in form of event flags. If the command is
in the RF driver queue, but has not yet start, the #RF_EventCmdCancelled event is
raised.


@anchor rf_event_callbacks
Event callbacks
===============

The RF core generates multiple interrupts during command execution. The RF
driver maps these interrupts 1:1 to callback events of the type #RF_EventMask.
Hence, it is unnecessary to implement own interrupt handlers. Callback events
are divided into 3 groups:

- Command-specific events, documented for each radio operation command. An example
  is the #RF_EventRxEntryDone for the `CMD_PROP_RX`.

- Generic events, defined for all radio operations and originating on the RF core.
  These are for instance #RF_EventCmdDone and #RF_EventLastCmdDone. Both events
  indicate the termination of one or more RF operations.

- Generic events, defined for all radio operations and originating in the RF driver,
  for instance #RF_EventCmdCancelled.

See also @ref RF_Core_Events, @ref RF_Driver_Events.

How callback events are subscribed was shown in the previous section. The
following snippet shows a typical event handler callback for a proprietary RX
operation:

@code
void rxCallback(RF_Handle handle, RF_CmdHandle command, RF_EventMask events)
{
    if (events & RF_EventRxEntryDone)
    {
        Semaphore_post(rxPacketSemaphore);
    }
    if (events & RF_EventLastCmdDone)
    {
        // ...
    }
}
@endcode

In addition, the RF driver can generate error and power-up events that do not
relate directly to the execution of a radio command. Such events can be
subscribed by specifying the callback function pointers #RF_Params::pErrCb and
#RF_Params::pPowerCb.

All callback functions run in software interrupt (SWI) context. Therefore,
only a minimum amount of code should be executed. When using absolute timed
commands with tight timing constraints, then it is recommended to set the RF
driver SWIs to a high priority.
See @ref rf_setup_and_configuration "Setup and configuration" for more details.


@anchor rf_power_management
Power management
================

The RF core is a hardware peripheral and can be switched on and off. The RF
driver handles that automatically and provides the following power
optimization features:

- Lazy power-up and radio setup caching
- Power-down on inactivity
- Deferred dispatching of commands with absolute timing


Lazy power-up and radio setup caching
-------------------------------------

The RF core optimizes the power consumption by enabling the RF core as late as
possible. For instance does #RF_open() not power up the RF core immediately.
Instead, it waits until the first radio operation command is dispatched by
#RF_postCmd() or #RF_runCmd().

The function #RF_open() takes a radio setup command as parameter and expects a
`CMD_FS` command to follow. The pointer to the radio setup command and the
whole `CMD_FS` command are cached internally in the RF driver. They will be
used for every proceeding power-up procedure. Whenever the client re-runs a
setup command or a `CMD_FS` command, the driver updates its internal cache
with the new settings.

By default, the RF driver measures the time that it needs for the power-up
procedure and uses that as an estimate for the next power cycle. On the
CC13x0/CC26x0 devices, power-up takes usually 1.6 ms. Automatic measurement
can be suppressed by specifying a custom power-up time with
#RF_Params::nPowerUpDuration. In addition, the client might set
#RF_Params::nPowerUpDurationMargin to cover any uncertainity when doing
automatic measurements. This is necessary in applications with a high hardware
interrupt load which can delay the RF driver's internal state machine
execution.


Power-down on inactivity
------------------------

Whenever a radio operation completes and there is no other radio operation in
the queue, the RF core might be powered down. There are two options in the RF
driver:

- **Automatic power-down** by setting the parameter
  #RF_Params::nInactivityTimeout. The RF core will then start a timer after
  the last command in the queue has completed. The default timeout is "forever"
  and this feature is disabled.

- **Manual power-down** by calling #RF_yield(). The client should do this
  whenever it knows that no further radio operation will be executed for a
  couple of milliseconds.

During the power-down procedure the RF driver stops the radio timer and saves
a synchronization timestamp for the next power-up. This keeps the radio timer
virtually in sync with the RTC even though it is not running all the time. The
synchronization is done in hardware.


Deferred dispatching of commands with absolute timing
-----------------------------------------------------

When dispatching a radio operation command with an absolute start trigger that
is ahead in the future, the RF driver defers the execution and powers the RF
core down until the command is due. It does that only, when:

1. `cmd.startTrigger.triggerType` is set to `TRIG_ABSTIME`

2. The difference between #RF_getCurrentTime() and `cmd.startTime`
   is at not more than 3/4 of a full RAT cycle. Otherwise the driver assumes
   that `cmd.startTime` is in the past.

3. There is enough time to run a full power cycle before `cmd.startTime` is
   due. That includes:

   - the power-down time (fixed value, 1 ms) if the RF core is already
     powered up,

   - the measured power-up duration or the value specified by
     #RF_Params::nPowerUpDuration,

   - the power-up safety margin #RF_Params::nPowerUpDurationMargin
     (the default is 282 microseconds).

If one of the conditions are not fullfilled, the RF core is kept up and
running and the command is dispatched immediately. This ensures, that the
command will execute on-time and not miss the configured start trigger.


@anchor rf_scheduling
Preemptive scheduling of RF commands in multi-client applications
=================================================================

Schedule BLE and proprietary radio commands.

@code
RF_Object rfObject_ble;
RF_Object rfObject_prop;

RF_Handle rfHandle_ble, rfHandle_prop;
RF_Params rfParams_ble, rfParams_prop;
RF_ScheduleCmdParams schParams_ble, schParams_prop;

RF_Mode rfMode_ble =
{
  .rfMode      = RF_MODE_MULTIPLE,  // rfMode for dual mode
  .cpePatchFxn = &rf_patch_cpe_ble,
  .mcePatchFxn = 0,
  .rfePatchFxn = &rf_patch_rfe_ble,
};

RF_Mode rfMode_prop =
{
  .rfMode      = RF_MODE_MULTIPLE,  // rfMode for dual mode
  .cpePatchFxn = &rf_patch_cpe_genfsk,
  .mcePatchFxn = 0,
  .rfePatchFxn = 0,
};

// Init RF and specify non-default parameters
RF_Params_init(&rfParams_ble);
rfParams_ble.nInactivityTimeout = 200;     // 200us

RF_Params_init(&rfParams_prop);
rfParams_prop.nInactivityTimeout = 200;    // 200us

// Configure RF schedule command parameters
schParams_ble.priority  = RF_PriorityNormal;
schParams_ble.endTime   = 0;
schParams_prop.priority = RF_PriorityNormal;
schParams_prop.endTime  = 0;

// Open BLE and proprietary RF handles
rfHandle_ble  = RF_open(rfObj_ble,  &rfMode_ble,  (RF_RadioSetup*)&RF_cmdRadioSetup,        &rfParams_ble);
rfHandle_prop = RF_open(rfObj_prop, &rfMode_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams_prop);

// Run a proprietary Fs command
RF_runCmd(rfHandle_pro, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, NULL);

// Schedule a proprietary RX command
RF_scheduleCmd(rfHandle_pro, (RF_Op*)&RF_cmdPropRx, &schParams_prop, &prop_callback, RF_EventRxOk);

// Schedule a BLE advertizer command
RF_scheduleCmd(rfHandle_ble, (RF_Op*)&RF_cmdBleAdv, &schParams_ble, &ble_callback,
             (RF_EventLastCmdDone | RF_EventRxEntryDone | RF_EventTxEntryDone));

@endcode

Get dual mode schedule map including timing and priority information for access requests and commands.

@code

RF_ScheduleMap rfSheduleMap;
RF_InfoVal     rfGetInfoVal;

// Get schedule map
rfGetInfoVal.pScheduleMap = &rfScheduleMap;
RF_getInfo(NULL, RF_GET_SCHEDULE_MAP, &rfGetInfoVal);

// RF_scheduleMap includes the following information:
// (RF_NUM_SCHEDULE_ACCESS_ENTRIES (default = 2)) entries of access request information
// (RF_NUM_SCHEDULE_COMMAND_ENTRIES (default = 3)) entries of radio command information
// Each entry has the type of RF_ScheduleMapElement.

@endcode


@anchor rf_convenience_features
Convenience features
====================

The RF driver simplifies often needed tasks and provides additional functions.
For instance, it can read the RSSI while the RF core is in RX mode using the
function :tidrivers_api:`RF_getRssi`:

@code
int8_t rssi = RF_getRssi(rfHandle);
assert (rssi != RF_GET_RSSI_ERROR_VAL); // Could not read the RSSI
@endcode

For defining absolute triggers in radio operation commands, one often needs to
know the current time. This can be achieved with the function
#RF_getCurrentTime(). When the RF core is not up and running and the radio
timer is disabled, this function can still return a previse value:

@code
uint32_t rfTime = RF_getCurrentTime();
@endcode
 */
#ifndef ti_drivers_rf__include
#define ti_drivers_rf__include

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_ble_cmd.h)

/**
 *  @name RF Core Events
 *  @anchor RF_Core_Events
 *
 *  Events originating on the RF core and caused during command execution.
 *  They are aliases for the corresponding interrupt flags.
 *  RF Core Events are command-specific and are explained in the Technical Reference Manual.
 *
 *  @sa RF_postCmd(), RF_pendCmd(), RF_runCmd()
 *  @{
 */
#define   RF_EventCmdDone             (1<<0)   ///< A radio operation command in a chain finished.
#define   RF_EventLastCmdDone         (1<<1)   ///< A stand-alone radio operation command or the last radio operation command in a chain finished.
#define   RF_EventFGCmdDone           (1<<2)   ///< A IEEE-mode radio operation command in a chain finished.
#define   RF_EventLastFGCmdDone       (1<<3)   ///< A stand-alone IEEE-mode radio operation command or the last command in a chain finished.
#define   RF_EventTxDone              (1<<4)   ///< Packet transmitted
#define   RF_EventTXAck               (1<<5)   ///< ACK packet transmitted
#define   RF_EventTxCtrl              (1<<6)   ///< Control packet transmitted
#define   RF_EventTxCtrlAck           (1<<7)   ///< Acknowledgement received on a transmitted control packet
#define   RF_EventTxCtrlAckAck        (1<<8)   ///< Acknowledgement received on a transmitted control packet, and acknowledgement transmitted for that packet
#define   RF_EventTxRetrans           (1<<9)   ///< Packet retransmitted
#define   RF_EventTxEntryDone         (1<<10)  ///< Tx queue data entry state changed to Finished
#define   RF_EventTxBufferChange      (1<<11)  ///< A buffer change is complete
#define   RF_EventRxOk                (1<<16)  ///< Packet received with CRC OK, payload, and not to be ignored
#define   RF_EventRxNOk               (1<<17)  ///< Packet received with CRC error
#define   RF_EventRxIgnored           (1<<18)  ///< Packet received with CRC OK, but to be ignored
#define   RF_EventRxEmpty             (1<<19)  ///< Packet received with CRC OK, not to be ignored, no payload
#define   RF_EventRxCtrl              (1<<20)  ///< Control packet received with CRC OK, not to be ignored
#define   RF_EventRxCtrlAck           (1<<21)  ///< Control packet received with CRC OK, not to be ignored, then ACK sent
#define   RF_EventRxBufFull           (1<<22)  ///< Packet received that did not fit in the Rx queue
#define   RF_EventRxEntryDone         (1<<23)  ///< Rx queue data entry changing state to Finished
#define   RF_EventDataWritten         (1<<24)  ///< Data written to partial read Rx buffer
#define   RF_EventNDataWritten        (1<<25)  ///< Specified number of bytes written to partial read Rx buffer
#define   RF_EventRxAborted           (1<<26)  ///< Packet reception stopped before packet was done
#define   RF_EventRxCollisionDetected (1<<27)  ///< A collision was indicated during packet reception
#define   RF_EventModulesUnlocked     (1<<29)  ///< As part of the boot process, the CM0 has opened access to RF core modules and memories
#define   RF_EventInternalError       (uint32_t)(1<<31)   ///< Internal error observed
#define   RF_EventMdmSoft             0x0000002000000000  ///< Synchronization word detected (MDMSOFT interrupt flag)
/** @}*/

/**
 *  @name RF Driver Events
 *  @anchor RF_Driver_Events
 *
 *  Event flags generated by the RF Driver.
 *  @{
 */
#define   RF_EventCmdCancelled        0x1000000000000000  ///< Command cancelled before it was started.
#define   RF_EventCmdAborted          0x2000000000000000  ///< Aprubt command termination caused by RF_cancelCmd() or RF_flushCmd().
#define   RF_EventCmdStopped          0x4000000000000000  ///< Graceful command termination caused by RF_cancelCmd() or RF_flushCmd().
#define   RF_EventRatCh               0x0800000000000000  ///< A user-programmable RAT channel triggered an event.
#define   RF_EventPowerUp             0x0400000000000000  ///< RF power up event. \deprecated This event is deprecated. Use #RF_ClientEventPowerUpFinished instead.
#define   RF_EventError               0x0200000000000000  ///< Event flag used for error callback functions to indicate an error. See RF_Params::pErrCb.
#define   RF_EventCmdPreempted        0x0100000000000000  ///< Command preempted by another command with higher priority. Applies only to multi-client applications.
#define   RF_EventRadioFree           0x0080000000000000  ///< Radio available to use. \deprecated Event is deprecated. Use #RF_ClientEventRadioFree instead.
/** @}*/

/**
 *  @name Control codes for driver configuration
 *  @anchor RF_CTRL
 *
 *  Control codes are used in RF_control().
 *
 *  @{
 */

/*!
 * @brief Control code used by RF_control to set inactivity timeout
 *
 * Setting this control allows RF to power down the radio upon completion of a radio
 * command after a specified timeout period (in us)
 * With this control code @b arg is a pointer to the timeout variable and returns RF_StatSuccess.
 */
#define RF_CTRL_SET_INACTIVITY_TIMEOUT            0
/*!
 * @brief Control code used by RF_control to update setup command
 *
 * Setting this control notifies RF that the setup command is to be updated, so that RF will take
 * proper actions when executing the next setup command.
 * Note the updated setup command will take effect in the next power up cycle when RF executes the
 * setup command. Prior to updating the setup command, user should make sure all pending commands
 * have completed.
 */
#define RF_CTRL_UPDATE_SETUP_CMD                  1
/*!
 * @brief Control code used by RF_control to set powerup duration margin
 *
 * Setting this control updates the powerup duration margin. Default is RF_DEFAULT_POWER_UP_MARGIN.
 */
#define RF_CTRL_SET_POWERUP_DURATION_MARGIN       2
/*!
 * @brief Control code used by RF_control to set max error tolerence for RAT/RTC
 *
 * Setting this control updates the error tol for how frequently the CMD_RAT_SYNC_STOP is sent.
 * Default is RF_DEFAULT_RAT_RTC_ERR_TOL_IN_US (5 us)
 * Client is recommeneded to change this setting before sending any commands.
 */
#define RF_CTRL_SET_RAT_RTC_ERR_TOL_VAL           3
/*!
 * @brief Control code used by RF_control to set power management
 *
 * Setting this control configures RF driver to enable or disable power management.
 * By default power management is enabled.
 * If disabled, once RF core wakes up, RF driver will not go to standby and will not power down RF core.
 * To configure power management, use this control to pass a parameter value of 0 to disable power management,
 * and pass a parameter value of 1 to re-enable power management.
 * This control is valid for dual-mode code only. Setting this control when using single-mode code has no effect
 * (power management always enabled).
 */
#define RF_CTRL_SET_POWER_MGMT                    4
/*!
 * @brief Control code used by RF_control to set the hardware interrupt priority level of the RF driver.
 *
 * This control code sets the hardware interrupt priority level that is used by the RF driver. Valid
 * values are INT_PRI_LEVEL1 (highest) until INT_PRI_LEVEL7 (lowest). The default interrupt priority is
 * set in the board support file. The default value is -1 which means "lowest possible priority".
 *
 * When using the TI-RTOS kernel, INT_PRI_LEVEL0 is reserved for zero-latency interrupts and must not be used.
 *
 * Execute this control code only while the RF core is powered down and the RF driver command queue is empty.
 * This is usually the case after calling RF_open(). Changing the interrupt priority level while the RF driver
 * is active will result in RF_StatBusyError being returned.
 *
 * Example:
 * @code
 * #include DeviceFamily_constructPath(driverlib/interrupt.h)
 *
 * int32_t hwiPriority = INT_PRI_LEVEL5;
 * RF_control(rfHandle, RF_CTRL_SET_HWI_PRIORITY, &hwiPriority);
 * @endcode
 */
#define RF_CTRL_SET_HWI_PRIORITY                  5
/*!
 * @brief Control code used by RF_control to set the software interrupt priority level of the RF driver.
 *
 * This control code sets the software interrupt priority level that is used by the RF driver. Valid
 * values are integers starting at 0 (lowest) until <tt>Swi_numPriorities - 1</tt> (highest). The default
 * interrupt priority is set in the board support file. The default value is 0 which means means
 * "lowest possible priority".
 *
 * Execute this control code only while the RF core is powered down and the RF driver command queue is empty.
 * This is usually the case after calling RF_open(). Changing the interrupt priority level while the RF driver
 * is active will result in RF_StatBusyError being returned.
 *
 * Example:
 * @code
 * #include <ti/sysbios/knl/Swi.h>
 *
 * // Set highest possible priority
 * uint32_t swiPriority = ~0;
 * RF_control(rfHandle, RF_CTRL_SET_SWI_PRIORITY, &swiPriority);
 * @endcode
 */
#define RF_CTRL_SET_SWI_PRIORITY                  6
/** @}*/

/**
 * @name Other defines
 * @{
 */
#define RF_GET_RSSI_ERROR_VAL                   (-128)   ///< Error return value for RF_getRssi()
#define RF_CMDHANDLE_FLUSH_ALL                  (-1)     ///< RF command handle to flush all RF commands
#define RF_ALLOC_ERROR                          (-2)     ///< RF command or RAT channel allocation error
#define RF_SCHEDULE_CMD_ERROR                   (-3)     ///< RF command schedule error
#define RF_ERROR_RAT_PROG                       (-255)   ///< A rat channel could not be programmed.
#define RF_ERROR_INVALID_RFMODE                 (-256)   ///< Invalid RF_Mode. Used in error callback.
#define RF_ERROR_CMDFS_SYNTH_PROG               (-257)   ///< Synthesizer error with CMD_FS. Used in error callback. If this error occurred in error callback, user needs to resend CMD_Fs to recover. See errata SWRA521.

#define RF_NUM_SCHEDULE_MAP_ENTRIES             5   ///< Number of schedule map entries. This is the sum of access request and scheduled command entries
#define RF_NUM_SCHEDULE_ACCESS_ENTRIES          2   ///< Number of access request entries
#define RF_NUM_SCHEDULE_COMMAND_ENTRIES         (RF_NUM_SCHEDULE_MAP_ENTRIES - RF_NUM_SCHEDULE_ACCESS_ENTRIES) ///< Number of scheduled command entries

#define RF_SCH_CMD_EXECUTION_TIME_UNKNOWN       0   ///< For unknown execution time for RF scheduler
/** @}*/

/** @brief Base type for all radio operation commands.
 *
 *  All radio operation commands share a common part.
 *  That includes the command id, a status field, chaining properties
 *  and a start trigger.
 *  Whenever an RF operation command is used with the RF driver, it needs
 *  to be casted to an RF_Op.
 *
 *  More information about RF operation commands can be found in the Proprietary RF
 *  User's Guide.
 *
 *  @sa RF_runCmd(), RF_postCmd(), RF_pendCmd()
 */
typedef rfc_radioOp_t RF_Op;


/** @brief Specifies a RF core firmware configuration.
 *
 *  %RF_Mode selects a mode of operation and points to firmware patches for the RF core.
 *  There exists one instance per radio PHY configuration, usually generated by
 *  SmartRF Studio.
 *  After assigning %RF_Mode configuration to the RF driver via RF_open(), the
 *  driver caches the containing information and re-uses it on every power-up.
 */
typedef struct {
    uint8_t rfMode;             ///< Specifies which PHY modes should be activated. Must be set to RF_MODE_MULTIPLE for dual-mode operation.
    void (*cpePatchFxn)(void);  ///< Pointer to CPE patch function
    void (*mcePatchFxn)(void);  ///< Pointer to MCE patch function
    void (*rfePatchFxn)(void);  ///< Pointer to RFE patch function
} RF_Mode;


/** @brief Scheduling priority of RF operation commands.
 *
 *  When multiple RF driver instances are used at the same time,
 *  commands from different clients may overlap.
 *  If an RF operation with a higher priority than the currently
 *  running operation is scheduled by RF_scheduleCmd(), then the
 *  running operation is interrupted.
 *
 *  In single-client applications, %RF_PriorityNormal should be used.
 */
typedef enum {
    RF_PriorityHighest = 2, ///< Highest priority. Only use this for urgent commands.
    RF_PriorityHigh    = 1, ///< High priority. Use this for time-critical commands in synchronous protocols.
    RF_PriorityNormal  = 0, ///< Default priority. Use this in single-client applications.
} RF_Priority;

/** @brief Status codes for various RF driver functions.
 *
 *  RF_Stat is reported as return value for RF driver functions which
 *  execute direct and immediate commands.
 *  Such commands are executed by RF_runDirectCmd() and RF_runImmediateCmd() in the
 *  first place, but also by some convenience functions like RF_cancelCmd(),
 *  RF_flushCmd(), RF_getInfo() and others.
 */
typedef enum {
    RF_StatBusyError,          ///< Command not executed because RF driver is busy.
    RF_StatRadioInactiveError, ///< Command not executed because RF core is powered down.
    RF_StatCmdDoneError,       ///< Command finished with an error.
    RF_StatInvalidParamsError, ///< Function was called with an invalid parameter.
    RF_StatCmdEnded,           ///< Cmd is found in the pool but was already ended.
    RF_StatError   = 0x80,     ///< General error specifier.
    RF_StatCmdDoneSuccess,     ///< Command finished with success.
    RF_StatCmdSch,             ///< Command successfully scheduled for execution.
    RF_StatSuccess             ///< Function finished with success.
} RF_Stat;

/** @brief Data type for events during command execution.
 *
 *  Possible event flags are listed in @ref RF_Core_Events and @ref RF_Driver_Events.
 */
typedef uint64_t RF_EventMask;

/** @brief A unified type for radio setup commands of different PHYs.
 *
 *  Radio setup commands are used to initialize a PHY on the RF core.
 *  Various partially similar commands exist, each one represented
 *  by a different data type.
 *  RF_RadioSetup is a generic container for all types.
 *  A specific setup command is usually exported from SmartRF Studio
 *  and then passed to the RF driver in RF_open().
 */
typedef union {
    rfc_command_t                   commandId; ///< Generic command identifier. This is the first field
                                               ///< in every radio operation command.

    rfc_CMD_RADIO_SETUP_t           common;    ///< Radio setup command for BLE and IEEE modes
    rfc_CMD_BLE5_RADIO_SETUP_t      ble5;      ///< Radio setup command for BLE5 mode
    rfc_CMD_PROP_RADIO_SETUP_t      prop;      ///< Radio setup command for PROPRIETARY mode on 2.4 GHz
    rfc_CMD_PROP_RADIO_DIV_SETUP_t  prop_div;  ///< Radio setup command for PROPRIETARY mode on Sub-1 Ghz
} RF_RadioSetup;


/** @brief Client-related RF driver events.
 *
 *  Events originating in the RF driver but not directly related to a  specific radio command,
 *  are called client events.
 *  Clients may subscribe to these events by specifying a callback function RF_Params::pClientEventCb.
 *  Events are activated by specifying a bitmask RF_Params::nClientEventMask.
 *  The callback is called separately for every event providing an optional argument.
 *
 *  @code
 *  void onClientEvent(RF_Handle h, RF_ClientEvent event, void* arg)
 *  {
 *      switch (event)
 *      {
 *      case RF_ClientEventPowerUpFinished:
 *          // Set output port
 *          break;
 *      default:
 *          // Unsubscribed events must not be issued.
 *          assert(false);
 *      }
 *  }
 *
 *  RF_Params params;
 *  params.pClientEventCb = &onClientEvent;
 *  params.nClientEventMask = RF_ClientEventPowerUpFinished;
 *  RF_open(...);
 *  @endcode
 */
typedef enum {
    RF_ClientEventPowerUpFinished     = 0x00000001,   ///< The RF core has been powered up the radio setup has been finished.
    RF_ClientEventRadioFree           = 0x00000002,   ///< Radio becomes free after a command has been preempted by a high-priority command of another client.
                                                      ///< This event is only triggered on a client that has been preempted.
                                                      ///< Clients may use this event to retry running their low-priority RF operation.

    RF_ClientEventSwitchClientEntered = 0x00000004    ///< Signals the client that the RF driver is about to switch over from another client.
} RF_ClientEvent;

/** @brief Event mask for combining #RF_ClientEvent event flags in #RF_Params::nClientEventMask.
 *
 */
typedef uint32_t RF_ClientEventMask;

/** @brief Command handle that is returned by RF_postCmd().
 *
 *  A command handle is an integer number greater equals zero and identifies
 *  a command container in the RF driver's internal command queue. A client
 *  can dispatch a command with RF_postCmd() and use the command handle
 *  later on to make the RF driver interact with the command.
 *
 *  A negative value has either a special meaning or indicates an error.
 *
 *  @sa RF_pendCmd(), RF_flushCmd(), RF_cancelCmd(), ::RF_ALLOC_ERROR,
 *      ::RF_CMDHANDLE_FLUSH_ALL
 */
typedef int16_t RF_CmdHandle;

/** @brief RF Hardware attributes.
 *
 *  This data structure contains platform-specific driver configuration.
 *  It is usually defined globally in a board support file.
 *
 */
typedef struct {
    uint8_t         hwiCpe0Priority;  ///< Priority for INT_RFC_CPE_0 interrupt
    uint8_t         hwiHwPriority;    ///< Priority for INT_RFC_HW_COMB interrupt
    uint8_t         swiCpe0Priority;  ///< Priority for CPE_0 SWI
    uint8_t         swiHwPriority;    ///< Priority for HW SWI. Used for all RAT channel callbacks.
} RFCC26XX_HWAttrs;


/** @struct RF_Object
 *  @brief Stores the client's internal confguration and states.
 *
 *  Before RF_open() can be called, an instance of RF_Object must be created where
 *  the RF driver can store its internal configuration and states.
 *  This object must remain persistent throughout application run-time and must not be
 *  modified by the application.
 *
 *  The size of #RF_Object can be optimized for single-mode applications by providing a
 *  `RF_SINGLEMODE` symbol at compilation time. The prebuilt single-mode archive was generated
 *  with this symbol defined, hence any project using this archive must also define `RF_SINGLEMODE`
 *  on project level.
 *
 *  @note Except configuration fields before call to RF_open(), modification of
 *        any field in %RF_Object is forbidden.
 */


/** @cond */

#if defined (RF_SINGLEMODE)
  typedef struct RF_ObjectSingleMode  RF_Object;
#else
  typedef struct RF_ObjectMultiMode   RF_Object;
#endif

/*  Definition of the RF_Object structure for single-mode applications.
 *  It is applicable with the single-mode RF driver through the #RF_Object common type.
 */
struct RF_ObjectSingleMode{
    /// Configuration
    struct {
        uint32_t            nInactivityTimeout;     ///< Inactivity timeout in us
        RF_Mode*            pRfMode;                ///< Mode of operation
        RF_RadioSetup*      pOpSetup;               ///< Radio setup radio operation, only ram right now.
        uint32_t            nPowerUpDuration;       ///< Measured poweruptime in us or specified startup time (if left default it will measure)
        bool                bPowerUpXOSC;           ///< Allways enable XOSC_HF at chip wakeup
        bool                bUpdateSetup;           ///< Setup command update
        uint16_t            nPowerUpDurationMargin; ///< Powerup duration margin in us
        void*               pPowerCb;               ///< \deprecated Power up callback, will go away in future versions, see clientConfig::pClienteventCb instead
        void*               pErrCb;                 ///< Error callback
    } clientConfig;
    /// State & variables
    struct {
        struct {
            rfc_CMD_FS_t        cmdFs;              ///< FS command encapsulating FS state
        } mode_state;                               ///< (Mode-specific) state structure
        SemaphoreP_Struct       semSync;            ///< Semaphore used by runCmd(), pendCmd() and powerdown sequence
        RF_EventMask volatile   eventSync;          ///< Event mask/value used by runCmd() and waitCmd()
        void*                   pCbSync;            ///< Internal storage for user callback
        RF_EventMask            unpendCause;        ///< Return value for RF_pendCmd()
        ClockP_Struct           clkInactivity;      ///< Clock used for inactivity timeouts
        RF_CmdHandle volatile   chLastPosted;       ///< Command handle of most recently posted command
        bool                    bYielded;           ///< Client has indicated that there are no more commands
    } state;
};

/** Definition of the RF_Object structure for multi mode applications.
 *  It is applicable with the multi mode RF driver through the #RF_Object common type.
 */
struct RF_ObjectMultiMode{
    /// Configuration
    struct {
        uint32_t            nInactivityTimeout;     ///< Inactivity timeout in us
        RF_Mode*            pRfMode;                ///< Mode of operation
        RF_RadioSetup*      pOpSetup;               ///< Radio setup radio operation, only ram right now.
        uint32_t            nPowerUpDuration;       ///< Measured poweruptime in us or specified startup time (if left default it will measure)
        bool                bPowerUpXOSC;           ///< Allways enable XOSC_HF at chip wakeup
        bool                bUpdateSetup;           ///< Setup command update
        uint16_t            nPowerUpDurationMargin; ///< Powerup duration margin in us
        void*               pPowerCb;               ///< \deprecated Power up callback, will go away in future versions, see clientConfig::pClienteventCb instead
        void*               pErrCb;                 ///< Error callback
        void*               pClientEventCb;         ///< Client event callback
        RF_ClientEventMask  nClientEventMask;       ///< Client event mask to activate client event callback
    } clientConfig;
    /// State & variables
    struct {
        struct {
            rfc_CMD_FS_t        cmdFs;              ///< FS command encapsulating FS state
        } mode_state;                               ///< (Mode-specific) state structure
        SemaphoreP_Struct       semSync;            ///< Semaphore used by runCmd(), pendCmd() and powerdown sequence
        RF_EventMask volatile   eventSync;          ///< Event mask/value used by runCmd() and waitCmd()
        void*                   pCbSync;            ///< Internal storage for user callback
        RF_EventMask            unpendCause;        ///< Return value for RF_pendCmd()
        ClockP_Struct           clkInactivity;      ///< Clock used for inactivity timeouts
        ClockP_Struct           clkReqAccess;       ///< Clock used for request access timeouts
        RF_CmdHandle volatile   chLastPosted;       ///< Command handle of most recently posted command
        bool                    bYielded;           ///< Client has indicated that there are no more commands
    } state;
};

/** @endcond */

/** @brief A handle that is returned by to RF_open().
 *
 *  %RF_Handle is used for further RF client interaction with the RF driver.
 *  An invalid handle has the value NULL.
 */
typedef RF_Object* RF_Handle;


/** @brief RAT handle that is returned by RF_ratCompare() or RF_ratCapture().
 *
 *  An %RF_RatHandle is an integer number with value greater than or equal to zero and identifies
 *  a Radio Timer Channel in the RF driver's internal RAT module. A client can interract with the
 *  RAT module through the RF_ratCompare(), RF_ratCapture() or RF_ratDisableChannel() APIs.
 *
 *  A negative value indicates an error. A typical example when RF_ratCompare() returns with RF_ALLOC_ERROR.
 */
typedef int8_t RF_RatHandle;


/** @brief Selects the entry of interest in RF_getInfo().
 *
 */
typedef enum {
    RF_GET_CURR_CMD,                              ///< Retrieve a command handle of the current command.
    RF_GET_AVAIL_RAT_CH,                          ///< Create a bitmask showing available RAT channels.
    RF_GET_RADIO_STATE,                           ///< Show the current RF core power state. 0: Radio OFF, 1: Radio ON.
    RF_GET_SCHEDULE_MAP,                          ///< Provide a timetable of all scheduled commands.
} RF_InfoType;

/** @brief Stores output parameters for RF_getInfo().
 *
 *  This union structure holds one out of multiple data types.
 *  The contained value is selected by #RF_InfoType.
 */
typedef union {
    RF_CmdHandle ch;                              ///< Command handle (#RF_GET_CURR_CMD).
    uint16_t     availRatCh;                      ///< Available RAT channels (RF_GET_AVAIL_RAT_CH).
    bool         bRadioState;                     ///< Current RF core power state (#RF_GET_RADIO_STATE).
    void         *pScheduleMap;                   ///< Pointer to scheduling map (#RF_GET_SCHEDULE_MAP).
} RF_InfoVal;

/// RF schedule map entry structure
typedef struct {
    RF_Handle    pClient;                          ///< Pointer to client object
    uint32_t     startTime;                        ///< Start time (in RAT tick) of the command or access request
    uint32_t     endTime;                          ///< End time (in RAT tick) of the command or access request
    RF_Priority  priority;                         ///< Priority of the command or access request
} RF_ScheduleMapElement;

///RF schedule map structure
typedef struct {
    RF_ScheduleMapElement  accessMap[RF_NUM_SCHEDULE_ACCESS_ENTRIES];    ///< Access request schedule map
    RF_ScheduleMapElement  commandMap[RF_NUM_SCHEDULE_COMMAND_ENTRIES];  ///< Command schedule map
} RF_ScheduleMap;


/** @brief Handles events related to RF command execution.
 *
 *  RF command callbacks notify the application of any events happening during RF command execution.
 *  Events may either refer to RF core interrupts (@ref RF_Core_Events) or may be generated by the RF driver
 *  (@ref RF_Driver_Events).
 *
 *  RF command callbacks are set up as parameter to RF_postCmd() or RF_runCmd() and provide:
 *
 *  - the relevant driver client handle \a h which was returned by RF_open(),
 *  - the relevant radio operation command handle \a ch,
 *  - an event mask \a e containing the occured events.
 *
 *  RF command callbacks are executed in Software Interrupt (SWI) context and must not perform any
 *  blocking operation.
 *  The priority is configurable via #RFCC26XX_HWAttrs in the board file or #RF_CTRL_SET_SWI_PRIORITY in RF_control().
 *
 *  The %RF_Callback function type is also used for signalling power events and
 *  errors.
 *  These are set in #RF_Params::pPowerCb and #RF_Params::pErrCb respectively.
 *  In case of a power event, \a ch can be ignored and \a e has #RF_EventPowerUp set.
 *  In case of an error callback, \a ch contains an error code instead of a command handle and
 *  \a e has the #RF_EventError flag set.
 *
 *  @note Error and power callbacks will be replaced by #RF_ClientCallback in future releases.
 */
typedef void (*RF_Callback)(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);

/**
 *  @brief Handles events related to a driver instance.
 *
 *  The RF driver produces additional events that are not directly related to the execution of a certain command, but
 *  happen during general RF driver operations.
 *  This includes power-up events, client switching events and others.
 *
 *  A client callback provides the following arguments:
 *  - the relevant driver client handle \a h which was returned by RF_open(),
 *  - an event identifier \a event,
 *  - an optional argument \a arg depending on the event.
 *
 *  RF client callbacks are executed in Software Interrupt (SWI) context and must not perform any blocking operation.
 *  The priority is configurable via #RFCC26XX_HWAttrs in the board file or #RF_CTRL_SET_SWI_PRIORITY in RF_control().
 */
typedef void (*RF_ClientCallback)(RF_Handle h, RF_ClientEvent event, void* arg);

/** @brief RF driver configuration parameters.
 *
 *  %RF_Params is used for initial RF driver configuration.
 *  It is initialized by RF_Params_init() and used by RF_open().
 *  Each client has its own set of parameters.
 *  They are reconfigured on a client switch.
 *  Some of the parameters can be changed during run-time using RF_control().
 */
typedef struct {
    uint32_t            nInactivityTimeout;      ///< Inactivity timeout in microseconds.
                                                 ///< The default value is 0xFFFFFFFF (infinite).

    uint32_t            nPowerUpDuration;        ///< A custom power-up duration in microseconds.
                                                 ///< If 0, the RF driver will start with a conservative value and measure the actual time during the first power-up.
                                                 ///< The default value is 0.

    RF_Callback         pPowerCb;                ///< \deprecated Power up callback, will be removed future versions, see RF_Params::pClienteventCb instead.
                                                 ///< The default value is NULL.

    RF_Callback         pErrCb;                  ///< Callback function for driver error events.
    bool                bPowerUpXOSC;            ///< Allways enable XOSC_HF when the chip wakes up.
                                                 ///< This allows to enable XOSC_HF early after wake-up and improves power consumption in cases that most wake-ups are triggered by RF-related events.
                                                 ///< Set this value to false when RF operations are only rarely the cause for a wake-up from standby.
                                                 ///< The default value is true.

    uint16_t            nPowerUpDurationMargin;  ///< An additional safety margin to be added to #RF_Params::nPowerUpDuration.
                                                 ///< This is necessary because of other hardware and software interrupts
                                                 ///< preempting the RF driver interrupt handlers and state machine.
                                                 ///< The default value is platform-dependent.

    RF_ClientCallback   pClientEventCb;          ///< Callback function for client-related events.
                                                 ///< The default value is NULL.

    RF_ClientEventMask  nClientEventMask;        ///< Event mask used to subscribe certain client events.
                                                 ///< The purpose is to keep the number of callback executions small.
} RF_Params;

/// @brief RF schedule command parameter struct
/// RF schedule command parameters are used with the RF_scheduleCmd() call.
typedef struct {
    uint32_t    endTime;           ///< End time in RAT Ticks for the radio command
    RF_Priority priority;          ///< Intra client priority
    bool        bIeeeBgCmd;        ///< IEEE 15.4 background command indication. 0: FG cmd, 1: BG cmd.
}RF_ScheduleCmdParams;

/// @brief RF request access parameter struct
/// RF request access command parameters are used with the RF_requestAccess() call.
typedef struct {
    uint32_t    duration;          ///< Radio access duration in RAT Ticks requested by the client
    uint32_t    startTime;         ///< Start time window in RAT Time for radio access
    RF_Priority priority;          ///< Access priority
}RF_AccessParams;

/** @brief Creates a a new client instance of the RF driver.
 *
 *  This function initializes an RF driver client instance using \a pObj as storage.
 *  It does not power up the RF core.
 *  Once the client starts the first RF opweration command later in the application,
 *  the RF core is powered up and set into a PHY mode specified by \a pRfMode.
 *  The chosen PHY is then configured by a radio setup command \a pOpSetup.
 *  Whenever the RF core is powered up, the RF driver re-executes the radio setup command \a pOpSetup.
 *  Additional driver behaviour may be set by an optional \a params.
 *
 *  @code
 *  // Define parameters
 *  RF_Params rfParams;
 *  rfParams.nInactivityTimeout = 4;
 *  RF_Params_init(&rfParams);
 *  rfParams.nInactivityTimeout = 1701; // microseconds
 *
 *  RF_Handle rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
 *  @endcode
 *
 *  @note Calling context : Task
 *
 *  @param pObj      Pointer to a #RF_Object that will hold the state for this
 *                   RF client. The object must be in persistent and writeable
 *                   memory.
 *  @param pRfMode   Pointer to a #RF_Mode struct holding PHY information
 *  @param pOpSetup  Pointer to the radio setup command used for this client.
 *                   This is re-executed by the RF Driver on each power-up.
 *  @param params    Pointer to an RF_Params object with the desired driver configuration.
 *                   A NULL pointer results in the default configuration being loaded.
 *  @return          A handle for further RF driver calls on success. Otherwise NULL.
 */
extern RF_Handle RF_open(RF_Object *pObj, RF_Mode *pRfMode, RF_RadioSetup *pOpSetup, RF_Params *params);


/**
 *  @brief  Close client connection to RF driver
 *
 *  Allows a RF client (high-level driver or application) to close its connection
 *  to the RF driver.
 *
 *  @note Calling context : Task
 *
 *  @param h  Handle previously returned by RF_open()
 */
extern void RF_close(RF_Handle h);


/**
 *  @brief  Return current radio timer value
 *
 *  If the radio is powered returns the current radio timer value, if not returns
 *  a conservative estimate of the current radio timer value
 *
 *  @note Calling context : Task/SWI/HWI
 *
 *  @return     Current radio timer value
 */
extern uint32_t RF_getCurrentTime(void);


/**
 *  @brief  Appends RF operation commands to the driver's command queue and returns a
 *          command handle.
 *
 *  The RF operation \a pOp may either represent a single operation or may be the first
 *  operation in a chain.
 *  If the command queue is empty, the \a pCmd is dispatched immediately. If there are
 *  other operations pending, then \a pCmd is processed after all other commands have been
 *  finished.
 *  The RF operation command must be compatible to the RF_Mode selected by RF_open(), e.g.
 *  proprietary commands can only be used when the RF core is configured for proprietary mode.
 *
 *  The returned command handle is an identifier that can be used to control command execution
 *  later on, for instance with RF_pendCmd() or RF_cancelCmd().
 *  It is a 16 Bit signed integer value, incremented on every new command.
 *  If the RF driver runs out of command containers, RF_ALLOC_ERROR is returned.
 *
 *  The priority \a ePri is only relevant in multi-client applications where commands of distinct
 *  clients may interrupt each other.
 *  Only commands started by RF_scheduleCmd() can preempt
 *  running commands. #RF_postCmd() or RF_runCmd() do never interrupt a running command.
 *  In single-client applications, \a ePri is ignored and should be set to ::RF_PriorityNormal.
 *
 *  A callback function \a pCb might be specified to get notified about events during command
 *  execution. Events are subscribed by the bit mask \a bmEvent.
 *  Valid event flags are specified in @ref RF_Core_Events and @ref RF_Driver_Events.
 *  If no callback is set, RF_pendCmd() can be used to synchronize the current task to command
 *  execution. For this it is necessary to subscribe all relevant events.
 *  The termination events ::RF_EventLastCmdDone, ::RF_EventCmdCancelled, ::RF_EventCmdAborted and
 *  ::RF_EventCmdStopped are always implicitly subscribed.
 *
 *  The following limitations apply to the execution of command chains:
 *
 *  - If TRIG_ABSTIME is used as a start trigger for the first command, TRIG_REL_FIRST_START
 *    can not be used for any other command. This is because the RF driver may insert a
 *    frequency-select command (CMD_FS) at the front of the chain when it performs an
 *    automatic power-up.
 *  - Having more than one CMD_FS in a chain may lead to unexpected behavior.
 *    If a chain contains a CMD_FS and the command can be reached by iterating over the pNextOp
 *    field, then RF driver will always update the cached CMD_FS with the new settings. On the
 *    next automatic power-up, the RF driver will use the updated frequency.
 *
 *  @note Calling context : Task/SWI
 *
 *  @sa RF_pendCmd(), RF_runCmd(), RF_scheduleCmd(), RF_RF_cancelCmd(), RF_flushCmd(), RF_getCmdOp()
 *
 *  @param h         Driver handle previously returned by RF_open()
 *  @param pOp       Pointer to the RF operation command.
 *  @param ePri      Priority of this RF command (used for arbitration in multi-client systems)
 *  @param pCb       Callback function called during command execution and upon completion.
 *                   If RF_postCmd() fails, no callback is made.
 *  @param bmEvent   Bitmask of events that will trigger the callback or that can be pended on.
 *  @return          A handle to the RF command. Return value of RF_ALLOC_ERROR indicates error.
 */
extern RF_CmdHandle RF_postCmd(RF_Handle h, RF_Op* pOp, RF_Priority ePri, RF_Callback pCb, RF_EventMask bmEvent);

/**
 *  @brief  Schedule an RF operation (chain) to the command queue.
 *
 *  Schedule an #RF_Op to the RF command queue of the client with handle h. <br>
 *  The command can be the first in a chain of RF operations or a standalone RF operation.
 *  If a chain of operations are posted they are treated atomically, i.e. either all
 *  or none of the chained operations are run. <br>
 *  All operations must be posted in strictly increasing chronological order. Function returns
 *  immediately. <br>
 *
 *  Limitations apply to the operations posted:
 *  - The operation must be in the set supported in the chosen radio mode when
 *    RF_open() was called
 *  - Only a subset of radio operations are supported
 *  - Only some of the trigger modes are supported with potential power saving (TRIG_NOW, TRIG_ABSTIME)
 *
 *  @note Calling context : Task/SWI
 *
 *  @param h         Handle previously returned by RF_open()
 *  @param pOp       Pointer to the #RF_Op. Must normally be in persistent and writeable memory
 *  @param pSchParams Pointer to the schdule command parameter structure
 *  @param pCb       Callback function called upon command completion (and some other events).
 *                   If RF_scheduleCmd() fails no callback is made
 *  @param bmEvent   Bitmask of events that will trigger the callback.
 *  @return          A handle to the RF command. Return value of RF_ALLOC_ERROR indicates error.
 */
extern RF_CmdHandle RF_scheduleCmd(RF_Handle h, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams, RF_Callback pCb, RF_EventMask bmEvent);

/**
 *  @brief  Synchronizes the calling task to an RF operation command \a ch and
 *          returns accumulated event flags.
 *
 *  After having dispatched an RF operation represented by \a ch with RF_postCmd(), the
 *  command is running in parallel on the RF core. Thus, it might be desirable to synchronize
 *  the calling task to the execution of the command.
 *  With #RF_pendCmd(), the application can block until one of the events specified in
 *  \a bmEvent occurs or until the command finishes.
 *  The function consumes and returns all accumulated event flags that occured during
 *  execution if they have been previously subscribed by RF_postCmd().
 *  Possible events are specified in @ref RF_Core_Events and @ref RF_Driver_Events.
 *  The termination events ::RF_EventLastCmdDone, ::RF_EventCmdCancelled,
 *  ::RF_EventCmdAborted and ::RF_EventCmdStopped are always implicitly subscribed and
 *  can not be masked.
 *
 *  #RF_pendCmd() may be called multiple times for the same command.
 *
 *  If #RF_pendCmd() is called for a command handle representing a finished command,
 *  then only the ::RF_EventLastCmdDone flag is returned, regardless of how the command
 *  finished.
 *
 *  If the command has also a callback set, the callback is executed before #RF_pendCmd()
 *  returns.
 *
 *  Example:
 *  @code
 *  // Dispatch a command to the RF driver's command queue
 *  RF_CmdHandle ch = RF_postCmd(driver, (RF_Op*)&CMD_PROP_RX, RF_PriorityNormal, NULL, RF_EventRxEntryDone);
 *  assert(ch != RF_ALLOC_ERROR);
 *
 *  bool finished = false;
 *  while (finished == false)
 *  {
 *      // Synchronize to events during command execution.
 *      uint32_t events = RF_pendCmd(driver, ch, RF_EventRxEntryDone);
 *      // Check events that happen during execution
 *      if (events & RF_EventRxEntryDone)
 *      {
 *          // Process packet
 *      }
 *      if (events & (RF_EventLastCmdDone | RF_EventCmdStopped | RF_EventCmdAborted | RF_EventCmdCancelled))
 *      {
 *          finished = true;
 *      }
 *      // ...
 *  }
 *  @endcode
 *
 *  @note Calling context : Task
 *
 *  @param h        Driver handle previously returned by RF_open()
 *  @param ch       Command handle previously returned by RF_postCmd().
 *  @param bmEvent  Bitmask of events that make RF_pendCmd() return. Termination events
 *                  are always implicitly subscribed.
 *  @return         Event flags accumulated during command execution.
 *
 *  @sa RF_postCmd()
 */
extern RF_EventMask RF_pendCmd(RF_Handle h, RF_CmdHandle ch, RF_EventMask bmEvent);


/**
 *  @brief  Runs synchronously an RF operation command or a chain of commands and returns
 *          the termination reason.
 *
 *  This function appends an RF operation command or a chain of commands to the RF driver's
 *  command queue and then waits for it to complete.
 *  A command is completed if one of the termination events ::RF_EventLastCmdDone,
 *  ::RF_EventCmdCancelled, ::RF_EventCmdAborted, ::RF_EventCmdStopped occured.
 *
 *  This function is a combination of RF_postCmd() and RF_pendCmd().
 *  All options and limitations for RF_postCmd() apply here as well.
 *
 *  An application should always ensure that the command completed in the expected way and
 *  with an expected status code.
 *
 *  @note Calling context : Task
 *
 *  @param h         Driver handle previously returned by RF_open()
 *  @param pOp       Pointer to the RF operation command.
 *  @param ePri      Priority of this RF command (used for arbitration in multi-client systems)
 *  @param pCb       Callback function called during command execution and upon completion.
 *                   If RF_runCmd() fails, no callback is made.
 *  @param bmEvent   Bitmask of events that will trigger the callback or that can be pended on.
 *  @return          The relevant termination event.
 *
 *  @sa RF_postCmd(), RF_pendCmd(), RF_cancelCmd(), RF_flushCmd()
 */
extern RF_EventMask RF_runCmd(RF_Handle h, RF_Op* pOp, RF_Priority ePri, RF_Callback pCb, RF_EventMask bmEvent);

/**
 *  @brief  Runs synchronously a (chain of) RF operation(s) for dual or single-mode.
 *
 *  Allows a (chain of) operation(s) to be scheduled to the command queue and then waits
 *  for it to complete. <br> A command is completed if one of the RF_EventLastCmdDone,
 *  RF_EventCmdCancelled, RF_EventCmdAborted, RF_EventCmdStopped occured.
 *
 *  @note Calling context : Task
 *  @note Only one call to RF_pendCmd() or RF_runScheduleCmd() can be made at a time for
 *        each client
 *
 *  @param h         Handle previously returned by RF_open()
 *  @param pOp       Pointer to the #RF_Op. Must normally be in persistent and writeable memory
 *  @param pSchParams Pointer to the schdule command parameter structure
 *  @param pCb       Callback function called upon command completion (and some other events).
 *                   If RF_runScheduleCmd() fails, no callback is made.
 *  @param bmEvent   Bitmask of events that will trigger the callback.
 *  @return          The relevant commmand completed event.
 */
extern RF_EventMask RF_runScheduleCmd(RF_Handle h, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams, RF_Callback pCb, RF_EventMask bmEvent);

/**
 *  @brief  Abort/stop/cancel single command in command queue.
 *
 *  If command is running, aborts/stops it and posts callback for the
 *  aborted/stopped command. <br>
 *  If command has not yet run, cancels it it and posts callback for the
 *  cancelled command. <br>
 *  If command has already run or been aborted/stopped/cancelled, has no effect.<br>
 *  If RF_cancelCmd is called from a Swi context with same or higher priority
 *  than RF Driver Swi, when the RF core is powered OFF -> the cancel callback will be delayed
 *  until the next power-up cycle.<br>
 *
 *  @note Calling context : Task/SWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param ch           Command handle previously returned by RF_postCmd().
 *  @param mode         1: Stop gracefully, 0: abort abruptly
 *  @return             RF_Stat indicates if command was successfully completed
 */
extern RF_Stat RF_cancelCmd(RF_Handle h, RF_CmdHandle ch, uint8_t mode);


/**
 *  @brief  Abort/stop/cancel command and any subsequent commands in command queue.
 *
 *  If command is running, aborts/stops it and then cancels all later commands in queue.<br>
 *  If command has not yet run, cancels it and all later commands in queue.<br>
 *  If command has already run or been aborted/stopped/cancelled, has no effect.<br>
 *  The callbacks for all cancelled commands are issued in chronological order.<br>
 *  If RF_flushCmd is called from a Swi context with same or higher priority
 *  than RF Driver Swi, when the RF core is powered OFF -> the cancel callback will be delayed
 *  until the next power-up cycle.<br>
 *
 *  @note Calling context : Task/SWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param ch           Command handle previously returned by RF_postCmd().
 *  @param mode         1: Stop gracefully, 0: abort abruptly
 *  @return             RF_Stat indicates if command was successfully completed
 */
extern RF_Stat RF_flushCmd(RF_Handle h, RF_CmdHandle ch, uint8_t mode);

/**
 *  @brief Send any Immediate command. <br>
 *
 *  Immediate Comamnd is send to RDBELL, if radio is active and the RF_Handle points
 *  to the current client. <br>
 *  In other appropriate RF_Stat values are returned. <br>
 *
 *  @note Calling context : Task/SWI/HWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param pCmdStruct   Pointer to the immediate command structure
 *  @return             RF_Stat indicates if command was successfully completed
*/
extern RF_Stat RF_runImmediateCmd(RF_Handle h, uint32_t* pCmdStruct);

/**
 *  @brief Send any Direct command. <br>
 *
 *  Direct Comamnd value is send to RDBELL immediately, if radio is active and
 *  the RF_Handle point to the current client. <br>
 *  In other appropriate RF_Stat values are returned. <br>
 *
 *  @note Calling context : Task/SWI/HWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param cmd          Direct command value.
 *  @return             RF_Stat indicates if command was successfully completed.
*/
extern RF_Stat RF_runDirectCmd(RF_Handle h, uint32_t cmd);

/**
 *  @brief  Signal that radio client is not going to issue more commands in a while. <br>
 *
 *  Hint to RF driver that, irrespective of inactivity timeout, no new further
 *  commands will be issued for a while and thus the radio can be powered down at
 *  the earliest convenience.<br>
 *
 *  @note Calling context : Task
 *
 *  @param h       Handle previously returned by RF_open()
 */
extern void RF_yield(RF_Handle h);

/**
 *  @brief  Function to initialize the RF_Params struct to its defaults.
 *
 *  @param  params      An pointer to RF_Params structure for
 *                      initialization
 *
 *  Defaults values are:
 *      nInactivityTimeout = BIOS_WAIT_FOREVER
 *      nPowerUpDuration   = RF_DEFAULT_POWER_UP_TIME
 *      bPowerUpXOSC       = true
 */
extern void RF_Params_init(RF_Params *params);

/**
 *  @brief Get value for some RF driver parameters. <br>
 *
 *  @note Calling context : Task/SWI/HWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param type         Request value paramter defined by RF_InfoType
 *  @param pValue       Pointer to return parameter values specified by RF_InfoVal
 *  @return             RF_Stat indicates if command was successfully completed
*/
extern RF_Stat RF_getInfo(RF_Handle h, RF_InfoType type, RF_InfoVal *pValue);

/**
 *  @brief Get RSSI value.
 *
 *  @note Calling context : Task/SWI/HWI
 *
 *  @param  h            Handle previously returned by RF_open()
 *  @return              RSSI value. Return value of RF_GET_RSSI_ERROR_VAL indicates error case.
 */
extern int8_t RF_getRssi(RF_Handle h);

/**
 *  @brief  Get command structure pointer.
 *
 *  @note Calling context : Task/SWI/HWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param cmdHnd       Command handle returned by RF_postCmd()
 *  @return             Pointer to the command structure.
*/
extern RF_Op* RF_getCmdOp(RF_Handle h, RF_CmdHandle cmdHnd);

/**
 *  @brief  Setup a Radio Timer (RAT) channel in compare mode.
 *
 *  The RF_ratCompare() API sets up one of the three available RAT channels in compare mode.
 *  When the compare event happens at the given compareTime, the registered callback (pRatCb)
 *  is invoked.
 *
 *  The API automatically handles power management. If the provided compareTime is far into the future
 *  (and there is no other constraints set i.e. due to radio command execution), the RF core will be
 *  powered OFF and the device will enter the lowest possible power state. The RF core will be
 *  automatically powered ON before the registered compare event, and the actively used RAT channels
 *  are resynchronized. The callback function is executed in SWI context upon expiration of the
 *  RAT channel. The API implements a "one-shot" compare event. Since the channel is automatically freed
 *  before the callback is called, the same channel can be reallocated from within the callback itself.
 *
 *  In case of programming a RAT channel is rejected by the RF core, the error callback registered through RF_open()
 *  is invoked with error code RF_ERROR_RAT_PROG. A typical use case when the provided compareTime is in the past.
 *  The error callback is executed in SWI context.
 *
 *  @note Calling context : Task/SWI
 *
 *  @param h             Handle previously returned by RF_open().
 *  @param pCmdStruct    Pointer to the RAT compare command structure.
 *  @param compareTime   Absolute compare time from the invocation of API in RAT ticks.
 *  @param pRatCb        User callback to be executed upon the compare event.
 *  @return              Allocated RAT channel. If allocation fails, RF_ALLOC_ERROR is returned.
 */
extern RF_RatHandle RF_ratCompare(RF_Handle h, rfc_CMD_SET_RAT_CMP_t* pCmdStruct, \
                                          uint32_t compareTime, RF_Callback pRatCb);
/**
 *  @brief  Setup a Radio Timer (RAT) channel in capture mode.
 *
 *  The RF_ratCapture() API sets up one of the three avaialble RAT channels in capture mode.
 *  When the capture event configured through the config argument happens, the registered callback (pRatCb)
 *  is invoked.
 *
 *  The API automatically handles power management. If the RF core is OFF when the RF_ratCapture() is called,
 *  it will be powered ON immediately and the RAT channel will be programmed. As long as at least one of the
 *  three RAT channels are in capture mode, the RF core will be kept ON to be able to serve the capture events.
 *  The callback function is executed in SWI context when the configured conditions met. In case the channel is
 *  configured into single capture mode, the channel is automatically freed before the callback is called. In
 *  repeated capture mode, the channel remains in use.
 *
 *  In case of programming a RAT channel is rejected by the RF core, the error callback registered through RF_open()
 *  is invoked with error code RF_ERROR_RAT_PROG, and the channel is freed. A typical use case when the
 *  configuration field in the config argument is invalid. The error callback is executed in SWI context.
 *
 *  @note Calling context : Task/SWI
 *
 *  @param h             Handle previously returned by RF_open().
 *  @param config        Bitwise configuration field of capture mode (source of event, repeat mode, etc.).
 *  @param pRatCb        User callback to be executed when the capture conditions met.
 *  @return              Allocated RAT channel. If allocation fails, RF_ALLOC_ERROR is returned.
 */
extern RF_RatHandle RF_ratCapture(RF_Handle h, uint16_t config, RF_Callback pRatCb);

/**
 *  @brief  Setup RAT HW output. Note radio needs to be ON and and the selected RAT channel should be configured
 *          to compare mode prior this method is called. If radio is OFF, this API returns with RF_StatRadioInactiveError.
 *
 *  @note Calling context : Task/SWI
 *
 *  @param h             Handle previously returned by RF_open().
 *  @param config        Config field of RAT HW output command structure.
 *  @return              RF_Stat indicates if command was successfully completed.
 */
extern RF_Stat RF_ratHwOutput(RF_Handle h, uint16_t config);

/**
 *  @brief  Disable a RAT channel.
 *
 *  The RF_RatHandle returned by the RF_ratCompare() or RF_ratCapture() APIs can be used for further interaction with the
 *  Radio Timer. Passing the handle to RF_ratDisableChannel() will abort a compare/capture event, and the provided channel
 *  is deallocated. This API can be called both if the RF core is ON or OFF. After the channel is freed, the next radio event
 *  will be rescheduled. A typical use case if a channel is configured in repeated capture mode, and the application decides
 *  to abort this operation.
 *
 *  @note Calling context : Task/SWI
 *
 *  @param h             Handle previously returned by RF_open().
 *  @param ratHandle     RF_RatHandle returned by RF_ratCompare() or RF_ratCapture().
 *  @return              RF_Stat indicates if command was successfully completed.
 */
extern RF_Stat RF_ratDisableChannel(RF_Handle h, RF_RatHandle ratHandle);

/**
 *  @brief  Set RF control parameters.
 *
 *  @note Calling context : Task
 *
 *  @param h             Handle previously returned by RF_open()
 *  @param ctrl          Control codes
 *  @param args          Pointer to control arguments
 *  @return              RF_Stat indicates if API call was successfully completed.
 */
extern RF_Stat RF_control(RF_Handle h, int8_t ctrl, void *args);

/**
 *  @brief  Request radio access. <br>
 *
 *  Scope:
 *  1. Only suppports request access which start immediately.<br>
 *  2. The #RF_AccessParams duration should be less than a pre-defined value
 *     RF_REQ_ACCESS_MAX_DUR_US in RFCC26XX_multiMode.c.<br>
 *  3. The #RF_AccessParams priority should be set RF_PriorityHighest.<br>
 *  4. Single request for a client at anytime.<br>
 *  5. Command from different client are blocked untill the radio access
 *     period is completed.<br>
 *
 *  @note Calling context : Task
 *
 *  @param h             Handle previously returned by RF_open()
 *  @param pParams       Pointer to RF_AccessRequest parameters
 *  @return              RF_Stat indicates if API call was successfully completed.
 */
extern RF_Stat RF_requestAccess(RF_Handle h, RF_AccessParams *pParams);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_rf__include */

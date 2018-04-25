/******************************************************************************

 @file       oad_persistent_app.c

 @brief This file contains the OAD Persistent App sample application for use with
        the CC2650 Bluetooth Low Energy Protocol Stack.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2017-2017, Texas Instruments Incorporated
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

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/display/Display.h>

#ifdef LED_DEBUG
#include <ti/drivers/PIN.h>
#endif //LED_DEBUG

#include "hci_tl.h"
#include "gatt.h"

#include "gapgattserver.h"
#include "gattservapp.h"

#include "oad.h"

#include "peripheral.h"
#include "gapbondmgr.h"

#include "osal_snv.h"
#include "icall_ble_apimsg.h"

#include "util.h"

#include "board.h"

#include "icall_ble_api.h"

// Needed for HAL_SYSTEM_RESET()
#include "hal_mcu.h"

#include "ble_user_config.h"

/*********************************************************************
 * CONSTANTS
 */
// Advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 12=15ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     12

// Maximum connection interval (units of 1.25ms, 12=15ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     12

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 100=1s) if automatic parameter
// update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          300

// Whether to enable automatic parameter update request when a connection is
// formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         GAPROLE_LINK_PARAM_UPDATE_WAIT_REMOTE_PARAMS

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// How often to perform periodic event (in msec)
#define PERSIST_APP_PERIODIC_EVT_PERIOD       5000

// Task configuration
#define PERSIST_APP_TASK_PRIORITY             1

// Warning! To optimize RAM, task stack size must be a multiple of 8 bytes
#ifndef PERSIST_APP_TASK_STACK_SIZE
#define PERSIST_APP_TASK_STACK_SIZE           640
#endif

// Application events used with app queue (appEvtHdr_t)
// These are not related to RTOS evts, but instead enqueued via state change CBs
#define PERSIST_APP_STATE_CHANGE_EVT          0x0001
#define PERSIST_APP_CONN_EVT_END_EVT          0x0002
#define PERSIST_APP_PASSCODE_NEEDED_EVT               0x0003

// Internal Events for RTOS application
#define PERSIST_APP_ICALL_EVT                 ICALL_MSG_EVENT_ID  // Event_Id_31
#define PERSIST_APP_QUEUE_EVT                 UTIL_QUEUE_EVENT_ID // Event_Id_30
#define PERSIST_APP_OAD_QUEUE_EVT             OAD_QUEUE_EVT       // Event_Id_01
#define PERSIST_APP_OAD_COMPLETE_EVT          OAD_DL_COMPLETE_EVT // Event_Id_02

#define PERSIST_APP_ALL_EVENTS              ( PERSIST_APP_ICALL_EVT         | \
                                              PERSIST_APP_QUEUE_EVT         | \
                                              PERSIST_APP_OAD_QUEUE_EVT     | \
                                              PERSIST_APP_OAD_COMPLETE_EVT)

/*********************************************************************
 * TYPEDEFS
 */

// App event passed from profiles.
typedef struct
{
  appEvtHdr_t hdr; // Event header
  uint8_t *pData;   // Event payload
} persistAppEvt_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Display Interface
Display_Handle dispHandle = NULL;

// BIM variable
#ifdef __TI_COMPILER_VERSION__
  #pragma location = BIM_VAR_ADDR
  #pragma NOINIT(_bim_var)
  uint32_t _bim_var;
#elif __IAR_SYSTEMS_ICC__
  __no_init uint32_t _bim_var @ BIM_VAR_ADDR;
#endif

/*********************************************************************
 * LOCAL VARIABLES
 */

// Entity ID globally used to check for source and/or destination of messages
static ICall_EntityID selfEntity;

// Event globally used to post local events and pend on system and
// local events.
static ICall_SyncHandle syncEvent;

// Queue object used for app messages
static Queue_Struct appMsg;
static Queue_Handle appMsgQueue;

// Task configuration
Task_Struct persistAppTask;

#if defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(persistAppTaskStack, 8)
#elif __IAR_SYSTEMS_ICC__
#pragma data_alignment=8
#endif    //__TI_COMPILER_VERSION__
uint8_t persistAppTaskStack[PERSIST_APP_TASK_STACK_SIZE];


// GAP - SCAN RSP data (max size = 31 bytes)
static uint8_t scanRspData[] =
{
  // complete name
  0x13,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'O',
  'A',
  'D',
  ' ',
  'P',
  'e',
  'r',
  's',
  'i',
  's',
  't',
  'e',
  'n',
  't',
  ' ',
  'A',
  'p',
  'p',

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),   // 100ms
  HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
  LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),   // 1s
  HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8_t advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16(OAD_SERVICE_UUID),
  HI_UINT16(OAD_SERVICE_UUID)
};

// GAP GATT Attributes
static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "OAD Persistent App";

// Variable used to store the number of messages pending once OAD completes
// The application cannot reboot until all pending messages are sent
static uint8_t numPendingMsgs = 0;

// Pin table for LED debug pins
static const PIN_Config persistAppLedPins[] = {
    Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

// State variable for debugging LEDs
PIN_State  persistAppLedState;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void OadPersistApp_init( void );
static void OadPersistApp_taskFxn(UArg a0, UArg a1);
static void OadPersistApp_processStackMsg(ICall_Hdr *pMsg);
static void OadPersistApp_processGATTMsg(gattMsgEvent_t *pMsg);
static void OadPersistApp_processAppMsg(persistAppEvt_t *pMsg);
static void OadPersistApp_processStateChangeEvt(gaprole_States_t newState);
static void OadPersistApp_stateChangeCB(gaprole_States_t newState);
static void OadPersistApp_enqueueMsg(uint8_t event, uint8_t state, uint8_t *pData);
static void OadPersistApp_processL2CAPMsg(l2capSignalEvent_t *pMsg);
static void OadPersistApp_processOadWriteCB(uint8_t event, uint16_t arg);
static void OadPersistApp_processPasscode(gapPasskeyNeededEvent_t *pData);
static void OadPersistApp_passcodeCB(uint8_t *deviceAddr,
                                            uint16_t connHandle,
                                            uint8_t uiInputs, uint8_t uiOutputs,
                                            uint32_t numComparison);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t OadPersistApp_gapRoleCBs =
{
  OadPersistApp_stateChangeCB     // Profile State Change Callbacks
};

// OAD Profile Callback
static oadTargetCBs_t OadPersistApp_oadCBs =
{
  OadPersistApp_processOadWriteCB // Write Callback.
};

// GAP Bond Manager Callbacks
static gapBondCBs_t OadPersistApp_BondMgrCBs =
{
  (pfnPasscodeCB_t)OadPersistApp_passcodeCB, // Passcode callback,
  NULL
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      OadPersistApp_createTask
 *
 * @brief   Task creation function for the Simple BLE Peripheral.
 *
 * @param   None.
 *
 * @return  None.
 */
void OadPersistApp_createTask(void)
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = persistAppTaskStack;
  taskParams.stackSize = PERSIST_APP_TASK_STACK_SIZE;
  taskParams.priority = PERSIST_APP_TASK_PRIORITY;

  Task_construct(&persistAppTask, OadPersistApp_taskFxn, &taskParams, NULL);
}

/*********************************************************************
 * @fn      OadPersistApp_init
 *
 * @brief   Called during initialization and contains application
 *          specific initialization (ie. hardware initialization/setup,
 *          table initialization, power up notification, etc), and
 *          profile initialization/setup.
 *
 * @param   None.
 *
 * @return  None.
 */
static void OadPersistApp_init(void)
{
  // ******************************************************************
  // NO STACK API CALLS CAN OCCUR BEFORE THIS CALL TO ICall_registerApp
  // ******************************************************************
  // Register the current thread as an ICall dispatcher application
  // so that the application can send and receive messages.
  ICall_registerApp(&selfEntity, &syncEvent);

  // Create an RTOS queue for message from profile to be sent to app.
  appMsgQueue = Util_constructQueue(&appMsg);

  // Setup the GAP
  GAP_SetParamValue(TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL);

  // Setup the GAP Peripheral Role Profile
  {
    // For all hardware platforms, device starts advertising upon initialization
    uint8_t initialAdvertEnable = TRUE;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16_t advertOffTime = 0;

    uint8_t enableUpdateRequest = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16_t desiredMinInterval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16_t desiredMaxInterval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16_t desiredSlaveLatency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16_t desiredConnTimeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                         &initialAdvertEnable);
    GAPRole_SetParameter(GAPROLE_ADVERT_OFF_TIME, sizeof(uint16_t),
                         &advertOffTime);

    GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData),
                         scanRspData);
    GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);

    GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE, sizeof(uint8_t),
                         &enableUpdateRequest);
    GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16_t),
                         &desiredMinInterval);
    GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16_t),
                         &desiredMaxInterval);
    GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY, sizeof(uint16_t),
                         &desiredSlaveLatency);
    GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER, sizeof(uint16_t),
                         &desiredConnTimeout);
  }

  // Set the GAP Characteristics
  GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName);

  // Set advertising interval
  {
    uint16_t advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, advInt);
    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, advInt);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, advInt);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, advInt);
  }

  // Setup the GAP Bond Manager
  {
    uint8_t pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8_t mitm = TRUE;
    uint8_t ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8_t bonding = TRUE;
    uint8_t scMode = GAPBOND_SECURE_CONNECTION_ALLOW;

    GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8_t), &pairMode);
    GAPBondMgr_SetParameter(GAPBOND_MITM_PROTECTION, sizeof(uint8_t), &mitm);
    GAPBondMgr_SetParameter(GAPBOND_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
    GAPBondMgr_SetParameter(GAPBOND_BONDING_ENABLED, sizeof(uint8_t), &bonding);
    GAPBondMgr_SetParameter(GAPBOND_SECURE_CONNECTION, sizeof(uint8_t), &scMode);
  }

   // Initialize GATT attributes
  GGS_AddService(GATT_ALL_SERVICES);           // GAP
  GATTServApp_AddService(GATT_ALL_SERVICES);   // GATT attributes

  // Add the OAD service to the application
  OAD_open(OAD_DEFAULT_INACTIVITY_TIME);

  // Resiter the OAD callback with the application
  OAD_register(&OadPersistApp_oadCBs);

  // Start Bond Manager
  VOID GAPBondMgr_Register(&OadPersistApp_BondMgrCBs);

  // Start the Device
  VOID GAPRole_StartDevice(&OadPersistApp_gapRoleCBs);

  // Register for GATT local events and ATT Responses pending for transmission
  GATT_RegisterForMsgs(selfEntity);


#ifdef LED_DEBUG
  // Open the LED debug pins
  if (PIN_open(&persistAppLedState, persistAppLedPins))
  {
    for(uint8_t numBlinks = 0; numBlinks < 20; ++numBlinks)
    {
      PIN_Id activeLed;
      if (numBlinks < 12)
      {
        activeLed = Board_LED1;
      }
      else
      {
        activeLed = Board_LED0;
      }

      PIN_setOutputValue(&persistAppLedState, activeLed, !PIN_getOutputValue(activeLed));

      // Sleep for 100ms, sys-tick for BLE-Stack is 10us,
      // Task sleep is in # of ticks
      Task_sleep(10000);

    }

    // Close the pins after using
    PIN_close(&persistAppLedState);
  }

#endif //LED_DEBUG

}

/*********************************************************************
 * @fn      OadPersistApp_taskFxn
 *
 * @brief   Application task entry point for the Simple BLE Peripheral.
 *
 * @param   a0, a1 - not used.
 *
 * @return  None.
 */
static void OadPersistApp_taskFxn(UArg a0, UArg a1)
{
  // Initialize application
  OadPersistApp_init();

  // Application main loop
  for (;;)
  {
    uint32_t events;

    events = Event_pend(syncEvent, Event_Id_NONE, PERSIST_APP_ALL_EVENTS,
                        ICALL_TIMEOUT_FOREVER);

    if (events)
    {
      ICall_EntityID dest;
      ICall_ServiceEnum src;
      ICall_HciExtEvt *pMsg = NULL;

      if (ICall_fetchServiceMsg(&src, &dest,
                                (void **)&pMsg) == ICALL_ERRNO_SUCCESS)
      {

        ICall_Stack_Event *pEvt = (ICall_Stack_Event *)pMsg;

        if ((src == ICALL_SERVICE_CLASS_BLE) && (dest == selfEntity))
        {
          // Check for BLE stack events first
          if(pEvt->signature == 0xffff)
          {
            if (pEvt->event_flag & PERSIST_APP_CONN_EVT_END_EVT)
            {
              if(numPendingMsgs == 0)
              {
                // Reset the system
                HAL_SYSTEM_RESET();
              }
              numPendingMsgs--;
            }
          }
          else
          {
            // Process inter-task message
            OadPersistApp_processStackMsg((ICall_Hdr *)pMsg);
          }

        }

        if (pMsg)
        {
          ICall_freeMsg(pMsg);
        }

      }

      // If RTOS queue is not empty, process app message.
      if (events & PERSIST_APP_QUEUE_EVT)
      {

        // Get the first message from the Queue
        persistAppEvt_t *pMsg = (persistAppEvt_t *)Util_dequeueMsg(appMsgQueue);

        while (pMsg != NULL)
        {
          if (pMsg)
          {
            // Process message.
            OadPersistApp_processAppMsg(pMsg);

            // Free the space from the message.
            ICall_free(pMsg);
          }

          // Dequeue the next message
          pMsg = (persistAppEvt_t *)Util_dequeueMsg(appMsgQueue);
        }
      }

      // OAD events
      if(events & PERSIST_APP_OAD_QUEUE_EVT)
      {
        // Process the OAD Message Queue
        uint8_t status = OAD_processQueue();

        // If the peer has attempted too many image ID
        // it may be an attacker, drop the connection
        if(status == OAD_IMG_ID_TIMEOUT)
        {
          // This may be an attack, terminate the link
          GAPRole_TerminateConnection();

          // If we have not erased the user application
          // jump back, else remain in persist app
          if(OAD_isUsrAppValid())
          {
            // Jump back to user application
            _bim_var = 0x0101;

            // Reset the device
            HAL_SYSTEM_RESET();
          }
        }
      }
      if(events & PERSIST_APP_OAD_COMPLETE_EVT)
      {
        // Register for L2CAP Flow Control Events
        L2CAP_RegisterFlowCtrlTask(selfEntity);
      }
    }
  }
}

/*********************************************************************
 * @fn      OadPersistApp_processStackMsg
 *
 * @brief   Process an incoming stack message.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
static void OadPersistApp_processStackMsg(ICall_Hdr *pMsg)
{
  switch (pMsg->event)
  {
    case GATT_MSG_EVENT:
      // Process GATT message
      OadPersistApp_processGATTMsg((gattMsgEvent_t *)pMsg);
      break;

    case L2CAP_SIGNAL_EVENT:
      // Process L2CAP signal
      OadPersistApp_processL2CAPMsg((l2capSignalEvent_t *)pMsg);
      break;

    default:
      // do nothing
      break;
  }
}

/*********************************************************************
 * @fn      OadPersistApp_processL2CAPMsg
 *
 * @brief   Process L2CAP messages and events.
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static void OadPersistApp_processL2CAPMsg(l2capSignalEvent_t *pMsg)
{
  static bool firstRun = true;

  switch (pMsg->opcode)
  {
    case L2CAP_NUM_CTRL_DATA_PKT_EVT:
    {
      /*
      * We cannot reboot the device immediately after receiving
      * the enable command, we must allow the stack enough time
      * to process and respond to the OAD_EXT_CTRL_ENABLE_IMG
      * command. This command will determine the number of
      * packets currently queued up by the LE controller.
      * BIM var is already set via OadPersistApp_processOadWriteCB
      */
      if(firstRun)
      {
        firstRun = false;

        // We only want to set the numPendingMsgs once
        numPendingMsgs = MAX_NUM_PDU - pMsg->cmd.numCtrlDataPktEvt.numDataPkt;

        // Wait the number of connection events
        HCI_EXT_ConnEventNoticeCmd(OAD_getactiveCxnHandle(), selfEntity,
                                        PERSIST_APP_CONN_EVT_END_EVT);
      }
    }
      break;

    default:
      break;
  }
}

/*********************************************************************
 * @fn      OadPersistApp_processGATTMsg
 *
 * @brief   Process GATT messages
 *
 * @return  None.
 */
static void OadPersistApp_processGATTMsg(gattMsgEvent_t *pMsg)
{
  if (pMsg->method == ATT_MTU_UPDATED_EVENT)
  {
    // Update the OAD Block Size based on MTU
    OAD_setBlockSize(pMsg->msg.mtuEvt.MTU);
  }
  GATT_bm_free(&pMsg->msg, pMsg->method);
}

/*********************************************************************
 * @fn      OadPersistApp_processAppMsg
 *
 * @brief   Process an incoming callback from a profile.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
static void OadPersistApp_processAppMsg(persistAppEvt_t *pMsg)
{
  switch (pMsg->hdr.event)
  {
    case PERSIST_APP_STATE_CHANGE_EVT:
    {
      OadPersistApp_processStateChangeEvt((gaprole_States_t)pMsg->hdr.state);
      break;
    }

    case PERSIST_APP_PASSCODE_NEEDED_EVT:
    {
      OadPersistApp_processPasscode((gapPasskeyNeededEvent_t*)pMsg->pData);
      // Free the app data
      ICall_free(pMsg->pData);
      break;
    }

    default:
      // Do nothing.
      break;
  }
}

/*********************************************************************
 * @fn      OadPersistApp_stateChangeCB
 *
 * @brief   Callback from GAP Role indicating a role state change.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void OadPersistApp_stateChangeCB(gaprole_States_t newState)
{
  OadPersistApp_enqueueMsg(PERSIST_APP_STATE_CHANGE_EVT, newState, NULL);
}

/*********************************************************************
 * @fn      OadPersistApp_processStateChangeEvt
 *
 * @brief   Process a pending GAP Role state change event.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void OadPersistApp_processStateChangeEvt(gaprole_States_t newState)
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      break;

    case GAPROLE_ADVERTISING:
      break;

    case GAPROLE_CONNECTED:
    {
      // Get the active connection handle
      uint16_t connHandle = 0;

      GAPRole_GetParameter(GAPROLE_CONNHANDLE, &connHandle);

      /*
       * In on-chip OAD the GATT table of the device will
       * will have changed between user and persistent apps
       * (OAD reset service vs OAD service)
       *
       * Some mobile applications may cache the service table.
       * The service changed indciation will cause them
       * to trigger service discovery
       *
       * Note: service changed only affects bonded devices.
       * You can force peer devices to pair and bond before
       * starting OAD by OAD_BLE_SECURITY in the
       * preprocessor defines of both the user application and
       * the persistent application.
       */
      GAPBondMgr_ServiceChangeInd( connHandle, TRUE);

      break;
    }

    case GAPROLE_WAITING:
    {
      // Cancel the OAD if one is going on
      // A disonnect forces the peer to re-identify
      OAD_cancel();
      break;
    }

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      break;

    case GAPROLE_ERROR:
      break;

    default:
      break;
  }
}

/*********************************************************************
* @fn      OadPersistApp_processPasscode
*
* @brief   Process the Passcode request.
*
* @return  none
*/
static void OadPersistApp_processPasscode(gapPasskeyNeededEvent_t *pData)
{
  // Use static passcode
  uint32_t passcode = 123456;
  // Send passcode to GAPBondMgr
  GAPBondMgr_PasscodeRsp(pData->connectionHandle, SUCCESS, passcode);
}

/*********************************************************************
 * Callback Functions - These run in the calling thread's context
 *********************************************************************/

/*********************************************************************
* @fn      OadPersistApp_passcodeCB
*
* @brief   Passcode callback.
*
* @param   deviceAddr - pointer to device address
*
* @param   connHandle - the connection handle
*
* @param   uiInputs - pairing User Interface Inputs
*
* @param   uiOutputs - pairing User Interface Outputs
*
* @param   numComparison - numeric Comparison 20 bits
*
* @return  none
*/
static void OadPersistApp_passcodeCB(uint8_t *deviceAddr,
                                            uint16_t connHandle,
                                            uint8_t uiInputs, uint8_t uiOutputs,
                                            uint32_t numComparison)
{
  gapPasskeyNeededEvent_t *pData;

  // Allocate space for the passcode event.
  if ((pData = ICall_malloc(sizeof(gapPasskeyNeededEvent_t))))
  {
    memcpy(pData->deviceAddr, deviceAddr, B_ADDR_LEN);
    pData->connectionHandle = connHandle;
    pData->uiInputs = uiInputs;
    pData->uiOutputs = uiOutputs;
    pData->numComparison = numComparison;

    // Enqueue the event.
    OadPersistApp_enqueueMsg(PERSIST_APP_PASSCODE_NEEDED_EVT, NULL,
                                    (uint8_t *) pData);
  }
}

/*********************************************************************
 * @fn      OadPersistApp_processOadWriteCB
 *
 * @brief   Allows the OAD profile to wake up the app to schedule proces
 *          of OAD packets
 *
 * @param   event - The app event to post. OAD uses two:
 *                  PERSIST_APP_OAD_QUEUE_EVT
 *                  PERSIST_APP_OAD_COMPLETE_EVT
 *
 * @return  None.
 */
static void OadPersistApp_processOadWriteCB(uint8_t event, uint16_t arg)
{
    Event_post(syncEvent, event);
    if(event == PERSIST_APP_OAD_COMPLETE_EVT)
    {
      _bim_var = (uint32_t)(arg);
    }
}

/*********************************************************************
 * @fn      OadPersistApp_enqueueMsg
 *
 * @brief   Creates a message and puts the message in RTOS queue.
 *
 * @param   event  - message event.
 * @param   state - message state.
 *
 * @return  None.
 */
static void OadPersistApp_enqueueMsg(uint8_t event, uint8_t state, uint8_t *pData)
{
  persistAppEvt_t *pMsg;

  // Create dynamic pointer to message.
  if ((pMsg = ICall_malloc(sizeof(persistAppEvt_t))))
  {
    pMsg->hdr.event = event;
    pMsg->hdr.state = state;
    pMsg->pData = pData;

    // Enqueue the message.
    Util_enqueueMsg(appMsgQueue, syncEvent, (uint8*)pMsg);
  }
}

/*********************************************************************
*********************************************************************/

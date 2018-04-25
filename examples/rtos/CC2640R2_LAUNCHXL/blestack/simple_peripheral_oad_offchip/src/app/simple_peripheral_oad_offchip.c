 /******************************************************************************

 @file       simple_peripheral_oad_offchip.c

 @brief This file contains the Oad User sample application  based on
        simple_peripheral for use with the CC2650 Bluetooth Low Energy
        Protocol Stack.

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

#include <icall.h>
#include "util.h"
/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include "devinfoservice.h"
#include "simple_gatt_profile.h"
#include "ll_common.h"

// Used for imgHdr_t structure
#include "oad_image_header.h"
// Needed for HAL_SYSTEM_RESET()
#include "hal_mcu.h"

#include "oad.h"

#include "peripheral.h"

#ifdef USE_RCOSC
#include "rcosc_calibration.h"
#endif //USE_RCOSC

#include "board.h"
#include "board_key.h"

#include "two_btn_menu.h"
#include "simple_peripheral_oad_offchip_menu.h"
#include "simple_peripheral_oad_offchip.h"

#include "ble_user_config.h"


/*********************************************************************
 * CONSTANTS
 */

// Advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 8=10ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     8

// Maximum connection interval (units of 1.25ms, 8=10ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     8

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter
// update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is
// formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         GAPROLE_LINK_PARAM_UPDATE_WAIT_REMOTE_PARAMS

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// How often to perform periodic event (in msec)
#define SBP_PERIODIC_EVT_PERIOD               5000

// Offset into the scanRspData string the software version info is stored
#define OAD_SOFT_VER_OFFSET                   15

// Application specific event ID for HCI Connection Event End Events
#define SBP_HCI_CONN_EVT_END_EVT              0x0001
#define SBP_OAD_CONN_EVT_END_EVT              0x0002

// Use UART display
#define SBP_DISPLAY_TYPE Display_Type_UART

// Task configuration
#define SBP_TASK_PRIORITY                     1

// Warning! To optimize RAM, task stack size must be a multiple of 8 bytes
#ifndef SBP_TASK_STACK_SIZE
#define SBP_TASK_STACK_SIZE                   1000
#endif

// Row numbers
#define SBP_ROW_DEV_ADDR      (TBM_ROW_APP)
#define SBP_ROW_CONN_STATUS   (TBM_ROW_APP + 1)
#define SBP_ROW_SECURITY      (TBM_ROW_APP + 2)
#define SBP_ROW_STATUS1       (TBM_ROW_APP + 3)
#define SBP_ROW_STATUS2       (TBM_ROW_APP + 4)

// Application events used with app queue (appEvtHdr_t)
// These are not related to RTOS evts, but instead enqueued via state change CBs
#define SBP_STATE_CHANGE_EVT                  0x0001
#define SBP_CHAR_CHANGE_EVT                   0x0002
#define SBP_KEY_CHANGE_EVT                    0x0003
#define SBP_PASSCODE_NEEDED_EVT               0x0004

#define SBP_OAD_QUEUE_EVT                     OAD_QUEUE_EVT       // Event_Id_01
#define SBP_OAD_COMPLETE_EVT                  OAD_DL_COMPLETE_EVT // Event_Id_02

// Internal Events for RTOS application
#define SBP_ICALL_EVT                         ICALL_MSG_EVENT_ID // Event_Id_31
#define SBP_QUEUE_EVT                         UTIL_QUEUE_EVENT_ID // Event_Id_30
#define SBP_PERIODIC_EVT                      Event_Id_00
#define SBP_ALL_EVENTS                        (SBP_ICALL_EVT        | \
                                               SBP_QUEUE_EVT        | \
                                               SBP_OAD_QUEUE_EVT    | \
                                               SBP_OAD_COMPLETE_EVT | \
                                               SBP_PERIODIC_EVT)

/*********************************************************************
 * TYPEDEFS
 */

// App event passed from profiles.
typedef struct
{
  appEvtHdr_t hdr;  // event header.
  uint8_t *pData;   // Event payload
} sbpEvt_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Display Interface
Display_Handle dispHandle = NULL;

extern const imgHdr_t _imgHdr;

/*********************************************************************
 * LOCAL VARIABLES
 */

// Entity ID globally used to check for source and/or destination of messages
static ICall_EntityID selfEntity;

// Event globally used to post local events and pend on system and
// local events.
static ICall_SyncHandle syncEvent;

// Clock instances for internal periodic events.
static Clock_Struct periodicClock;

// Queue object used for app messages
static Queue_Struct appMsg;
static Queue_Handle appMsgQueue;

// Task configuration
Task_Struct sbpTask;

#if defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(sbpTaskStack, 8)
#elif __IAR_SYSTEMS_ICC__
#pragma data_alignment=8
#endif    //__TI_COMPILER_VERSION__
uint8_t sbpTaskStack[SBP_TASK_STACK_SIZE];


// GAP - SCAN RSP data (max size = 31 bytes)
static uint8_t scanRspData[] =
{
  // complete name
  0x12,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'S',
  'B',
  'P',
  ' ',
  'O',
  'A',
  'D',
  ' ',
  'A',
  'P',
  'P',
  ' ',
  'v',
  ' ', // These 4 octets are placeholders for the SOFTVER field
  ' ', // which will be updated at init time
  ' ',
  ' ',

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),   // 10ms
  HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
  LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),   // 10ms
  HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertising)
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
  0x05,  // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16(OAD_SERVICE_UUID),
  HI_UINT16(OAD_SERVICE_UUID),
  LO_UINT16(SIMPLEPROFILE_SERV_UUID),
  HI_UINT16(SIMPLEPROFILE_SERV_UUID)
};

// GAP GATT Attributes
static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "SBP OAD off-chip";

// Globals used for ATT Response retransmission
static gattMsgEvent_t *pAttRsp = NULL;
static uint8_t rspTxRetry = 0;

// Variable used to store the number of messages pending once OAD completes
// The application cannot reboot until all pending messages are sent
static uint8_t numPendingMsgs = 0;

// Flag to be stored in NV that tracks whether service changed
// indications needs to be sent out
static uint32_t  sendSvcChngdOnNextBoot = FALSE;

#ifdef LED_DEBUG
// Pin table for LED debug pins
static const PIN_Config sbpLedPins[] = {
    Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};
#endif //LED_DEBUG

// State variable for debugging LEDs
PIN_State  sbpLedState;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SimpleBLEPeripheral_init( void );
static void SimpleBLEPeripheral_taskFxn(UArg a0, UArg a1);

static uint8_t SimpleBLEPeripheral_processStackMsg(ICall_Hdr *pMsg);
static uint8_t SimpleBLEPeripheral_processGATTMsg(gattMsgEvent_t *pMsg);
static void SimpleBLEPeripheral_processAppMsg(sbpEvt_t *pMsg);
static void SimpleBLEPeripheral_processStateChangeEvt(gaprole_States_t newState);
static void SimpleBLEPeripheral_processCharValueChangeEvt(uint8_t paramID);
static void SimpleBLEPeripheral_performPeriodicTask(void);
static void SimpleBLEPeripheral_clockHandler(UArg arg);
static void SimpleBLEPeripheral_sendAttRsp(void);
static void SimpleBLEPeripheral_freeAttRsp(uint8_t status);
static void SimpleBLEPeripheral_stateChangeCB(gaprole_States_t newState);
static void SimpleBLEPeripheral_charValueChangeCB(uint8_t paramID);
static void SimpleBLEPeripheral_enqueueMsg(uint8_t event, uint8_t state, uint8_t *pData);
static void SimpleBLEPeripheral_processOadWriteCB(uint8_t event, uint16_t arg);
static void SimpleBLEPeripheral_keyChangeHandler(uint8_t keys);
static void SimpleBLEPeripheral_handleKeys(uint8_t keys);
static uint8_t SimpleBLEPeripheral_processL2CAPMsg(l2capSignalEvent_t *pMsg);
static void SimpleBLEPeripheral_processPasscode(gapPasskeyNeededEvent_t *pData);
static void SimpleBLEPeripheral_passcodeCB(uint8_t *deviceAddr,
                                            uint16_t connHandle,
                                            uint8_t uiInputs, uint8_t uiOutputs,
                                            uint32_t numComparison);


/*********************************************************************
 * EXTERN FUNCTIONS
 */
extern void AssertHandler(uint8_t assertCause, uint8_t assertSubcause);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t SimpleBLEPeripheral_gapRoleCBs =
{
  SimpleBLEPeripheral_stateChangeCB     // Profile State Change Callbacks
};

// GAP Bond Manager Callbacks
static gapBondCBs_t SimpleBLEPeripheral_BondMgrCBs =
{
  (pfnPasscodeCB_t)SimpleBLEPeripheral_passcodeCB, // Passcode callback,
  NULL  // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t SimpleBLEPeripheral_simpleProfileCBs =
{
  SimpleBLEPeripheral_charValueChangeCB // Characteristic value change callback
};

static oadTargetCBs_t SimpleBLEPeripheral_oadCBs =
{
  SimpleBLEPeripheral_processOadWriteCB // Write Callback.
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEPeripheral_createTask
 *
 * @brief   Task creation function for the OAD User App.
 *
 * @param   None.
 *
 * @return  None.
 */
void SimpleBLEPeripheral_createTask(void)
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = sbpTaskStack;
  taskParams.stackSize = SBP_TASK_STACK_SIZE;
  taskParams.priority = SBP_TASK_PRIORITY;

  Task_construct(&sbpTask, SimpleBLEPeripheral_taskFxn, &taskParams, NULL);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_init
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
static void SimpleBLEPeripheral_init(void)
{
  // ******************************************************************
  // NO STACK API CALLS CAN OCCUR BEFORE THIS CALL TO ICall_registerApp
  // ******************************************************************
  // Register the current thread as an ICall dispatcher application
  // so that the application can send and receive messages.
  ICall_registerApp(&selfEntity, &syncEvent);

#ifdef USE_RCOSC
  RCOSC_enableCalibration();
#endif // USE_RCOSC

  // Create an RTOS queue for message from profile to be sent to app.
  appMsgQueue = Util_constructQueue(&appMsg);

  // Create one-shot clocks for internal periodic events.
  Util_constructClock(&periodicClock, SimpleBLEPeripheral_clockHandler,
                      SBP_PERIODIC_EVT_PERIOD, 0, false, SBP_PERIODIC_EVT);

  dispHandle = Display_open(SBP_DISPLAY_TYPE, NULL);

  // Read in the OAD Software version
  uint8_t swVer[OAD_SW_VER_LEN];
  OAD_getSWVersion(swVer, OAD_SW_VER_LEN);

  Board_initKeys(SimpleBLEPeripheral_keyChangeHandler);

  /**************Init Menu*****************************/
  tbm_setItemStatus(&sbpMenuMain, TBM_ITEM_0 | TBM_ITEM_5,
                    TBM_ITEM_1 | TBM_ITEM_2 | TBM_ITEM_3 | TBM_ITEM_4 );

  // Init two button menu
  tbm_initTwoBtnMenu(dispHandle, &sbpMenuMain, 1, NULL);

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

    // Setup the dyanmic portion of the scanRspData
    scanRspData[OAD_SOFT_VER_OFFSET] = swVer[0];
    scanRspData[OAD_SOFT_VER_OFFSET + 1] = swVer[1];
    scanRspData[OAD_SOFT_VER_OFFSET + 2] = swVer[2];
    scanRspData[OAD_SOFT_VER_OFFSET + 3] = swVer[3];

    // Set scanRspData
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
  DevInfo_AddService();                        // Device Information Service

  SimpleProfile_AddService(GATT_ALL_SERVICES); // Simple GATT Profile


  // Open the OAD module and add the OAD service to the application
  if(OAD_SUCCESS != OAD_open(OAD_DEFAULT_INACTIVITY_TIME))
  {
    Display_print0(dispHandle, 0, 0, "OAD failed to open");
  }
  else
  {
    // Resiter the OAD callback with the application
    OAD_register(&SimpleBLEPeripheral_oadCBs);
  }

  // Setup the SimpleProfile Characteristic Values
  {
    uint8_t charValue1 = 1;
    uint8_t charValue2 = 2;
    uint8_t charValue3 = 3;
    uint8_t charValue4 = 4;
    uint8_t charValue5[SIMPLEPROFILE_CHAR5_LEN] = { 1, 2, 3, 4, 5 };

    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR1, sizeof(uint8_t),
                               &charValue1);
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR2, sizeof(uint8_t),
                               &charValue2);
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, sizeof(uint8_t),
                               &charValue3);
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR4, sizeof(uint8_t),
                               &charValue4);
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN,
                               charValue5);
  }

  // Register callback with SimpleGATTprofile
  SimpleProfile_RegisterAppCBs(&SimpleBLEPeripheral_simpleProfileCBs);

  // Start the Device
  VOID GAPRole_StartDevice(&SimpleBLEPeripheral_gapRoleCBs);

  // Start Bond Manager
  VOID GAPBondMgr_Register(&SimpleBLEPeripheral_BondMgrCBs);

  // Register with GAP for HCI/Host messages
  GAP_RegisterForMsgs(selfEntity);

  // Register for GATT local events and ATT Responses pending for transmission
  GATT_RegisterForMsgs(selfEntity);

#if !defined (USE_LL_CONN_PARAM_UPDATE)
  // Get the currently set local supported LE features
  // The HCI will generate an HCI event that will get received in the main
  // loop
  HCI_LE_ReadLocalSupportedFeaturesCmd();
#endif // !defined (USE_LL_CONN_PARAM_UPDATE)

  uint8_t versionStr[OAD_SW_VER_LEN + 1];

  memcpy(versionStr, swVer, OAD_SW_VER_LEN);

  // Add in Null terminator
  versionStr[OAD_SW_VER_LEN] = NULL;

  // Display Image version
  Display_print1(dispHandle, 0, 0, "SBP Off-chip OAD v%s",
                 versionStr);

#ifdef LED_DEBUG
  // Open the LED debug pins
  if (!PIN_open(&sbpLedState, sbpLedPins))
  {
    Display_print0(dispHandle, 0, 0, "Debug PINs failed to open");
  }
  else
  {
    for(uint8_t numBlinks = 0; numBlinks < 20; ++numBlinks)
    {
      PIN_Id activeLed;
      if (numBlinks < 12)
      {
        activeLed = Board_LED0;
      }
      else
      {
        activeLed = Board_LED1;
      }

      PIN_setOutputValue(&sbpLedState, activeLed, !PIN_getOutputValue(activeLed));

      // Sleep for 100ms, sys-tick for BLE-Stack is 10us,
      // Task sleep is in # of ticks
      Task_sleep(10000);

    }

    // Close the pins after using
    PIN_close(&sbpLedState);
  }
#endif //LED_DEBUG

  /*
   * When switching from persistent app back to the user application for the
   * for the first time after an OAD the device must send a service changed
   * indication. This will cause any peers to rediscover services.
   *
   * To prevent sending a service changed IND on every boot, a flag is stored
   * in NV to determine whether or not the service changed IND needs to be
   * sent
   */
  uint8_t status = osal_snv_read(BLE_NVID_CUST_START,
                                  sizeof(sendSvcChngdOnNextBoot),
                                  (uint8 *)&sendSvcChngdOnNextBoot);
  if(status != SUCCESS)
  {
    /*
     * On first boot the NV item will not have yet been initialzed, and the read
     * will fail. Do a write to set the initial value of the flash in NV
     */
     osal_snv_write(BLE_NVID_CUST_START, sizeof(sendSvcChngdOnNextBoot),
                    (uint8 *)&sendSvcChngdOnNextBoot);
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_taskFxn
 *
 * @brief   Application task entry point for the OAD User App.
 *
 * @param   a0, a1 - not used.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_taskFxn(UArg a0, UArg a1)
{
  // Initialize application
  SimpleBLEPeripheral_init();

  // Application main loop
  for (;;)
  {
    uint32_t events;

    // Waits for an event to be posted associated with the calling thread.
    // Note that an event associated with a thread is posted when a
    // message is queued to the message receive queue of the thread
    events = Event_pend(syncEvent, Event_Id_NONE, SBP_ALL_EVENTS,
                        ICALL_TIMEOUT_FOREVER);

    if (events)
    {
      ICall_EntityID dest;
      ICall_ServiceEnum src;
      ICall_HciExtEvt *pMsg = NULL;

      if (ICall_fetchServiceMsg(&src, &dest,
                                (void **)&pMsg) == ICALL_ERRNO_SUCCESS)
      {
        uint8 safeToDealloc = TRUE;

        if ((src == ICALL_SERVICE_CLASS_BLE) && (dest == selfEntity))
        {
          ICall_Stack_Event *pEvt = (ICall_Stack_Event *)pMsg;

          // Check for BLE stack events first
          if (pEvt->signature == 0xffff)
          {
            if (pEvt->event_flag & SBP_HCI_CONN_EVT_END_EVT)
            {
              // Try to retransmit pending ATT Response (if any)
              SimpleBLEPeripheral_sendAttRsp();
            }
            else if(pEvt->event_flag & SBP_OAD_CONN_EVT_END_EVT)
            {
              // Wait until all pending messages are sent
              if(numPendingMsgs == 0)
              {
                // Store the flag to indicate that a service changed IND will
                // be sent at the next boot
                sendSvcChngdOnNextBoot = TRUE;

                uint8_t status = osal_snv_write(BLE_NVID_CUST_START,
                                                sizeof(sendSvcChngdOnNextBoot),
                                                (uint8 *)&sendSvcChngdOnNextBoot);
                if(status != SUCCESS)
                {
                  Display_print1(dispHandle, 5, 0, "SNV WRITE FAIL: %d", status);
                }

                // Reset the system
                HAL_SYSTEM_RESET();
              }
              numPendingMsgs--;
            }
          }
          else
          {
            // Process inter-task message
            safeToDealloc = SimpleBLEPeripheral_processStackMsg((ICall_Hdr *)pMsg);
          }
        }

        if (pMsg && safeToDealloc)
        {
          ICall_freeMsg(pMsg);
        }
      }

      // If RTOS queue is not empty, process app message.
      if (events & SBP_QUEUE_EVT)
      {

        // Get the first message from the Queue
        sbpEvt_t *pMsg = (sbpEvt_t *)Util_dequeueMsg(appMsgQueue);

        while (pMsg != NULL)
        {
          if (pMsg)
          {
            // Process message.
            SimpleBLEPeripheral_processAppMsg(pMsg);

            if (pMsg->pData != NULL)
            {
              // Free the Queue payload if there is one
              ICall_free(pMsg->pData);
            }

            // Free the space from the message.
            ICall_free(pMsg);
          }

          // Dequeue the next message
          pMsg = (sbpEvt_t *)Util_dequeueMsg(appMsgQueue);
        }
      }
      // OAD events
      if(events & SBP_OAD_QUEUE_EVT)
      {
        // Process the OAD Message Queue
        uint8_t status = OAD_processQueue();

        // If the OAD state machine encountered an error, print it
        // Return codes can be found in oad_constants.h
        if(status == OAD_DL_COMPLETE)
        {
          Display_print0(dispHandle, SBP_ROW_STATUS1, 0, "OAD DL Complete, wait for Enable");
        }
        else if(status == OAD_IMG_ID_TIMEOUT)
        {
          Display_print0(dispHandle, SBP_ROW_STATUS1, 0, "ImgID Timeout, disconnecting");

          // This may be an attack, terminate the link
          GAPRole_TerminateConnection();
        }
        else if(status != OAD_SUCCESS)
        {
          Display_print1(dispHandle, SBP_ROW_STATUS1, 0, "OAD Error: %d", status);
        }

      }
      if(events & SBP_OAD_COMPLETE_EVT)
      {
        // Register for L2CAP Flow Control Events
        L2CAP_RegisterFlowCtrlTask(selfEntity);
      }

      if (events & SBP_PERIODIC_EVT)
      {
        Util_startClock(&periodicClock);

        // Perform periodic application task
        SimpleBLEPeripheral_performPeriodicTask();
      }
    }
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets called
 *          every five seconds (SBP_PERIODIC_EVT_PERIOD). In this example,
 *          the value of the third characteristic in the SimpleGATTProfile
 *          service is retrieved from the profile, and then copied into the
 *          value of the the fourth characteristic.
 *
 * @param   None.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_performPeriodicTask(void)
{
  uint8_t valueToCopy;

  // Call to retrieve the value of the third characteristic in the profile
  if (SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR3, &valueToCopy) == SUCCESS)
  {
    // Call to set that value of the fourth characteristic in the profile.
    // Note that if notifications of the fourth characteristic have been
    // enabled by a GATT client device, then a notification will be sent
    // every time this function is called.
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR4, sizeof(uint8_t),
                               &valueToCopy);
  }
}


/*********************************************************************
 * @fn      SimpleBLEPeripheral_enqueueMsg
 *
 * @brief   Creates a message and puts the message in RTOS queue.
 *
 * @param   event - message event.
 * @param   state - message state.
 * @param   pData - message payload.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_enqueueMsg(uint8_t event, uint8_t state, uint8_t *pData)
{
  sbpEvt_t *pMsg;

  // Create dynamic pointer to message.
  if ((pMsg = ICall_malloc(sizeof(sbpEvt_t))))
  {
    pMsg->hdr.event = event;
    pMsg->hdr.state = state;
    pMsg->pData = pData;

    // Enqueue the message.
    Util_enqueueMsg(appMsgQueue, syncEvent, (uint8*)pMsg);
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processStackMsg
 *
 * @brief   Process an incoming stack message.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static uint8_t SimpleBLEPeripheral_processStackMsg(ICall_Hdr *pMsg)
{
  uint8_t safeToDealloc = TRUE;

  switch (pMsg->event)
  {
    case GATT_MSG_EVENT:
      // Process GATT message
      safeToDealloc = SimpleBLEPeripheral_processGATTMsg((gattMsgEvent_t *)pMsg);
      break;

    case HCI_GAP_EVENT_EVENT:
      {

        // Process HCI message
        switch(pMsg->status)
        {
          case HCI_COMMAND_COMPLETE_EVENT_CODE:
            // Process HCI Command Complete Event
            {

#if !defined (USE_LL_CONN_PARAM_UPDATE)
              // This code will disable the use of the LL_CONNECTION_PARAM_REQ
              // control procedure (for connection parameter updates, the
              // L2CAP Connection Parameter Update procedure will be used
              // instead). To re-enable the LL_CONNECTION_PARAM_REQ control
              // procedures, define the symbol USE_LL_CONN_PARAM_UPDATE

              // Parse Command Complete Event for opcode and status
              hciEvt_CmdComplete_t* command_complete = (hciEvt_CmdComplete_t*) pMsg;
              uint8_t   pktStatus = command_complete->pReturnParam[0];

              //find which command this command complete is for
              switch (command_complete->cmdOpcode)
              {
                case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES:
                  {
                    if (pktStatus == SUCCESS)
                    {
                      uint8_t featSet[8];

                      // get current feature set from received event (bits 1-9 of
                      // the returned data
                      memcpy( featSet, &command_complete->pReturnParam[1], 8 );

                      // Clear bit 1 of byte 0 of feature set to disable LL
                      // Connection Parameter Updates
                      CLR_FEATURE_FLAG( featSet[0], LL_FEATURE_CONN_PARAMS_REQ );

                      // Update controller with modified features
                      HCI_EXT_SetLocalSupportedFeaturesCmd( featSet );
                    }
                  }
                  break;

                default:
                  //do nothing
                  break;
              }
#endif // !defined (USE_LL_CONN_PARAM_UPDATE)

            }
            break;

          case HCI_BLE_HARDWARE_ERROR_EVENT_CODE:
            AssertHandler(HAL_ASSERT_CAUSE_HARDWARE_ERROR,0);
            break;

          default:
            break;
        }
      }
      break;

    case L2CAP_SIGNAL_EVENT:
      // Process L2CAP signal
      safeToDealloc = SimpleBLEPeripheral_processL2CAPMsg((l2capSignalEvent_t *)pMsg);
      break;

      default:
        // do nothing
        break;

    }

  return (safeToDealloc);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processL2CAPMsg
 *
 * @brief   Process L2CAP messages and events.
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static uint8_t SimpleBLEPeripheral_processL2CAPMsg(l2capSignalEvent_t *pMsg)
{
  uint8_t safeToDealloc = TRUE;
  static bool firstRun = TRUE;

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
                                        SBP_OAD_CONN_EVT_END_EVT);
      }

      break;
    }
    default:
      break;
  }

  // It's safe to free the incoming message
  return (safeToDealloc);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processGATTMsg
 *
 * @brief   Process GATT messages and events.
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static uint8_t SimpleBLEPeripheral_processGATTMsg(gattMsgEvent_t *pMsg)
{
  // See if GATT server was unable to transmit an ATT response
  if (pMsg->hdr.status == blePending)
  {
    // No HCI buffer was available. Let's try to retransmit the response
    // on the next connection event.
    if (HCI_EXT_ConnEventNoticeCmd(pMsg->connHandle, selfEntity,
                                   SBP_HCI_CONN_EVT_END_EVT) == SUCCESS)
    {
      // First free any pending response
      SimpleBLEPeripheral_freeAttRsp(FAILURE);

      // Hold on to the response message for retransmission
      pAttRsp = pMsg;

      // Don't free the response message yet
      return (FALSE);
    }
  }
  else if (pMsg->method == ATT_FLOW_CTRL_VIOLATED_EVENT)
  {
    // ATT request-response or indication-confirmation flow control is
    // violated. All subsequent ATT requests or indications will be dropped.
    // The app is informed in case it wants to drop the connection.

    // Display the opcode of the message that caused the violation.
    Display_print1(dispHandle, SBP_ROW_STATUS2, 0, "FC Violated: %d", pMsg->msg.flowCtrlEvt.opcode);
  }
  else if (pMsg->method == ATT_MTU_UPDATED_EVENT)
  {
    OAD_setBlockSize(pMsg->msg.mtuEvt.MTU);
    Display_print1(dispHandle, SBP_ROW_STATUS2, 0, "MTU Size: %d", pMsg->msg.mtuEvt.MTU);
  }

  // Free message payload. Needed only for ATT Protocol messages
  GATT_bm_free(&pMsg->msg, pMsg->method);

  // It's safe to free the incoming message
  return (TRUE);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_sendAttRsp
 *
 * @brief   Send a pending ATT response message.
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleBLEPeripheral_sendAttRsp(void)
{
  // See if there's a pending ATT Response to be transmitted
  if (pAttRsp != NULL)
  {
    uint8_t status;

    // Increment retransmission count
    rspTxRetry++;

    // Try to retransmit ATT response till either we're successful or
    // the ATT Client times out (after 30s) and drops the connection.
    status = GATT_SendRsp(pAttRsp->connHandle, pAttRsp->method, &(pAttRsp->msg));
    if ((status != blePending) && (status != MSG_BUFFER_NOT_AVAIL))
    {
      // Disable connection event end notice
      HCI_EXT_ConnEventNoticeCmd(pAttRsp->connHandle, selfEntity, 0);

      // We're done with the response message
      SimpleBLEPeripheral_freeAttRsp(status);
    }
    else
    {
      // Continue retrying
      Display_print1(dispHandle, SBP_ROW_STATUS2, 0, "Rsp send retry: %d", rspTxRetry);
    }
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_freeAttRsp
 *
 * @brief   Free ATT response message.
 *
 * @param   status - response transmit status
 *
 * @return  none
 */
static void SimpleBLEPeripheral_freeAttRsp(uint8_t status)
{
  // See if there's a pending ATT response message
  if (pAttRsp != NULL)
  {
    // See if the response was sent out successfully
    if (status == SUCCESS)
    {
      Display_print1(dispHandle, SBP_ROW_STATUS2, 0, "Rsp sent retry: %d", rspTxRetry);
    }
    else
    {
      // Free response payload
      GATT_bm_free(&pAttRsp->msg, pAttRsp->method);

      Display_print1(dispHandle, SBP_ROW_STATUS2, 0, "Rsp retry failed: %d", rspTxRetry);
    }

    // Free response message
    ICall_freeMsg(pAttRsp);

    // Reset our globals
    pAttRsp = NULL;
    rspTxRetry = 0;
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processAppMsg
 *
 * @brief   Process an incoming callback from a profile.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_processAppMsg(sbpEvt_t *pMsg)
{
  switch (pMsg->hdr.event)
  {
    case SBP_STATE_CHANGE_EVT:
    {
      SimpleBLEPeripheral_processStateChangeEvt((gaprole_States_t)pMsg->
                                                hdr.state);
      break;
    }

    case SBP_CHAR_CHANGE_EVT:
    {
      SimpleBLEPeripheral_processCharValueChangeEvt(pMsg->hdr.state);

      break;
    }

    case SBP_KEY_CHANGE_EVT:
    {
      SimpleBLEPeripheral_handleKeys(pMsg->hdr.state);
      break;
    }

    case SBP_PASSCODE_NEEDED_EVT:
    {
      SimpleBLEPeripheral_processPasscode((gapPasskeyNeededEvent_t*)pMsg->pData);
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
 * @fn      SimpleBLEPeripheral_processStateChangeEvt
 *
 * @brief   Process a pending GAP Role state change event.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_processStateChangeEvt(gaprole_States_t newState)
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8_t ownAddress[B_ADDR_LEN];
        uint8_t systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

        // Display device address
        Display_print1(dispHandle, SBP_ROW_DEV_ADDR, 0, "BD Addr: %s",
                       Util_convertBdAddr2Str(ownAddress));
        Display_print0(dispHandle, SBP_ROW_STATUS1, 0, "GAPRole Initialized");
      }
      break;

    case GAPROLE_ADVERTISING:
      Display_print0(dispHandle, SBP_ROW_CONN_STATUS, 0, "Advertising");
      break;

    case GAPROLE_CONNECTED:
      {
        linkDBInfo_t linkInfo;
        uint8_t numActive = 0;
        uint16_t connHandle = 0;

        GAPRole_GetParameter(GAPROLE_CONNHANDLE, &connHandle);

        Util_startClock(&periodicClock);

        numActive = linkDB_NumActive();

        // Use numActive to determine the connection handle of the last
        // connection
        if ( linkDB_GetInfo( numActive - 1, &linkInfo ) == SUCCESS )
        {
          Display_print1(dispHandle, SBP_ROW_CONN_STATUS, 0,
                          "Connected to: %s", Util_convertBdAddr2Str(linkInfo.addr));
        }
        else
        {
          uint8_t peerAddress[B_ADDR_LEN];

          GAPRole_GetParameter(GAPROLE_CONN_BD_ADDR, peerAddress);

          Display_print1(dispHandle, SBP_ROW_CONN_STATUS, 0,
                          "Connected to : %s", Util_convertBdAddr2Str(peerAddress));
        }

        if(sendSvcChngdOnNextBoot == TRUE)
        {
          GAPBondMgr_ServiceChangeInd( connHandle, TRUE);

          sendSvcChngdOnNextBoot = FALSE;
        }
      }
      break;

    case GAPROLE_CONNECTED_ADV:
      Display_print0(dispHandle, SBP_ROW_STATUS1, 0, "Connected Advertising");
      break;

    case GAPROLE_WAITING:
      Util_stopClock(&periodicClock);
      SimpleBLEPeripheral_freeAttRsp(bleNotConnected);

      Display_print0(dispHandle, SBP_ROW_CONN_STATUS, 0, "Disconnected");

      // Cancel the OAD if one is going on
      // A disconnect forces the peer to re-identify
      OAD_cancel();

      // Clear remaining lines
      Display_clearLines(dispHandle, SBP_ROW_CONN_STATUS, 5);
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      SimpleBLEPeripheral_freeAttRsp(bleNotConnected);

      Display_print0(dispHandle, SBP_ROW_STATUS1, 0, "Timed Out");

      // Clear remaining lines
      Display_clearLines(dispHandle, SBP_ROW_CONN_STATUS, 5);

      break;

    case GAPROLE_ERROR:
      Display_print0(dispHandle, SBP_ROW_STATUS1, 0, "Error");
      break;

    default:
      Display_clearLine(dispHandle, SBP_ROW_STATUS1);
      break;
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processCharValueChangeEvt
 *
 * @brief   Process a pending Simple Profile characteristic value change
 *          event.
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_processCharValueChangeEvt(uint8_t paramID)
{
  uint8_t newValue;

  switch(paramID)
  {
    case SIMPLEPROFILE_CHAR1:
      SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR1, &newValue);

      Display_print1(dispHandle, SBP_ROW_STATUS2, 0, "Char 1: %d", (uint16_t)newValue);
      break;

    case SIMPLEPROFILE_CHAR3:
      SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR3, &newValue);

      Display_print1(dispHandle, SBP_ROW_STATUS2, 0, "Char 3: %d", (uint16_t)newValue);
      break;

    default:
      // should not reach here!
      break;
  }
}

/*********************************************************************
* @fn      SimpleBLEPeripheral_processPasscode
*
* @brief   Process the Passcode request.
*
* @return  none
*/
static void SimpleBLEPeripheral_processPasscode(gapPasskeyNeededEvent_t *pData)
{
  // Use static passcode
  uint32_t passcode = 123456;
  Display_print1(dispHandle, SBP_ROW_SECURITY, 0, "Passcode: %d", passcode);
  // Send passcode to GAPBondMgr
  GAPBondMgr_PasscodeRsp(pData->connectionHandle, SUCCESS, passcode);
}

/*********************************************************************
* @fn      SimpleBLEPeripheral_handleKeys
*
* @brief   Handles all key events for this device.
*
* @param   keys - bit field for key events. Valid entries:
*                 HAL_KEY_SW_2
*                 HAL_KEY_SW_1
*
* @return  none
*/
static void SimpleBLEPeripheral_handleKeys(uint8_t keys)
{
  if (keys & KEY_LEFT)
  {
    // Check if the key is still pressed
    if (PIN_getInputValue(Board_BUTTON0) == 0)
    {
      tbm_buttonLeft();
    }
  }
  else if (keys & KEY_RIGHT)
  {
    // Check if the key is still pressed
    if (PIN_getInputValue(Board_BUTTON1) == 0)
    {
      tbm_buttonRight();
    }
  }
}

/*********************************************************************
 * Callback Functions - These run in the calling thread's context
 *********************************************************************/

 /*********************************************************************
* @fn      SimpleBLEPeripheral_passcodeCB
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
static void SimpleBLEPeripheral_passcodeCB(uint8_t *deviceAddr,
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
    SimpleBLEPeripheral_enqueueMsg(SBP_PASSCODE_NEEDED_EVT, NULL,
                                    (uint8_t *) pData);
  }
}


/*********************************************************************
* @fn      SimpleBLEPeripheral_keyChangeHandler
*
* @brief   Key event handler function
*
* @param   keys - bitmask of keys pressed
*
* @return  none
*/
static void SimpleBLEPeripheral_keyChangeHandler(uint8_t keys)
{
  SimpleBLEPeripheral_enqueueMsg(SBP_KEY_CHANGE_EVT, keys, NULL);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_stateChangeCB
 *
 * @brief   Callback from GAP Role indicating a role state change.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_stateChangeCB(gaprole_States_t newState)
{
  SimpleBLEPeripheral_enqueueMsg(SBP_STATE_CHANGE_EVT, newState, NULL);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_charValueChangeCB
 *
 * @brief   Callback from Simple Profile indicating a characteristic
 *          value change.
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_charValueChangeCB(uint8_t paramID)
{
  SimpleBLEPeripheral_enqueueMsg(SBP_CHAR_CHANGE_EVT, paramID, NULL);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processOadWriteCB
 *
 * @brief   Process a write request to the OAD reset service
 *
 * @param   connHandle - the connection Handle this request is from.
 * @param   bim_var    - bim_var to set before resetting.
 *
 * @return  None.
 */
void SimpleBLEPeripheral_processOadWriteCB(uint8_t event, uint16_t arg)
{
  Event_post(syncEvent, event);
}

/*********************************************************************
 * SWI Functions - These functions run at higher priority than any task
 *********************************************************************/

/*********************************************************************
 * @fn      SimpleBLEPeripheral_clockHandler
 *
 * @brief   Handler function for clock timeouts.
 *
 * @param   arg - event type
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_clockHandler(UArg arg)
{
  // Wake up the application.
  Event_post(syncEvent, arg);
}

/*********************************************************************
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdlib.h>

#include <ti/display/Display.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/BIOS.h>

#include "bcomdef.h"

#include "board_key.h"
#include "board.h"

// DriverLib
#include <driverlib/aon_batmon.h>

#include <urfc.h>
#include <uble.h>
#include <ugap.h>

#include <util.h>
#ifdef USE_ICALL
  #include <icall.h>
#endif /* USE_ICALL */

#include <gap.h>
#include "micro_cm_demo.h"
#include "micro_ble_cm.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Type of Display to open
#if !defined(Display_DISABLE_ALL)
  #if defined(BOARD_DISPLAY_USE_LCD) && (BOARD_DISPLAY_USE_LCD!=0)
    #define UCD_DISPLAY_TYPE Display_Type_LCD
  #elif defined (BOARD_DISPLAY_USE_UART) && (BOARD_DISPLAY_USE_UART!=0)
    #define UCD_DISPLAY_TYPE Display_Type_UART
  #else // !BOARD_DISPLAY_USE_LCD && !BOARD_DISPLAY_USE_UART
    #define UCD_DISPLAY_TYPE 0 // Option not supported
  #endif // BOARD_DISPLAY_USE_LCD && BOARD_DISPLAY_USE_UART
#else // BOARD_DISPLAY_USE_LCD && BOARD_DISPLAY_USE_UART
  #define UCD_DISPLAY_TYPE 0 // No Display
#endif // Display_DISABLE_ALL

// Task configuration
#define UCD_TASK_PRIORITY                     3

#ifndef UCD_TASK_STACK_SIZE
#define UCD_TASK_STACK_SIZE                   600
#endif

// Internal Events for RTOS application
#ifdef USE_ICALL
#define UCD_ICALL_EVT                           ICALL_MSG_EVENT_ID // Event_Id_31
#else /* !USE_ICALL */
#define UCD_ICALL_EVT                           Event_Id_NONE
#endif /* USE_ICALL */
#define UCD_QUEUE_EVT                           UTIL_QUEUE_EVENT_ID // Event_Id_30

// Application Events
#define UCD_EVT_MICROSTACK   0x0001

// Row numbers
#define UCD_ROW_MONITOR_STATE   3
#define UCD_ROW_MONITOR_1       4
#define UCD_ROW_MONITOR_2       5
#define UCD_ROW_MONITOR_3       6

/*********************************************************************
 * TYPEDEFS
 */

// App to App event
typedef struct {
  uint16 event;
  uint8 data;
} ucdEvt_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Display Interface
Display_Handle dispHandle = NULL;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern bool ubCm_init(void);

/*********************************************************************
 * LOCAL VARIABLES
 */

#ifdef USE_ICALL
// Event globally used to post local events and pend on local events.
static ICall_SyncHandle syncEvent;
#else
// Event globally used to post local events and pend on local events.
static Event_Handle syncEvent;
#endif /* USE_ICALL */

// Queue object used for app messages
static Queue_Struct appMsg;
static Queue_Handle appMsgQueue;

// Task configuration
Task_Struct ucdTask;
uint8 ucdTaskStack[UCD_TASK_STACK_SIZE];

// Monitor complete callback count
static uint32_t ucdNumMonitorComplete;


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void MicroCmDemo_init(void);
static void MicroCmDemo_taskFxn(UArg a0, UArg a1);

static void MicroCmDemo_processAppMsg(ucdEvt_t *pMsg);

void MicroCmDemo_monitorStateChangeCB(ugapMonitorState_t newState);
void MicroCmDemo_monitorIndicationCB(bStatus_t status, uint8_t sessionId,
                                     uint8_t len, uint8_t *pPayload);
void MicroCmDemo_monitorCompleteCB(bStatus_t status, uint8_t sessionId);

static bStatus_t MicroCmDemo_enqueueMsg(uint16 event, uint8 data);

static void uBLEStack_eventProxy(void);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      MicroCmDemo_createTask
 *
 * @brief   Task creation function for the Micro Eddystone Beacon.
 *
 * @param   None.
 *
 * @return  None.
 */
void MicroCmDemo_createTask(void)
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = ucdTaskStack;
  taskParams.stackSize = UCD_TASK_STACK_SIZE;
  taskParams.priority = UCD_TASK_PRIORITY;

  Task_construct(&ucdTask, MicroCmDemo_taskFxn, &taskParams, NULL);
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      MicroCmDemo_init
 *
 * @brief   Initialization function for the Connection Monitor Demo App
 *          Task. This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notification ...).
 *
 * @param   none
 *
 * @return  none
 */
static void MicroCmDemo_init(void)
{

  // Create an RTOS event used to wake up this application to process events.
  syncEvent = Event_create(NULL, NULL);

  // Create an RTOS queue for message from profile to be sent to app.
  appMsgQueue = Util_constructQueue(&appMsg);

  // Open Display.
  dispHandle = Display_open(UCD_DISPLAY_TYPE, NULL);

  /* Initilaize Connection Monitor */
  if (ubCm_init() == true)
  {
    /*
     * The ubCM_Start() is called here to kick off CM sessions as a demo.
     * This API should be called from host uNPI once the uNPI host
     * if available. The currentStartTime, access address, and scan duration
     * are hard-coded. They should be provided by the host. In this demo, the
     * very first ubCM_start() call will cover UB_MAX_MONITOR_DURATION.
     */
    ubCMConnInfo.ArrayOfConnInfo[0].accessAddr = 0x694dc42a;
    ubCMConnInfo.ArrayOfConnInfo[0].currentStartTime = 0;
    ubCMConnInfo.ArrayOfConnInfo[0].nextStartTime = RF_getCurrentTime() + 100 * BLE_TO_RAT;
    ubCMConnInfo.ArrayOfConnInfo[0].scanDuration = UBLE_MAX_MONITOR_DURATION; // 40.96s
    ubCMConnInfo.ArrayOfConnInfo[0].connInterval = 1600;  // 1s
    ubCMConnInfo.ArrayOfConnInfo[0].hopValue = 0x05;
    ubCMConnInfo.ArrayOfConnInfo[0].combSCA = 90; // Master+Slave = 50+40
    ubCMConnInfo.ArrayOfConnInfo[0].chanMap[0] = 0xFF;
    ubCMConnInfo.ArrayOfConnInfo[0].chanMap[1] = 0xFF;
    ubCMConnInfo.ArrayOfConnInfo[0].chanMap[2] = 0xFF;
    ubCMConnInfo.ArrayOfConnInfo[0].chanMap[3] = 0xFF;
    ubCMConnInfo.ArrayOfConnInfo[0].chanMap[4] = 0x1F;
    if (ubCM_start(1) == CM_SUCCESS)
    {
      Display_print0(dispHandle, 0, 0,
                     "Micro Connection Monitor Started. Wait for 40 seconds...");
      Display_print1(dispHandle, 1, 0, "Access Address: 0x%x",
                     ubCMConnInfo.ArrayOfConnInfo[0].accessAddr);
      Display_print1(dispHandle, 2, 0, "Connection Interval: %d",
                     ubCMConnInfo.ArrayOfConnInfo[0].connInterval);
    }
    else
    {
      Display_print0(dispHandle, 0, 0, "Monitor init failed");
    }
  }
}


/*********************************************************************
 * @fn      MicroCmDemo_taskFxn
 *
 * @brief   Application task entry point for the Micro Connection
 *          Monitor Demo.
 *
 * @param   none
 *
 * @return  none
 */
static void MicroCmDemo_taskFxn(UArg a0, UArg a1)
{
  volatile uint32 keySwi;

  uble_stackInit(UBLE_ADDRTYPE_PUBLIC, NULL, uBLEStack_eventProxy,
                 RF_TIME_CRITICAL);

  // Initialize application
  MicroCmDemo_init();

  for (;;)
  {
    // Waits for an event to be posted associated with the calling thread.
    // Note that an event associated with a thread is posted when a
    // message is queued to the message receive queue of the thread
    Event_pend(syncEvent, Event_Id_NONE, UCD_QUEUE_EVT, BIOS_WAIT_FOREVER);

    // If RTOS queue is not empty, process app message.
    while (!Queue_empty(appMsgQueue))
    {
      ucdEvt_t *pMsg;

      // malloc() is not thread safe. Must disable SWI.
      keySwi = Swi_disable();
      pMsg = (ucdEvt_t *) Util_dequeueMsg(appMsgQueue);
      Swi_restore(keySwi);

      if (pMsg)
      {
        // Process message.
        MicroCmDemo_processAppMsg(pMsg);

        // free() is not thread safe. Must disable SWI.
        keySwi = Swi_disable();

        // Free the space from the message.
        free(pMsg);
        Swi_restore(keySwi);
      }
    }
  }
}


/*********************************************************************
 * @fn      MicroCmDemo_processAppMsg
 *
 * @brief   Process an incoming callback from a profile.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
static void MicroCmDemo_processAppMsg(ucdEvt_t *pMsg)
{
  switch (pMsg->event)
  {
  case UCD_EVT_MICROSTACK:
    uble_processMsg();
    break;

  default:
    // Do nothing.
    break;
  }
}

/*********************************************************************
 * @fn      MicroCmDemo_enqueueMsg
 *
 * @brief   Creates a message and puts the message in RTOS queue.
 *
 * @param   event - message event.
 * @param   data - message data.
 *
 * @return  TRUE or FALSE
 */
static bStatus_t MicroCmDemo_enqueueMsg(uint16 event, uint8 data)
{
  volatile uint32 keySwi;
  ucdEvt_t *pMsg;
  uint8_t status = FALSE;

  // malloc() is not thread safe. Must disable SWI.
  keySwi = Swi_disable();

  // Create dynamic pointer to message.
  pMsg = (ucdEvt_t*) malloc(sizeof(ucdEvt_t));
  if (pMsg != NULL)
  {
    pMsg->event = event;
    pMsg->data = data;

    // Enqueue the message.
    status = Util_enqueueMsg(appMsgQueue, syncEvent, (uint8*) pMsg);
  }
  Swi_restore(keySwi);
  return status;
}

/*********************************************************************
 * @fn      uBLEStack_eventProxy
 *
 * @brief   Post an event to the application so that a Micro BLE Stack internal
 *          event is processed by Micro BLE Stack later in the application
 *          task's context.
 *
 * @param   None
 */
void uBLEStack_eventProxy(void)
{
  if (MicroCmDemo_enqueueMsg(UCD_EVT_MICROSTACK, 0) == FALSE)
  {
    /* post event anyway when heap is out to avoid malloc error */
    Event_post(syncEvent, UTIL_QUEUE_EVENT_ID);
  }
}


/* *******************************************************************
 *                  Connection Monitor Demo functions
 * *******************************************************************/

/*********************************************************************
 * @fn      MicroCmDemo_monitorStateChangeCB
 *
 * @brief   Callback from Micro Monitor indicating a state change.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
void MicroCmDemo_monitorStateChangeCB(ugapMonitorState_t newState)
{
  switch (newState)
  {
  case UGAP_MONITOR_STATE_INITIALIZED:
    Display_print0(dispHandle, UCD_ROW_MONITOR_STATE, 0,
                   "uM_State: Initialized");
    break;

  case UGAP_MONITOR_STATE_IDLE:
    Display_print0(dispHandle, UCD_ROW_MONITOR_STATE, 0,
                   "uM_State: Idle");
    break;

  case UGAP_MONITOR_STATE_MONITORING:
    Display_print0(dispHandle, UCD_ROW_MONITOR_STATE, 0,
                   "uM_State: Monitoring");
    break;

  default:
    break;
  }
}

/*********************************************************************
 * @fn      MicroCmDemo_monitorIndicationCB
 *
 * @brief   Callback from Micro monitor notifying that a advertising
 *          packet is received.
 *
 * @param   status status of a monitoring scan
 * @param   sessionId session ID
 * @param   len length of the payload
 * @param   pPayload pointer to payload
 *
 * @return  None.
 */
void MicroCmDemo_monitorIndicationCB(bStatus_t status, uint8_t sessionId,
                                     uint8_t len, uint8_t *pPayload)
{
  static int8   rssi;
  static uint8  chan;
  static uint32 timeStamp;

  /* We have a packet (dvertisment packek as an example):
   *
   * | Preamble  | Access Addr | PDU         | CRC     |
   * | 1-2 bytes | 4 bytes     | 2-257 bytes | 3 bytes |
   *
   * The PDU is expended to:
   * | Header  | Payload     |
   * | 2 bytes | 1-255 bytes |
   *
   * The Header is expended to:
   * | PDU Type...RxAdd | Length |
   * | 1 byte           | 1 byte |
   *
   * The Payload is expended to:
   * | AdvA    | AdvData    |
   * | 6 bytes | 0-31 bytes |
   *
   * The radio stripps the CRC and replaces it with the postfix.
   *
   * The Postfix is expended to:
   * | RSSI   | Status | TimeStamp |
   * | 1 byte | 1 byte | 4 bytes   |
   *
   * The Status is expended to:
   * | bCrcErr | bIgnore | channel  |
   * | bit 7   | bit 6   | bit 5..0 |
   *
   * Note the advPke is the beginning of PDU; the dataLen includes
   * the postfix length.
   *
   */
  chan = (*(pPayload + len - 5) & 0x3F);
  rssi = *(pPayload + len - 6);
  timeStamp = *(uint32 *)(pPayload + len - 4);
  if (status == 1)
  {
    Display_print4(dispHandle, UCD_ROW_MONITOR_1, 0,
                   "Master SessionId: %d Chan: %d RSSI: %d timeStamp: %ul",
                   sessionId, chan, rssi, timeStamp);
  }
  else
  {
    Display_print4(dispHandle, UCD_ROW_MONITOR_2, 0,
                   "Slave  SessionId: %d Chan: %d RSSI: %d timeStamp: %ul",
                   sessionId, chan, rssi, timeStamp);
  }
}

/*********************************************************************
 * @fn      MicroCmDemo_monitorCompleteCB
 *
 * @brief   Callback from Micro Monitor notifying that a monitoring
 *          scan is completed.
 *
 * @param   status - How the last event was done. SUCCESS or FAILURE.
 * @param   sessionId - Session ID
 *
 * @return  None.
 */
void MicroCmDemo_monitorCompleteCB (bStatus_t status, uint8_t sessionId)
{
  ucdNumMonitorComplete++;
  Display_print3(dispHandle, UCD_ROW_MONITOR_3, 0,
                 "%d Monitor duration\'s done. sessionId: %d, Status: %d",
                 ucdNumMonitorComplete, sessionId, status);
}

/*********************************************************************
 *********************************************************************/

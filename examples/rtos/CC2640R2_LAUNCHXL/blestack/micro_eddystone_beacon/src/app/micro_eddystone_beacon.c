/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdlib.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/display/Display.h>

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
#include "micro_eddystone_beacon.h"

/*********************************************************************
 * MACROS
 */

// Eddystone Base 128-bit UUID: EE0CXXXX-8786-40BA-AB96-99B91AC981D8
#define EDDYSTONE_BASE_UUID_128( uuid )  0xD8, 0x81, 0xC9, 0x1A, 0xB9, 0x99, \
                                         0x96, 0xAB, 0xBA, 0x40, 0x86, 0x87, \
                           LO_UINT16( uuid ), HI_UINT16( uuid ), 0x0C, 0xEE

/*********************************************************************
 * CONSTANTS
 */

// Advertising interval (units of 0.625 millisec)
#define DEFAULT_ADVERTISING_INTERVAL          800 // 500 ms

// Type of Display to open
#if !defined(Display_DISABLE_ALL)
  #if defined(BOARD_DISPLAY_USE_LCD) && (BOARD_DISPLAY_USE_LCD!=0)
    #define UEB_DISPLAY_TYPE Display_Type_LCD
  #elif defined (BOARD_DISPLAY_USE_UART) && (BOARD_DISPLAY_USE_UART!=0)
    #define UEB_DISPLAY_TYPE Display_Type_UART
  #else // !BOARD_DISPLAY_USE_LCD && !BOARD_DISPLAY_USE_UART
    #define UEB_DISPLAY_TYPE 0 // Option not supported
  #endif // BOARD_DISPLAY_USE_LCD && BOARD_DISPLAY_USE_UART
#else // BOARD_DISPLAY_USE_LCD && BOARD_DISPLAY_USE_UART
  #define UEB_DISPLAY_TYPE 0 // No Display
#endif // Display_DISABLE_ALL

// Task configuration
#define UEB_TASK_PRIORITY                     1

#ifndef UEB_TASK_STACK_SIZE
#define UEB_TASK_STACK_SIZE                   600
#endif

// Internal Events for RTOS application
#ifdef USE_ICALL
#define UEB_ICALL_EVT                           ICALL_MSG_EVENT_ID // Event_Id_31
#else /* !USE_ICALL */
#define UEB_ICALL_EVT                           Event_Id_NONE
#endif /* USE_ICALL */
#define UEB_QUEUE_EVT                           UTIL_QUEUE_EVENT_ID // Event_Id_30
#define UEB_EVT_KEY_CHANGE                      0x0001
#define UEB_EVT_MICROBLESTACK                   0x0002

#define UEB_ALL_EVENTS                          (UEB_ICALL_EVT     | \
                                                UEB_QUEUE_EVT      | \
                                                UEB_EVT_KEY_CHANGE | \
                                                UEB_EVT_MICROBLESTACK)

// Pre-generated Random Static Address
#define UEB_PREGEN_RAND_STATIC_ADDR    {0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc}

// Eddystone definitions
#define EDDYSTONE_SERVICE_UUID                  0xFEAA

#define EDDYSTONE_FRAME_TYPE_UID                0x00
#define EDDYSTONE_FRAME_TYPE_URL                0x10
#define EDDYSTONE_FRAME_TYPE_TLM                0x20

#define EDDYSTONE_FRAME_OVERHEAD_LEN            8
#define EDDYSTONE_SVC_DATA_OVERHEAD_LEN         3
#define EDDYSTONE_MAX_URL_LEN                   18

// # of URL Scheme Prefix types
#define EDDYSTONE_URL_PREFIX_MAX        4
// # of encodable URL words
#define EDDYSTONE_URL_ENCODING_MAX      14

#define EDDYSTONE_URI_DATA_DEFAULT      "http://www.ti.com/ble"

/*********************************************************************
 * TYPEDEFS
 */

// App to App event
typedef struct {
  uint16 event;
  uint8 data;
} uebEvt_t;

// Eddystone UID frame
typedef struct {
  uint8 frameType;      // UID
  int8 rangingData;
  uint8 namespaceID[10];
  uint8 instanceID[6];
  uint8 reserved[2];
} eddystoneUID_t;

// Eddystone URL frame
typedef struct {
  uint8 frameType;      // URL | Flags
  int8 txPower;
  uint8 encodedURL[EDDYSTONE_MAX_URL_LEN];  // the 1st byte is prefix
} eddystoneURL_t;

// Eddystone TLM frame
typedef struct {
  uint8 frameType;      // TLM
  uint8 version;        // 0x00 for now
  uint8 vBatt[2];       // Battery Voltage, 1mV/bit, Big Endian
  uint8 temp[2];        // Temperature. Signed 8.8 fixed point
  uint8 advCnt[4];      // Adv count since power-up/reboot
  uint8 secCnt[4];      // Time since power-up/reboot
                          // in 0.1 second resolution
} eddystoneTLM_t;

typedef union {
  eddystoneUID_t uid;
  eddystoneURL_t url;
  eddystoneTLM_t tlm;
} eddystoneFrame_t;

typedef struct {
  uint8 length1;        // 2
  uint8 dataType1;      // for Flags data type (0x01)
  uint8 data1;          // for Flags data (0x04)
  uint8 length2;        // 3
  uint8 dataType2;      // for 16-bit Svc UUID list data type (0x03)
  uint8 data2;          // for Eddystone UUID LSB (0xAA)
  uint8 data3;          // for Eddystone UUID MSB (0xFE)
  uint8 length;         // Eddystone service data length
  uint8 dataType3;      // for Svc Data data type (0x16)
  uint8 data4;          // for Eddystone UUID LSB (0xAA)
  uint8 data5;          // for Eddystone UUID MSB (0xFE)
  eddystoneFrame_t frame;
} eddystoneAdvData_t;

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
Task_Struct uebTask;
Char uebTaskStack[UEB_TASK_STACK_SIZE];

// Broadcaster state
static bool uebBcastActive;

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static eddystoneAdvData_t eddystoneAdv = {
// Flags; this sets the device to use general discoverable mode
    0x02,// length of this data
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED | GAP_ADTYPE_FLAGS_GENERAL,

    // Complete list of 16-bit Service UUIDs
    0x03,// length of this data including the data type byte
    GAP_ADTYPE_16BIT_COMPLETE, LO_UINT16(EDDYSTONE_SERVICE_UUID), HI_UINT16(
        EDDYSTONE_SERVICE_UUID),

    // Service Data
    0x03,// to be set properly later
    GAP_ADTYPE_SERVICE_DATA, LO_UINT16(EDDYSTONE_SERVICE_UUID), HI_UINT16(
        EDDYSTONE_SERVICE_UUID) };

eddystoneUID_t eddystoneUID;
eddystoneURL_t eddystoneURL;
eddystoneTLM_t eddystoneTLM;

uint8 eddystoneURLDataLen;

// Array of URL Scheme Prefices
static char* eddystoneURLPrefix[EDDYSTONE_URL_PREFIX_MAX] = { "http://www.",
    "https://www.", "http://", "https://" };

// Array of URLs to be encoded
static char* eddystoneURLEncoding[EDDYSTONE_URL_ENCODING_MAX] = { ".com/",
    ".org/", ".edu/", ".net/", ".info/", ".biz/", ".gov/", ".com/", ".org/",
    ".edu/", ".net/", ".info/", ".biz/", ".gov/" };

static uint32 advCount = 0;

// Eddystone frame type currently used
static uint8 currentFrameType = EDDYSTONE_FRAME_TYPE_UID;

// Pointer to application callback
keysPressedCB_t appKeyChangeHandler_st = NULL;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void MicroEddystoneBeacon_init(void);
static void MicroEddystoneBeacon_taskFxn(UArg a0, UArg a1);

static void MicroEddystoneBeacon_updateTLM(void);
static void MicroEddystoneBeacon_initUID(void);
static void MicroEddystoneBeacon_initURL(void);
static void MicroEddystoneBeacon_updateAdvDataWithFrame(uint8 frameType);
static void MicroEddystoneBeacon_startBroadcast(void);

static void MicroEddystoneBeacon_keyChangeHandler(uint8 keys);

static void MicroEddystoneBeacon_processAppMsg(uebEvt_t *pMsg);

static void MicroEddystoneBeacon_bcast_stateChangeCB(ugapBcastState_t newState);
static void MicroEddystoneBeacon_bcast_advPrepareCB(void);
static void MicroEddystoneBeacon_bcast_advDoneCB(bStatus_t status);

static bStatus_t MicroEddystoneBeacon_enqueueMsg(uint16 event, uint8 data);

static void uBLEStack_eventProxy(void);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      MicroEddystoneBeacon_createTask
 *
 * @brief   Task creation function for the Micro Eddystone Beacon.
 *
 * @param   None.
 *
 * @return  None.
 */
void MicroEddystoneBeacon_createTask(void)
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = uebTaskStack;
  taskParams.stackSize = UEB_TASK_STACK_SIZE;
  taskParams.priority = UEB_TASK_PRIORITY;

  Task_construct(&uebTask, MicroEddystoneBeacon_taskFxn, &taskParams, NULL);
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      MicroEddystoneBeacon_init
 *
 * @brief   Initialization function for the Micro Eddystone Beacon App
 *          Task. This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notification ...).
 *
 * @param   none
 *
 * @return  none
 */
static void MicroEddystoneBeacon_init(void)
{
#if defined(FEATURE_STATIC_ADDR)
  uint8 staticAddr[] = UEB_PREGEN_RAND_STATIC_ADDR;
#endif /* FEATURE_STATIC_ADDR */
  ugapBcastCBs_t bcastCBs = {
    MicroEddystoneBeacon_bcast_stateChangeCB,
    MicroEddystoneBeacon_bcast_advPrepareCB,
    MicroEddystoneBeacon_bcast_advDoneCB };

  // Create an RTOS event used to wake up this application to process events.
  syncEvent = Event_create(NULL, NULL);

  // Create an RTOS queue for message from profile to be sent to app.
  appMsgQueue = Util_constructQueue(&appMsg);

  // Setup keycallback for keys
  Board_initKeys(MicroEddystoneBeacon_keyChangeHandler);

  // Open Display.
  dispHandle = Display_open(UEB_DISPLAY_TYPE, NULL);

  // Initialize UID frame
  MicroEddystoneBeacon_initUID();

  // Initialize URL frame
  MicroEddystoneBeacon_initURL();

  /* Initialize Micro BLE Stack */
#if defined(FEATURE_STATIC_ADDR)
  uble_stackInit(UB_ADDRTYPE_STATIC, staticAddr, uBLEStack_eventProxy,
               RF_TIME_CRITICAL);
#else  /* FEATURE_STATIC_ADDR */
  uble_stackInit(UBLE_ADDRTYPE_PUBLIC, NULL, uBLEStack_eventProxy, RF_TIME_CRITICAL);
#endif /* FEATURE_STATIC_ADDR */

  /* Initilaize Micro GAP Broadcaster Role */
  ugap_bcastInit(&bcastCBs);

  Display_print0(dispHandle, 0, 0, "*MicroEddystoneBeacon");
  Display_print0(dispHandle, 1, 0, "< Bcast Start/Stop");
  Display_print0(dispHandle, 2, 0, " Beacon Mode Change >");
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_initUID
 *
 * @brief   initialize UID frame
 *
 * @param   none
 *
 * @return  none
 */
static void MicroEddystoneBeacon_initUID(void)
{
  // Set Eddystone UID frame with meaningless numbers for example.
  // This need to be replaced with some algorithm-based formula
  // for production.
  eddystoneUID.namespaceID[0] = 0x00;
  eddystoneUID.namespaceID[1] = 0x01;
  eddystoneUID.namespaceID[2] = 0x02;
  eddystoneUID.namespaceID[3] = 0x03;
  eddystoneUID.namespaceID[4] = 0x04;
  eddystoneUID.namespaceID[5] = 0x05;
  eddystoneUID.namespaceID[6] = 0x06;
  eddystoneUID.namespaceID[7] = 0x07;
  eddystoneUID.namespaceID[8] = 0x08;
  eddystoneUID.namespaceID[9] = 0x09;

  eddystoneUID.instanceID[0] = 0x04;
  eddystoneUID.instanceID[1] = 0x51;
  eddystoneUID.instanceID[2] = 0x40;
  eddystoneUID.instanceID[3] = 0x00;
  eddystoneUID.instanceID[4] = 0xB0;
  eddystoneUID.instanceID[5] = 0x00;
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_encodeURL
 *
 * @brief   Encodes URL in accordance with Eddystone URL frame spec
 *
 * @param   urlOrg - Plain-string URL to be encoded
 *          urlEnc - Encoded URL. Should be URLCFGSVC_CHAR_URI_DATA_LEN-long.
 *
 * @return  0 if the prefix is invalid
 *          The length of the encoded URL including prefix otherwise
 */
uint8 MicroEddystoneBeacon_encodeURL(char* urlOrg, uint8* urlEnc)
{
  uint8 i, j;
  uint8 urlLen;
  uint8 tokenLen;

  urlLen = (uint8) strlen(urlOrg);

  // search for a matching prefix
  for (i = 0; i < EDDYSTONE_URL_PREFIX_MAX; i++)
  {
    tokenLen = strlen(eddystoneURLPrefix[i]);
    if (strncmp(eddystoneURLPrefix[i], urlOrg, tokenLen) == 0)
    {
      break;
    }
  }

  if (i == EDDYSTONE_URL_PREFIX_MAX)
  {
    return 0;       // wrong prefix
  }

  // use the matching prefix number
  urlEnc[0] = i;
  urlOrg += tokenLen;
  urlLen -= tokenLen;

  // search for a token to be encoded
  for (i = 0; i < urlLen; i++)
  {
    for (j = 0; j < EDDYSTONE_URL_ENCODING_MAX; j++)
    {
      tokenLen = strlen(eddystoneURLEncoding[j]);
      if (strncmp(eddystoneURLEncoding[j], urlOrg + i, tokenLen) == 0)
      {
        // matching part found
        break;
      }
    }

    if (j < EDDYSTONE_URL_ENCODING_MAX)
    {
      memcpy(&urlEnc[1], urlOrg, i);
      // use the encoded byte
      urlEnc[i + 1] = j;
      break;
    }
  }

  if (i < urlLen)
  {
    memcpy(&urlEnc[i + 2], urlOrg + i + tokenLen, urlLen - i - tokenLen);
    return urlLen - tokenLen + 2;
  }

  memcpy(&urlEnc[1], urlOrg, urlLen);
  return urlLen + 1;
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_initUID
 *
 * @brief   initialize URL frame
 *
 * @param   none
 *
 * @return  none
 */
static void MicroEddystoneBeacon_initURL(void)
{
  // Set Eddystone URL frame with the URL of TI BLE site.
  eddystoneURLDataLen = MicroEddystoneBeacon_encodeURL(
      EDDYSTONE_URI_DATA_DEFAULT, eddystoneURL.encodedURL);
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_updateTLM
 *
 * @brief   Update TLM elements
 *
 * @param   none
 *
 * @return  none
 */
static void MicroEddystoneBeacon_updateTLM(void)
{
  uint32 time100MiliSec;
  uint32 batt;

  // Battery voltage (bit 10:8 - integer, but 7:0 fraction)
  batt = AONBatMonBatteryVoltageGet();
  batt = (batt * 125) >> 5; // convert V to mV
  eddystoneTLM.vBatt[0] = HI_UINT16(batt);
  eddystoneTLM.vBatt[1] = LO_UINT16(batt);

  // Temperature - 19.5 (Celcius) for example
  eddystoneTLM.temp[0] = 19;
  eddystoneTLM.temp[1] = 256 / 2;

  // advertise packet cnt;
  eddystoneTLM.advCnt[0] = BREAK_UINT32(advCount, 3);
  eddystoneTLM.advCnt[1] = BREAK_UINT32(advCount, 2);
  eddystoneTLM.advCnt[2] = BREAK_UINT32(advCount, 1);
  eddystoneTLM.advCnt[3] = BREAK_UINT32(advCount, 0);

  // running time
  // the number of 100-ms periods that have passed since the beginning.
  // no consideration of roll over for now.
  time100MiliSec = Clock_getTicks() / (100000 / Clock_tickPeriod);
  eddystoneTLM.secCnt[0] = BREAK_UINT32(time100MiliSec, 3);
  eddystoneTLM.secCnt[1] = BREAK_UINT32(time100MiliSec, 2);
  eddystoneTLM.secCnt[2] = BREAK_UINT32(time100MiliSec, 1);
  eddystoneTLM.secCnt[3] = BREAK_UINT32(time100MiliSec, 0);

  Display_print0(dispHandle, 3, 0, "TLM data updates:");
  Display_print2(dispHandle, 4, 0, "Cnt=%d Time=%d", advCount, time100MiliSec);
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_selectFrame
 *
 * @brief   Selecting the type of frame to be put in the service data
 *
 * @param   frameType - Eddystone frame type
 *
 * @return  none
 */
static void MicroEddystoneBeacon_updateAdvDataWithFrame(uint8 frameType)
{
  if (frameType == EDDYSTONE_FRAME_TYPE_UID
      || frameType == EDDYSTONE_FRAME_TYPE_URL
      || frameType == EDDYSTONE_FRAME_TYPE_TLM)
  {
    eddystoneFrame_t* pFrame;
    uint8 frameSize;

    eddystoneAdv.length = EDDYSTONE_SVC_DATA_OVERHEAD_LEN;
    // Fill with 0s first
    memset((uint8*) &eddystoneAdv.frame, 0x00, sizeof(eddystoneFrame_t));

    switch (frameType) {
    case EDDYSTONE_FRAME_TYPE_UID:
      eddystoneUID.frameType = EDDYSTONE_FRAME_TYPE_UID;
      frameSize = sizeof(eddystoneUID_t);
      pFrame = (eddystoneFrame_t *) &eddystoneUID;

      Display_print0(dispHandle, 6, 0, "Eddystone UID Mode");
      break;

    case EDDYSTONE_FRAME_TYPE_URL:
      eddystoneURL.frameType = EDDYSTONE_FRAME_TYPE_URL;
      frameSize = sizeof(eddystoneURL_t) - EDDYSTONE_MAX_URL_LEN
          + eddystoneURLDataLen;
      pFrame = (eddystoneFrame_t *) &eddystoneURL;

      Display_print0(dispHandle, 6, 0, "Eddystone URL Mode");
      break;

    case EDDYSTONE_FRAME_TYPE_TLM:
      eddystoneTLM.frameType = EDDYSTONE_FRAME_TYPE_TLM;
      frameSize = sizeof(eddystoneTLM_t);
      pFrame = (eddystoneFrame_t *) &eddystoneTLM;
      break;
    }

    memcpy((uint8 *) &eddystoneAdv.frame, (uint8 *) pFrame, frameSize);
    eddystoneAdv.length += frameSize;

    uble_setParameter(UBLE_PARAM_ADVDATA,
                      EDDYSTONE_FRAME_OVERHEAD_LEN + eddystoneAdv.length,
                      &eddystoneAdv);
  }
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_startBroadcast
 *
 * @brief   Start broadcasting.
 *          If configuration mode was on going, stop it.
 *
 * @param   none
 *
 * @return  none
 */
static void MicroEddystoneBeacon_startBroadcast(void)
{
  uint8 tempPower;

  uble_getParameter(UBLE_PARAM_TXPOWER, &tempPower);
  eddystoneUID.rangingData = tempPower;
  eddystoneURL.txPower = tempPower;

  // Select UID or URL frame as adv data initially
  MicroEddystoneBeacon_updateAdvDataWithFrame(currentFrameType);

//  ugap_bcastStart(100); /* Broadcaster will stop after 100th Adv event */
  ugap_bcastStart(0); /* Broadcaster runs indefinitely */
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_processEvent
 *
 * @brief   Application task entry point for the Micro Eddystone Beacon.
 *
 * @param   none
 *
 * @return  none
 */
static void MicroEddystoneBeacon_taskFxn(UArg a0, UArg a1)
{
  // Initialize application
  MicroEddystoneBeacon_init();

  for (;;)
  {
    uint32 events;

    // Waits for an event to be posted associated with the calling thread.
    // Note that an event associated with a thread is posted when a
    // message is queued to the message receive queue of the thread
#ifdef USE_ICALL
    events = Event_pend(syncEvent, Event_Id_NONE, UEB_ALL_EVENTS,
                        ICALL_TIMEOUT_FOREVER);
#else /* !USE_ICALL */
    events = Event_pend(syncEvent, Event_Id_NONE, UEB_ALL_EVENTS,
                        BIOS_WAIT_FOREVER);
#endif /* USE_ICALL */

    if (events | UEB_QUEUE_EVT)
    {
      // If RTOS queue is not empty, process app message.
      while (!Queue_empty(appMsgQueue))
      {
        uebEvt_t *pMsg = (uebEvt_t *) Util_dequeueMsg(appMsgQueue);
        if (pMsg)
        {
          // Process message.
          MicroEddystoneBeacon_processAppMsg(pMsg);

          // Free the space from the message.
#ifdef USE_ICALL
          ICall_free(pMsg);
#else /* USE_ICALL */
          free(pMsg);
#endif /* USE_ICALL */
        }
      }
    }
  }
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_handleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 KEY_LEFT
 *                 KEY_RIGHT
 *
 * @return  none
 */
static void MicroEddystoneBeacon_handleKeys(uint8 shift, uint8 keys) {
  (void) shift;  // Intentionally unreferenced parameter

  if (keys & KEY_LEFT)
  {
    // Toggle Advertising On/Off
    if (uebBcastActive)
    {
      ugap_bcastStop();
    }
    else
    {
      MicroEddystoneBeacon_startBroadcast();
    }

    return;
  }

  if (keys & KEY_RIGHT)
  {
    // Toggle frame type
    if (currentFrameType == EDDYSTONE_FRAME_TYPE_UID)
    {
      currentFrameType = EDDYSTONE_FRAME_TYPE_URL;
    }
    else
    {
      currentFrameType = EDDYSTONE_FRAME_TYPE_UID;
    }

    MicroEddystoneBeacon_updateAdvDataWithFrame(currentFrameType);

    return;
  }
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_keyChangeHandler
 *
 * @brief   Key event handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
void MicroEddystoneBeacon_keyChangeHandler(uint8 keys)
{
  MicroEddystoneBeacon_enqueueMsg(UEB_EVT_KEY_CHANGE, keys);
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_processAppMsg
 *
 * @brief   Process an incoming event message.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
static void MicroEddystoneBeacon_processAppMsg(uebEvt_t *pMsg)
{
  switch (pMsg->event)
  {
  case UEB_EVT_KEY_CHANGE:
    MicroEddystoneBeacon_handleKeys(0, pMsg->data);
    break;

  case UEB_EVT_MICROBLESTACK:
    uble_processMsg();
    break;

  default:
    // Do nothing.
    break;
  }
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_bcast_stateChange_CB
 *
 * @brief   Callback from Micro Broadcaster indicating a state change.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void MicroEddystoneBeacon_bcast_stateChangeCB(ugapBcastState_t newState)
{
  switch (newState)
  {
  case UGAP_BCAST_STATE_INITIALIZED:
    uebBcastActive = FALSE;
    Display_print0(dispHandle, 7, 0, "BC_State:Initialized");
    {
      // Parameter containers
      uint16 param16; /* 16-bit parameter */

      // Setup broadcaster duty cycle
//    ugap_bcastSetDuty(100, 50); /* OnTime: 10 sec, OffTime: 5 sec */

      // Setup adv interval
      param16 = DEFAULT_ADVERTISING_INTERVAL;
      uble_setParameter(UBLE_PARAM_ADVINTERVAL, sizeof(uint16), &param16);

      //uint8  param8;  /* 8-bit parameter */
      // Setup adv channel map
//    param8 = UB_ADV_CHAN_38 | UB_ADV_CHAN_39; /* Use only channels 38 & 39 */
//    uble_setParameter(UB_PARAM_ADVCHANMAP, sizeof(uint8), &param8);
    }
    // Start advertising
    MicroEddystoneBeacon_startBroadcast();
    break;

  case UGAP_BCAST_STATE_IDLE:
    uebBcastActive = FALSE;
    Display_print0(dispHandle, 7, 0, "BC_State:Idle");
    break;

  case UGAP_BCAST_STATE_ADVERTISING:
    uebBcastActive = TRUE;
    Display_print0(dispHandle, 7, 0, "BC_State:Advertising");
    break;

  case UGAP_BCAST_STATE_WAITING:
    Display_print0(dispHandle, 7, 0, "BC_State:Waiting");
    break;

  default:
    break;
  }
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_bcast_advPrepareCB
 *
 * @brief   Callback from Micro Broadcaster notifying that the next
 *          advertising event is about to start so it's time to update
 *          the adv payload.
 *
 * @param   None.
 *
 * @return  None.
 */
static void MicroEddystoneBeacon_bcast_advPrepareCB(void)
{
  uint8 timeToAdv = 0;

  MicroEddystoneBeacon_updateTLM();
  MicroEddystoneBeacon_updateAdvDataWithFrame(EDDYSTONE_FRAME_TYPE_TLM);

  // Disable ADV_PREPARE notification from the next Adv
  uble_setParameter(UBLE_PARAM_TIMETOADV, sizeof(timeToAdv), &timeToAdv);
}

/*********************************************************************
 * @fn      MicroEddystoneBeacon_bcast_advDoneCB
 *
 * @brief   Callback from Micro Broadcaster notifying that an
 *          advertising event has been done.
 *
 * @param   status - How the last event was done. SUCCESS or FAILURE.
 *
 * @return  None.
 */
static void MicroEddystoneBeacon_bcast_advDoneCB(bStatus_t status)
{
  advCount++;

  if ((advCount % 10) == 8)
  {
    uint8 timeToAdv = 30;

    // Set TimeToAdv parameter to get MicroEddystoneBeacon_bcast_advPrepareCB()
    // callback issued 30 ms before the second next advertising event.
    uble_setParameter(UBLE_PARAM_TIMETOADV, sizeof(timeToAdv), &timeToAdv);
  }
  else if ((advCount % 10) == 0)
  {
    // Send UID or URL
    MicroEddystoneBeacon_updateAdvDataWithFrame(currentFrameType);
  }

  Display_print1(dispHandle, 5, 0, "%d Adv's done", advCount);
}


/*********************************************************************
 * @fn      MicroEddystoneBeacon_enqueueMsg
 *
 * @brief   Creates a message and puts the message in RTOS queue.
 *
 * @param   event - message event.
 * @param   data - message data.
 *
 * @return  TRUE or FALSE
 */
static bStatus_t MicroEddystoneBeacon_enqueueMsg(uint16 event, uint8 data)
{
  uebEvt_t *pMsg;

  // Create dynamic pointer to message.
#ifdef USE_ICALL
  if ((pMsg = (uebEvt_t*) ICall_malloc(sizeof(uebEvt_t))))
#else /* USE_ICALL */
  if ((pMsg = (uebEvt_t*) malloc(sizeof(uebEvt_t))))
#endif /* USE_ICALL */
  {
    pMsg->event = event;
    pMsg->data = data;

    // Enqueue the message.
    return Util_enqueueMsg(appMsgQueue, syncEvent, (uint8_t *) pMsg);
  }

  return FALSE;
}


/*********************************************************************
 * @fn      uBLEStack_eventProxy
 *
 * @brief   Post an event to the application so that a Micro BLE Stack internal
 *          event is processed by Micro BLE Stack later in the application
 *          task's context.
 *
 * @param   None
 *
 * @return  None
 */
void uBLEStack_eventProxy()
{
  MicroEddystoneBeacon_enqueueMsg(UEB_EVT_MICROBLESTACK, 0);
}

/*********************************************************************
 *********************************************************************/

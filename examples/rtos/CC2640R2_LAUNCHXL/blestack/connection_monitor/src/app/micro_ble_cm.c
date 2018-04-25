/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdlib.h>

#include <ti/display/Display.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/BIOS.h>

#include "bcomdef.h"

#include "board.h"
#include "board_key.h"

#include <ti/mw/lcd/LCDDogm1286.h>

#include "uble.h"
#include "ugap.h"
#include "urfc.h"

#include "util.h"
#include "gap.h"

#include "micro_ble_cm.h"

#ifdef NPI_USE_UART
#include "micro_cm_app.h"
#include "npi_task.h"
#include "npi_data.h"
#endif // NPI_USE_UART

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define BLE_RAT_IN_16US              64   // Connection Jitter
#define BLE_RAT_IN_64US              256  // Radio Rx Settle Time
#define BLE_RAT_IN_100US             400  // 1M / 2500 RAT ticks (SCA PPM)
#define BLE_RAT_IN_140US             560  // Rx Back-end Time
#define BLE_RAT_IN_150US             600  // T_IFS
#define BLE_RAT_IN_256US             1024 // Radio Overhead + FS Calibration
#define BLE_RAT_IN_2021US            8084 // Maximum packet size

#define BLE_SYNTH_CALIBRATION        (BLE_RAT_IN_256US)
#define BLE_RX_SETTLE_TIME           (BLE_RAT_IN_64US)
#define BLE_RX_RAMP_OVERHEAD         (BLE_SYNTH_CALIBRATION + \
                                      BLE_RX_SETTLE_TIME)
#define BLE_RX_SYNCH_OVERHEAD        (BLE_RAT_IN_140US)
#define BLE_JITTER_CORRECTION        (BLE_RAT_IN_16US)

#define BLE_OVERLAP_TIME             (BLE_RAT_IN_150US)
#define BLE_EVENT_PAD_TIME           (BLE_RX_RAMP_OVERHEAD +  \
                                      BLE_JITTER_CORRECTION + \
                                      BLE_RAT_IN_100US)

#define BLE_HOP_VALUE_MAX            16
#define BLE_HOP_VALUE_MIN            5
#define BLE_COMB_SCA_MAX             540
#define BLE_COMB_SCA_MIN             40

// The most significant nibble for possible "nextStartTime" wrap around.
#define BLE_MS_NIBBLE                0xF0000000

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern Display_Handle dispHandle;
extern uint8 tbmRowItemLast;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
#ifndef NPI_USE_UART
extern void MicroCmDemo_monitorStateChangeCB(ugapMonitorState_t newState);
extern void MicroCmDemo_monitorIndicationCB(bStatus_t status, uint8_t sessionId,
                                            uint8_t len, uint8_t *pPayload);
extern void MicroCmDemo_monitorCompleteCB (bStatus_t status, uint8_t sessionId);
#endif // NPI_USE_UART

/*********************************************************************
 * LOCAL VARIABLES
 */
ubCM_GetConnInfoComplete_t ubCMConnInfo;

uint8_t lastSessionMask = 0x00; //! Mask to track which sessions have recently been watched
uint8_t sessionMask   = 0x00;   //! Mask to track which sessions are being watched

bool    cmMaster = true; //! Flag to identify a packet from master

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      setNextDataChan
 *
 * @brief   This function returns the next data channel for a CM connection
 *          based on the previous data channel, the hop length, and one
 *          connection interval to the next active event. If the
 *          derived channel is "used", then it is returned. If the derived
 *          channel is "unused", then a remapped data channel is returned.
 *
 *          Note: nextChan is updated, and must remain that way, even if the
 *                remapped channel is returned.
 *
 * @param   sessionId - Value identifying the CM session.
 * @param   numEvents - Number of events.
 *
 * @return  Next data channel.
 */
static uint8_t setNextDataChan( uint8_t sessionId, uint16_t numEvents )
{
  ubCM_ConnInfo_t *connInfo;
  uint8_t         chanMapTable[CM_MAX_NUM_DATA_CHAN];
  uint8_t         i, j;
  uint8_t         numUsedChans = 0;

  /* get pointer to connection info */
  connInfo = &ubCMConnInfo.ArrayOfConnInfo[sessionId-1];

  // channels 37..39 are not data channels and these bits should not be set
  connInfo->chanMap[CM_NUM_BYTES_FOR_CHAN_MAP-1] &= 0x1F;

  // the used channel map uses 1 bit per data channel, or 5 bytes for 37 chans
  for (i=0; i<CM_NUM_BYTES_FOR_CHAN_MAP; i++)
  {
    // save each valid channel for every channel map bit that's set
    // Note: When i is on the last byte, only 5 bits need to be checked, but
    //       it is easier here to check all 8 with the assumption that the rest
    //       of the reserved bits are zero.
    for (j=0; j<CM_BITS_PER_BYTE; j++)
    {
      // check if the channel is used; only interested in used channels
      if ( (connInfo->chanMap[i] >> j) & 1 )
      {
        // sequence used channels in ascending order
        chanMapTable[ numUsedChans ] = (i*8U)+j;

        // count it
        numUsedChans++;
      }
    }
  }

  // calculate the data channel based on hop and number of events to next event
  // Note: nextChan is now called UnmappedChannel in the spec.
  // Note: currentChan is now called lastUnmappedChannel in the spec.
  connInfo->nextChan = (connInfo->currentChan + (connInfo->hopValue * numEvents)) %
                       CM_MAX_NUM_DATA_CHAN;

  // check if next channel is a used data channel
  for (i=0; i<numUsedChans; i++)
  {
    // check if next channel is in the channel map table
    if ( connInfo->nextChan == chanMapTable[i] )
    {
      // it is, so return the used channel
      return( connInfo->nextChan );
    }
  }

  // next channel is unused, so return the remapped channel
  return( chanMapTable[connInfo->nextChan % numUsedChans] );
}

/*********************************************************************
 * @fn      ubCM_findNextPriorityEvt
 *
 * @brief   Find the next connection event.
 *
 * @param   None.
 *
 * @return  next sessionId.
 */
uint8_t ubCM_findNextPriorityEvt(void)
{
  uint8_t  i;
  uint8_t  rtnSessionId = 0;
  uint8_t  currentSessionId;
  uint8_t  checkBit;
  uint32_t oldestAnchorPoint = 0;

  // This will determine which connection has been heard the
  // least recently based on its saved timeStampMaster which is only updated
  // if a master packet was successfully caught. It also never monitors the
  // same session until all sessions being monitored have been checked.

  // Note: This scheme should not work well if connection intervals are very different
  // since the masking scheme will ignore channels that should be monitored
  // multiple times while waiting for the next conneciton on a longer interval.
  currentSessionId = ubCMConnInfo.ArrayOfConnInfo[0].sessionId;
  checkBit = 1 << currentSessionId;
  if((lastSessionMask & checkBit) == 0 && ((sessionMask & checkBit) == checkBit))
  {
    rtnSessionId = ubCMConnInfo.ArrayOfConnInfo[0].sessionId;
    oldestAnchorPoint = ubCMConnInfo.ArrayOfConnInfo[0].timeStampMaster;
  }

  for (i=1; i < CM_MAX_SESSIONS; i++)
  {
    currentSessionId = ubCMConnInfo.ArrayOfConnInfo[i].sessionId;
    checkBit = 1 << currentSessionId;
    if((lastSessionMask & checkBit) == 0 && (sessionMask & checkBit) == checkBit)
    {
      if(ubCMConnInfo.ArrayOfConnInfo[i].timeStampMaster <= oldestAnchorPoint ||
         oldestAnchorPoint == 0)
      {
        //We want to pick the oldest anchor point connection not recently monitored
        oldestAnchorPoint = ubCMConnInfo.ArrayOfConnInfo[i].timeStampMaster;
        rtnSessionId = currentSessionId;
      }
    }
  }

  //Fill in mask
  lastSessionMask |= (1 << rtnSessionId);

  //If all sessions have been checked clear lastSessionId mask
  if((lastSessionMask & sessionMask) == sessionMask)
  {
    lastSessionMask = 0x00;
  }

  if(rtnSessionId != 0)
  {
    ubCM_setupNextCMEvent(rtnSessionId);
  }

  return rtnSessionId;
}

/*********************************************************************
 * @fn      ubCM_setupNextCMEvent
 *
 * @brief   The connection updates will be managed by the Host device
 *          and we are interested in connection events even when the
 *          slave may not send data. The following connInfo are updated
 *          after the call: currentChan, ScanDuration, currentStartTime,
 *          and nextStartTime.
 *
 * @param   sessionId - Value identifying the CM session.
 *
 * @return  None.
 */
void ubCM_setupNextCMEvent(uint8_t sessionId)
{
  ubCM_ConnInfo_t *connInfo;
  uint32_t        timeToNextEvt;
  uint32_t        scanDuration;
  uint32_t        currentTime;
  uint32_t        deltaTime;
  uint16_t        numEvents;
  volatile uint32 keySwi;

  keySwi = Swi_disable();

  // get pointer to connection info
  connInfo = &ubCMConnInfo.ArrayOfConnInfo[sessionId-1];

  currentTime = RF_getCurrentTime();

  // deltaTime is the distance between the current time and
  // the last anchor point.
  if (connInfo->timeStampMaster < currentTime)
  {
    deltaTime = currentTime - connInfo->timeStampMaster;
  }
  else
  {
    //Rx clock must have rolled over so adjust for wrap around
    deltaTime = currentTime + (0xFFFFFFFF - connInfo->timeStampMaster);
  }

  // Figure out how many connection events have passed.
  numEvents = deltaTime / (connInfo->connInterval * BLE_TO_RAT) + 1;

  // Update start time to the new anchor point.
  connInfo->currentStartTime = connInfo->timeStampMaster;

  // time to next event is just the connection intervals in 625us ticks
  timeToNextEvt = connInfo->connInterval * numEvents;

  // advance the anchor point in RAT ticks
  connInfo->currentStartTime += (timeToNextEvt * BLE_TO_RAT);

  //if not enough time to start scan, bump to next connection interval
  if(connInfo->currentStartTime - currentTime < BLE_EVENT_PAD_TIME)
  {
    connInfo->currentStartTime += connInfo->connInterval * BLE_TO_RAT;
  }

  // account for radio startup overhead and jitter per the spec, pull values
  // from BLE stack
  connInfo->currentStartTime -= (BLE_RX_RAMP_OVERHEAD + BLE_JITTER_CORRECTION);

  // Calc timerDrift, scaFactor is (CM ppm + Master ppm).
  connInfo->timerDrift = ( (timeToNextEvt * connInfo->combSCA) / BLE_RAT_IN_100US ) + 1;

  // setup the Start Time of the receive window
  // Note: In the case we don't receive a packet at the first connection
  //       event, (and thus, don't have an updated anchor point), this anchor
  //       point will be used for finding the start of the connection event
  //       after that. That is, the update is relative to the last valid anchor
  //       point.
  // Note: If the AP is valid, we have to adjust the AP by timer drift. If the
  //       AP is not valid, we still have to adjust the AP based on the amount
  //       of timer drift that results from a widened window. Since SL is
  //       disabled when the AP is invalid (i.e. a RX Timeout means no packet
  //       was received, and by the spec, SL is discontinued until one is),
  //       the time to next event is the connection interval, and timer drift
  //       was re-calculated based on (SL+1)*CI where SL=0.
  connInfo->nextStartTime = connInfo->currentStartTime - connInfo->timerDrift;

  // setup the receiver Timeout time
  // Note: If the AP is valid, then timeoutTime was previously cleared and any
  //       previous window widening accumulation was therefore reset to zero.
  // Note: Timeout trigger remains as it was when connection was formed.
  scanDuration = (2 * connInfo->timerDrift);

  // additional widening based on whether the AP is valid
  // Note: The overhead to receive a preamble and synch word to detect a
  //       packet is added in case a packet arrives right at the end of the
  //       receive window.
  scanDuration += BLE_RX_RAMP_OVERHEAD + (2 * BLE_JITTER_CORRECTION) + BLE_RX_SYNCH_OVERHEAD;

  // The CM endTime is a bit different than a standard slave. We only want to keep Rx on
  // long enough to receive 2 packets at each AP, one Master and one Slave if present.
  // Therefore we only need to at most remain active for the full timeoutTime + the duration
  // it takes to receive to max size packets + 300 us or 2*T_IFS for the min inter frame timing.
  // 2021 us is based on the time it takes to receive 2 packets with max 251 byte payloads + headers.
  scanDuration += (BLE_RAT_IN_2021US + 2 * BLE_RAT_IN_150US);
  connInfo->scanDuration = (scanDuration / BLE_TO_RAT) + 1;

  // Finally we will update the next channel.
  connInfo->currentChan = setNextDataChan( sessionId, numEvents );

  Swi_restore(keySwi);
}

#ifdef NPI_USE_UART
/*********************************************************************
 * @fn      cmEnd
 *
 * @brief   Pass critical information for each CM session to
 *          uNPI host. Setup the next expected connection.
 *
 * @param   sessionId - Value identifying the CM session to end.
 *
 * @return  None.
 */
static void cmEnd(uint8_t id, uint8_t rssi, uint32_t timeStamp)
{
  volatile uint32 keyHwi;
  /*
    Here we will need another function or code to parse the Rx buffer data obtained during the scan session.
    The required parsed results will be packed and passed back to the uNPI to go to the host.

    The command to forward data to the Host is listed below. To call it the file needs to include npi_task.h
  */
  uint8_t len = 6;
  uint8_t *temp;
  _npiFrame_t *pMsg;
  keyHwi = Hwi_disable();
  pMsg = (_npiFrame_t *)NPITask_mallocFrame(sizeof(_npiFrame_t)+ len);
  Hwi_restore(keyHwi);

  pMsg ->dataLen = len;
  pMsg ->cmd0    = UCA_ASYNC_REQ;
  pMsg ->cmd1    = CM_SESSION_DATA_RSP;
  pMsg ->pData = (uint8_t *)((uint32_t)pMsg + sizeof(_npiFrame_t));
  temp = pMsg ->pData;
  *temp++ = id;
  *temp++ = rssi;
  *temp++ = BREAK_UINT32(timeStamp,0);
  *temp++ = BREAK_UINT32(timeStamp,1);
  *temp++ = BREAK_UINT32(timeStamp,2);
  *temp = BREAK_UINT32(timeStamp,3);

  // forward npiFrame to uNPI
  NPITask_sendToHost(pMsg);
}
#endif // NPI_USE_UART

/*********************************************************************
 * CALLBACKS
 */

/*********************************************************************
 * @fn      monitor_stateChangeCB
 *
 * @brief   Callback from Micro Monitor indicating a state change.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void monitor_stateChangeCB(ugapMonitorState_t newState)
{
#ifndef NPI_USE_UART
  /* optional, dump info to the display or via uNPI */
  MicroCmDemo_monitorStateChangeCB(newState);
#endif // NPI_USE_UART
}

/*********************************************************************
 * @fn      monitor_indicationCB
 *
 * @brief   Callback from Micro monitor notifying that a data
 *          packet is received.
 *
 * @param   status status of a monitoring scan
 * @param   sessionId session ID
 * @param   len length of the payload
 * @param   pPayload pointer to payload
 *
 * @return  None.
 */
static void monitor_indicationCB(bStatus_t status, uint8_t sessionId,
                                 uint8_t len, uint8_t *pPayload)
{
  uint8_t  rawRssi;
  int8_t   rssi;
  uint32_t timeStamp;
  ubCM_ConnInfo_t *connInfo;

  /* Access the connection info array */
  connInfo = &ubCMConnInfo.ArrayOfConnInfo[sessionId-1];

  rawRssi = *(pPayload + len + CM_RSSI_OFFSET);
  timeStamp = *(uint32_t *)(pPayload + len + CM_TIMESTAMP_OFFSET);

  /* corrects RSSI if valid */
  rssi = CM_CHECK_LAST_RSSI( rawRssi );

  if (cmMaster == true)
  {
    /* Master */
    connInfo->timeStampMaster = timeStamp;
    connInfo->rssiMaster = rssi;
    connInfo->timesScanned++;
    cmMaster = false;

#ifndef NPI_USE_UART
    /* optional, dump info to the display or via uNPI */
    MicroCmDemo_monitorIndicationCB(1, sessionId, len, pPayload);
#endif // NPI_USE_UART
  }
  else
  {
    /* Slave */
    connInfo->timeStampSlave = timeStamp;
    connInfo->rssiSlave = rssi;

#ifndef NPI_USE_UART
    /* optional, dump info to the display or via uNPI */
    MicroCmDemo_monitorIndicationCB(0, sessionId, len, pPayload);
#endif // NPI_USE_UART
  }

#ifdef NPI_USE_UART
  cmEnd(sessionId, rssi, timeStamp);
#endif // NPI_USE_UART
}

/*********************************************************************
 * @fn      monitor_completeCB
 *
 * @brief   Callback from Micro Monitor notifying that a monitoring
 *          scan is completed.
 *
 * @param   status - How the last event was done. SUCCESS or FAILURE.
 * @param   sessionId - Session ID
 *
 * @return  None.
 */
static void monitor_completeCB (bStatus_t status, uint8_t sessionId)
{
  ubCM_ConnInfo_t *connInfo;

  /* Access the connection info array */
  connInfo = &ubCMConnInfo.ArrayOfConnInfo[sessionId-1];
  if (cmMaster == false)
  {
    /* An M->S packet was received. We are not sure about the S->M packet.
     * Reset the cmMaster to true for the next connection interval.
     */
    cmMaster = true;

#ifndef NPI_USE_UART
    /* optional, dump info to the display or via uNPI */
    MicroCmDemo_monitorCompleteCB(CM_SUCCESS, sessionId);
#endif // NPI_USE_UART
  }
  else
  {

#ifndef NPI_USE_UART
    /* optional, dump info to the display or via uNPI */
    MicroCmDemo_monitorCompleteCB(CM_FAILED_NOT_ACTIVE, sessionId);
#endif // NPI_USE_UART

    // Kepp a record of missing packets in this monitor session
    connInfo->timesMissed++;
  }

  // If status indicates no resources, there is no need to continue.
  if (status == SUCCESS)
  {
#ifndef NPI_USE_UART
    // continue attempt to monitor other sessions.
    // The context of this callback is from the uStack task.
    (void)ubCM_start(ubCM_findNextPriorityEvt());
#else
    // Need to setup an event to findNext or handle new start command
    Event_post(syncAppEvent, UCA_FIND_NEXT_PRIOR_EVT);
#endif // NPI_USE_UART
  }
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      ubCm_init
 *
 * @brief   Initialization function for micro BLE connection monitor.
 *          This function initializes the callbacks and default connection
 *          info.
 *
 * @param   none
 *
 * @return  true: CM initialization successful
 *          false: CM initialization failed
 */
bool ubCm_init(void)
{
  uint8_t i;

  ugapMonitorCBs_t monitorCBs = {
    monitor_stateChangeCB,
    monitor_indicationCB,
    monitor_completeCB };

  /* Initialize default connection info */
  ubCMConnInfo.numHandles = 0;
  for (i = 0; i < CM_MAX_SESSIONS; i++)
  {
    ubCMConnInfo.ArrayOfConnInfo[i].sessionId = CM_INVALID_SESSION_ID;
    ubCMConnInfo.ArrayOfConnInfo[i].connInterval = CM_CONN_INTERVAL_MAX;
    ubCMConnInfo.ArrayOfConnInfo[i].timesScanned = 0;
  }

  /* Initilaize Micro GAP Monitor */
  if (SUCCESS == ugap_monitorInit(&monitorCBs))
  {
    return true;
  }

  return false;
}

/*********************************************************************
 * @fn      ubCM_isSessionActive
 *
 * @brief   Check if the CM sessionId is active
 *
 * @param   sessionId - Value identifying the CM session to check.
 *
 * @return  CM_SUCCESS = 0: CM session active
 *          CM_FAILED_NOT_FOUND = 2: CM session not active
 */
uint8_t ubCM_isSessionActive(uint8_t sessionId)
{
  ubCM_ConnInfo_t *connInfo;

  if (sessionId == 0 || sessionId > CM_MAX_SESSIONS)
  {
    /* Not a valid sessionId or no CM session has started. */
    return CM_FAILED_NOT_FOUND;
  }

  /* Access the connection info array */
  connInfo = &ubCMConnInfo.ArrayOfConnInfo[sessionId-1];
  if (connInfo->sessionId == 0)
  {
    /* CM session has not been started. */
    return CM_FAILED_NOT_ACTIVE;
  }

  /* CM session is indeed active */
  return CM_SUCCESS;
}

/*********************************************************************
 * @fn      ubCM_start
 *
 * @brief   To establish a new CM session or continue monitor an
 *          existing CM session.
 *
 * @param   sessionId - Value identifying the CM session requested
 *          to start.
 *
 * @return  CM_SUCCESS = 0: CM session started.
 *          CM_FAILED_TO_START = 1: Failed to start because next
 *          anchor point is missed.
 */
uint8_t ubCM_start(uint8_t sessionId)
{
  ubCM_ConnInfo_t *connInfo;
  uint8_t status = CM_FAILED_TO_START;

  if (ubCM_isSessionActive(sessionId) == CM_SUCCESS ||
      ubCM_isSessionActive(sessionId) == CM_FAILED_NOT_ACTIVE)
  {
    /* Access the connection info array */
    connInfo = &ubCMConnInfo.ArrayOfConnInfo[sessionId-1];

    /* Range checks */
    if (connInfo->hopValue > BLE_HOP_VALUE_MAX ||
        connInfo->hopValue < BLE_HOP_VALUE_MIN ||
        connInfo->combSCA > BLE_COMB_SCA_MAX ||
        connInfo->combSCA < BLE_COMB_SCA_MIN)
    {
      return CM_FAILED_OUT_OF_RANGE;
    }

    /* Stop the current monitoring activity */
    ugap_monitorStop();

    /* Save the next sessionId */
    uble_setParameter(UBLE_PARAM_SESSIONID, sizeof(uint8_t), &sessionId);

    /* Kick off the monitor request */
    if (ugap_monitorRequest(connInfo->currentChan,
                            connInfo->accessAddr,
                            connInfo->nextStartTime,
                            connInfo->scanDuration) == SUCCESS)
    {
      /* Is the sessionId new? */
      if (ubCM_isSessionActive(sessionId) == CM_FAILED_NOT_ACTIVE &&
          ubCMConnInfo.numHandles < CM_MAX_SESSIONS)
      {
        /* Activate this session ID */
        connInfo->sessionId = sessionId;
        ubCMConnInfo.numHandles++;
        //set bit in mask
        sessionMask |= (1 << sessionId);
      }
      status = CM_SUCCESS;
    }
  }
  return status;
}

/*********************************************************************
 * @fn      ubCM_stop
 *
 * @brief   To discontinue a CM session.
 *
 * @param   sessionId - Value identifying the CM session requested to stop.
 *          For an invalid sessionId, this function will return
 *          CM_FAILED_NOT_FOUND status.
 *
 * @return  CM_SUCCESS = 0: CM session ended
 *          CM_FAILED_NOT_FOUND = 2: Could not find CM session to stop
 */
uint8_t ubCM_stop(uint8_t sessionId)
{
  ubCM_ConnInfo_t *connInfo;

  if (ubCM_isSessionActive(sessionId) == CM_SUCCESS &&
      ubCMConnInfo.numHandles > 0)
  {
    /* Access the connection info array */
    connInfo = &ubCMConnInfo.ArrayOfConnInfo[sessionId-1];

    /* Mark the sessionId as invalid and
     * decrement the number of connection handles.
     * Note this will leave a hole in the array.
     */
    connInfo->sessionId = CM_INVALID_SESSION_ID;
    ubCMConnInfo.numHandles--;
    //clear bit
    sessionMask &= ~(1 << sessionId);
    return CM_SUCCESS;
  }
  else
  {
    return CM_FAILED_NOT_FOUND;
  }
}

/*********************************************************************
 * @fn      ubCM_startExt
 *
 * @brief   Initializes new connection data to start new CM sessions
 *
 * @param   *data - Bit stream of necessary connection data provided by the
 *                  CM_START Host API command. Currently this command payload
 *                  has 11 bytes of data. Including:
 *                  AccessAddress(4bytes),ConnInterval(2bytes),hopValue(1byte),
 *                  mSCA(2bytes),currChan(1byte),chanMap(5bytes)
 *
 * @return  index: A valid index will be less than CM_MAX_SESSIONS an invalid
 *                 index will be greater than or equal to CM_MAX_SESSIONS.
 */
uint8_t ubCM_startExt(uint8_t *data)
{
  //i's initial value will make cmStart() fail if ubCMConnInfo.numHandles >= CM_MAX_SESSIONS
  uint8_t i = ubCMConnInfo.numHandles;
  uint8_t sessionId;

  //First make sure we have not reached our session limit
  if (ubCMConnInfo.numHandles <= CM_MAX_SESSIONS)
  {
    //set sessionId to first inactive session id
    for (sessionId=1; sessionId <= CM_MAX_SESSIONS; sessionId++)
    {
      if(ubCM_isSessionActive(sessionId) == CM_FAILED_NOT_ACTIVE)
      {
        i = sessionId-1;//index of session id is 1 less than acctual id
        break;
      }
    }
    ubCMConnInfo.ArrayOfConnInfo[i].accessAddr = (uint32_t)((data[0]<<24) +
                                                 (data[1]<<16) + (data[2]<<8) + data[3]);

    ubCMConnInfo.ArrayOfConnInfo[i].currentStartTime = RF_getCurrentTime() + 20 * BLE_TO_RAT;
    ubCMConnInfo.ArrayOfConnInfo[i].nextStartTime = ubCMConnInfo.ArrayOfConnInfo[i].currentStartTime;

    ubCMConnInfo.ArrayOfConnInfo[i].connInterval = (uint16_t)((data[4]<<8) + data[5]);
    if(ubCMConnInfo.ArrayOfConnInfo[i].connInterval < CM_CONN_INTERVAL_MIN ||
       ubCMConnInfo.ArrayOfConnInfo[i].connInterval > CM_CONN_INTERVAL_MAX)
    {
      return CM_FAILED_OUT_OF_RANGE;
    }

    ubCMConnInfo.ArrayOfConnInfo[i].hopValue = data[6];

    // Set initial scan duration to 4x(connInterval). Gives a nice wide window to
    // catch anchor point 2 connections in the future
    ubCMConnInfo.ArrayOfConnInfo[i].scanDuration = (uint16_t)(((data[4]<<8) + data[5]) * 4);

    ubCMConnInfo.ArrayOfConnInfo[i].combSCA = 90; // Master = 50 + Slave = 40 leave hard coded for now

    ubCMConnInfo.ArrayOfConnInfo[i].currentChan = data[9];
    setNextDataChan(sessionId, 2); //jump 2 channels into the future
    ubCMConnInfo.ArrayOfConnInfo[i].currentChan = ubCMConnInfo.ArrayOfConnInfo[i].nextChan;

    if((data[14] | data[13] | data[12] | data[11] | (data[10]& 0x1F)) == 0)
    {
      // Must set at least 1 valid BLE data channel
      return CM_FAILED_OUT_OF_RANGE;
    }

    ubCMConnInfo.ArrayOfConnInfo[i].chanMap[0] = data[14];
    ubCMConnInfo.ArrayOfConnInfo[i].chanMap[1] = data[13];
    ubCMConnInfo.ArrayOfConnInfo[i].chanMap[2] = data[12];
    ubCMConnInfo.ArrayOfConnInfo[i].chanMap[3] = data[11];
    // Mask last 8 bits to ensure we only consider data channels and not advert
    ubCMConnInfo.ArrayOfConnInfo[i].chanMap[4] = data[10] & 0x1F;

    //If i+1 > CM_MAX_SESSIONS then we know this function failed. Let cmStart()
    //return a failure message since i will be an out of range index
    return ubCM_start(sessionId);
  }
  else
  {
    //Fail to start if no more session slots avaiable
    return CM_FAILED_TO_START;
  }
}

/*********************************************************************
 *********************************************************************/

/**************************************************************************************************
  Filename:       micro_ble_cm.h

  Description:    This file contains the Connection Monitor sample application
                  definitions and prototypes.

* Copyright (c) 2015, Texas Instruments Incorporated
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
**************************************************************************************************/

#ifndef MICROBLECM_H
#define MICROBLECM_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "uble.h"

/*********************************************************************
*  EXTERNAL VARIABLES
*/


/*********************************************************************
 * CONSTANTS
 */
#define CM_SUCCESS                 0
#define CM_FAILED_TO_START         1
#define CM_FAILED_NOT_FOUND        2
#define CM_FAILED_NOT_ACTIVE       3
#define CM_FAILED_OUT_OF_RANGE     4

#define CM_MAX_SESSIONS            UBLE_MAX_MONITOR_HANDLE
#define CM_NUM_BYTES_FOR_CHAN_MAP  5
#define CM_DEVICE_ADDR_LEN         6

#define CM_INVALID_SESSION_ID      0
#define CM_CONN_INTERVAL_MIN       12   // 7.5ms in 625ms
#define CM_CONN_INTERVAL_MAX       6400 // 4s in 625ms

#define CM_CHAN_MASK               0x3F
#define CM_TIMESTAMP_OFFSET        (-4)
#define CM_CHAN_OFFSET             (-5)
#define CM_RSSI_OFFSET             (-6)

/* RSSI */
#define CM_RF_RSSI_INVALID         0x81  /* reported by RF */
#define CM_RF_RSSI_UNDEFINED       0x80  /* reported by RF */
#define CM_RSSI_NOT_AVAILABLE      0x7F  // report to user

#define CM_BITS_PER_BYTE           8
#define CM_MAX_NUM_DATA_CHAN       37    // 0..36

#define CM_MAX_MISSED_PACKETS      3     // Number of consecutive missed
                                         // packets before giving up

#define CM_SESSION_DATA_RSP        4
/*********************************************************************
 * MACROS
 */
#define CM_GET_RSSI_OFFSET()       (0)

/* corrects RSSI if valid, otherwise returns not available */
#define CM_CHECK_LAST_RSSI( rssi )                                             \
          ((rssi) == CM_RF_RSSI_UNDEFINED || (rssi) == CM_RF_RSSI_INVALID)  ?  \
          (int8)CM_RSSI_NOT_AVAILABLE                                       :  \
          ((rssi) - CM_GET_RSSI_OFFSET())

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint8_t   sessionId;                           //! Number 1-255 assigned as they are created identifying each connection monitor session
  uint8_t   timesScanned;                        //! track count of recent events monitored to determine next priority CM session to avoid starvation
  uint8_t   timesMissed;                         //! missed count of recent events monitored
  uint32_t  accessAddr;                          //! return error code if failed to get conn info
  uint16_t  connInterval;                        //! connection interval time, range 12 to 6400 in  625us increments (7.5ms to 4s)
  uint16_t  scanDuration;                        //! Required scan window to capture minimum of 1 packet from Master and Slave up to max possible packet size
  uint8_t   hopValue;                            //! Hop value for conn alg 1, integer range (5,16)
  uint16_t  combSCA;                             //! mSCA + cmSCA
  uint8_t   currentChan;                         //! current data channel
  uint8_t   nextChan;                            //! next data channel
  uint8_t   chanMap[CM_NUM_BYTES_FOR_CHAN_MAP];  //! bitmap of used channels, use to reconstruct chanTableMap
  uint8_t   masterAddress[CM_DEVICE_ADDR_LEN];   //! BLE address of connection master
  uint8_t   slaveAddress[CM_DEVICE_ADDR_LEN];    //! BLE address of connection slave
  uint8_t   rssiMaster;                          //! last Rssi value master
  uint8_t   rssiSlave;                           //! last Rssi value slave
  uint32_t  timeStampMaster;                     //! last timeStamp master
  uint32_t  timeStampSlave;                      //! last timeStamp slave
  uint32_t  currentStartTime;                    //! Current anchor point
  uint32_t  nextStartTime;                       //! Record next planned scan anchor point to compare with competing CM sessions
  uint32_t  timerDrift;                          //! Clock timer drift
} ubCM_ConnInfo_t;

typedef struct
{
  uint8_t         numHandles;                     //! number of active handles corresponds to number of instances of ubCM_ConnInfo_t
  ubCM_ConnInfo_t ArrayOfConnInfo[CM_MAX_SESSIONS]; //! pointer to set of connection information for all active connecitons
} ubCM_GetConnInfoComplete_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern ubCM_GetConnInfoComplete_t ubCMConnInfo;

/*********************************************************************
 * API FUNCTIONS
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
bool ubCm_init(void);

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
uint8_t ubCM_isSessionActive(uint8_t sessionId);

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
uint8_t ubCM_start(uint8_t sessionId);

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
uint8_t ubCM_stop(uint8_t sessionId);


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
uint8_t ubCM_startExt(uint8_t *data);

/*********************************************************************
 * @fn      ubCM_findNextPriorityEvt
 *
 * @brief   Find the next connection event.
 *
 * @param   None.
 *
 * @return  next sessionId.
 */
uint8_t ubCM_findNextPriorityEvt(void);

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
void ubCM_setupNextCMEvent(uint8_t sessionId);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MICROBLECM_H */

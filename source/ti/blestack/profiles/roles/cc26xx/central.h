/******************************************************************************

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2011-2017, Texas Instruments Incorporated
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

/**
 *  @defgroup Central GAPRole (Central)
 *  @brief This module implements the Central GAP Role
 *  @{
 *  @file       central.h
 *  @brief      Central layer interface
 */

#ifndef CENTRAL_H
#define CENTRAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "osal.h"
#include "gap.h"

/*********************************************************************
 * CONSTANTS
 */

/** @defgroup Central_Constants Central GAPRole Constants
 * @{
 */

/** @defgroup Central_Params Central GAPRole Parameters
 * @{
 * Parameters set via @ref GAPCentralRole_SetParameter
 */

/**
 * @brief Identity Resolving Key (Read/Write)
 *
 * @note If this is set to all 0x00's, the SRK will be randomly generated
 *
 * size: uint8_t[16]
 *
 * default: 0x00000000000000000000000000000000
 *
 * range: 0x00000000000000000000000000000000 - 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
 */
#define GAPCENTRALROLE_IRK                         0x400  //!< Identity Resolving Key. Read/Write. Size is uint8_t[KEYLEN]. Default is all 0, which means that the IRK will be randomly generated.

/**
 * @brief Signature Resolving Key (Read/Write)
 *
 * @note If this is set to all 0x00's, the SRK will be randomly generated
 *
 * size: uint8_t[16]
 *
 * default: 0x00000000000000000000000000000000
 *
 * range: 0x00000000000000000000000000000000 - 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
 */
#define GAPCENTRALROLE_SRK                         0x401  //!< Signature Resolving Key. Read/Write. Size is uint8_t[KEYLEN]. Default is all 0, which means that the SRK will be randomly generated.

/**
 * @brief Sign Counter (Read/Write)
 *
 * size: uint32_t
 *
 * default: 0x0000
 *
 * range: 0x0000 - 0xFFFF
 */
#define GAPCENTRALROLE_SIGNCOUNTER                 0x402

/**
 * @brief Device Address read from the controller (Read-only)
 *
 * The BDADDR is read, in increasing order of priortiy, from the info page,
 * secondary address from flash, or set from @ref HCI_ReadBDADDRCmd
 *
 * size: uint8_t[6]
 *
 * default: BDADDR from info page
 *
 * range: 0x000000000000 - 0xFFFFFFFFFFFE
 */
#define GAPCENTRALROLE_BD_ADDR                     0x403

/**
 * @brief Maximmum number of scan reports to store from @ref GAPCentralRole_StartDiscovery (Read/Write)
 *
 * size: uint8_t
 *
 * default: 8
 *
 * range: 0-256 but this will be constrained by available RAM
 */
#define GAPCENTRALROLE_MAX_SCAN_RES                0x404

/**
 * @brief Action taken when a parameter update fails (Read/Write)
 *
 * size: uint8_t[16]
 *
 * default: @ref GAPCENTRALROLE_PARAM_UPDATE_REQ_AUTO_ACCEPT
 *
 * range: @ref Central_Param_Update_Options
 */
#define GAPCENTRALROLE_LINK_PARAM_UPDATE_REQ_REPLY 0x405
/** @} End Central_Params */

/** @defgroup Central_Param_Update_Options Param Update Request Reply operations
 * @{
 */
#define GAPCENTRALROLE_PARAM_UPDATE_REQ_AUTO_REJECT 0x00  //!< always reject what the remote peripheral is requesting.
#define GAPCENTRALROLE_PARAM_UPDATE_REQ_AUTO_ACCEPT 0x01  //!< always accept what the remote peripheral is requesting.
#define GAPCENTRALROLE_PARAM_UPDATE_REQ_SEND_TO_APP 0x02  //!< pass request onto application, requires a response before timeout.
/** @}  End Central_Param_Update_Options */

/** @} End Central_Constants */

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/** @defgroup Central_Structs Central GAPRole Structs
 * @{
 */

/// @brief Central Event Structure
typedef union
{
  gapEventHdr_t                gap;             //!< @ref GAP_MSG_EVENT and status.
  gapDeviceInitDoneEvent_t     initDone;        //!< GAP initialization done.
  gapDeviceInfoEvent_t         deviceInfo;      //!< Discovery device information event structure.
  gapDevDiscEvent_t            discCmpl;        //!< Discovery complete event structure.
  gapEstLinkReqEvent_t         linkCmpl;        //!< Link complete event structure.
  gapLinkUpdateEvent_t         linkUpdate;      //!< Link update event structure.
  gapUpdateLinkParamReqEvent_t linkUpdateReply; //!< Link update request reply structure.
  gapTerminateLinkEvent_t      linkTerminate;   //!< Link terminated event structure.
} gapCentralRoleEvent_t;

/** @} End Central_Structs */

/** @defgroup Central_CBs Central GAPRole Callbacks
 * @{
 * These are functions whose pointers are passed from the application
 * to the GAPRole so that the GAPRole can send events to the application
 */

/**
 * @brief Central Event Callback Function
 *
 * This callback is used by the Central GAPRole to forward GAP_Events to the
 * application that it receives from the stack.
 *
 * If the message is successfully queued to the application for later processing,
 * FALSE is returned because the application deallocates it later. Consider the
 * following state change event from simple_central as an example of this:
 *
 * @code{.c}
 * static void SimpleBLECentral_processAppMsg(sbcEvt_t *pMsg)
 * {
 *   switch (pMsg->hdr.event)
 *   {
 *     case SBC_STATE_CHANGE_EVT:
 *       SimpleBLECentral_processStackMsg((ICall_Hdr *)pMsg->pData);
 *
 *       // Free the stack message
 *       ICall_freeMsg(pMsg->pData);
 *       break;
 * @endcode
 *
 * If the message is not successfully queued to the application, TRUE is returned
 * so that the GAPRole can deallocate the message. If the heap has enough room,
 * the message must always be successfully enqueued.
 *
 * @param pEvent Pointer to event structure.
 *
 * @return  TRUE if safe to deallocate event message
 * @return  FALSE otherwise
 */
typedef uint8_t (*pfnGapCentralRoleEventCB_t)
(
  gapCentralRoleEvent_t *pEvent
);

/**
 * @brief Central GAPRole Callback Structure
 *
 * This must be setup by the application and passed to the GAPRole when
 * @ref GAPCentralRole_StartDevice is called.
 *
 * @param eventCB event callback
 */
typedef struct
{
  pfnGapCentralRoleEventCB_t  eventCB;      //!< event callback
} gapCentralRoleCB_t;
/** @} End Central_CBs */

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * API FUNCTIONS
 */

/*-------------------------------------------------------------------
 * Central Profile Public APIs
 */

/**
 * @brief   Start the device in Central role.  This function is typically
 *          called once during system startup.
 *
 * @param   pAppCallbacks pointer to application callbacks
 *
 * @return  @ref SUCCESS : Operation successful.
 * @return  @ref bleAlreadyInRequestedMode : Device already started.
 */
extern bStatus_t GAPCentralRole_StartDevice(gapCentralRoleCB_t *pAppCallbacks);

/**
 * @brief   Set a parameter in the Central Profile.
 *
 * @param   param profile parameter ID: @ref Central_Params
 * @param   len length of data to write
 * @param   pValue pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type.
 *
 * @return  @ref SUCCESS : Operation successful.
 * @return  @ref INVALIDPARAMETER : Invalid parameter ID.
 * @return  @ref bleInvalidRange : len is invalid for the given param
 */
extern bStatus_t GAPCentralRole_SetParameter(uint16_t param, uint8_t len, void *pValue);

/**
 * @brief   Get a parameter in the Central Profile.
 *
 * @param   param profile parameter ID: @ref Central_Params
 * @param   pValue pointer to buffer to contain the read data
 *
 * @return  @ref SUCCESS : Operation successful.
 * @return  @ref INVALIDPARAMETER : Invalid parameter ID.
 */
extern bStatus_t GAPCentralRole_GetParameter(uint16_t param, void *pValue);

/**
 * @brief   Terminate a link.
 *
 * @param   connHandle connection handle of link to terminate
 *          or @ref GAP_Conn_Handle
 *
 * @return  @ref SUCCESS : Terminate started.
 * @return  @ref bleIncorrectMode : No link to terminate.
 * @return  @ref HCI_ERROR_CODE_CONTROLLER_BUSY : terminate procedure has already started
 */
extern bStatus_t GAPCentralRole_TerminateLink(uint16_t connHandle);

/**
 * @brief   Establish a link to a peer device.
 *
 * @param   highDutyCycle TRUE to high duty cycle scan, FALSE if not
 * @param   whiteList determines use of the white list: @ref GAP_Whitelist
 * @param   addrTypePeer @ref Addr_type
 * @param   peerAddr peer device address
 *
 * @return  @ref SUCCESS : started establish link process.
 * @return  @ref bleIncorrectMode : invalid profile role.
 * @return  @ref bleNotReady : a scan is in progress.
 * @return  @ref bleAlreadyInRequestedMode : can't process now.
 * @return  @ref bleNoResources : too many links.
 */
extern bStatus_t GAPCentralRole_EstablishLink(uint8_t highDutyCycle, uint8_t whiteList,
                                              uint8_t addrTypePeer, uint8_t *peerAddr);

/**
 * @brief   Update the link connection parameters.
 *
 * @param   connHandle connection handle
 * @param   connIntervalMin minimum connection interval in 1.25ms units
 * @param   connIntervalMax maximum connection interval in 1.25ms units
 * @param   connLatency number of LL latency connection events
 * @param   connTimeout connection timeout in 10ms units
 *
 * @return  @ref SUCCESS : Connection update started started.
 * @return  @ref bleNotConnected : No connection to update.
 * @return  @ref INVALIDPARAMETER : connection parameters are invalid
 * @return  @ref HCI_ERROR_CODE_INVALID_HCI_CMD_PARAMS : connection parameters do not meet
 * Bluetooth low energy specification requirements:
 * > (Supervision Timeout) > (1 + Slave Latency) × (Connection Interval × 2)
 * @return  @ref HCI_ERROR_CODE_UNKNOWN_CONN_ID : connHandle is inactive
 * @return  @ref HCI_ERROR_CODE_CONTROLLER_BUSY : there is already a param update in process
 * @return  @ref HCI_ERROR_CODE_UNACCEPTABLE_CONN_PARAMETERS (0x3B): connection interval
 * does not work because it is not a multiple or divisor of intervals of the other simultaneous
 * connection or the interval of the connection is not less than the allowed maximum
 * connection interval as determined by the maximum number of connections times the
 * number of slots per connection
 */
extern bStatus_t GAPCentralRole_UpdateLink(uint16_t connHandle, uint16_t connIntervalMin,
                                           uint16_t connIntervalMax, uint16_t connLatency,
                                           uint16_t connTimeout);
/**
 * @brief   Start a device discovery scan.
 *
 * @param   mode discovery mode: @ref GAP_Discovery
 * @param   activeScan TRUE to perform active scan
 * @param   whiteList TRUE to only scan for devices in the white list
 *
 * @return  @ref SUCCESS : Discovery discovery has started.
 * @return  @ref bleIncorrectMode : Invalid profile role.
 * @return  @ref bleAlreadyInRequestedMode : Device discovery already started
 * @return  @ref HCI_ERROR_CODE_INVALID_HCI_CMD_PARAMS : bad parameter
 */
extern bStatus_t GAPCentralRole_StartDiscovery(uint8_t mode, uint8_t activeScan, uint8_t whiteList);

/**
 * @brief   Cancel a device discovery scan.
 *
 * @return  @ref SUCCESS : Cancel started.
 * @return  @ref bleInvalidTaskID : Not the task that started discovery.
 * @return  @ref bleIncorrectMode : Not in discovery mode.
 */
extern bStatus_t GAPCentralRole_CancelDiscovery(void);

/// @cond NODOC

/*-------------------------------------------------------------------
 * TASK FUNCTIONS - Don't call these. These are system functions.
 */

/**
 * @brief       Task creation function for the GAP Central Role.
 *
 */
extern void GAPCentralRole_createTask(void);

/// @endcond // NODOC

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CENTRAL_H */

/** @} End Central */

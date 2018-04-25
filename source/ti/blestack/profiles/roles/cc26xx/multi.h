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
 *  @defgroup Multi GAPRole (Multi)
 *  @brief This module implements the Multi GAP Role
 *  @{
 *  @file       multi.h
 *  @brief      Multi layer interface
 */

#ifndef MULTI_H
#define MULTI_H

#ifdef __cplusplus
extern "C"
{
#endif

/*-------------------------------------------------------------------
 * INCLUDES
 */
#include "gap.h"

/*-------------------------------------------------------------------
 * CONSTANTS
 */

/** @defgroup Multi_Constants Multi GAPRole Constants
 * @{
 */

/** @defgroup Multi_Params Multi GAPRole Parameters
 * @{
 * Parameters set via @ref GAPRole_SetParameter
 */

/**
 * @brief This parameter will return GAP Role type (Read-only)
 *
 * size: uint8_t
 *
 * range: @ref GAP_Profile_Roles
 */
#define GAPROLE_PROFILEROLE         0x300

/**
 * @brief Identity Resolving Key (Read/Write) Size is uint8_t[KEYLEN].
 *
 * @note If this is set to all 0x00's, the IRK will be randomly generated
 *
 * size: uint8_t[16]
 *
 * default: 0x00000000000000000000000000000000
 *
 * range: 0x00000000000000000000000000000000 - 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
 */
#define GAPROLE_IRK                 0x301

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
#define GAPROLE_SRK                 0x302

/**
 * @brief Sign Counter (Read/Write)
 *
 * size: uint32_t
 *
 * default: 0x0000
 *
 * range: 0x0000 - 0xFFFF
 */
#define GAPROLE_SIGNCOUNTER         0x303

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
#define GAPROLE_BD_ADDR             0x304

/**
 * @brief Enable/Disable Connectable Advertising (Read/Write)
 *
 * @warning @ref GAPROLE_ADV_NONCONN_ENABLED must be set to FALSE in order to enable this
 *
 * size: uint8_t
 *
 * default: TRUE
 *
 * range: TRUE (enabled) or FALSE (disabled)
 */
#define GAPROLE_ADVERT_ENABLED      0x305

/**
 * @brief How long to remain off (in sec) after advertising stops before starting again (Read/Write)
 *
 * If set to 0, advertising will not start again.
 *
 * size: uint16
 *
 * default: 30
 *
 * range: 0-65535
 */
#define GAPROLE_ADVERT_OFF_TIME     0x306

/**
 * @brief Advertisement data (Read/Write)
 *
 * @note The third byte sets limited / general advertising as defined in Vol 3, Part C, section 11.1.3
 * of the BT 4.2 Core Spec.
 *
 * size: a uint8_t array of up to 31 bytes
 *
 * default: 02:01:01 (general advertising)
 */
#define GAPROLE_ADVERT_DATA         0x307

/**
 * @brief Scan Response Data (Read/Write)
 *
 * @note This should be formatted as define d in Vol 3, Part C, section 11.1.3 of the BT 4.2 Core Spec.
 *
 * size: a uint8_t array of up to 31 bytes
 *
 * default: all 0x00's
 */
#define GAPROLE_SCAN_RSP_DATA       0x308

/**
 * @brief Advertisement Types (Read/Write)
 *
 * size: uint8_t
 *
 * default: @ref GAP_ADTYPE_ADV_IND
 *
 * range: @ref GAP_Adv_Types
 */
#define GAPROLE_ADV_EVENT_TYPE      0x309

/**
 * @brief Direct Advertisement Type (Read/Write)
 *
 * size: uint8_t
 *
 * default: @ref ADDRMODE_PUBLIC
 *
 * range: @ref Gap_Addr_Modes
 */
#define GAPROLE_ADV_DIRECT_TYPE     0x30A

/**
 * @brief Direct Advertisement Address (Read/Write)
 *
 * size: uint8_t[6]
 *
 * default: NULL
 *
 * range: 0x000000000000 - 0xFFFFFFFFFFFE
 */
#define GAPROLE_ADV_DIRECT_ADDR     0x30B

/**
 * @brief Which channels to advertise on (Read/Write)
 *
 * Multiple channels can be selected by ORing the bit values below.
 *
 * size: uint8_t
 *
 * default: @ref GAP_ADVCHAN_ALL
 *
 * range: @ref GAP_Adv_Chans
 */
#define GAPROLE_ADV_CHANNEL_MAP     0x30C

/**
 * @brief Policy for filtering advertisements (Read/Write)
 *
 * @note This is ignored for direct advertising.
 *
 * size: uint8_t
 *
 * default: @ref GAP_FILTER_POLICY_ALL
 *
 * range: @ref GAP_Adv_Filter_Policices
 */
#define GAPROLE_ADV_FILTER_POLICY   0x30D

/**
 * @brief  Minimum connection interval (n * 1.25 ms) to use when performing param update (Read/Write)
 *
 * size: uint16_t
 *
 * default: 6
 *
 * range: 6 - @ref GAPROLE_MAX_CONN_INTERVAL
 */
#define GAPROLE_MIN_CONN_INTERVAL   0x311

/**
 * @brief Maximum connection interval (n * 1.25 ms) to use when performing param update (Read/Write)
 *
 * size: uint16_t
 *
 * default: 3200
 *
 * range: @ref GAPROLE_MIN_CONN_INTERVAL - 3200
 */
#define GAPROLE_MAX_CONN_INTERVAL   0x312

/**
 * @brief Slave latency to use when performing param update (Read/Write)
 *
 * size: uint16_t
 *
 * default: 0
 *
 * range: 0 - 499
 */
#define GAPROLE_SLAVE_LATENCY       0x313

/**
 * @brief Supervision timeout (n x 10 ms) to use when performing param update (Read/Write)
 *
 * size: uint16_t
 *
 * default: 1000
 *
 * range: 10-3200
 */
#define GAPROLE_TIMEOUT_MULTIPLIER  0x314

/**
 * @brief Enable / Disable non-connectable advertising (Read/Write)
 *
 * @warning @ref GAPROLE_ADVERT_ENABLED must be set to FALSE in order to enable this
 *
 * size: uint8_t
 *
 * default: FALSE
 *
 * range: TRUE (enable) or FALSE (disable)
 */
#define GAPROLE_ADV_NONCONN_ENABLED 0x31B

/**
 * @brief Maximmum number of scan reports to store from @ref GAPCentralRole_StartDiscovery (Read/Write)
 *
 * size: uint8_t
 *
 * default: 8
 *
 * range: 0-256 but this will be constrained by available RAM
 */
#define GAPROLE_MAX_SCAN_RES        0x404
/** @} End Multi_Params */

/** @defgroup Multi_Param_Update_Fail_Actions Failed Parameter Update Actions
 * @{
 *  Possible actions the device may take if an unsuccessful parameter
 *  update is received.
 */
#define GAPROLE_NO_ACTION                    0 //!< Take no action upon unsuccessful parameter updates
#define GAPROLE_RESEND_PARAM_UPDATE          1 //!< Continue to resend request until successful update
#define GAPROLE_TERMINATE_LINK               2 //!< Terminate link upon unsuccessful parameter updates
/** @} End Multi_Param_Update_Fail_Actions */

/** @defgroup Multi_Param_Update_Options Parameter Update Options
 * @{
 *  Possible actions the device may take when it receives a
 *  Connection Parameter Update Request.
 */
#define GAPROLE_LINK_PARAM_UPDATE_ACCEPT       0 //!< Accept all parameter update requests
#define GAPROLE_LINK_PARAM_UPDATE_REJECT       1 //!< Reject all parameter update requests
#define GAPROLE_LINK_PARAM_UPDATE_APP_DECIDES  2 //!< Notify app for it to decide
#define GAPROLE_LINK_PARAM_UPDATE_NUM_OPTIONS  3 //!< Number of options. Used for parameter checking.
/** @} End Multi_Param_Update_Options */

/** @} End Multi_Constants */

/*-------------------------------------------------------------------
 * TYPEDEFS
 */

/** @defgroup Multi_Structs Multi GAPRole Structures
 * @{
 */

/// @brief Multi GAPRole Event Structure
typedef union
{
  gapEventHdr_t             gap;                //!< @ref GAP_MSG_EVENT and status.
  gapDeviceInitDoneEvent_t  initDone;           //!< GAP initialization done.
  gapDeviceInfoEvent_t      deviceInfo;         //!< Discovery device information event structure.
  gapDevDiscEvent_t         discCmpl;           //!< Discovery complete event structure.
  gapEstLinkReqEvent_t      linkCmpl;           //!< Link complete event structure.
  gapLinkUpdateEvent_t      linkUpdate;         //!< Link update event structure.
  gapTerminateLinkEvent_t   linkTerminate;      //!< Link terminated event structure.
} gapMultiRoleEvent_t;

/// @brief Multi GAPRole Parameter Update Structure
typedef struct
{
  uint8_t paramUpdateEnable;            //!< @ref Multi_Param_Update_Options
  uint16_t connHandle;                  //!< connection handle
  uint16_t minConnInterval;             //!< minimum connection interval
  uint16_t maxConnInterval;             //!< maximum connection interval
  uint16_t slaveLatency;                //!< slave latency
  uint16_t timeoutMultiplier;           //!< supervision timeout
} gapRole_updateConnParams_t;
/** @} End Multi_Structs */

/*-------------------------------------------------------------------
 * MACROS
 */

/*-------------------------------------------------------------------
 * Profile Callbacks
 */

/** @defgroup Multi_CBs Multi GAPRole Callbacks
 * @{
 * These are functions whose pointers are passed from the application
 * to the GAPRole so that the GAPRole can send events to the application
 */

/**
 * @brief Multi Event Callback Function
 *
 * This callback is used by the Multi GAPRole to forward GAP_Events to the
 * application.
 *
 * If the message is successfully queued to the application for later processing,
 * FALSE is returned because the application deallocates it later. Consider the
 * following state change event from multi_role as an example of this:
 *
 * @code{.c}
 * static void multi_role_processAppMsg(mrEvt_t *pMsg)
 * {
 *   switch (pMsg->event)
 *   {
 *   case MR_STATE_CHANGE_EVT:
 *     multi_role_processStackMsg((ICall_Hdr *)pMsg->pData);
 *     // Free the stack message
 *     ICall_freeMsg(pMsg->pData);
 *     break;
 * @endcode
 *
 * If the message is not successfully queued to the application, TRUE is returned
 * so that the GAPRole can deallocate the message. If the heap has enough room,
 * the message must always be successfully enqueued.
 *
 * @param pEvent Pointer to event structure
 *
 * @return  TRUE if safe to deallocate event message
 * @return  FALSE otherwise
 */
typedef uint8_t (*passThroughToApp_t)
(
  gapMultiRoleEvent_t *pEvent
);

/**
 * @brief Callback for the app to decide on a parameter update request
 *
 * This callback will be used if the @ref GAP_UPDATE_LINK_PARAM_REQ_EVENT parameter
 * is set to @ref GAPROLE_LINK_PARAM_UPDATE_APP_DECIDES
 *
 * @param pReq Pointer to param update request
 * @param pRsp Pointer to param update response.
 */
typedef void (*paramUpdateAppDecision_t)
(
  gapUpdateLinkParamReq_t *pReq,
  gapUpdateLinkParamReqReply_t *pRsp
);

/**
 * @brief Multi GAPRole Callback structure
 *
 * This must be setup by the application and passed to the GAPRole when
 * @ref GAPRole_StartDevice is called.
 */
typedef struct
{
  passThroughToApp_t        pfnPassThrough;             //!< When the event should be processed by the app instead of the GAP Role
  paramUpdateAppDecision_t  pfnParamUpdateAppDecision;  //!< When the app should decide on a param update request
} gapRolesCBs_t;

/** @} End Multi_Structs */

/*-------------------------------------------------------------------
 * API FUNCTIONS
 */

/**
 * @brief       Set a GAP Role parameter.
 *
 * @note
 *        The "len" field must be set to the size of a "uint16_t" and the
 *        "pValue" field must point to a "uint16_t".
 *
 * @param param     @ref Multi_Params
 * @param len       length of data to write
 * @param pValue    pointer to data to write.  This is dependent on
 *   the parameter ID and will be cast to the appropriate
 *   data type (example: data type of uint16_t will be cast to
 *   uint16_t pointer).
 * @param           connHandle connection handle
 *
 * @return @ref SUCCESS
 * @return @ref INVALIDPARAMETER
 * @return  @ref bleInvalidRange : len is invalid for the given param
 */
extern bStatus_t GAPRole_SetParameter(uint16_t param, uint8_t len, void *pValue, uint8 connHandle);

/**
 * @brief       Get a GAP Role parameter.
 *
 * @note
 *        The "pValue" field must point to a "uint16_t".
 *
 * @param param     @ref Multi_Params
 * @param pValue    pointer to location to get the value.  This is dependent on
 *   the parameter ID and will be cast to the appropriate
 *   data type (example: data type of uint16_t will be cast to
 *   uint16_t pointer).
 * @param           connHandle connection handle
 *
 * @return @ref SUCCESS
 * @return @ref INVALIDPARAMETER
 */
extern bStatus_t GAPRole_GetParameter(uint16_t param, void *pValue, uint8 connHandle);

/**
 * @brief Initialize the GAP layer.
 *
 * @warning Only call this function once.
 *
 * @param       pAppCallbacks @ref gapRolesCBs_t
 * @param       numConns a pointer to the desired number of connections that the
 *              application wants is passed in with this parameter. the GAPRole
 *              will use this value to negotiate with the amount of connections
 *              that the stack supports and place the negotiated value in this
 *              memory location for return to the app.
 *
 * @return      @ref SUCCESS
 * @return  @ref bleAlreadyInRequestedMode : Device already started.
 */
bStatus_t GAPRole_StartDevice(gapRolesCBs_t *pAppCallbacks, uint8_t* numConns);

/**
 * @brief       Terminates the existing connection.
 *
 * @param       connHandle handle of connection to terminate
 *
 * @return      @ref SUCCESS
 * @return      @ref bleIncorrectMode
 * @return      @ref HCI_ERROR_CODE_CONTROLLER_BUSY : terminate procedure has already started
 */
extern bStatus_t GAPRole_TerminateConnection(uint16_t connHandle);

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
extern bStatus_t GAPRole_StartDiscovery(uint8_t mode, uint8_t activeScan, uint8_t whiteList);

/**
 * @brief   Cancel a device discovery scan.
 *
 * @return  @ref SUCCESS : Cancel started.
 * @return  @ref bleInvalidTaskID : Not the task that started discovery.
 * @return  @ref bleIncorrectMode : Not in discovery mode.
 */
extern bStatus_t GAPRole_CancelDiscovery(void);

/**
 * @brief   Establish a link to a peer device.
 *
 * @param   highDutyCycle  TRUE to high duty cycle scan, FALSE if not
 * @param   whiteList determines use of the white list: @ref GAP_Whitelist
 * @param   addrTypePeer @ref Addr_type
 * @param   peerAddr peer device address
 *
 * @return  @ref SUCCESS : started establish link process
 * @return  @ref bleIncorrectMode : invalid profile role
 * @return  @ref bleNotReady : a scan is in progress
 * @return  @ref bleAlreadyInRequestedMode : can’t process now
 * @return  @ref bleNoResources : too many links
 */
extern bStatus_t GAPRole_EstablishLink(uint8_t highDutyCycle, uint8_t whiteList,
                                              uint8_t addrTypePeer, uint8_t *peerAddr);


/**
 * @brief   Send a connection parameter update to a connected device
 *
 * @param   handleFailure @ref Multi_Param_Update_Fail_Actions
 * @param   pConnParams pointer to connection parameters
 *
 * @return  @ref SUCCESS : operation was successful.
 * @return  @ref INVALIDPARAMETER : Data can not fit into one packet.
 * @return  @ref MSG_BUFFER_NOT_AVAIL : No HCI buffer is available.
 * @return  @ref bleInvalidRange : params do not satisfy spec
 * @return  @ref bleIncorrectMode : invalid profile role.
 * @return  @ref bleAlreadyInRequestedMode : already updating link parameters.
 * @return  @ref bleNotConnected : Connection is down
 * @return  @ref bleMemAllocError : Memory allocation error occurred.
 * @return  @ref bleNoResources : No available resource
 */
extern bStatus_t gapRole_connUpdate(uint8_t handleFailure,
                                       gapRole_updateConnParams_t *pConnParams);

/// @cond NODOC

/*-------------------------------------------------------------------
 * TASK FUNCTIONS - Don't call these. These are system functions.
 */

/**
 * @brief       Task creation function for the GAP Multi Role.
 *
 */
extern void GAPRole_createTask(void);

extern void gapRole_abort(void);

/// @endcond // NODOC

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* MULTI_H */

/** @} End Multi */

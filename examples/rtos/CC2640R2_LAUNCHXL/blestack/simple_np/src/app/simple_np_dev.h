/******************************************************************************

 @file       simple_np_dev.h

 @brief This file contains the parsing of Device related command for the
        Simple Peripheral sample application, for use with the
        CC2650 Bluetooth Low Energy Protocol Stack.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2015-2017, Texas Instruments Incorporated
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
@page SNP_DEV_API  SNP API: DEVICE

  @section SNP_DEV_API_SUMMARY Device API summary
   Commands Acronyms / Event Name                 | Origin  | Type        | TL Cmd0                       | TL Cmd1                        | Parameter structure
--------------------------------------------------|---------|-------------|-------------------------------|--------------------------------|---------------------
   [SNP_POWER_UP_IND   ] (\ref SNP_powerUp_ind)   | NP      | Async       | SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_POWER_UP_IND   (0x01)     |  |
   [SNP_MASK_EVT_REQ   ] (\ref SNP_maskEvt)       | AP      | Sync Req    | SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_MASK_EVT_REQ   (0x02)     | #snpMaskEventReq_t
   [SNP_MASK_EVENT_RSP ] (\ref SNP_maskEvt)       | NP      | Sync Rsp    | SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_MASK_EVENT_RSP (0x02)     | #snpMaskEventRsp_t
   [SNP_GET_REVISION_REQ] (\ref SNP_getRev)       | AP      | Sync Req    | SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_GET_REVISION_REQ (0x03)   |  |
   [SNP_GET_REVISION_RSP] (\ref SNP_getRev)       | NP      | Sync Rsp    | SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_GET_REVISION_RSP (0x03)   | #snpGetRevisionRsp_t
   [SNP_HCI_CMD_REQ] (\ref SNP_executeHCIcmd)     | AP      | Async       | SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_HCI_CMD_REQ (0x04)        | #snpHciCmdReq_t
   [SNP_HCI_CMD_RSP] (\ref SNP_executeHCIcmd)     | NP      | Async       | SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_HCI_CMD_RSP (0x04)        | #snpHciCmdRsp_t
   [SNP_EVENT_IND] (\ref SNP_Event_ind)           | NP      | Async       | SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_EVENT_IND (0x05)          | #snpEvt_t
   [SNP_GET_STATUS_REQ] (\ref SNP_getStatus)      | AP      | Sync Req    | SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_GET_STATUS_REQ (0x06)     |  |
   [SNP_GET_STATUS_RSP] (\ref SNP_getStatus)      | NP      | Sync Rsp    | SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_GET_STATUS_RSP (0x06)     | #snpGetStatusCmdRsp_t
   [SNP_GET_RAND_REQ] (\ref SNP_getRand)          | AP      | Sync Req    | SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_GET_RAND_REQ (0x07)       |  |
   [SNP_GET_RAND_RSP] (\ref SNP_getRand)          | NP      | Sync Rsp    | SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_GET_RAND_RSP (0x07)       | #snpGetRandRsp_t
   [SNP_TEST_REQ] (\ref SNP_executeTestCmd)       | AP      | Sync Req    | SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_TEST_REQ (0x10)           |  |
   [SNP_TEST_RSP] (\ref SNP_executeTestCmd)       | NP      | Sync Rsp    | SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_TEST_RSP (0x10)           | #snpTestCmdRsp_t
   [SNP_SYNC_ERROR_CMD_IND] (\ref Sync_error)     | NP      | Async       | SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_SYNC_ERROR_CMD_IND (0x07) |  |

  All these command have some parameters. The parameters can be found/set using the indicated Parameter structure.
  All Parameter/Structures should be little-endian format (LSB first).
  For UUID parameters, @ref UUID_desc

  @section SNP_powerUp_ind SNP Power Up Indication

  NP -> AP (N/A) \n
   CMD 0                  | CMD 1
  ----------------------- | --------------------------
   0x55                   | 0x01
   SNP_NPI_ASYNC_CMD_TYPE | SNP_GET_REVISION_RSP

  This event is send by the SNP once the device has powered up.
  This event will be received if the device is reset unexpectedly.
  One reception of this event the AP should consider that the NP lost any previous configuration.

  When this event is received, the following services are initialized and ready to be used:
  - GAP service
  - GATT service
  - Device Info Service.

  This command doesn't have any parameters

  @section SNP_Event_ind SNP Event Indication

  NP -> AP (N/A) \n
   CMD 0                  | CMD 1
  ----------------------- | --------------------------
   0x55                   | 0x05
   SNP_NPI_ASYNC_CMD_TYPE | SNP_EVENT_IND

  This indication is send by the SNP to signal an event.
  Events can be masked by using the SNP_maskEvt API (\ref SNP_maskEvt).
  Only one event will be indicated at a time per packet. If several events occur, each will be encapsulated in their own TL packet.

  @par Payload Structure

  Each event has a 2-bytes Event type, @ref SNP_EVENT
  Along with Event type, some events might have some parameters associated with them.

  Events parameters are mapped on the @ref snpEvt_t structure, which contains a union of #snpConnEstEvt_t, #snpConnTermEvt_t , #snpUpdateConnParamEvt_t , #snpAdvStatusEvt_t and #snpATTMTUSizeEvt_t

  The table below list the events and the structure used to MAP the parameter of those events:

  Event                      |  Parameter Structure                 | Related
 --------------------------- | ------------------------------------ | ---------------------
  SNP_CONN_EST_EVT           | #snpEvt_t + #snpConnEstEvt_t         | N / A
  SNP_CONN_TERM_EVT          | #snpEvt_t + #snpConnTermEvt_t        | #SNP_terminateConn
  SNP_CONN_PARAM_UPDATED_EVT | #snpEvt_t + #snpUpdateConnParamEvt_t | #SNP_updateConnParam
  SNP_ADV_STARTED_EVT        | #snpEvt_t + #snpAdvStatusEvt_t       | #SNP_startAdv
  SNP_ADV_ENDED_EVT          | #snpEvt_t + #snpAdvStatusEvt_t       | #SNP_stopAdv
  SNP_ATT_MTU_EVT            | #snpEvt_t + #snpATTMTUSizeEvt_t      | @ref SNP_charRead
  SNP_ERROR_EVT              | #snpEvt_t + #snpErrorEvt_t           | @ref Sync_error

  NP -> AP \n
  @ref snpEvt_t \n
   Size    | Description
  -------- | -----------------------------------------
   2 Byte  | @ref SNP_CONN_PARAM_UPDATED_EVT
   X Byte  | Parameter Structure based on table above

  @subsection SNP_event_MTU ATT MTU Size
  The default ATT_MTU size is set to 23 bytes.
  The GATT client can request an ATT_MTU_EXCHANGE method to change the maximum possible ATT MTU size.
  The SNP is compatible with ATT MTU sizes up to 251 Bytes.
  If an update occurs due to a request from a peer device, the corresponding event will be send by the SNP to inform AP that MTU size has changed.
  If this event is not received, the AP must assume that ATT MTU size is 23.

  @par Payload Structure

  NP -> AP \n
  @ref snpEvt_t \n
   Size    | Description
  -------- | -----------------------------------------
   2 Byte  | @ref SNP_ATT_MTU_EVT
   1 Byte  | Connection Handle
   2 Byte  | MTU Size of Connection

  @subsection SNP_event_Error Error Event Indication
  This error event occurs if something unexpected occurs while SNP is running some operation.
  The list of all possible SNP errors can be found here \ref SNP_ERRORS

  @section Sync_error   SNP Synchronous Invalid Command Indication (0x07)

  NP -> AP (N/A) \n
   CMD 0                  | CMD 1
  ----------------------- | --------------------------
   0x55                   | 0x07
   SNP_NPI_ASYNC_CMD_TYPE | SNP_SYNC_ERROR_CMD_IND

  NP -> AP (N/A) \n
   CMD 0                  | CMD 1
  ----------------------- | --------------------------
   0x55                   | 0x05
   SNP_NPI_ASYNC_CMD_TYPE | SNP_EVENT_IND

  This indication packet will be send over TL if a unknown synchronous packet is send to the TL.

  This indication is sent back to prevent TL lock up while waiting for a response.

  @par Payload Structure

  NP -> AP \n
  No Parameter Structure

  In addition to the indication, @ref SNP_EVENT_IND with @ref SNP_ERROR_EVT will be sent to AP with faulting opcode in parameter structure.

  NP -> AP \n
  @ref snpEvt_t \n
   Size    | Description
  -------- | -----------------------------------------
   2 Byte  | @ref SNP_ERROR_EVT
   2 Byte  | SNP Opcode which produced error
   1 Byte  | Error Status @ref SNP_ERRORS

*/

/**
 *  @addtogroup SNP
 *  @{
 *  @defgroup SNP_DEV SNP Device API
 *  @{
 *  @file       simple_np_dev.h
 *  @brief  SNP Device API
 *
 *  This file contains the interface for SNP Device.
 */

#ifndef SIMPLENP_DEV_H
#define SIMPLENP_DEV_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */

/**********************************************************************
* TYPEDEFS - Initialization and Configuration
*/

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/**
 *  @brief  SNP Get Revision
 *
 *  AP -> NP (SNP_getRev) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x03
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_GET_REVISION_REQ
 *
 *  NP -> AP (SNP_getRev) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x03
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_GET_REVISION_RSP
 *
 *  The SNP Get Revision command is used to get the current revision of the SNP API
 *  as well as the full stack revision number as defined in the HCI vendor guide.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_GET_REVISION_REQ (from AP) | @ ref  SNP_GET_REVISION_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  No Parameter Payload
 *
 *  NP -> AP \n
 *  @ref snpGetRevisionRsp_t \n
 *  Size    | Description
 * -------- | -------------------------------------
 *  1  Byte | Command Status @ref SNP_ERRORS
 *  2  Byte | Version of SNP [Major, Minor]
 *  10 Byte | See TI Vendor Specific HCI Guide
 *
 *  @param[out]  Msg : Get Revision Response Structure, @ref snpGetRevisionRsp_t
 *
 */
void SNP_getRev(snpGetRevisionRsp_t *Msg);

/**
 *  @brief  SNP Get Random Number
 *
 *  AP -> NP (SNP_getRand) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x08
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_GET_RAND_REQ
 *
 *  NP -> AP (SNP_getRand) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x08
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_GET_RAND_RSP
 *
 *  This SNP Test command is used to access the
 *  true random number generator on the SNP and return a
 *  random number to the AP.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_GET_RAND_REQ (from AP) | @ref SNP_GET_RAND_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  No Parameter Payload
 *
 *  NP -> AP \n
 *  @ref snpGetRandRsp_t \n
 *  Size    | Description
 * -------- | -------------------------------------
 *  4  Byte | 32-bit Random Number from TRNG
 *
 *  @param[out]  pRsp : Get Random Number Response Structure @ref snpGetRandRsp_t
 *
 */
void SNP_getRand(snpGetRandRsp_t *pRsp);

/**
 *  @brief SNP Mask Event (0x02)
 *
 *  AP -> NP (SNP_maskEvt) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x02
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_MASK_EVENT_RSP
 *
 *  NP -> AP (SNP_maskEvt) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x02
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_MASK_EVENT_RSP
 *
 *  The SNP Mask Event command enables the AP to mask some events
 *  returned from the SNP if they are not needed by the AP.
 *
 *  Masking events may be useful in order to limit the possible
 *  wake up conditions of the AP, thus reducing power consumption.
 *
 *  All of the events that can be masked by this command are triggered
 *  asynchronously due to an action from the AP, an action of the
 *  remote peer, or timer expiration.
 *
 *  By default, ALL events are enabled and will be returned to the AP.
 *
 *  For a list of events, see @ref SNP_EVENT.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_MASK_EVENT_RSP (from AP) | @ref SNP_MASK_EVENT_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpMaskEventReq_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------
 *  2  Byte | Events Requested to be Masked @ref SNP_EVENT
 *
 *  NP -> AP \n
 *  @ref snpMaskEventRsp_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------
 *  2  Byte | Events that have been Masked @ref SNP_EVENT
 *
 *  @param[in]   pReq      : Mask Event Request Structure, @ref snpMaskEventReq_t
 *  @param[out]  pRsp      : Mask Event Request Response, @ref snpMaskEventRsp_t
 *
 */
void SNP_maskEvt(snpMaskEventReq_t* pReq, snpMaskEventRsp_t *pRsp);

/**
 *  @brief  SNP Get Status
 *
 *  AP -> NP (SNP_getStatus) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x06
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_GET_STATUS_REQ
 *
 *  NP -> AP (SNP_getStatus) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x06
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_GET_STATUS_RSP
 *
 *  The SNP Get Status command is used to get the current status of the SNP
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_GET_STATUS_REQ (from AP) | SNP_GET_STATUS_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  No Parameter Structure
 *
 *  NP -> AP \n
 *  @ref snpGetStatusCmdRsp_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------
 *  1  Byte | Current State of the GAP Role @ref gaprole_States_t
 *  1  Byte | Current State of Advertising (TRUE | FALSE)
 *  1  Byte | Current State of the GATT server (TRUE | FALSE)
 *  1  Byte | Current GATT Methods (only if previous byte is TRUE)
 *
 *  @param[out]  pRsp : Get Status Response Structure, @ref snpGetStatusCmdRsp_t
 *
 */
void SNP_getStatus(snpGetStatusCmdRsp_t *pRsp);

/**
 *  @brief  SNP Execute Memory Test
 *
 *  AP -> NP (SNP_executeTestCmd) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x10
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_GET_STATUS_REQ
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x10
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_GET_STATUS_RSP
 *
 *  This SNP Test command is used to profile the heap usage of the SNP.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_TEST_REQ (from AP) | @ref SNP_TEST_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  No Parameter Structure
 *
 *  NP -> AP \n
 *  @ref snpTestCmdRsp_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------
 *  2  Byte | HEAP allocated in bytes
 *  2  Byte | Peak HEAP Allocated since startup
 *  2  Byte | Total HEAP Size
 *
 *  @param[out]  pRsp :  Heap Test Command Response Structure, @ref snpTestCmdRsp_t
 *
 */
void SNP_executeTestCmd(snpTestCmdRsp_t *pRsp);

/**
 *  @brief  SNP Execute HCI Command (0x04)
 *
 *  AP -> NP (SNP_executeTestCmd) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x04
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_HCI_CMD_REQ
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x04
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_HCI_CMD_RSP
 *
 *  The SNP Encapsulated HCI Command is used to encapsulate and send an HCI command
 *  to the SNP. Only the HCI commands listed in the parameter section below are supported.
 *
 *  The functionality of the HCI extension commands can be found in the HCI Vendor Specific Guide.
 *  The format of these commands can also be found in the Vendor Specific guide.
 *  Note that some HCI commands are defined in the Bluetooth Specification
 *
 *  The list of supported HCI commands are @ref SNP_ALLOWED_HCI
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_HCI_CMD_REQ (from AP) | @ref SNP_HCI_CMD_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpHciCmdReq_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------
 *  2  Byte | HCI Command Opcode to Execute @ref SNP_ALLOWED_HCI
 *  X  Byte | Parameters, if required by HCI Command
 *
 *  NP -> AP \n
 *  @ref snpHciCmdRsp_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------
 *  1  Byte | Command Status @ref SNP_ERRORS
 *  2  Byte | HCI Command Opcode Executed @ref SNP_ALLOWED_HCI
 *  X  Byte | Return Values if HCI Command has any
 *
 *  @param[in]   pReq     :HCI Request Structure, @ref snpHciCmdReq_t
 *  @param[in]   dataLen  :Length in Bytes of snpHciCmdReq_t::pData
 *
 *  @return  1 Byte Error Code, @ref SNP_ERRORS
 *
 */
uint8_t SNP_executeHCIcmd(snpHciCmdReq_t *pReq, uint16_t dataLen);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLENP_DEV_H */

/** @} End SNP_DEV */

/** @} */ // end of SNP

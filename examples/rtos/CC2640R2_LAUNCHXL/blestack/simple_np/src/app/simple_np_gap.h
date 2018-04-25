/******************************************************************************

 @file       simple_np_gap.h

 @brief This file contains the parsing of GAP related command for the
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
@page SNP_GAP_API  SNP API: GAP

 @section SNP_GAP_API_SUMMARY SNP GAP API summary

 Commands Acronyms / Event Name                                       | Origin | Type     | TL Cmd0                        | TL Cmd1                                  | Parameter structure
--------------------------------------------------------------------- | ------ |--------- |------------------------------- |----------------------------------------- | --------------------
 [SNP_START_ADV_REQ]  (\ref SNP_startAdv)                             | AP     | Async    | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_START_ADV_REQ              (0x42)   | #snpStartAdvReq_t
 [SNP_SET_ADV_DATA_REQ] (\ref SNP_setAdvData )                        | AP     | Async    | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_SET_ADV_DATA_REQ           (0x43)   | #snpSetAdvDataReq_t
 [SNP_SET_ADV_DATA_CNF] (\ref SNP_setAdvData )                        | NP     | Async    | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_SET_ADV_DATA_CNF           (0x43)   | #snpSetAdvDataCnf_t
 [SNP_STOP_ADV_REQ]   (\ref SNP_stopAdv)                              | AP     | Async    | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_STOP_ADV_REQ               (0x44)   |  |
 [SNP_UPDATE_CONN_PARAM_REQ] (\ref SNP_updateConnParam )              | AP     | Async    | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_UPDATE_CONN_PARAM_REQ      (0x45)   | #snpUpdateConnParamReq_t
 [SNP_UPDATE_CONN_PARAM_CNF] (\ref SNP_updateConnParam )              | NP     | Async    | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_UPDATE_CONN_PARAM_CNF      (0x45)   | #snpUpdateConnParamCnf_t
 [SNP_TERMINATE_CONN_REQ]  (\ref SNP_terminateConn)                   | AP     | Async    | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_TERMINATE_CONN_REQ         (0x46)   | #snpTermConnReq_t
 [SNP_SET_GAP_PARAM_REQ] (\ref SNP_setGapParam )                      | AP     | Sync Req | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_SET_GAP_PARAM_REQ          (0x48)   | #snpSetGapParamReq_t
 [SNP_SET_GAP_PARAM_RSP] (\ref SNP_setGapParam )                      | NP     | Sync Rsp | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_SET_GAP_PARAM_RSP          (0x48)   | #snpSetGapParamRsp_t
 [SNP_GET_GAP_PARAM_REQ] (\ref SNP_getGapParam )                      | AP     | Sync Req | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_GET_GAP_PARAM_REQ          (0x49)   | #snpGetGapParamReq_t
 [SNP_GET_GAP_PARAM_RSP] (\ref SNP_getGapParam )                      | NP     | Sync Rsp | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_GET_GAP_PARAM_RSP          (0x49)   | #snpGetGapParamRsp_t
 [SNP_SET_SECURITY_PARAM_REQ] (\ref SNP_setSecurityParams )           | AP     | Sync Req | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_SET_SECURITY_PARAM_REQ     (0x4A)   | #snpSetSecParamReq_t
 [SNP_SET_SECURITY_PARAM_RSP] (\ref SNP_setSecurityParams )           | NP     | Sync Rsp | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_SET_SECURITY_PARAM_REQ     (0x4A)   | #snpSetSecParamRsp_t
 [SNP_SEND_SECURITY_REQUEST_REQ] (\ref SNP_sendSecurityRequest)       | AP     | Async    | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_SEND_SECURITY_REQUEST_REQ  (0x4B)   |  |
 [SNP_SET_AUTHENTICATION_DATA_REQ] (\ref SNP_setAuthenticationData)   | AP     | Sync Req | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_SET_AUTHENTICATION_DATA_REQ  (0x4C) | #snpSetAuthDataReq_t
 [SNP_SEND_AUTHENTICATION_DATA_RSP] (\ref SNP_setAuthenticationData)  | NP     | Sync Rsp | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_SEND_AUTHENTICATION_DATA_RSP (0x4C) | #snpSetAuthDataRsp_t
 [SNP_SET_WHITE_LIST_POLICY_REQ] (\ref SNP_setWhiteListFilterPolicy ) | AP     | Sync Req | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_SET_WHITE_LIST_POLICY_REQ  (0x4D)   | #snpSetWhiteListReq_t
 [SNP_SET_WHITE_LIST_POLICY_RSP] (\ref SNP_setWhiteListFilterPolicy ) | NP     | Sync Rsp | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_SET_WHITE_LIST_POLICY_REQ  (0x4D)   | #snpSetWhiteListRsp_t

  All these command have some parameters. The parameters can be found/set using the indicated Parameter structure.
  All Parameter/Structures should be little-endian format (LSB first).
  For UUID parameters, @ref UUID_desc

 */

/**
 *  @addtogroup SNP
 *  @{
 *  @defgroup SNP_GAP SNP GAP API
 *  @{
 *  @file       simple_np_gap.h
 *  @brief  SNP GAP Layer API
 *
 *  This file contains the interface to the SNP GAP Layer
 */

#ifndef SIMPLEBLEPROCESSOR_GAP_H
#define SIMPLEBLEPROCESSOR_GAP_H

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

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/**
 *  @brief      SNP Start Advertisement
 *
 *  AP -> NP (SNP_startAdv) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x42
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_START_ADV_REQ
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x05
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_EVENT_IND
 *
 *  The SNP Start Advertisement command is sent to the SNP to start advertising
 *  on all 3 channels.
 *
 *  Note that the @ref SNP_setAdvData (0x43) command should be called before
 *  this command in order to set the advertising data.
 *
 *  @par         Additional Notes
 *
 *  If a timeout value equal to 0 is used, the SNP will advertise infinitely
 *  if it is in general advertisement mode or for 180 seconds if it is in
 *  limited discovery mode. See @ref SNP_setGapParam (0x49) command
 *  for setting the advertising mode.
 *
 *  If an interval value equal to 0 is used, the default value of 100 ms
 *  will be used.
 *
 *  Since the SNP only supports one connection, advertisement in a connection
 *  can only be non-connectable advertisement.
 *
 *  If the behavior parameter is set to @ref SNP_ADV_RESTART_ON_CONN_EST,
 *  advertising will continue with non-connectable advertising when a connection
 *  is established. The advertising interval in this case is set by the
 *  @ref TGAP_CONN_ADV_INT_MIN and @ref TGAP_CONN_ADV_INT_MAX parameters.
 *
 *  By default, those parameters are set to 1280ms. They can be changed by
 *  using @ref SNP_setGapParam (0x49) command.
 *
 *  Directed Advertisements are not supported.
 *
 *  @par         Corresponding Event/Command
 *  @ref  SNP_START_ADV_REQ (from AP) \n
 *  @ref  SNP_EVENT_IND with @ref SNP_ADV_STARTED_EVT (from SNP) when Advertisement is starting.
 *
 *  This event also gets sent upon power up, upon reception of this event previous connection is lost.
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpStartAdvReq_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  1  Byte | Type of Advertisement @ref SNP_AdvType
 *  2  Byte | Timeout of advertisement (in multiple of 625us)
 *  2  Byte | Advertisement Interval (in multiple of 625us)
 *  1  Byte | (Reserved) Advertisement Filter Policy
 *  1  Byte | (Reserved) Initiator Address Type
 *  6  Byte | (Reserved) Initiator Address
 *  1  Byte | Behavior of Advertisement upon Connection @ref SNP_AdvBehavior
 *
 *  NP -> AP \n
 *  @ref snpEvt_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  2  Byte | @ref SNP_ADV_STARTED_EVT
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @param[in]  *cmdStartAdv :advertising request structure, @ref snpStartAdvReq_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_startAdv(snpStartAdvReq_t* cmdStartAdv);

/**
 *  @brief      SNP Stop Advertisement
 *
 *  AP -> NP (SNP_stopAdv) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x44
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_STOP_ADV_REQ
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x05
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_EVENT_IND
 *
 *  The SNP Stop Advertisement command is sent to the SNP to stop advertising.
 *
 *  @par         Corresponding Event/Command
 *  @ref  SNP_STOP_ADV_REQ (from AP) \n
 *  @ref  SNP_EVENT_IND with @ref SNP_ADV_ENDED_EVT (from SNP) upon advertisement stop.
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  No Parameter Structure
 *
 *  NP -> AP \n
 *  @ref snpEvt_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  2  Byte | @ref SNP_ADV_ENDED_EVT
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_stopAdv( void );

/**
 *  @brief      SNP Set Advertisement Data
 *
 *  AP -> NP (SNP_stopAdv) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x43
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_SET_ADV_DATA_REQ
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x43
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_SET_ADV_DATA_CNF
 *
 *  The SNP Set Advertisement Data command is sent to the SNP to update
 *  the raw data of either the scan response or the advertisement information.
 *
 *  There are 2 buffers for the advertisement data:
 *        - A buffer for the non-connected state (device is not in a connection)
 *        - A buffer for the connected state (device is in a connection)
 *
 *  When not in a connection, if advertisement is requested, the advertisement
 *  data stored in the non-connected state buffer will be advertised.
 *
 *  When in a connection, if advertisement is requested, the advertisement data
 *  stored in the connected state buffer will be advertised. If the connected state
 *  buffer has not been set, then the advertising data of the non-connected mode
 *  will be used. If the user does not care about differentiating advertising data
 *  in connected mode and non-connected modes, the connected mode data buffer does
 *  not have to be set.
 *
 *  @par         Additional Notes
 *
 *  The maximum advertisement / scan response size is 31 Bytes.
 *
 *  @par         Default Advertisement Data
 *
 *  [(length of flags) 0x02, (Adv type flags) 0x01,
 *  (General Disc, no BDEDR) 0x06, 'S' 0x53, 'N' 0x4E, 'P' 0x50]
 *
 *  @par         Corresponding Events/Commands
 *  @ref SNP_SET_ADV_DATA_REQ (from AP) | @ref  SNP_SET_ADV_DATA_CNF (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpSetAdvDataReq_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  1  Byte | Type of Advertisement Data to update @ref SNP_Advbuffer
 *  X  Byte | New Advertisement data to update with, up to 31 Bytes
 *
 *  NP -> AP \n
 *  @ref snpSetAdvDataCnf_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @param[in]  *cmdData : Advertisement Data Structure, @ref snpSetAdvDataReq_t
 *  @param[in]  len : Length in Bytes of snpSetAdvDataReq_t::pData
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_setAdvData(snpSetAdvDataReq_t *cmdData, uint8_t len);
/**
 *  @brief      SNP Update Connection Parameters
 *
 *  AP -> NP (SNP_updateConnParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x45
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_UPDATE_CONN_PARAM_REQ
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x45
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_UPDATE_CONN_PARAM_CNF
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x05
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_EVENT_IND
 *
 *  This command is sent to the SNP to
 *  update the connection parameters while in a connection.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_UPDATE_CONN_PARAM_REQ (from AP) | @ref SNP_UPDATE_CONN_PARAM_CNF (from SNP) \n
 *  @ref  SNP_EVENT_IND with @ref SNP_CONN_PARAM_UPDATED_EVT (from SNP)
 *
 *  @ref SNP_UPDATE_CONN_PARAM_CNF will follow the request @ref SNP_UPDATE_CONN_PARAM_REQ,
 *  containing the status of the request as @ref snpUpdateConnParamCnf_t structure.
 *
 *  Once connection parameters are updated and active, @ref SNP_EVENT_IND with
 *  @ref SNP_CONN_PARAM_UPDATED_EVT is sent to the AP.
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpUpdateConnParamReq_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------------------
 *  2  Byte | Connection Handle to update parameters on
 *  2  Byte | Connection Interval Minimum (multiple of 1.25 ms) 7.5 ms to 4 s
 *  2  Byte | Connection Interval Maximum (multiple of 1.25 ms) 7.5 ms to 4 s
 *  2  Byte | Slave Latency (number of connection events) 0x0000 to 0x0C80
 *  2  Byte | Supervision Timeout (multiple of 10 ms) 0x000A to 0x0C80
 *
 *  NP -> AP \n
 *  @ref snpUpdateConnParamCnf_t \n
 *  Size    | Description
 * -------- | -------------------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | Connection Handle that connection parameter updates will occur for
 *
 *  NP -> AP \n
 *  @ref snpEvt_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  2  Byte | @ref SNP_CONN_PARAM_UPDATED_EVT
 *  2  Byte | Connection Handle to update parameters on
 *  2  Byte | Connection Interval (multiple of 1.25 ms) 7.5 ms to 4 s
 *  2  Byte | Slave Latency (number of connection events) 0x0000 to 0x0C80
 *  2  Byte | Supervision Timeout (multiple of 10 ms) 0x000A to 0x0C80
 *
 *  @param[in]  *cmdData : Update Connection Parameter Request Structure, @ref snpUpdateConnParamReq_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_updateConnParam(snpUpdateConnParamReq_t *cmdData);

/**
 *  @brief      SNP Terminate Connection
 *
 *  AP -> NP (SNP_terminateConn) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x46
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_TERMINATE_CONN_REQ
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x55                   | 0x05
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_EVENT_IND
 *
 *  Terminate current BLE connection.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_TERMINATE_CONN_REQ (from AP)  \n
 *  @ref  SNP_EVENT_IND with @ref SNP_CONN_TERM_EVT (from SNP)
 *
 *  Once connection has been terminated, @ref SNP_EVENT_IND with
 *  @ref SNP_CONN_TERM_EVT is sent to the AP.
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpTermConnReq_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  2  Byte | Connection Handle to Terminate
 *  1  Byte | Termination Method @ref SNP_GAP_TERM_CONN_OPTION
 *
 *  NP -> AP \n
 *  @ref snpEvt_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  2  Byte | @ref SNP_ADV_ENDED_EVT
 *  2  Byte | Connection Handle Terminated
 *  1  Byte | Reason of Termination @ref SNP_connTerm
 *
 *  @param[in]  cmdStruct  :Terminate Connection Request Structure, @ref snpTermConnReq_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_terminateConn( snpTermConnReq_t* cmdStruct);

/**
 *  @brief      SNP Set GAP Parameter
 *
 *  AP -> NP (SNP_setGapParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x48
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_SET_GAP_PARAM_REQ
 *
 *  NP -> AP (SNP_setGapParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x48
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_SET_GAP_PARAM_RSP
 *
 *  The SNP Set GAP Parameter command is sent to the SNP to modify the value
 *  of a GAP parameter.
 *
 *  The GAP parameters that can be set are @ref GAP_Params
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_SET_GAP_PARAM_REQ (from AP) | @ref SNP_SET_GAP_PARAM_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpSetGapParamReq_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  2  Byte | GAP Parameter ID to Set value to. @ref GAP_Params
 *  2  Byte | Value to set
 *
 *  NP -> AP \n
 *  @ref snpSetGapParamRsp_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @param[in]  cmdStruct :Set GAP Parameter Request Structure, @ref snpSetGapParamReq_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_setGapParam( snpSetGapParamReq_t* cmdStruct);

/**
 *  @brief      SNP Get GAP Parameter
 *
 *  AP -> NP (SNP_getGapParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x49
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_GET_GAP_PARAM_REQ
 *
 *  NP -> AP (SNP_getGapParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x49
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_GET_GAP_PARAM_RSP
 *
 *  The SNP Get GAP Parameter command is sent to the SNP to read the value of a GAP parameter.
 *
 *  The GAP parameters that can be retrieved are @ref GAP_Params
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_GET_GAP_PARAM_REQ (from AP) | @ref SNP_GET_GAP_PARAM_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpGetGapParamReq_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  2  Byte | GAP Parameter ID to Get value from. @ref GAP_Params
 *
 *  NP -> AP \n
 *  @ref snpGetGapParamRsp_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | GAP Parameter ID, @ref GAP_Params
 *  2  Byte | Value of GAP Parameter
 *
 *  @param[in]  cmdStruct :Get GAP Parameter Request Structure, @ref snpGetGapParamReq_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_getGapParam( snpGetGapParamReq_t* cmdStruct);

/**
 *  @brief      SNP Set Security Parameters
 *
 *  AP -> NP (SNP_setSecurityParams) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x4A
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_SET_SECURITY_PARAM_REQ
 *
 *  NP -> AP (SNP_setSecurityParams) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x4A
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_SET_SECURITY_PARAM_RSP
 *
 *  The SNP Set Security Parameter command is used to set
 *  how device responses to security requests.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_SET_SECURITY_PARAM_REQ (from AP) | @ref SNP_SET_SECURITY_PARAM_RSP (from SNP)
 *
 *  @ref SNP_SUCCESS in @ref SNP_SET_SECURITY_PARAM_RSP does not mean parameters have been changed, only
 *  that the NP has successfully sent the request.
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpSetSecParamReq_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  2  Byte | GAP Parameter ID to Set value to. @ref GAP_Params
 *  2  Byte | Value to set
 *
 *  NP -> AP \n
 *  @ref snpSetSecParamRsp_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @param[in]  pReq :Security parameter to set, @ref snpSetSecParamReq_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_setSecurityParams(snpSetSecParamReq_t *pReq);

/**
 *  @brief      SNP Set White List Filter Policy
 *
 *  AP -> NP (SNP_setGapParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ------------------------------
 *   0x35                   | 0x4D
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_SET_WHITE_LIST_POLICY_REQ
 *
 *  NP -> AP (SNP_setGapParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ------------------------------
 *   0x75                   | 0x4D
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_SET_WHITE_LIST_POLICY_RSP
 *
 *  This command is can be sent to the SNP in order to
 *  set the White List Filter Policy for Scanning and Connections.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_SET_WHITE_LIST_POLICY_REQ (from AP) | @ref SNP_SET_WHITE_LIST_POLICY_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpSetWhiteListReq_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------------------
 *  1  Byte | Requested white list filter policy @ref SNP_AdvFilterPolicyType
 *
 *  NP -> AP \n
 *  @ref snpSetWhiteListRsp_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @param[in]  pReq : Set White List Request Structure, @ref snpSetWhiteListReq_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_setWhiteListFilterPolicy(snpSetWhiteListReq_t *pReq);

/**
 *  @brief      SNP Send Security Request
 *
 *  AP -> NP (SNP_sendSecurityRequest) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ------------------------------
 *   0x55                   | 0x4B
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_SEND_SECURITY_REQUEST_REQ
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ------------------------------
 *   0x55                   | 0x05
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_EVENT_IND
 *
 *  The SNP Send Security command is sent to the SNP to request
 *  the central device to establish security.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_SEND_SECURITY_REQUEST_REQ (from AP) \n
 *  @ref  SNP_EVENT_IND with @ref SNP_SECURITY_EVT (from SNP)
 *
 *  This command does not start pairing or bonding, the central
 *  may or may not choose to start the pairing/bonding process.
 *
 *  If the central does require pairing/bonding, @ref SNP_EVENT_IND with
 *  @ref SNP_SECURITY_EVT will be sent to the AP.
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  No Parameter Structure
 *
 *  NP -> AP \n
 *  @ref snpEvt_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  2  Byte | @ref SNP_SECURITY_EVT
 *  1  Byte | Security State @ref SNP_SECURITY_STATE
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_sendSecurityRequest(void);

/**
 *  @brief      SNP Set Authentication Data
 *
 *  AP -> NP (SNP_setAuthenticationData) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------------
 *   0x35                   | 0x4C
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_SET_AUTHENTICATION_DATA_REQ
 *
 *  NP -> AP (SNP_setAuthenticationData) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------------
 *   0x75                   | 0x4C
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_SEND_AUTHENTICATION_DATA_RSP
 *
 *  The SNP Set Authentication data command is sent to the SNP to provide
 *  Authentication data for the pairing process.
 *
 *  @par         Corresponding Events/Commands
 *  @ref  SNP_SET_AUTHENTICATION_DATA_REQ (from AP) | @ref SNP_SEND_AUTHENTICATION_DATA_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpSetAuthDataReq_t \n
 *  Size    | Description
 * -------- | ------------------------------------------------------------------
 *  4 Byte  | Authentication Data, 000000 - 999999 for passcode or TRUE/FALSE
 *
 *  NP -> AP \n
 *  @ref snpSetAuthDataRsp_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @param[in]  pReq : Set Authentication Data Request Structure, @ref snpSetAuthDataReq_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_setAuthenticationData(snpSetAuthDataReq_t *pReq);
/**
 * @}
 */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEBLEPROCESSOR_H */

/** @} End SNP_GAP */

/** @} */ // end of SNP

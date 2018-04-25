/******************************************************************************

 @file       simple_np_gatt.h

 @brief This file contains the parsing of GATT related command for the
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
@page SNP_GATT_API  SNP API: GATT

@section SNP_GATT_API_SUMMARY SNP GATT API Summary

 Commands Acronyms                                             | Origin  | Type      | TL Cmd0                        | TL Cmd1                            | Parameter structure
-------------------------------------------------------------- |---------|-----------|--------------------------------|----------------------------------- | --------------------
 [SNP_ADD_SERVICE_REQ] (\ref SNP_addService )                  | AP      | Sync Req  | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_ADD_SERVICE_REQ        (0x81) | #snpAddServiceReq_t
 [SNP_ADD_SERVICE_RSP] (\ref SNP_addService )                  | NP      | Sync Rsp  | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_ADD_SERVICE_RSP        (0x81) | #snpAddServiceRsp_t
 [SNP_ADD_CHAR_VAL_DECL_REQ] (\ref SNP_addCharValueDecl )      | AP      | Sync Req  | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_ADD_CHAR_VAL_DECL_REQ  (0x82) | #snpAddCharValueDeclReq_t
 [SNP_ADD_CHAR_VAL_DECL_RSP] (\ref SNP_addCharValueDecl )      | NP      | Sync Rsp  | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_ADD_CHAR_VAL_DECL_RSP  (0x82) | #snpAddCharValueDeclRsp_t
 [SNP_ADD_CHAR_DESC_DECL_REQ] (\ref SNP_addDescriptionValue )  | AP      | Sync Req  | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_ADD_CHAR_DESC_DECL_REQ (0x83) |  |
 [SNP_ADD_CHAR_DESC_DECL_RSP] (\ref SNP_addDescriptionValue )  | NP      | Sync Rsp  | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_ADD_CHAR_DESC_DECL_RSP (0x83) |  |
 [SNP_REGISTER_SERVICE_REQ] (\ref SNP_registerService )        | AP      | Sync Req  | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_REGISTER_SERVICE_REQ   (0x84) |  |
 [SNP_REGISTER_SERVICE_RSP] (\ref SNP_registerService )        | NP      | Sync Rsp  | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_REGISTER_SERVICE_RSP   (0x84) | #snpRegisterServiceRsp_t
 [SNP_GET_ATTR_VALUE_REQ] (\ref SNP_getAttrValue )             | AP      | Sync Req  | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_GET_ATTR_VALUE_REQ     (0x85) | #snpGetAttrValueReq_t
 [SNP_GET_ATTR_VALUE_RSP] (\ref SNP_getAttrValue )             | NP      | Sync Rsp  | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_GET_ATTR_VALUE_RSP     (0x85) | #snpGetAttrValueRsp_t
 [SNP_SET_ATTR_VALUE_REQ] (\ref SNP_setAttrValue )             | AP      | Sync Req  | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_SET_ATTR_VALUE_REQ     (0x86) | #snpSetAttrValueReq_t
 [SNP_SET_ATTR_VALUE_RSP] (\ref SNP_setAttrValue )             | NP      | Sync Rsp  | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_SET_ATTR_VALUE_RSP     (0x86) | #snpSetAttrValueRsp_t
 [SNP_CHAR_READ_IND] (\ref SNP_charRead )                      | NP      | Async     | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_CHAR_READ_IND          (0x87) | #snpCharReadInd_t
 [SNP_CHAR_READ_CNF] (\ref SNP_readCharCnf )                   | AP      | Async     | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_CHAR_READ_CNF          (0x87) | #snpCharReadCnf_t
 [SNP_CHAR_WRITE_IND] (\ref SNP_charWrite )                    | NP      | Async     | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_CHAR_WRITE_IND         (0x88) | #snpCharWriteInd_t
 [SNP_CHAR_WRITE_CNF] (\ref SNP_writeCharCnf )                 | AP      | Async     | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_CHAR_WRITE_CNF         (0x88) | #snpCharWriteCnf_t
 [SNP_SEND_NOTIF_IND_REQ] (\ref SNP_sendNotifInd )             | AP      | Async     | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_SEND_NOTIF_IND_REQ     (0x89) | #snpNotifIndReq_t
 [SNP_SEND_NOTIF_IND_CNF] (\ref SNP_sendNotifInd )             | NP      | Async     | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_SEND_NOTIF_IND_CNF     (0x89) | #snpNotifIndCnf_t
 [SNP_CCCD_UPDATED_IND] (\ref SNP_processCharConfigUpdatedCnf) | NP      | Async     | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_CCCD_UPDATED_IND       (0x8B) | #snpCharCfgUpdatedInd_t
 [SNP_CCCD_UPDATED_CNF] (\ref SNP_processCharConfigUpdatedCnf) | AP      | Async     | #SNP_NPI_ASYNC_CMD_TYPE (0x55) | #SNP_CCCD_UPDATED_CNF       (0x8B) | #snpCharCfgUpdatedRsp_t
 [SNP_SET_GATT_PARAM_REQ] (\ref SNP_setGATTParam )             | AP      | Sync Req  | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_SET_GATT_PARAM_REQ     (0x8C) | #snpSetGattParamReq_t
 [SNP_SET_GATT_PARAM_RSP] (\ref SNP_setGATTParam )             | NP      | Sync Rsp  | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_SET_GATT_PARAM_RSP     (0x8C) | #snpSetGattParamRsp_t
 [SNP_GET_GATT_PARAM_REQ] (\ref SNP_getGATTParam )             | AP      | Sync Req  | #SNP_NPI_SYNC_REQ_TYPE  (0x35) | #SNP_GET_GATT_PARAM_REQ     (0x8D) | #snpGetGattParamReq_t
 [SNP_GET_GATT_PARAM_RSP] (\ref SNP_getGATTParam )             | NP      | Sync Rsp  | #SNP_NPI_SYNC_RSP_TYPE  (0x75) | #SNP_GET_GATT_PARAM_RSP     (0x8D) | #snpGetGattParamRsp_t

  All these command have some parameters. The parameters can be found/set using the indicated Parameter structure.
  All Parameter/Structures should be little-endian format (LSB first).
  For UUID parameters, @ref UUID_desc

@section SNP_charRead SNP Characteristic Read Indication (0x87)

NP -> AP (N/A) \n
 CMD 0                  | CMD 1
----------------------- | --------------------------
 0x55                   | 0x87
 SNP_NPI_ASYNC_CMD_TYPE | SNP_CHAR_READ_IND

AP -> NP (SNP_readCharCnf) \n
 CMD 0                  | CMD 1
----------------------- | ---------------------------
 0x55                   | 0x87
 SNP_NPI_ASYNC_CMD_TYPE | SNP_CHAR_READ_CNF

The @ref SNP_CHAR_READ_IND Event is sent by the SNP when a remote GATT client wants to read a characteristic value managed by the AP.

The AP must answer with @ref SNP_readCharCnf / @ref SNP_CHAR_READ_CNF within 30 seconds.
If there's no response, an ATT Timeout will occur and the BLE connection will be lost.

A characteristic value can be up to 512 Bytes long. However, not more than ATT_MTU_SIZE can be read at once by a ATT operation.

If the characteristic size is bigger than ATT_MTU_SIZE, the remote GATT client will send several @ref SNP_CHAR_READ_IND, until it reads the complete value.
There is several different ways for the GATT client to fully read a characteristic; one way is that each read request sent by the GATT client is translated in a @ref SNP_CHAR_READ_IND with @ref snpCharReadInd_t.

The snpCharReadInd_t::offset represents the start of the data to read, the snpCharReadInd_t::maxSize represents maximum size of the characteristic.

The AP needs to reply with the @ref SNP_readCharCnf command which contains the data payload. The AP will indicate the offset the data were read from, the size of the data, and the data with @ref snpCharReadCnf_t.

The default ATT_MTU_SIZE is set to 27. If MTU Size is changed (negotiated between GATT server and GATT client), then a @ref SNP_EVENT_IND with @ref SNP_ATT_MTU_EVT event will be send by the NP to the AP
The AP cannot initiate this change.

@par Corresponding Events/Commands

@ref SNP_CHAR_READ_IND (from SNP) | @ref SNP_CHAR_READ_CNF (from AP)

NP -> AP \n
@ref snpCharReadInd_t \n
 Size    | Description
-------- | -----------------------------------------
 2 Byte  | Connection Handle
 2 Byte  | Attribute Handle
 2 Byte  | Offset in Bytes
 2 Byte  | Max Size

@par Related Structures

#snpCharReadInd_t for @ref SNP_CHAR_READ_IND \n
#snpCharReadCnf_t for @ref SNP_CHAR_READ_CNF

@section SNP_charWrite SNP Characteristic Write Indication (0x88)

NP -> AP (N/A) \n
 CMD 0                  | CMD 1
----------------------- | --------------------------
 0x55                   | 0x88
 SNP_NPI_ASYNC_CMD_TYPE | SNP_CHAR_WRITE_IND

AP -> NP (SNP_readCharCnf) \n
 CMD 0                  | CMD 1
----------------------- | ---------------------------
 0x55                   | 0x88
 SNP_NPI_ASYNC_CMD_TYPE | SNP_CHAR_WRITE_CNF

The @ref SNP_CHAR_WRITE_IND Event is sent by the SNP when a remote GATT client wants to write a characteristic value managed by the AP.

The AP needs to answer with a @ref SNP_CHAR_WRITE_CNF / @ref SNP_writeCharCnf within 30 seconds.

The GATT client may request a partial write, hence the usage of the offset where the write should begin relative to the beginning of the characteristic value (the offset is a number of Bytes).
The characteristic value length is application specific. Neither the SNP or the AP SNP library know the length of the characteristic value.

If the GATT client starts to write a characteristic value with offset 0, the entire value will be written.
If the GATT client starts to write a characteristic value with an offset different than 0, a partial write will occur.

It is the responsibility of the application to know the length of each characteristic value.

The snpCharWriteInd_t::rspNeeded field set to 1 indicates that the AP must send @ref SNP_CHAR_WRITE_CNF / @ref SNP_writeCharCnf within 30 seconds.
If this field is set to 0, the SNP_charWrite_rsp is not needed. This can happen if a ATT_WRITE_CMD operation has been used by the remote GATT client.
If a confirmation is send anyway, it will be ignored by the NP.

The application can also have their own permission mechanism and rules. In this case it is the responsibility of the application to deny the writing of the value by return a error status with @ref SNP_CHAR_WRITE_CNF / @ref SNP_writeCharCnf.

@par Corresponding Events/Commands

@ref SNP_CHAR_WRITE_IND (from SNP) | @ref SNP_CHAR_WRITE_CNF (from AP)

NP -> AP \n
@ref snpCharWriteInd_t \n
 Size    | Description
-------- | -----------------------------------------
 2 Byte  | Connection Handle
 2 Byte  | Attribute Handle
 1 Byte  | Response Needed? @ref SNP_GATT_RSP_OPTIONS
 2 Byte  | Offset in Bytes
 X Byte  | Data to be Written

@par Related Structures

#snpCharWriteInd_t for @ref SNP_CHAR_WRITE_IND \n
#snpCharWriteCnf_t for @ref SNP_CHAR_WRITE_CNF

*/

/**
 *  @addtogroup SNP
 *  @{
 *  @defgroup SNP_GATT SNP GATT API
 *  @{
 *  @file       simple_np_gatt.h
 *  @brief  SNP GATT Layer API
 *
 *  This file contains the interface for SNP GATT Layer.
 */

#ifndef SIMPLENP_GATT_H
#define SIMPLENP_GATT_H

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
 * TYPDEFS
 */

/*********************************************************************
 * FUNCTIONS
 */


/**
 *  @brief       SNP Add Service
 *
 *  AP -> NP (SNP_addService) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------
 *   0x35                   | 0x81
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_ADD_SERVICE_REQ
 *
 *  NP -> AP (SNP_addService) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------
 *   0x75                   | 0x81
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_ADD_SERVICE_RSP
 *
 *  The SNP Add Service command is sent to the SNP to start the addition
 *  of a new service to the GATT server running on the SNP.
 *
 *  This command should be followed by @ref SNP_addCharValueDecl (0x82) command
 *  and optionally @ref SNP_addDescriptionValue (0x83) command to add
 *  characteristics to the service.
 *
 *  Lastly, when all characteristics have been added, @ref SNP_registerService (0x84)
 *  command should be sent to the device.
 *
 *  Only one service can be added at a time. That is, once this command has
 *  been sent, all characteristics must then be added and the service must to be
 *  registered before adding another service.
 *
 *  @par         Additional Notes
 *
 *  Services and characteristic cannot be added if the Device has started. \n
 *  Device is started when @ref SNP_startAdv or @ref SNP_setAdvData is send to the SNP.\n
 *  Once started, the only way to stop the device is to reset it.
 *
 *  When NP is reset, all previously added services and characteristics are lost.
 *
 *  @par         Corresponding Events/Commands
 *  @ref SNP_ADD_SERVICE_REQ (from AP) | @ref  SNP_ADD_SERVICE_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpAddServiceReq_t \n
 *  Size    | Description
 * -------- | -------------------------------------------------
 *  1  Byte | Type of the service to add @ref SNP_SERVICE_TYPE
 *  2  Byte | 2 Byte UUID @ref UUID_desc
 *
 *  NP -> AP \n
 *  @ref snpAddServiceRsp_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @param[in]   pReq    : Add Service Request Structure, @ref snpAddServiceReq_t
 *  @param[in]   uuidLen : 1 Byte UUID Length of Service
 *  @param[out]  pRsp    : Add Service Response Structure, @ref snpAddServiceRsp_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_addService(snpAddServiceReq_t *pReq, uint8_t uuidLen,
                       snpAddServiceRsp_t *pRsp);

/**
 *  @brief       SNP Add Characteristic Value Declaration
 *
 *  AP -> NP (SNP_addCharValueDecl) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x35                   | 0x82
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_ADD_CHAR_VAL_DECL_REQ
 *
 *  NP -> AP (SNP_addCharValueDecl) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | --------------------------
 *   0x75                   | 0x82
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_ADD_CHAR_VAL_DECL_RSP
 *
 *  This command is sent to the SNP to add the two Bluetooth Specification
 *  mandatory attributes for a characteristic:  \n
 *      - characteristic value                  \n
 *      - characteristic declaration
 *
 *  This command should be sent after @ref SNP_addService (0x81) command has been
 *  sent to begin service declaration.
 *
 *  It is possible to add optional attributes to the characteristic using
 *  @ref SNP_addDescriptionValue (0x83) command.
 *
 *  @par         Additional Notes
 *
 *  This command must be called before adding additional attributes with
 *  @ref SNP_addDescriptionValue (0x83).
 *
 *  If the GATT Notify or GATT Indicate properties are enabled, a CCCD characteristic
 *  must be added with the @ref SNP_addDescriptionValue (0x83) command.
 *
 *  The TI BLE Stack expects the properties to be aligned logically with
 *  the permissions. For example, a characteristic with read properties
 *  would most likely have read permissions.
 *
 *  @par         Corresponding Events/Commands
 *  @ref SNP_ADD_CHAR_VAL_DECL_REQ (from AP) | @ref  SNP_ADD_CHAR_VAL_DECL_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpAddCharValueDeclReq_t \n
 *  Size    | Description
 * -------- | ------------------------------------------------------------------------
 *  1  Byte | Permissions of the value attribute @ref SNP_PERMIT_BITMAPS_DEFINES
 *  2  Byte | Properties of the value attribute @ref SNP_PROP_BITMAPS_DEFINES
 *  1  Byte | Reserved
 *  2  Byte | Reserved
 *  2  Byte | 2 Byte UUID @ref UUID_desc
 *
 *  NP -> AP \n
 *  @ref snpAddCharValueDeclRsp_t \n
 *  Size    | Description
 * -------- | ---------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | Attribute Handle
 *
 *  @param[in]   pReq     : Add Characteristic Value & Declaration Request Structure
 *                          @ref snpAddCharValueDeclReq_t
 *  @param[in]   uuidLen  : 1 Byte UUID Length of Service
 *  @param[out]  pRsp     : Add Characteristic Value & Declaration Response Structure
 *                          @ref snpAddCharValueDeclRsp_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_addCharValueDecl(snpAddCharValueDeclReq_t *pReq, uint8_t uuidLen,
                             snpAddCharValueDeclRsp_t *pRsp);

/**
 *  @brief       SNP Add Characteristic Descriptor Declaration
 *
 *  AP -> NP (SNP_addDescriptionValue) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x35                   | 0x83
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_ADD_CHAR_DESC_DECL_REQ
 *
 *  NP -> AP (SNP_addDescriptionValue) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x75                   | 0x83
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_ADD_CHAR_DESC_DECL_RSP
 *
 *  This command is used to add one or more of the following attributes
 *  to a characteristic:
 *
 *      User Description String
 *      CCCD
 *      Presentation format
 *      Server Characteristic Config (Reserved)
 *      Aggregate format
 *      Generic Attribute using short UUID
 *      Generic Attribute using long UUID
 *
 *  For User Description String, the attribute permissions is forced to be
 *  read-only.
 *
 *  @par        Additional Notes
 *
 *  All those attributes are optional. At least one needs to be added when using this command.\n
 *  CCCD, Presentation Format, and User Description String attributes can be added once for each characteristic. \n
 *  Trying to create a second will lead to a @ref SNP_CMD_REJECTED.
 *
 *  If the NOTIFY and INDICATE properties are enabled by the CCCD attribute, and a GATT Clint
 *  writes to this attribute - @ref SNP_writeCharCnf is forwarded to AP. AP must then determine
 *  if the write is authorized and send write to the attribute via @ref SNP_setGATTParam.
 *
 *  @par         Corresponding Events/Commands
 *  @ref SNP_ADD_CHAR_DESC_DECL_REQ (from AP) | @ref  SNP_ADD_CHAR_DESC_DECL_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpAddCharDescDeclReq_t \n
 *  Size    | Description
 * -------- | ------------------------------------------------------------------------
 *  1  Byte | Header of Attributes to add @ref SNP_GATT_DESC_HEADER_LIST
 *  X  Byte | Depending on Header, the length can very
 *
 *  NP -> AP \n
 *  @ref snpAddCharDescDeclRsp_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  1  Byte | Header of Added Attributes @ref SNP_GATT_DESC_HEADER_LIST
 *  X  Byte | Handles of Attributes added, depending on header
 *
 *  @par        Related Structures
 *
 *  @ref snpAddCharDescDeclReq_t for @ref SNP_ADD_CHAR_DESC_DECL_REQ \n
 *  @ref snpAddCharDescDeclRsp_t for @ref SNP_ADD_CHAR_DESC_DECL_RSP \n
 *  @ref snpAddAttrGenShortUUID_t for Short UUID                     \n
 *  @ref snpAddAttrGenLongUUID_t for Long UUID                       \n
 *  @ref snpAddAttrCccd_t for CCCD                                   \n
 *  @ref snpAddAttrFormat_t for Presentation Format                  \n
 *  @ref snpAddAttrUserDesc_t for User Description
 *
 *  @param[in]   lDescReq  : Add Descriptor Request Structure, @ref snpAddCharDescDeclReq_t
 *  @param[out]  lDescRsp  : Add Descriptor Response Structure, @ref snpAddCharDescDeclRsp_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_addDescriptionValue(snpAddCharDescDeclReq_t *lDescReq,
                                snpAddCharDescDeclRsp_t *lDescRsp);

/**
 *  @brief        SNP Register Service
 *
 *  AP -> NP (SNP_registerService) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x35                   | 0x84
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_REGISTER_SERVICE_REQ
 *
 *  NP -> AP (SNP_registerService) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x75                   | 0x84
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_REGISTER_SERVICE_RSP
 *
 *  This command should be sent to the device to register the previously
 *  added service and characteristics to the devices GATT server.
 *
 *  This command should be sent after @ref SNP_addService and @ref SNP_addCharValueDecl;
 *  along with any optional attributes desired from @ref SNP_addDescriptionValue.
 *
 *  When NP is reset, all previously added services and characteristics are lost.
 *
 *  @par         Corresponding Events/Commands
 *  @ref SNP_REGISTER_SERVICE_REQ (from AP) | @ref  SNP_REGISTER_SERVICE_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  No Parameter Structure
 *
 *  NP -> AP \n
 *  @ref snpRegisterServiceRsp_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | Start Handle of the Registered Service
 *  2  Byte | End Handle of Registered Service
 *
 *  @param[out]   pRsp : Register Service Response Structure, @ref snpRegisterServiceRsp_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_registerService(snpRegisterServiceRsp_t *pRsp);

/**
 *  @brief        SNP Get Attribute Value
 *
 *  AP -> NP (SNP_getAttrValue) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x35                   | 0x85
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_GET_ATTR_VALUE_REQ
 *
 *  NP -> AP (SNP_getAttrValue) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x75                   | 0x85
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_GET_ATTR_VALUE_RSP
 *
 *  This command is sent to the SNP to retrieve the value of an attribute on
 *  the GATT server. This includes non-user added Services, the Device Information Service
 *  and the Generic Access Service.
 *
 *  This command can only be used to get read from User Description and Presentation Format Attributes.
 *  These attribute values are stored on SNP. Other attribute values are Reserved for Future Use.
 *
 *  For getting the value of a SNP managed attribute, the command @ref SNP_getAttrValue (0x8D)
 *  should be used.
 *
 *  @par         Corresponding Events/Commands
 *  @ref SNP_GET_ATTR_VALUE_REQ (from AP) | @ref  SNP_GET_ATTR_VALUE_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpGetAttrValueReq_t \n
 *  Size    | Description
 * -------- | ------------------------------------------------------------------------
 *  2  Byte | Attribute Handle to get Value from
 *
 *  NP -> AP \n
 *  @ref snpGetAttrValueRsp_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | Attribute Handle of where Value is from
 *  X  Byte | Value of the Attribute Handle
 *
 *  @param[in]    pReq          : Get Attribute Value Request Structure, @ref snpGetAttrValueReq_t
 *  @param[out]   pRsp          : Get Attribute Value Response Structure, @ref snpGetAttrValueRsp_t
 *  @param[out]   pDataLenght   : Length in Bytes of snpGetAttrValueRsp_t::pData
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_getAttrValue(snpGetAttrValueReq_t *pReq,
                         snpGetAttrValueRsp_t *pRsp, uint16_t *pDataLenght);

/**
 *  @brief        SNP Set Attribute Value
 *
 *  AP -> NP (SNP_setAttrValue) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x35                   | 0x86
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_SET_ATTR_VALUE_REQ
 *
 *  NP -> AP (SNP_setAttrValue) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x75                   | 0x86
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_SET_ATTR_VALUE_RSP
 *
 *  This command is sent to the SNP in order to write the value of an attribute on
 *  the GATT server. This includes non-user added Services, the Device Information Service
 *  and the Generic Access Service.
 *
 *  This command can only be used to write User Description and Presentation Format Attributes.
 *
 *  For writing to Characteristic Value Attributes, use @ref SNP_setGATTParam (0x8C)
 *
 *  For reading the value of a SNP managed attribute, the command @ref SNP_getAttrValue (0x85)
 *  should be used.
 *
 *  @par         Corresponding Events/Commands
 *  @ref SNP_SET_ATTR_VALUE_REQ (from AP) | @ref  SNP_SET_ATTR_VALUE_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpSetAttrValueReq_t \n
 *  Size    | Description
 * -------- | ------------------------------------------------------------------------
 *  2  Byte | Attribute Handle to get Value from
 *  X  Byte | Value to Set the Attribute
 *
 *  NP -> AP \n
 *  @ref snpSetAttrValueRsp_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | Attribute Handle of where Value is from
 *
 *  @param[in]    pReq       : Set Attribute Value Request Structure, @ref snpSetAttrValueReq_t
 *  @param[in]    dataLenght : Length in Bytes of snpSetAttrValueReq_t::pData
 *  @param[out]   pRsp       : Set Attribute Value Response Structure, @ref snpSetAttrValueRsp_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_setAttrValue(snpSetAttrValueReq_t *pReq, uint16_t dataLenght,
                         snpSetAttrValueRsp_t *pRsp);

/**
 *  @brief   SNP Set GATT Parameter
 *
 *  AP -> NP (SNP_setGATTParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x35                   | 0x8C
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_SET_GATT_PARAM_REQ
 *
 *  NP -> AP (SNP_setGATTParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x75                   | 0x8C
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_SET_GATT_PARAM_RSP
 *
 *  This command is used to write the attribute value of a characteristic
 *  managed on the SNP GATT Server. This includes non-user added Services, the Device Information Service
 *  and the Generic Access Service.
 *
 *  This command can only write to Characteristic Value attributes.
 *
 *  For other types of Attributes, use @ref SNP_setAttrValue (0x86)
 *
 *  For reading the value of a SNP managed attribute, the command @ref SNP_getAttrValue (0x85)
 *  should be used.
 *
 *  @par         Additional Information
 *
 *  The SNP_setGATTParam::reqDataLen should never exceed the maximum length of the attribute value
 *
 *  @par         Corresponding Event
 *  @ref  SNP_SET_GATT_PARAM_REQ (from AP) | @ref SNP_SET_GATT_PARAM_RSP (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpSetGattParamReq_t \n
 *  Size    | Description
 * -------- | ------------------------------------------------------------------------
 *  1  Byte | Service ID containing parameter @ref SNP_PREDEF_SERVICE
 *  2  Byte | Parameter ID
 *  X  Byte | Value to Set the Parameter
 *
 *  NP -> AP \n
 *  @ref snpSetGattParamRsp_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *
 *  @param[in]   pReq       : Set GATT Parameter Request Structure, @ref snpSetGattParamReq_t
 *  @param[in]   reqDataLen : Length in Bytes of snpSetGattParamReq_t::pData
 *  @param[out]  pRsp       : Set GATT Parameter Response Structure, @ref snpSetGattParamRsp_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_setGATTParam(snpSetGattParamReq_t *pReq, uint16_t reqDataLen,
                         snpSetGattParamRsp_t *pRsp);

/**
 *  @brief       SNP Get GATT Parameter
 *
 *  AP -> NP (SNP_getGATTParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x35                   | 0x8D
 *   SNP_NPI_SYNC_REQ_TYPE  | SNP_GET_GATT_PARAM_REQ
 *
 *  NP -> AP (SNP_getGATTParam) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x75                   | 0x8D
 *   SNP_NPI_SYNC_RSP_TYPE  | SNP_GET_GATT_PARAM_RSP
 *
 *  This command is used to get the value of a Characteristic Value attribute
 *  from a characteristic managed by SNP GATT Server. This includes non-user added
 *  Services, the Device Information Service and the Generic Access Service.
 *
 *  For reading other types of Attributes, use @ref SNP_getAttrValue (0x85)
 *
 *  For writing the value of a Characteristic Value Attribute, use @ref SNP_setGATTParam (0x8C)
 *
 *  @par         Corresponding Event
 *        @ref  SNP_GET_GATT_PARAM_RSP
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpGetGattParamReq_t \n
 *  Size    | Description
 * -------- | ------------------------------------------------------------------------
 *  1  Byte | Service ID containing parameter @ref SNP_PREDEF_SERVICE
 *  2  Byte | Parameter ID to get data from
 *
 *  NP -> AP \n
 *  @ref snpGetGattParamRsp_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------------
 *  1  Byte | Service ID containing parameter @ref SNP_PREDEF_SERVICE
 *  2  Byte | Parameter ID to get data from
 *  X  Byte | Parameter Data
 *
 *  @param[in]   pReq     : Get GATT Parameter Request Structure, @ref snpGetGattParamReq_t
 *  @param[out]  pRsp     : Get GATT Parameter Response Structure, @ref snpGetGattParamRsp_t
 *  @param[out]  pDataLen : Length in Bytes of snpGetGattParamRsp_t::pData
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_getGATTParam(snpGetGattParamReq_t *pReq,
                         snpGetGattParamRsp_t *pRsp, uint16_t *pDataLen);

/**
 *  @brief       SNP Send Notification Indication
 *
 *  AP -> NP (SNP_sendNotifInd) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x55                   | 0x89
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_SEND_NOTIF_IND_REQ
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x55                   | 0x89
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_SEND_NOTIF_IND_CNF
 *
 *  This command is used to request SNP to send a Notification or Indication
 *  to the GATT Client. This is only possible if a CCCD for the characteristic
 *  has been created and notifications / indications have been enabled by
 *  the GATT Client writing to the CCCD.
 *
 *  If CCCD Attribute of a characteristic is not configured correctly,
 *  the GATT server may not send out notification or indication.
 *
 *  Indications require a confirmation from the GATT client. Thus only one
 *  indication can be sent at a time. Once confirmation is received, it is
 *  then forwarded to AP via @ref SNP_SEND_NOTIF_IND_CNF with a @ref SNP_SUCCESS.
 *
 *  If no confirmation is received from the GATT Client within 30 seconds,
 *  @ref SNP_SEND_NOTIF_IND_CNF with a @ref SNP_FAILURE will be sent back to AP.
 *  An ATT timeout will also occur, dropping the BLE connection.
 *
 *  Notifications don't require a confirmation from the GATT Client. SNP will
 *  produce a @ref SNP_SEND_NOTIF_IND_CNF with a @ref SNP_SUCCESS when command
 *  has been forwarded to BLE-Stack Host.
 *
 *  @par         Additional Notes
 *
 *  The maximum size possible for a notification or indication value is ATT_MTU_SIZE.
 *  If the value sent is larger than ATT_MTU_SIZE bytes, it will be truncated.
 *
 *  @par         Corresponding Event
 *  @ref SNP_SEND_NOTIF_IND_REQ (from AP) | @ref  SNP_SEND_NOTIF_IND_CNF (from SNP)
 *
 *  @par    Payload Structures
 *
 *  AP -> NP \n
 *  @ref snpNotifIndReq_t \n
 *  Size    | Description
 * -------- | ------------------------------------------------------------------------
 *  2  Byte | Connection Handle to send Notification/Indication
 *  2  Byte | Attribute Handle of Characteristic Value to notify/indicate
 *  1  Byte | Reserved
 *  1  Byte | Type of the Request @ref SNP_IND_TYPE
 *  X  Byte | Characteristic Value to Send
 *
 *  NP -> AP \n
 *  @ref snpNotifIndCnf_t \n
 *  Size    | Description
 * -------- | ----------------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | Connection Handle Notified/Indicated
 *
 *  @param[in]   pCmdStruct     :Parameter Structure for Notification/Indication, @ref snpNotifIndReq_t
 *  @param[in]   length         :Length in Bytes of snpNotifIndReq_t::pData
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_sendNotifInd(snpNotifIndReq_t *pCmdStruct, int16_t length);

/**
 *  @brief       SNP Write Characteristic Confirmation
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x55                   | 0x88
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_CHAR_WRITE_IND
 *
 *  AP -> NP (SNP_writeCharCnf) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x55                   | 0x88
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_CHAR_WRITE_CNF
 *
 *  If a GATT client requests a write of a characteristic that was added by the AP,
 *  the SNP GATT server will forward an @ref SNP_CHAR_WRITE_IND (0x88) to the AP.
 *
 *  The AP then has 30 seconds to do the following:
 *      - Determine if write is allowed based on application-specific rules
 *      - Perform the write using @ref SNP_setGATTParam or @ref SNP_setAttrValue
 *      - Verify that the write occurred successfully
 *      - Send this command back/ @ref SNP_CHAR_WRITE_CNF with the appropriate response
 *
 *  If no response occurs in 30 seconds, an ATT timeout will occur and BLE connection
 *  will be lost.
 *
 *  @par         Corresponding Event
 *  @ref   SNP_CHAR_WRITE_IND (from SNP) | @ref SNP_CHAR_WRITE_CNF (from AP)
 *
 *  @par    Payload Structures
 *
 *  NP -> AP \n
 *  @ref snpCharWriteInd_t \n
 *  Size    | Description
 * -------- | -----------------------------------------------------------------
 *  2  Byte | Connection Handle generating request to write
 *  2  Byte | Attribute Handle of Characteristic Value to write to
 *  1  Byte | Indicate if Response is Needed from AP @ref SNP_GATT_RSP_OPTIONS
 *  2  Byte | Offset in Bytes
 *  X  Byte | Data to write to characteristic
 *
 *  AP -> NP \n
 *  @ref snpCharWriteCnf_t \n
 *  Size    | Description
 * -------- | ---------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | Connection Handle Confirmation
 *
 *  @param[in]   pReq    : Characteristic Write Confirmation Request Structure, @ref snpCharWriteCnf_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_writeCharCnf(snpCharWriteCnf_t *pReq);

/**
 *  @brief       SNP Read Characteristic Confirmation
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x55                   | 0x87
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_CHAR_READ_IND
 *
 *  AP -> NP (SNP_readCharCnf) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x55                   | 0x87
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_CHAR_READ_CNF
 *
 *  If a GATT client requests a read of a characteristic that was added by the AP,
 *  the SNP GATT server will forward an @ref SNP_CHAR_READ_IND (0x87) to the AP.
 *
 *  The AP then has 30 seconds to do the following:
 *      - Determine if read is allowed based on application-specific rules
 *      - Perform the write using @ref SNP_getGATTParam or @ref SNP_getAttrValue
 *      - Verify that the read occurred successfully
 *      - Send this command back / @ref SNP_CHAR_READ_CNF with the appropriate response.
 *
 *  If no response occurs in 30 seconds, an ATT timeout will occur and BLE connection
 *  will be lost.
 *
 *  @par         Corresponding Event
 *  @ref   SNP_CHAR_READ_IND (from SNP) | @ref SNP_CHAR_READ_CNF (from AP)
 *
 *  @par    Payload Structures
 *
 *  NP -> AP \n
 *  @ref snpCharReadInd_t \n
 *  Size    | Description
 * -------- | -----------------------------------------------------------------
 *  2  Byte | Connection Handle generating request to read
 *  2  Byte | Attribute Handle of Characteristic Value to read from
 *  2  Byte | Offset in Bytes
 *  2  Byte | Max Size of Data to read
 *
 *  AP -> NP \n
 *  @ref snpCharReadCnf_t \n
 *  Size    | Description
 * -------- | -----------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | Connection Handle generating request to read
 *  2  Byte | Attribute Handle of Characteristic Value to read
 *  2  Byte | Offset in Bytes
 *  X  Byte | Data to write to characteristic
 *
 *  @param[in]   pReq     : Read Characteristic Confirmation Structure, @ref snpCharReadCnf_t
 *  @param[in]   dataSize : Length in Bytes of snpCharReadCnf_t::pData
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_readCharCnf(snpCharReadCnf_t *pReq, uint16_t dataSize);

/**
 *  @brief       SNP Characteristic Configuration Update Confirm (0x8B)
 *
 *  NP -> AP (N/A) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x55                   | 0x8B
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_CCCD_UPDATED_IND
 *
 *  AP -> NP (SNP_processCharConfigUpdatedCnf) \n
 *   CMD 0                  | CMD 1
 *  ----------------------- | ---------------------------
 *   0x55                   | 0x8B
 *   SNP_NPI_ASYNC_CMD_TYPE | SNP_CCCD_UPDATED_CNF
 *
 *  The @ref SNP_CCCD_UPDATED_IND event is sent from the SNP when the remote
 *  GATT Client has requested to update the CCCD value added by the AP.
 *
 *  Upon receiving this event, if snpCharCfgUpdatedRsp_t::rspNeeded is
 *  @ref SNP_RESPONSE_NEEDED then within 30 seconds the AP must:
 *          - determine if CCCD update is allowed by application-specific rules
 *          - send this command / @ref SNP_CCCD_UPDATED_CNF with the proper parameters
 *
 *  If there is no response within 30 seconds, an ATT timeout will occur and BLE
 *  connection will be lost.
 *
 *  AP is responsible for remembering the CCCD value, accept/decline authorize the CCCD,
 *  and if associated characteristic value changes, the AP must send out notification or
 *  indication of the change.
 *
 *  If snpCharCfgUpdatedRsp_t::rspNeeded is @ref SNP_RESPONSE_NOT_NEEDED then
 *  no response is needed.
 *
 *  @par         Corresponding Event
 *  @ref   SNP_CCCD_UPDATED_IND (from SNP) | @ref SNP_CCCD_UPDATED_CNF (from AP)
 *
 *  @par    Payload Structures
 *
 *  NP -> AP \n
 *  @ref snpCharCfgUpdatedInd_t \n
 *  Size    | Description
 * -------- | -----------------------------------------------------------------
 *  2  Byte | Connection Handle generating request to write
 *  2  Byte | Handle to CCCD attribute being written to
 *  1  Byte | Indicate if Response is Needed from AP @ref SNP_GATT_RSP_OPTIONS
 *  2  Byte | Value to write to CCCD
 *
 *  AP -> NP \n
 *  @ref snpCharCfgUpdatedRsp_t \n
 *  Size    | Description
 * -------- | -----------------------------------------------------
 *  1  Byte | Status @ref SNP_ERRORS
 *  2  Byte | Connection Handle generating request to read
 *
 *  @param[in]   pReq    : structure of the confirmation,
 *                           see snpCharCfgUpdatedRsp_t
 *
 *  @return     1 Byte Error Code, @ref SNP_ERRORS
 */
uint8_t SNP_processCharConfigUpdatedCnf(snpCharCfgUpdatedRsp_t *pReq);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLENP_GATT_H */

/** @} End SNP_GATT */

/** @} */ // end of SNP

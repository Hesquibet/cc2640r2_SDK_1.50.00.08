/******************************************************************************

 @file       icall_ble_api.h

 @brief Redefinition of all BLE APIs when ICALL_LITE is in use

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2016-2017, Texas Instruments Incorporated
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

#ifndef ICALL_BLE_API_H
#define ICALL_BLE_API_H

#ifdef __cplusplus
extern "C"
{
#endif

/* This Header file contains all call structure definition */
#include "icall_ble_apimsg.h"

#ifdef ICALL_LITE
/*********************************************************************
 * INCLUDES
 */

#include "TRNGCC26XX.h"

#include "icall_api_idx.h"
/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern uint8 buildRevision(ICall_BuildRevision *pBuildRev);

#ifdef STACK_LIBRARY
extern void *L2CAP_bm_alloc( uint16 size );
extern void GATT_bm_free(gattMsg_t *pMsg, uint8 opcode);
extern void *GATT_bm_alloc(uint16 connHandle, uint8 opcode, uint16 size,
                           uint16 *pSizeAlloc);
#endif /* STACK_LIBRARY */


/*********************************************************************
 * MACROS
 */

/* all the API bellow has been extracted from the following files:
  - gap.h
  - hci.h
  - l2cap.h
  - gatt.h
  - gattserverapp.h
  - linkdb.h
  - att.h
*/

// this macro should only be used within HCI test and host test app, where
// two host can request HCI command (internal embedded application and
// external Host or internal stack host)
// _api is API function name to call, e.g. HCI_ReadBDADDRCmd,
// for example, if the HCI application wants to call HCI_ReadBDADDRCmd();, it
// will be translated into:
// EMBEDDED_HOST(HCI_ReadBDADDRCmd)
// Another example, if the HCI application wants to call
// HCI_EXT_ConnEventNoticeCmd(pAttRsp->connHandle, selfEntity, 0); , it will be
// translated into:
// EMBEDDED_HOST(HCI_EXT_ConnEventNoticeCmd, pAttRsp->connHandle, selfEntity, 0)

#define PREFIX(_name) (IDX_##_name)
#define EMBEDDED_HOST(_api, ...) \
{ \
  lastAppOpcodeIdxSent = (uint32_t)  PREFIX(_api); \
  _api(##__VA_ARGS__); \
}

/* HCI API */
/***********/
#define HCI_ReadRssiCmd(...)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ReadRssiCmd , ##__VA_ARGS__))
#define HCI_SetEventMaskCmd(...)                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_SetEventMaskCmd , ##__VA_ARGS__))
#define HCI_SetEventMaskPage2Cmd(...)               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_SetEventMaskPage2Cmd , ##__VA_ARGS__))
#define HCI_ResetCmd(...)                           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ResetCmd , ##__VA_ARGS__))
#define HCI_ReadLocalVersionInfoCmd(...)            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ReadLocalVersionInfoCmd , ##__VA_ARGS__))
#define HCI_ReadLocalSupportedCommandsCmd(...)      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ReadLocalSupportedCommandsCmd , ##__VA_ARGS__))
#define HCI_ReadLocalSupportedFeaturesCmd(...)      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ReadLocalSupportedFeaturesCmd , ##__VA_ARGS__))
#define HCI_ReadBDADDRCmd(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ReadBDADDRCmd , ##__VA_ARGS__))
#define HCI_HostNumCompletedPktCmd(...)             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_HostNumCompletedPktCmd , ##__VA_ARGS__))
#define HCI_HostBufferSizeCmd(...)                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_HostBufferSizeCmd , ##__VA_ARGS__))
#define HCI_SetControllerToHostFlowCtrlCmd(...)     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_SetControllerToHostFlowCtrlCmd , ##__VA_ARGS__))
#define HCI_ReadRemoteVersionInfoCmd(...)           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ReadRemoteVersionInfoCmd , ##__VA_ARGS__))
#define HCI_DisconnectCmd(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_DisconnectCmd , ##__VA_ARGS__))
#define HCI_ReadTransmitPowerLevelCmd(...)          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ReadTransmitPowerLevelCmd , ##__VA_ARGS__))

#define HCI_LE_ReceiverTestCmd(...)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReceiverTestCmd , ##__VA_ARGS__))
#define HCI_LE_TransmitterTestCmd(...)              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_TransmitterTestCmd , ##__VA_ARGS__))
#define HCI_LE_TestEndCmd(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_TestEndCmd , ##__VA_ARGS__))
#define HCI_LE_EncryptCmd(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_EncryptCmd , ##__VA_ARGS__))
#define HCI_LE_RandCmd(...)                         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_RandCmd , ##__VA_ARGS__))
#define HCI_LE_ReadSupportedStatesCmd(...)          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadSupportedStatesCmd , ##__VA_ARGS__))
#define HCI_LE_ReadWhiteListSizeCmd(...)            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadWhiteListSizeCmd , ##__VA_ARGS__))
#define HCI_LE_ClearWhiteListCmd(...)               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ClearWhiteListCmd , ##__VA_ARGS__))
#define HCI_LE_AddWhiteListCmd(...)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_AddWhiteListCmd , ##__VA_ARGS__))
#define HCI_LE_RemoveWhiteListCmd(...)              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_RemoveWhiteListCmd , ##__VA_ARGS__))
#define HCI_LE_SetEventMaskCmd(...)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetEventMaskCmd , ##__VA_ARGS__))
#define HCI_LE_ReadLocalSupportedFeaturesCmd(...)   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadLocalSupportedFeaturesCmd , ##__VA_ARGS__))
#define HCI_LE_ReadBufSizeCmd(...)                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadBufSizeCmd , ##__VA_ARGS__))
#define HCI_LE_SetRandomAddressCmd(...)             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetRandomAddressCmd , ##__VA_ARGS__))
#define HCI_LE_ReadAdvChanTxPowerCmd(...)           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadAdvChanTxPowerCmd , ##__VA_ARGS__))
#define HCI_LE_ReadChannelMapCmd(...)               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadChannelMapCmd , ##__VA_ARGS__))
#define HCI_LE_ReadRemoteUsedFeaturesCmd(...)       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadRemoteUsedFeaturesCmd , ##__VA_ARGS__))
#define HCI_LE_SetHostChanClassificationCmd(...)    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetHostChanClassificationCmd , ##__VA_ARGS__))
/* HCI V4.2 DLE API */
/*******************/
#define HCI_LE_SetDataLenCmd(...)                   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetDataLenCmd , ##__VA_ARGS__))
#define HCI_LE_ReadSuggestedDefaultDataLenCmd(...)  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadSuggestedDefaultDataLenCmd , ##__VA_ARGS__))
#define HCI_LE_WriteSuggestedDefaultDataLenCmd(...) (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_WriteSuggestedDefaultDataLenCmd , ##__VA_ARGS__))
#define HCI_LE_ReadMaxDataLenCmd(...)               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadMaxDataLenCmd , ##__VA_ARGS__))
/* HCI V4.1 API */
/****************/
#define HCI_ReadAuthPayloadTimeoutCmd(...)          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ReadAuthPayloadTimeoutCmd , ##__VA_ARGS__))
#define HCI_WriteAuthPayloadTimeoutCmd(...)         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_WriteAuthPayloadTimeoutCmd , ##__VA_ARGS__))
#define HCI_LE_RemoteConnParamReqReplyCmd(...)      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_RemoteConnParamReqReplyCmd , ##__VA_ARGS__))
#define HCI_LE_RemoteConnParamReqNegReplyCmd(...)   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_RemoteConnParamReqNegReplyCmd , ##__VA_ARGS__))


/* HCI V4.2 Privacy API */
/************************/
#define HCI_LE_AddDeviceToResolvingListCmd(...)           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_AddDeviceToResolvingListCmd , ##__VA_ARGS__))
#define HCI_LE_RemoveDeviceFromResolvingListCmd(...)      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_RemoveDeviceFromResolvingListCmd , ##__VA_ARGS__))
#define HCI_LE_ClearResolvingListCmd(...)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ClearResolvingListCmd , ##__VA_ARGS__))
#define HCI_LE_ReadResolvingListSizeCmd(...)              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadResolvingListSizeCmd , ##__VA_ARGS__))
#define HCI_LE_ReadPeerResolvableAddressCmd(...)          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadPeerResolvableAddressCmd , ##__VA_ARGS__))
#define HCI_LE_ReadLocalResolvableAddressCmd(...)         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadLocalResolvableAddressCmd , ##__VA_ARGS__))
#define HCI_LE_SetAddressResolutionEnableCmd(...)         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetAddressResolutionEnableCmd , ##__VA_ARGS__))
#define HCI_LE_SetResolvablePrivateAddressTimeoutCmd(...) (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetResolvablePrivateAddressTimeoutCmd , ##__VA_ARGS__))
/* HCI V4.2 SC API */
/********************/
#define HCI_LE_ReadLocalP256PublicKeyCmd(...)             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadLocalP256PublicKeyCmd , ##__VA_ARGS__))
#define HCI_LE_GenerateDHKeyCmd(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_GenerateDHKeyCmd , ##__VA_ARGS__))
/* HCI V5.0 - 2Mbps PHY */
/************************/
#define HCI_LE_ReadPhyCmd(...)                            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ReadPhyCmd , ##__VA_ARGS__))
#define HCI_LE_SetDefaultPhyCmd(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetDefaultPhyCmd , ##__VA_ARGS__))
#define HCI_LE_SetPhyCmd(...)                             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetPhyCmd , ##__VA_ARGS__))
#define HCI_LE_EnhancedRxTestCmd(...)                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_EnhancedRxTestCmd , ##__VA_ARGS__))
#define HCI_LE_EnhancedTxTestCmd(...)                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_EnhancedTxTestCmd , ##__VA_ARGS__))
//ROLES
#define HCI_LE_SetAdvParamCmd(...)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetAdvParamCmd , ##__VA_ARGS__))
#define HCI_LE_SetAdvDataCmd(...)                         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetAdvDataCmd , ##__VA_ARGS__))
#define HCI_LE_SetScanRspDataCmd(...)                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetScanRspDataCmd , ##__VA_ARGS__))
#define HCI_LE_SetAdvEnableCmd(...)                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetAdvEnableCmd , ##__VA_ARGS__))
#define HCI_LE_SetScanParamCmd(...)                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetScanParamCmd , ##__VA_ARGS__))
#define HCI_LE_SetScanEnableCmd(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_SetScanEnableCmd , ##__VA_ARGS__))
#define HCI_LE_CreateConnCmd(...)                         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_CreateConnCmd , ##__VA_ARGS__))
#define HCI_LE_CreateConnCancelCmd(...)                   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_CreateConnCancelCmd , ##__VA_ARGS__))
#define HCI_LE_StartEncyptCmd(...)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_StartEncyptCmd , ##__VA_ARGS__))
#define HCI_LE_ConnUpdateCmd(...)                         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_ConnUpdateCmd , ##__VA_ARGS__))
#define HCI_LE_LtkReqReplyCmd(...)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_LtkReqReplyCmd , ##__VA_ARGS__))
#define HCI_LE_LtkReqNegReplyCmd(...)                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_LE_LtkReqNegReplyCmd , ##__VA_ARGS__))

/* HCI Extented API */
/********************/
#define HCI_EXT_SetFastTxResponseTimeCmd(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetFastTxResponseTimeCmd , ##__VA_ARGS__))
#define HCI_EXT_SetSlaveLatencyOverrideCmd(...)                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetSlaveLatencyOverrideCmd , ##__VA_ARGS__))
#define HCI_EXT_SetTxPowerCmd(...)                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetTxPowerCmd , ##__VA_ARGS__))
#define HCI_EXT_BuildRevisionCmd(...)                              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_BuildRevisionCmd , ##__VA_ARGS__))
#define HCI_EXT_DelaySleepCmd(...)                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_DelaySleepCmd , ##__VA_ARGS__))
#define HCI_EXT_DecryptCmd(...)                                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_DecryptCmd , ##__VA_ARGS__))
#define HCI_EXT_EnablePTMCmd(...)                                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_EnablePTMCmd , ##__VA_ARGS__))
#define HCI_EXT_ModemTestTxCmd(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_ModemTestTxCmd , ##__VA_ARGS__))
#define HCI_EXT_ModemHopTestTxCmd(...)                             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_ModemHopTestTxCmd , ##__VA_ARGS__))
#define HCI_EXT_ModemTestRxCmd(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_ModemTestRxCmd , ##__VA_ARGS__))
#define HCI_EXT_EndModemTestCmd(...)                               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_EndModemTestCmd , ##__VA_ARGS__))
#define HCI_EXT_SetBDADDRCmd(...)                                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetBDADDRCmd , ##__VA_ARGS__))
#define HCI_EXT_ResetSystemCmd(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_ResetSystemCmd , ##__VA_ARGS__))
#define HCI_EXT_SetLocalSupportedFeaturesCmd(...)                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetLocalSupportedFeaturesCmd , ##__VA_ARGS__))
#define HCI_EXT_SetMaxDtmTxPowerCmd(...)                           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetMaxDtmTxPowerCmd , ##__VA_ARGS__))
#define HCI_EXT_SetRxGainCmd(...)                                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetRxGainCmd , ##__VA_ARGS__))
#define HCI_EXT_ExtendRfRangeCmd(...)                              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_ExtendRfRangeCmd , ##__VA_ARGS__))
#define HCI_EXT_HaltDuringRfCmd(...)                               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_HaltDuringRfCmd , ##__VA_ARGS__))
#define HCI_EXT_ClkDivOnHaltCmd(...)                               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_ClkDivOnHaltCmd , ##__VA_ARGS__))
#define HCI_EXT_DeclareNvUsageCmd(...)                             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_DeclareNvUsageCmd , ##__VA_ARGS__))
#define HCI_EXT_MapPmIoPortCmd(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_MapPmIoPortCmd , ##__VA_ARGS__))
#define HCI_EXT_SetFreqTuneCmd(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetFreqTuneCmd , ##__VA_ARGS__))
#define HCI_EXT_SaveFreqTuneCmd(...)                               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SaveFreqTuneCmd , ##__VA_ARGS__))
#define HCI_EXT_ConnEventNoticeCmd(connHandle, taskID, taskEvent ) (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_ConnEventNoticeCmd, connHandle, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID), taskEvent))
#define HCI_EXT_DisconnectImmedCmd(...)                            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_DisconnectImmedCmd , ##__VA_ARGS__))
#define HCI_EXT_PacketErrorRateCmd(...)                            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_PacketErrorRateCmd , ##__VA_ARGS__))
#define HCI_EXT_NumComplPktsLimitCmd(...)                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_NumComplPktsLimitCmd , ##__VA_ARGS__))
#define HCI_EXT_OnePktPerEvtCmd(...)                               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_OnePktPerEvtCmd , ##__VA_ARGS__))
#define HCI_EXT_SetSCACmd(...)                                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetSCACmd , ##__VA_ARGS__))
#define HCI_EXT_GetConnInfoCmd(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_GetConnInfoCmd , ##__VA_ARGS__))
#define HCI_EXT_GetActiveConnInfoCmd(...)                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_GetActiveConnInfoCmd , ##__VA_ARGS__))
#define HCI_EXT_OverlappedProcessingCmd(...)                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_OverlappedProcessingCmd , ##__VA_ARGS__))
#define HCI_EXT_ScanReqRptCmd(...)                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_ScanReqRptCmd , ##__VA_ARGS__))
#define HCI_EXT_SetMaxDataLenCmd(...)                              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_SetMaxDataLenCmd , ##__VA_ARGS__))
#define HCI_EXT_LLTestModeCmd(...)                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_LLTestModeCmd , ##__VA_ARGS__))
#define HCI_EXT_AdvEventNoticeCmd(taskID, taskEvent )              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_AdvEventNoticeCmd, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID), taskEvent))
#define HCI_EXT_ScanEventNoticeCmd(taskID, taskEvent )             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_ScanEventNoticeCmd, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID), taskEvent))
#define HCI_EXT_PERbyChanCmd(...)                                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_EXT_PERbyChanCmd , ##__VA_ARGS__))

/* HCI API Cont'd */
/******************/
/* TODO this may not be needed */
#define HCI_ProcessRawMessage(...)                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ProcessRawMessage , ##__VA_ARGS__))


#define HCI_SendDataPkt(...)                                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_SendDataPkt , ##__VA_ARGS__))
#define HCI_TL_getCmdResponderID(...)                              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_TL_getCmdResponderID, ##__VA_ARGS__))
#define HCI_CommandCompleteEvent(...)                              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_CommandCompleteEvent, ##__VA_ARGS__))
#define HCI_bm_alloc(...)                                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_bm_alloc , ##__VA_ARGS__))
/* the following API are not available for now */

#define HCI_TestAppTaskRegister(...)                               (AssertHandler(0,0)) // icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_TestAppTaskRegister , ##__VA_ARGS__) // =>need taskId
#define HCI_GAPTaskRegister(...)                                   (AssertHandler(0,0)) // icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_GAPTaskRegister , ##__VA_ARGS__) // =>need taskId
#define HCI_L2CAPTaskRegister(...)                                 (AssertHandler(0,0)) // icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_L2CAPTaskRegister , ##__VA_ARGS__) // =>need taskId
#define HCI_SMPTaskRegister(...)                                   (AssertHandler(0,0)) // icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_SMPTaskRegister , ##__VA_ARGS__) // =>need taskId
#define HCI_ExtTaskRegister(...)                                   (AssertHandler(0,0)) // icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_HCI_ExtTaskRegister , ##__VA_ARGS__) // =>need taskId


/* SNV API */
/***********/
#define osal_snv_read(...)      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_osal_snv_read , ##__VA_ARGS__))
#define osal_snv_write(...)     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_osal_snv_write , ##__VA_ARGS__))
/* the following API are not available for now */
#define osal_snv_init(...)      (AssertHandler(0,0))
#define osal_snv_ext_write(...) (AssertHandler(0,0))
#define osal_snv_compact(...)   (AssertHandler(0,0))


/* UTIL API */
/************/
#define NPI_RegisterTask(taskID) (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_NPI_RegisterTask, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))

#define buildRevision(...)       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_buildRevision , ##__VA_ARGS__))

/* GAP-GATT service API */
/************************/
#define GGS_SetParameter(...)   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GGS_SetParameter  , ##__VA_ARGS__))
#define GGS_AddService(...)     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GGS_AddService , ##__VA_ARGS__))
#define GGS_GetParameter(...)   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GGS_GetParameter , ##__VA_ARGS__))
#define GGS_RegisterAppCBs(...) (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GGS_RegisterAppCBs , ##__VA_ARGS__))
#define GGS_SetParamValue(...)  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GGS_SetParamValue , ##__VA_ARGS__))
#define GGS_GetParamValue(...)  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GGS_GetParamValue , ##__VA_ARGS__))
/* the following API are not available for now */
#define GGS_DelService(...)     (AssertHandler(0,0))


/* GAP Bond Manager API */
/************************/
#define GAPBondMgr_SetParameter(...)            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_SetParameter , ##__VA_ARGS__))
#define GAPBondMgr_GetParameter(...)            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_GetParameter , ##__VA_ARGS__))
#define GAPBondMgr_LinkEst(...)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_LinkEst , ##__VA_ARGS__))
#define GAPBondMgr_LinkTerm(...)                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_LinkTerm , ##__VA_ARGS__))
#define GAPBondMgr_ServiceChangeInd(...)        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_ServiceChangeInd , ##__VA_ARGS__))
#define GAPBondMgr_Register(...)                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_Register , ##__VA_ARGS__))
#define GAPBondMgr_PasscodeRsp(...)             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_PasscodeRsp , ##__VA_ARGS__))
#define GAPBondMgr_SlaveReqSecurity(...)        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_SlaveReqSecurity , ##__VA_ARGS__))
#define GAPBondMgr_ResolveAddr(...)             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_ResolveAddr , ##__VA_ARGS__))
#define GAPBondMgr_UpdateCharCfg(...)           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_UpdateCharCfg , ##__VA_ARGS__))
#define GAPBondMgr_ProcessGAPMsg(...)           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAPBondMgr_ProcessGAPMsg , ##__VA_ARGS__))
/* the following API are not available for now */
#define GAPBondMgr_ReadCentAddrResChar(...)     (AssertHandler(0,0))
#define GAPBondMgr_SupportsEnhancedPriv(...)    (AssertHandler(0,0))
#define GAPBondMgr_syncResolvingList(...)       (AssertHandler(0,0))


/* GAP API */
/***********/
// Initialization and Configuration
#define GAP_DeviceInit(taskID, profileRole, maxScanResponses, pIRK, pSRK, pSignCounter) (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_DeviceInit, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID), profileRole, maxScanResponses, pIRK, pSRK, pSignCounter))
#define GAP_RegisterForMsgs(taskID)                                                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_RegisterForMsgs, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))

#define GAP_SetAdvToken(...)                                                            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_SetAdvToken , ##__VA_ARGS__))
#define GAP_RemoveAdvToken(...)                                                         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_RemoveAdvToken , ##__VA_ARGS__))
#define GAP_UpdateAdvTokens(...)                                                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_UpdateAdvTokens , ##__VA_ARGS__))
#define GAP_SetParamValue(...)                                                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_SetParamValue , ##__VA_ARGS__))
#define GAP_GetParamValue(...)                                                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_GetParamValue , ##__VA_ARGS__))
#define GAP_ConfigDeviceAddr(...)                                                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_ConfigDeviceAddr , ##__VA_ARGS__))
/* the following API are not available for now */
#define GAP_GetAdvToken(...)                                                            (AssertHandler(0,0))
#define GAP_RegisterBondMgrCBs(...)                                                     (AssertHandler(0,0))

// Device Discovery
#define GAP_MakeDiscoverable(taskID, pParams)                                           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_MakeDiscoverable, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID), pParams))
#define GAP_UpdateAdvertisingData(taskID, adType, dataLen, pAdvertData)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_UpdateAdvertisingData, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID), adType, dataLen, pAdvertData))
#define GAP_EndDiscoverable(taskID)                                                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_EndDiscoverable, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GAP_DeviceDiscoveryCancel(taskID)                                               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_DeviceDiscoveryCancel, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GAP_TerminateLinkReq(taskID, connHandle, reason)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_TerminateLinkReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID), connHandle, reason))

#define GAP_DeviceDiscoveryRequest(...)                                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_DeviceDiscoveryRequest , ##__VA_ARGS__))
#define GAP_ResolvePrivateAddr(...)                                                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_ResolvePrivateAddr , ##__VA_ARGS__))

// Link Establishment
#define GAP_EstablishLinkReq(...)                                                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_EstablishLinkReq , ##__VA_ARGS__))
#define GAP_UpdateLinkParamReq(...)                                                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_UpdateLinkParamReq , ##__VA_ARGS__))
#define GAP_UpdateLinkParamReqReply(...)                                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_UpdateLinkParamReqReply , ##__VA_ARGS__))
/* the following API are not available for now */
#define GAP_NumActiveConnections(...)            AssertHandler(0,0)

// Pairing
#define GAP_Authenticate(...)                                                           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_Authenticate , ##__VA_ARGS__))
#define GAP_TerminateAuth(...)                                                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_TerminateAuth , ##__VA_ARGS__))
#define GAP_PasskeyUpdate(...)                                                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_PasskeyUpdate , ##__VA_ARGS__))
#define GAP_SendSlaveSecurityRequest(...)                                               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_SendSlaveSecurityRequest , ##__VA_ARGS__))
#define GAP_Signable(...)                                                               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_Signable , ##__VA_ARGS__))
#define GAP_Bond(...)                                                                   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GAP_Bond , ##__VA_ARGS__))
/* the following API are not available for now */
#define GAP_PasscodeUpdate(...)                                                         (AssertHandler(0,0))



/* L2CAP API */
/*************/
#define L2CAP_DeregisterPsm(taskID, psm)                         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_DeregisterPsm, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID), psm))
#define L2CAP_ConnParamUpdateReq(connHandle, pUpdateReq, taskID) (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_ConnParamUpdateReq, connHandle, pUpdateReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define L2CAP_RegisterFlowCtrlTask(taskID)                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_RegisterFlowCtrlTask, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define L2CAP_InfoReq(connHandle, pInfoReq, taskID)              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_InfoReq, connHandle, pInfoReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))

#define L2CAP_RegisterPsm(...)                                   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_RegisterPsm , ##__VA_ARGS__))
#define L2CAP_PsmInfo(...)                                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_PsmInfo , ##__VA_ARGS__))
#define L2CAP_PsmChannels(...)                                   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_PsmChannels , ##__VA_ARGS__))
#define L2CAP_ChannelInfo(...)                                   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_ChannelInfo , ##__VA_ARGS__))
#define L2CAP_ConnectReq(...)                                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_ConnectReq , ##__VA_ARGS__))
#define L2CAP_ConnectRsp(...)                                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_ConnectRsp , ##__VA_ARGS__))
#define L2CAP_DisconnectReq(...)                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_DisconnectReq , ##__VA_ARGS__))
#define L2CAP_FlowCtrlCredit(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_FlowCtrlCredit , ##__VA_ARGS__))
#define L2CAP_SendSDU(...)                                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_SendSDU , ##__VA_ARGS__))

/* the following API are not available for now */
#define L2CAP_CmdReject(...)                                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_CmdReject , ##__VA_ARGS__))                    //(AssertHandler(0,0)) //icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_CmdReject , ##__VA_ARGS__)
#define L2CAP_ConnParamUpdateRsp(...)                            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_ConnParamUpdateRsp , ##__VA_ARGS__))           //(AssertHandler(0,0)) //icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_ConnParamUpdateRsp , ##__VA_ARGS__)
#define L2CAP_SetUserConfig(...)                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_SetUserConfig , ##__VA_ARGS__))                //(AssertHandler(0,0)) //icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_SetUserConfig , ##__VA_ARGS__)
#define L2CAP_SetBufSize(...)                                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_SetBufSize , ##__VA_ARGS__))                   //(AssertHandler(0,0)) //icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_SetBufSize , ##__VA_ARGS__)
#define L2CAP_GetMTU(...)                                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_GetMTU , ##__VA_ARGS__))                       //(AssertHandler(0,0)) //icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_GetMTU , ##__VA_ARGS__)

// L2CAP Utility API Functions
#define L2CAP_SetParamValue(...)                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_SetParamValue , ##__VA_ARGS__))
#define L2CAP_GetParamValue(...)                                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_GetParamValue , ##__VA_ARGS__))
/* the following API are not available for now */
#define L2CAP_RegisterApp(...)                                   (AssertHandler(0,0)) // =>need taskId
#define L2CAP_SendData(...)                                      (AssertHandler(0,0))
#define L2CAP_SetControllerToHostFlowCtrl(...)                   (AssertHandler(0,0)) //icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_SetControllerToHostFlowCtrl , ##__VA_ARGS__)
#define L2CAP_HostNumCompletedPkts(...)                          (AssertHandler(0,0)) //icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_L2CAP_HostNumCompletedPkts , ##__VA_ARGS__)


/* GATT API */
/************/
// GATT Client
#define GATT_RegisterForInd(taskID)                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_RegisterForInd, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_RegisterForReq(taskID)                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_RegisterForReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_InitClient(...)                           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_InitClient , ##__VA_ARGS__))

// GATT Server API
#define GATT_SendRsp(...)                              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_SendRsp   , ##__VA_ARGS__))
#define GATT_GetNextHandle(...)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_GetNextHandle   , ##__VA_ARGS__))

#define GATT_PrepareWriteReq(connHandle, pReq, taskID) (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_PrepareWriteReq, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_ExecuteWriteReq(connHandle, pReq, taskID) (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_ExecuteWriteReq, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))


/* the following API are not available for now */
#define GATT_InitServer(...)                           (AssertHandler(0,0))
#define GATT_VerifyReadPermissions(...)                (AssertHandler(0,0))
#define GATT_VerifyWritePermissions(...)               (AssertHandler(0,0))
#define GATT_ServiceChangedInd(...)                    (AssertHandler(0,0))// =>need taskId
#define GATT_FindHandleUUID(...)                       (AssertHandler(0,0))
#define GATT_FindHandle(...)                           (AssertHandler(0,0))
#define GATT_FindNextAttr(...)                         (AssertHandler(0,0))
#define GATT_ServiceNumAttrs(...)                      (AssertHandler(0,0))
#define GATT_ServiceEncKeySize(...)                    (AssertHandler(0,0))

// GATT Server Sub-Procedure APIs
#define GATT_ExchangeMTU(connHandle, pReq, taskID)                              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_ExchangeMTU, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_DiscAllPrimaryServices(connHandle, taskID)                         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_DiscAllPrimaryServices, connHandle, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_DiscPrimaryServiceByUUID(connHandle, pUUID, len,  taskID)          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_DiscPrimaryServiceByUUID, connHandle, pUUID, len, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_FindIncludedServices(connHandle, startHandle, endHandle,  taskID)  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_FindIncludedServices, connHandle, startHandle, endHandle, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_DiscAllChars(connHandle, startHandle, endHandle,  taskID)          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_DiscAllChars, connHandle, startHandle, endHandle, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_DiscCharsByUUID(connHandle, pReq, taskID)                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_DiscCharsByUUID, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_DiscAllCharDescs(connHandle, startHandle, endHandle,  taskID)      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_DiscAllCharDescs, connHandle, startHandle, endHandle, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_ReadCharValue(connHandle, pReq, taskID)                            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_ReadCharValue, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_ReadUsingCharUUID(connHandle, pReq, taskID)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_ReadUsingCharUUID, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_ReadLongCharValue(connHandle, pReq, taskID)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_ReadLongCharValue, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_ReadMultiCharValues(connHandle, pReq, taskID)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_ReadMultiCharValues, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_WriteCharValue(connHandle, pReq, taskID)                           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_WriteCharValue, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_WriteLongCharDesc(connHandle, pReq, taskID)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_WriteLongCharDesc, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_WriteLongCharValue(connHandle, pReq, taskID)                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_WriteLongCharValue, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_ReliableWrites(connHandle, pReq, numReqs, flags, taskID)           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_ReliableWrites, connHandle, pReq, numReqs, flags, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_ReadCharDesc(connHandle, pReq, taskID)                             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_ReadCharDesc, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_ReadLongCharDesc(connHandle, pReq, taskID)                         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_ReadLongCharDesc, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_WriteCharDesc(connHandle, pReq, taskID)                            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_WriteCharDesc, connHandle, pReq, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATT_Indication(connHandle, pInd, authenticated, taskID)                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_Indication, connHandle, pInd, authenticated, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))

#define GATT_WriteNoRsp(...)                                                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_WriteNoRsp , ##__VA_ARGS__))
#define GATT_SignedWriteNoRsp(...)                                              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_SignedWriteNoRsp , ##__VA_ARGS__))
#define GATT_Notification(...)                                                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_Notification , ##__VA_ARGS__))

#define GATT_FindUUIDRec(...)                                                   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_FindUUIDRec , ##__VA_ARGS__))
#define GATT_RegisterService(...)                                               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_RegisterService , ##__VA_ARGS__))
#define GATT_DeregisterService(...)                                             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_DeregisterService , ##__VA_ARGS__))



// GATT Client and Server Common APIs
#define GATT_RegisterForMsgs(taskID)                                            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_RegisterForMsgs, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
/* the following API are not available for now */
#define GATT_NotifyEvent(...)                                                   (AssertHandler(0,0))
#define GATT_UpdateMTU(...)                                                     (AssertHandler(0,0))

//GATT Buffer Management APIs
// Those are enabled in a different way...,
// do not uncomment
//#define GATT_bm_alloc(...)                                                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) GATT_bm_alloc))
//#define GATT_bm_free(...)                                                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) GATT_bm_free))

//GATT Flow Control APIs
 #define GATT_SetHostToAppFlowCtrl(...)                                         (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_SetHostToAppFlowCtrl , ##__VA_ARGS__))
 #define GATT_AppCompletedMsg(...)                                              (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATT_AppCompletedMsg , ##__VA_ARGS__))


/* GATT SERVER APPLICATION API */
/*******************************/
#define GATTServApp_SendServiceChangedInd(connHandle, taskID)   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_SendServiceChangedInd, connHandle, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))
#define GATTServApp_RegisterForMsg(taskID)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_RegisterForMsg, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))

#define GATTServApp_RegisterService(...)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_RegisterService , ##__VA_ARGS__))
#define GATTServApp_AddService(...)                             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_AddService , ##__VA_ARGS__))
#define GATTServApp_DeregisterService(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_DeregisterService , ##__VA_ARGS__))
#define GATTServApp_SetParameter(...)                           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_SetParameter , ##__VA_ARGS__))
#define GATTServApp_GetParameter(...)                           (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_GetParameter , ##__VA_ARGS__))
#define GATTServApp_SendCCCUpdatedEvent(...)                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_SendCCCUpdatedEvent , ##__VA_ARGS__))
#define GATTServApp_ReadRsp(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_ReadRsp , ##__VA_ARGS__))
#define GATTQual_AddService(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTQual_AddService , ##__VA_ARGS__))
#define GATTTest_AddService(...)                                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTTest_AddService , ##__VA_ARGS__))
#define GATTServApp_GetParamValue(...)                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_GetParamValue , ##__VA_ARGS__))
#define GATTServApp_SetParamValue(...)                          (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_GATTServApp_SetParamValue , ##__VA_ARGS__))

/* the following API are not available for now */
#define GATTServApp_DelService(...)                             (AssertHandler(0,0))
#define GATTServApp_UpdateCharCfg(...)                          (AssertHandler(0,0))
#define GATTServApp_ReadAttr(...)                               (AssertHandler(0,0))
#define GATTServApp_WriteAttr(...)                              (AssertHandler(0,0))
// part of the application, do not uncomment :
// #define GATTServApp_InitCharCfg(...)                         (AssertHandler(0,0))
// #define GATTServApp_ProcessCharCfg(...)                      (AssertHandler(0,0)) // => need taskId
// #define GATTServApp_FindAttr(...)                            (AssertHandler(0,0))
// #define GATTServApp_ProcessCCCWriteReq(...)                  (AssertHandler(0,0))
// #define GATTServApp_ReadCharCfg(...)                         (AssertHandler(0,0))
// #define GATTServApp_WriteCharCfg(...)                        (AssertHandler(0,0))


/* LINK DB API */
/***************/
#define linkDB_NumActive(...)                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_linkDB_NumActive , ##__VA_ARGS__))
#define linkDB_GetInfo(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_linkDB_GetInfo , ##__VA_ARGS__))
#define linkDB_State(...)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_linkDB_State , ##__VA_ARGS__))
#define linkDB_NumConns(...)                     (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_linkDB_NumConns , ##__VA_ARGS__))
/* the following API are not available for now */
#define linkDB_Init(...)                         (AssertHandler(0,0))
#define linkDB_Register(...)                     (AssertHandler(0,0))
#define linkDB_Add(...)                          (AssertHandler(0,0)) // => need taskId
#define linkDB_Remove(...)                       (AssertHandler(0,0))
#define linkDB_Update(...)                       (AssertHandler(0,0))
#define linkDB_UpdateMTU(...)                    (AssertHandler(0,0))
#define linkDB_MTU(...)                          (AssertHandler(0,0))
#define linkDB_Find(...)                         (AssertHandler(0,0))
#define linkDB_FindFirst(...)                    (AssertHandler(0,0)) // => need taskId
#define linkDB_Authen(...)                       (AssertHandler(0,0))
#define linkDB_Role(...)                         (AssertHandler(0,0))
#define linkDB_PerformFunc(...)                  (AssertHandler(0,0))
#define linkDB_SecurityModeSCOnly(...)           (AssertHandler(0,0))

/* ATT API */
/***********/
//Attribute Client Public APIs
#define ATT_HandleValueCfm(...)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_HandleValueCfm , ##__VA_ARGS__))
#define ATT_ErrorRsp(...)                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_ErrorRsp , ##__VA_ARGS__))
#define ATT_ReadBlobRsp(...)                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_ReadBlobRsp , ##__VA_ARGS__))
#define ATT_ExecuteWriteRsp(...)                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_ExecuteWriteRsp , ##__VA_ARGS__))
#define ATT_WriteRsp(...)                       (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_WriteRsp , ##__VA_ARGS__))
#define ATT_ReadRsp(...)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_ReadRsp , ##__VA_ARGS__))
#define ATT_ParseExchangeMTUReq(...)            (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_ParseExchangeMTUReq , ##__VA_ARGS__))

   /* the following API are not available for now */
#define ATT_ExchangeMTUReq(...)                 (AssertHandler(0,0))
#define ATT_FindInfoReq(...)                    (AssertHandler(0,0))
#define ATT_FindByTypeValueReq(...)             (AssertHandler(0,0))
#define ATT_ReadByTypeReq(...)                  (AssertHandler(0,0))
#define ATT_ReadReq(...)                        (AssertHandler(0,0))
#define ATT_ReadBlobReq(...)                    (AssertHandler(0,0))
#define ATT_ReadMultiReq(...)                   (AssertHandler(0,0))
#define ATT_ReadByGrpTypeReq(...)               (AssertHandler(0,0))
#define ATT_WriteReq(...)                       (AssertHandler(0,0))
#define ATT_PrepareWriteReq(...)                (AssertHandler(0,0))
#define ATT_ExecuteWriteReq(...)                (AssertHandler(0,0))

//Attribute Server Public APIs
#define ATT_ExchangeMTURsp(...)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_ExchangeMTURsp , ##__VA_ARGS__))
#define ATT_FindInfoRsp(...)                    (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_FindInfoRsp , ##__VA_ARGS__))
#define ATT_FindByTypeValueRsp(...)             (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_FindByTypeValueRsp , ##__VA_ARGS__))
#define ATT_ReadByTypeRsp(...)                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_ReadByTypeRsp , ##__VA_ARGS__))
#define ATT_ReadMultiRsp(...)                   (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_ReadMultiRsp , ##__VA_ARGS__))
#define ATT_ReadByGrpTypeRsp(...)               (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_ReadByGrpTypeRsp , ##__VA_ARGS__))
#define ATT_PrepareWriteRsp(...)                (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_PrepareWriteRsp , ##__VA_ARGS__))
/* the following API are not available for now */
#define ATT_HandleValueNoti(...)                (AssertHandler(0,0)
#define ATT_HandleValueInd(...)                 (AssertHandler(0,0)


// Attribute Common Public APIs
#define ATT_SetParamValue(...)                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_SetParamValue , ##__VA_ARGS__))
#define ATT_GetParamValue(...)                  (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_ATT_GetParamValue , ##__VA_ARGS__))
/* the following API are not available for now */
#define ATT_UpdateMTU(...)                      (AssertHandler(0,0))
#define ATT_GetMTU(...)                         (AssertHandler(0,0))
#define ATT_RegisterServer(...)                 (AssertHandler(0,0))
#define ATT_RegisterClient(...)                 (AssertHandler(0,0))

/* Security Manager API */
/************************/
#define SM_GetScConfirmOob(...)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_SM_GetScConfirmOob , ##__VA_ARGS__))
#define SM_GetEccKeys(...)                      (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_SM_GetEccKeys , ##__VA_ARGS__))
#define SM_GetDHKey(...)                        (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_SM_GetDHKey , ##__VA_ARGS__))
#define SM_RegisterTask(taskID)                 (icall_directAPI(ICALL_SERVICE_CLASS_BLE, (uint32_t) IDX_SM_RegisterTask, ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, taskID)))

#endif /* ICALL_LITE */
#ifdef __cplusplus
}
#endif

#endif /* ICALL_BLE_API_H */

/******************************************************************************

 @file       icall_api_idx.h

 @brief implementation of API override for the case the stack is use as library.

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

#ifndef ICALL_API_IDX_H
#define ICALL_API_IDX_H


#ifndef STACK_LIBRARY
#include "ble_dispatch_lite_idx.h"
#else
/* GAP-GATT service API */
/************************/
#define IDX_GGS_SetParameter                          GGS_SetParameter
#define IDX_GGS_AddService                            GGS_AddService
#define IDX_GGS_GetParameter                          GGS_GetParameter
#define IDX_GGS_RegisterAppCBs                        GGS_RegisterAppCBs
#define IDX_GGS_SetParamValue                         GGS_SetParamValue
#define IDX_GGS_GetParamValue                         GGS_GetParamValue

/* GAP Bond Manager API */
/************************/
#define IDX_GAPBondMgr_SetParameter                   GAPBondMgr_SetParameter
#define IDX_GAPBondMgr_GetParameter                   GAPBondMgr_GetParameter
#define IDX_GAPBondMgr_LinkEst                        GAPBondMgr_LinkEst
#define IDX_GAPBondMgr_LinkTerm                       GAPBondMgr_LinkTerm
#define IDX_GAPBondMgr_ServiceChangeInd               GAPBondMgr_ServiceChangeInd
#define IDX_GAPBondMgr_Register                       GAPBondMgr_Register
#define IDX_GAPBondMgr_PasscodeRsp                    GAPBondMgr_PasscodeRsp
#define IDX_GAPBondMgr_SlaveReqSecurity               GAPBondMgr_SlaveReqSecurity
#define IDX_GAPBondMgr_ResolveAddr                    GAPBondMgr_ResolveAddr
#define IDX_GAPBondMgr_UpdateCharCfg                  GAPBondMgr_UpdateCharCfg
#define IDX_GAPBondMgr_ProcessGAPMsg                  GAPBondMgr_ProcessGAPMsg

/* GAP API */
/***********/
#define IDX_GAP_DeviceInit                            GAP_DeviceInit
#define IDX_GAP_RegisterForMsgs                       GAP_RegisterForMsgs
#define IDX_GAP_SetAdvToken                           GAP_SetAdvToken
#define IDX_GAP_RemoveAdvToken                        GAP_RemoveAdvToken
#define IDX_GAP_UpdateAdvTokens                       GAP_UpdateAdvTokens
#define IDX_GAP_SetParamValue                         GAP_SetParamValue
#define IDX_GAP_GetParamValue                         GAP_GetParamValue
#define IDX_GAP_ConfigDeviceAddr                      GAP_ConfigDeviceAddr
#define IDX_GAP_RegisterBondMgrCBs                    GAP_RegisterBondMgrCBs
#define IDX_GAP_MakeDiscoverable                      GAP_MakeDiscoverable
#define IDX_GAP_UpdateAdvertisingData                 GAP_UpdateAdvertisingData
#define IDX_GAP_EndDiscoverable                       GAP_EndDiscoverable
#define IDX_GAP_DeviceDiscoveryCancel                 GAP_DeviceDiscoveryCancel
#define IDX_GAP_TerminateLinkReq                      GAP_TerminateLinkReq
#define IDX_GAP_DeviceDiscoveryRequest                GAP_DeviceDiscoveryRequest
#define IDX_GAP_ResolvePrivateAddr                    GAP_ResolvePrivateAddr
#define IDX_GAP_EstablishLinkReq                      GAP_EstablishLinkReq
#define IDX_GAP_UpdateLinkParamReq                    GAP_UpdateLinkParamReq
#define IDX_GAP_UpdateLinkParamReqReply               GAP_UpdateLinkParamReqReply
#define IDX_GAP_Authenticate                          GAP_Authenticate
#define IDX_GAP_TerminateAuth                         GAP_TerminateAuth
#define IDX_GAP_PasskeyUpdate                         GAP_PasskeyUpdate
#define IDX_GAP_SendSlaveSecurityRequest              GAP_SendSlaveSecurityRequest
#define IDX_GAP_Signable                              GAP_Signable
#define IDX_GAP_Bond                                  GAP_Bond

/* HCI API */
/***********/
#define IDX_HCI_ReadRemoteVersionInfoCmd              HCI_ReadRemoteVersionInfoCmd
#define IDX_HCI_SetEventMaskCmd                       HCI_SetEventMaskCmd
#define IDX_HCI_SetEventMaskPage2Cmd                  HCI_SetEventMaskPage2Cmd
#define IDX_HCI_ResetCmd                              HCI_ResetCmd
#define IDX_HCI_ReadTransmitPowerLevelCmd             HCI_ReadTransmitPowerLevelCmd
#define IDX_HCI_ReadLocalVersionInfoCmd               HCI_ReadLocalVersionInfoCmd
#define IDX_HCI_ReadLocalSupportedCommandsCmd         HCI_ReadLocalSupportedCommandsCmd
#define IDX_HCI_ReadLocalSupportedFeaturesCmd         HCI_ReadLocalSupportedFeaturesCmd
#define IDX_HCI_ReadBDADDRCmd                         HCI_ReadBDADDRCmd
#define IDX_HCI_ReadRssiCmd                           HCI_ReadRssiCmd
#define IDX_HCI_ValidConnTimeParams                   HCI_ValidConnTimeParams
#define IDX_HCI_LE_SetEventMaskCmd                    HCI_LE_SetEventMaskCmd
#define IDX_HCI_LE_ReadLocalSupportedFeaturesCmd      HCI_LE_ReadLocalSupportedFeaturesCmd
#define IDX_HCI_LE_ReadAdvChanTxPowerCmd              HCI_LE_ReadAdvChanTxPowerCmd
#define IDX_HCI_LE_ReadWhiteListSizeCmd               HCI_LE_ReadWhiteListSizeCmd
#define IDX_HCI_LE_ClearWhiteListCmd                  HCI_LE_ClearWhiteListCmd
#define IDX_HCI_LE_AddWhiteListCmd                    HCI_LE_AddWhiteListCmd
#define IDX_HCI_LE_RemoveWhiteListCmd                 HCI_LE_RemoveWhiteListCmd
#define IDX_HCI_LE_SetHostChanClassificationCmd       HCI_LE_SetHostChanClassificationCmd
#define IDX_HCI_LE_ReadChannelMapCmd                  HCI_LE_ReadChannelMapCmd
#define IDX_HCI_LE_ReadRemoteUsedFeaturesCmd          HCI_LE_ReadRemoteUsedFeaturesCmd
#define IDX_HCI_LE_EncryptCmd                         HCI_LE_EncryptCmd
#define IDX_HCI_LE_ReadSupportedStatesCmd             HCI_LE_ReadSupportedStatesCmd
#define IDX_HCI_LE_ReceiverTestCmd                    HCI_LE_ReceiverTestCmd
#define IDX_HCI_LE_TransmitterTestCmd                 HCI_LE_TransmitterTestCmd
#define IDX_HCI_LE_TestEndCmd                         HCI_LE_TestEndCmd
#define IDX_HCI_LE_ReadMaxDataLenCmd                  HCI_LE_ReadMaxDataLenCmd
#define IDX_HCI_LE_ReadSuggestedDefaultDataLenCmd     HCI_LE_ReadSuggestedDefaultDataLenCmd
#define IDX_HCI_LE_WriteSuggestedDefaultDataLenCmd    HCI_LE_WriteSuggestedDefaultDataLenCmd
#define IDX_HCI_LE_SetDataLenCmd                      HCI_LE_SetDataLenCmd

/* HCI Extented API */
/********************/
#define IDX_HCI_EXT_AdvEventNoticeCmd                    HCI_EXT_AdvEventNoticeCmd
#define IDX_HCI_EXT_ScanEventNoticeCmd                   HCI_EXT_ScanEventNoticeCmd
#define IDX_HCI_EXT_ConnEventNoticeCmd                   HCI_EXT_ConnEventNoticeCmd
#define IDX_HCI_EXT_SetTxPowerCmd                        HCI_EXT_SetTxPowerCmd
#define IDX_HCI_EXT_OnePktPerEvtCmd                      HCI_EXT_OnePktPerEvtCmd
#define IDX_HCI_EXT_DecryptCmd                           HCI_EXT_DecryptCmd
#define IDX_HCI_EXT_SetLocalSupportedFeaturesCmd         HCI_EXT_SetLocalSupportedFeaturesCmd
#define IDX_HCI_EXT_SetFastTxResponseTimeCmd             HCI_EXT_SetFastTxResponseTimeCmd
#define IDX_HCI_EXT_SetSlaveLatencyOverrideCmd           HCI_EXT_SetSlaveLatencyOverrideCmd
#define IDX_HCI_EXT_ModemTestTxCmd                       HCI_EXT_ModemTestTxCmd
#define IDX_HCI_EXT_ModemHopTestTxCmd                    HCI_EXT_ModemHopTestTxCmd
#define IDX_HCI_EXT_ModemTestRxCmd                       HCI_EXT_ModemTestRxCmd
#define IDX_HCI_EXT_EndModemTestCmd                      HCI_EXT_EndModemTestCmd
#define IDX_HCI_EXT_SetBDADDRCmd                         HCI_EXT_SetBDADDRCmd
#define IDX_HCI_EXT_SetSCACmd                            HCI_EXT_SetSCACmd
#define IDX_HCI_EXT_EnablePTMCmd                         HCI_EXT_EnablePTMCmd
#define IDX_HCI_EXT_SetMaxDtmTxPowerCmd                  HCI_EXT_SetMaxDtmTxPowerCmd
#define IDX_HCI_EXT_DisconnectImmedCmd                   HCI_EXT_DisconnectImmedCmd
#define IDX_HCI_EXT_PacketErrorRateCmd                   HCI_EXT_PacketErrorRateCmd
#define IDX_HCI_EXT_PERbyChanCmd                         HCI_EXT_PERbyChanCmd
#define IDX_HCI_EXT_BuildRevisionCmd                     HCI_EXT_BuildRevisionCmd
#define IDX_HCI_EXT_DelaySleepCmd                        HCI_EXT_DelaySleepCmd
#define IDX_HCI_EXT_ResetSystemCmd                       HCI_EXT_ResetSystemCmd
#define IDX_HCI_EXT_NumComplPktsLimitCmd                 HCI_EXT_NumComplPktsLimitCmd
#define IDX_HCI_EXT_GetConnInfoCmd                       HCI_EXT_GetConnInfoCmd
#define IDX_HCI_EXT_GetActiveConnInfoCmd                 HCI_EXT_GetActiveConnInfoCmd
#define IDX_HCI_TL_getCmdResponderID                     HCI_TL_getCmdResponderID
#define IDX_HCI_LE_RandCmd                               HCI_LE_RandCmd
#define IDX_HCI_LE_ReadBufSizeCmd                        HCI_LE_ReadBufSizeCmd
#define IDX_HCI_LE_SetRandomAddressCmd                   HCI_LE_SetRandomAddressCmd
#define IDX_HCI_DisconnectCmd                            HCI_DisconnectCmd
#define IDX_HCI_SetControllerToHostFlowCtrlCmd           HCI_SetControllerToHostFlowCtrlCmd
#define IDX_HCI_HostBufferSizeCmd                        HCI_HostBufferSizeCmd
#define IDX_HCI_HostNumCompletedPktCmd                   HCI_HostNumCompletedPktCmd
#define IDX_HCI_ReadAuthPayloadTimeoutCmd                HCI_ReadAuthPayloadTimeoutCmd
#define IDX_HCI_WriteAuthPayloadTimeoutCmd               HCI_WriteAuthPayloadTimeoutCmd
#define IDX_HCI_LE_RemoteConnParamReqReplyCmd            HCI_LE_RemoteConnParamReqReplyCmd
#define IDX_HCI_LE_RemoteConnParamReqNegReplyCmd         HCI_LE_RemoteConnParamReqNegReplyCmd
#define IDX_HCI_LE_AddDeviceToResolvingListCmd           HCI_LE_AddDeviceToResolvingListCmd
#define IDX_HCI_LE_RemoveDeviceFromResolvingListCmd      HCI_LE_RemoveDeviceFromResolvingListCmd
#define IDX_HCI_LE_ClearResolvingListCmd                 HCI_LE_ClearResolvingListCmd
#define IDX_HCI_LE_ReadResolvingListSizeCmd              HCI_LE_ReadResolvingListSizeCmd
#define IDX_HCI_LE_ReadPeerResolvableAddressCmd          HCI_LE_ReadPeerResolvableAddressCmd
#define IDX_HCI_LE_ReadLocalResolvableAddressCmd         HCI_LE_ReadLocalResolvableAddressCmd
#define IDX_HCI_LE_SetAddressResolutionEnableCmd         HCI_LE_SetAddressResolutionEnableCmd
#define IDX_HCI_LE_SetResolvablePrivateAddressTimeoutCmd HCI_LE_SetResolvablePrivateAddressTimeoutCmd
#define IDX_HCI_LE_ReadLocalP256PublicKeyCmd             HCI_LE_ReadLocalP256PublicKeyCmd
#define IDX_HCI_LE_GenerateDHKeyCmd                      HCI_LE_GenerateDHKeyCmd
#define IDX_HCI_LE_SetAdvParamCmd                        HCI_LE_SetAdvParamCmd
#define IDX_HCI_LE_SetAdvDataCmd                         HCI_LE_SetAdvDataCmd
#define IDX_HCI_LE_SetScanRspDataCmd                     HCI_LE_SetScanRspDataCmd
#define IDX_HCI_LE_SetAdvEnableCmd                       HCI_LE_SetAdvEnableCmd
#define IDX_HCI_LE_SetScanParamCmd                       HCI_LE_SetScanParamCmd
#define IDX_HCI_LE_SetScanEnableCmd                      HCI_LE_SetScanEnableCmd
#define IDX_HCI_LE_CreateConnCmd                         HCI_LE_CreateConnCmd
#define IDX_HCI_LE_CreateConnCancelCmd                   HCI_LE_CreateConnCancelCmd
#define IDX_HCI_LE_StartEncyptCmd                        HCI_LE_StartEncyptCmd
#define IDX_HCI_LE_ConnUpdateCmd                         HCI_LE_ConnUpdateCmd
#define IDX_HCI_LE_LtkReqReplyCmd                        HCI_LE_LtkReqReplyCmd
#define IDX_HCI_LE_LtkReqNegReplyCmd                     HCI_LE_LtkReqNegReplyCmd
#define IDX_HCI_EXT_SetRxGainCmd                         HCI_EXT_SetRxGainCmd
#define IDX_HCI_EXT_ExtendRfRangeCmd                     HCI_EXT_ExtendRfRangeCmd
#define IDX_HCI_EXT_HaltDuringRfCmd                      HCI_EXT_HaltDuringRfCmd
#define IDX_HCI_EXT_ClkDivOnHaltCmd                      HCI_EXT_ClkDivOnHaltCmd
#define IDX_HCI_EXT_DeclareNvUsageCmd                    HCI_EXT_DeclareNvUsageCmd
#define IDX_HCI_EXT_MapPmIoPortCmd                       HCI_EXT_MapPmIoPortCmd
#define IDX_HCI_EXT_SetFreqTuneCmd                       HCI_EXT_SetFreqTuneCmd
#define IDX_HCI_EXT_SaveFreqTuneCmd                      HCI_EXT_SaveFreqTuneCmd
#define IDX_HCI_EXT_OverlappedProcessingCmd              HCI_EXT_OverlappedProcessingCmd
#define IDX_HCI_EXT_ScanReqRptCmd                        HCI_EXT_ScanReqRptCmd
#define IDX_HCI_EXT_SetMaxDataLenCmd                     HCI_EXT_SetMaxDataLenCmd
#define IDX_HCI_EXT_LLTestModeCmd                        HCI_EXT_LLTestModeCmd
#define IDX_HCI_SendDataPkt                              HCI_SendDataPkt
#define IDX_HCI_CommandCompleteEvent                     HCI_CommandCompleteEvent
#define IDX_HCI_bm_alloc                                 HCI_bm_alloc
#define IDX_HCI_LE_ReadPhyCmd                            HCI_LE_ReadPhyCmd
#define IDX_HCI_LE_SetDefaultPhyCmd                      HCI_LE_SetDefaultPhyCmd
#define IDX_HCI_LE_SetPhyCmd                             HCI_LE_SetPhyCmd
#define IDX_HCI_LE_EnhancedRxTestCmd                     HCI_LE_EnhancedRxTestCmd
#define IDX_HCI_LE_EnhancedTxTestCmd                     HCI_LE_EnhancedTxTestCmd




/* L2CAP API */
/*************/
#define IDX_L2CAP_DeregisterPsm                       L2CAP_DeregisterPsm
#define IDX_L2CAP_ConnParamUpdateReq                  L2CAP_ConnParamUpdateReq
#define IDX_L2CAP_ParseParamUpdateReq                 L2CAP_ParseParamUpdateReq
#define IDX_L2CAP_ParseInfoReq                        L2CAP_ParseInfoReq
#define IDX_L2CAP_RegisterPsm                         L2CAP_RegisterPsm
#define IDX_L2CAP_PsmInfo                             L2CAP_PsmInfo
#define IDX_L2CAP_PsmChannels                         L2CAP_PsmChannels
#define IDX_L2CAP_ChannelInfo                         L2CAP_ChannelInfo
#define IDX_L2CAP_ConnectReq                          L2CAP_ConnectReq
#define IDX_L2CAP_ConnectRsp                          L2CAP_ConnectRsp
#define IDX_L2CAP_DisconnectReq                       L2CAP_DisconnectReq
#define IDX_L2CAP_FlowCtrlCredit                      L2CAP_FlowCtrlCredit
#define IDX_L2CAP_SendSDU                             L2CAP_SendSDU
#define IDX_L2CAP_SetParamValue                       L2CAP_SetParamValue
#define IDX_L2CAP_GetParamValue                       L2CAP_GetParamValue
#define IDX_L2CAP_InfoReq                             L2CAP_InfoReq

/* GATT API */
/************/
#define IDX_GATT_RegisterForInd                       GATT_RegisterForInd
#define IDX_GATT_RegisterForReq                       GATT_RegisterForReq
#define IDX_GATT_PrepareWriteReq                      GATT_PrepareWriteReq
#define IDX_GATT_ExecuteWriteReq                      GATT_ExecuteWriteReq
#define IDX_GATT_InitClient                           GATT_InitClient
#define IDX_GATT_InitServer                           GATT_InitServer
#define IDX_GATT_SendRsp                              GATT_SendRsp
#define IDX_GATT_GetNextHandle                        GATT_GetNextHandle
#define IDX_GATT_PrepareWriteReq                      GATT_PrepareWriteReq
#define IDX_GATT_ExecuteWriteReq                      GATT_ExecuteWriteReq
#define IDX_GATT_FindUUIDRec                          GATT_FindUUIDRec
#define IDX_GATT_RegisterService                      GATT_RegisterService
#define IDX_GATT_DeregisterService                    GATT_DeregisterService
#define IDX_GATT_Indication                           GATT_Indication
#define IDX_GATT_ExchangeMTU                          GATT_ExchangeMTU
#define IDX_GATT_DiscAllPrimaryServices               GATT_DiscAllPrimaryServices
#define IDX_GATT_DiscPrimaryServiceByUUID             GATT_DiscPrimaryServiceByUUID
#define IDX_GATT_FindIncludedServices                 GATT_FindIncludedServices
#define IDX_GATT_DiscAllChars                         GATT_DiscAllChars
#define IDX_GATT_DiscCharsByUUID                      GATT_DiscCharsByUUID
#define IDX_GATT_DiscAllCharDescs                     GATT_DiscAllCharDescs
#define IDX_GATT_ReadCharValue                        GATT_ReadCharValue
#define IDX_GATT_ReadUsingCharUUID                    GATT_ReadUsingCharUUID
#define IDX_GATT_ReadLongCharValue                    GATT_ReadLongCharValue
#define IDX_GATT_ReadMultiCharValues                  GATT_ReadMultiCharValues
#define IDX_GATT_WriteCharValue                       GATT_WriteCharValue
#define IDX_GATT_WriteLongCharValue                   GATT_WriteLongCharValue
#define IDX_GATT_ReliableWrites                       GATT_ReliableWrites
#define IDX_GATT_ReadCharDesc                         GATT_ReadCharDesc
#define IDX_GATT_ReadLongCharDesc                     GATT_ReadLongCharDesc
#define IDX_GATT_WriteCharDesc                        GATT_WriteCharDesc
#define IDX_GATT_WriteLongCharDesc                    GATT_WriteLongCharDesc
#define IDX_GATT_Notification                         GATT_Notification
#define IDX_GATT_WriteNoRsp                           GATT_WriteNoRsp
#define IDX_GATT_SignedWriteNoRsp                     GATT_SignedWriteNoRsp
#define IDX_GATT_RegisterForMsgs                      GATT_RegisterForMsgs
#define IDX_GATT_SetHostToAppFlowCtrl                 GATT_SetHostToAppFlowCtrl
#define IDX_GATT_AppCompletedMsg                      GATT_AppCompletedMsg

/* GATT SERVER APPLICATION API */
/*******************************/
#define IDX_GATTServApp_SendServiceChangedInd         GATTServApp_SendServiceChangedInd
#define IDX_GATTServApp_RegisterService               GATTServApp_RegisterService
#define IDX_GATTServApp_AddService                    GATTServApp_AddService
#define IDX_GATTServApp_AddService                    GATTServApp_AddService
#define IDX_GATTServApp_DeregisterService             GATTServApp_DeregisterService
#define IDX_GATTServApp_SetParameter                  GATTServApp_SetParameter
#define IDX_GATTServApp_GetParameter                  GATTServApp_GetParameter
#define IDX_GATTServApp_SendCCCUpdatedEvent           GATTServApp_SendCCCUpdatedEvent
#define IDX_GATTServApp_ReadRsp                       GATTServApp_ReadRsp
#define IDX_GATTQual_AddService                       GATTQual_AddService
#define IDX_GATTTest_AddService                       GATTTest_AddService
#define IDX_GATTServApp_GetParamValue                 GATTServApp_GetParamValue
#define IDX_GATTServApp_SetParamValue                 GATTServApp_SetParamValue
#define IDX_GATTServApp_RegisterForMsg                GATTServApp_RegisterForMsg

/* LINK DB API */
/***************/
#define IDX_linkDB_NumActive                          linkDB_NumActive
#define IDX_linkDB_GetInfo                            linkDB_GetInfo
#define IDX_linkDB_State                              linkDB_State
#define IDX_linkDB_NumConns                           linkDB_NumConns

/* ATT API */
/***********/
#define IDX_ATT_HandleValueCfm                        ATT_HandleValueCfm
#define IDX_ATT_ErrorRsp                              ATT_ErrorRsp
#define IDX_ATT_ReadBlobRsp                           ATT_ReadBlobRsp
#define IDX_ATT_ExecuteWriteRsp                       ATT_ExecuteWriteRsp
#define IDX_ATT_WriteRsp                              ATT_WriteRsp
#define IDX_ATT_ReadRsp                               ATT_ReadRsp
#define IDX_ATT_ParseExchangeMTUReq                   ATT_ParseExchangeMTUReq
#define IDX_ATT_ExchangeMTURsp                        ATT_ExchangeMTURsp
#define IDX_ATT_FindInfoRsp                           ATT_FindInfoRsp
#define IDX_ATT_FindByTypeValueRsp                    ATT_FindByTypeValueRsp
#define IDX_ATT_ReadByTypeRsp                         ATT_ReadByTypeRsp
#define IDX_ATT_ReadMultiRsp                          ATT_ReadMultiRsp
#define IDX_ATT_ReadByGrpTypeRsp                      ATT_ReadByGrpTypeRsp
#define IDX_ATT_PrepareWriteRsp                       ATT_PrepareWriteRsp
#define IDX_ATT_SetParamValue                         ATT_SetParamValue
#define IDX_ATT_GetParamValue                         ATT_GetParamValue

/* Security Manager API */
/***********/
#define IDX_SM_GetScConfirmOob                        SM_GetScConfirmOob
#define IDX_SM_GetEccKeys                             SM_GetEccKeys
#define IDX_SM_GetDHKey                               SM_GetDHKey
#define IDX_SM_RegisterTask                           SM_RegisterTask

/* SNV API */
/***********/
#define IDX_osal_snv_read                             osal_snv_read
#define IDX_osal_snv_write                            osal_snv_write

/* UTIL API */
/************/
#define IDX_NPI_RegisterTask                          NPI_RegisterTask
#define IDX_buildRevision                             buildRevision

#endif /* !STACK_LIBRARY */

#endif /* ICALL_API_IDX_H */

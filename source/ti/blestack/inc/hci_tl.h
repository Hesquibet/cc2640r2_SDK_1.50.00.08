/******************************************************************************

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2009-2017, Texas Instruments Incorporated
 All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License"). You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless embedded on a Texas Instruments microcontroller
 or used solely and exclusively in conjunction with a Texas Instruments radio
 frequency transceiver, which is integrated into your product. Other than for
 the foregoing purpose, you may not use, reproduce, copy, prepare derivative
 works of, modify, distribute, perform, display or sell this Software and/or
 its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
 LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
 OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
 OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.

 ******************************************************************************
 Release Name: simplelink_cc2640r2_sdk_01_50_00_58
 Release Date: 2017-10-17 18:09:51
 *****************************************************************************/

/**
 *  @addtogroup HCI
 *  @{
 *  @file       hci_tl.h
 * @brief Types, constants, external functions etc. for
        the BLE HCI Transport Layer.
 */

#ifndef HCI_TL_H
#define HCI_TL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */

#include "hci.h"
#include "osal.h"
#include "hci_data.h"
#include "hci_event.h"

/// @cond NODOC
extern uint8 hciTaskID;
extern uint8 hciTestTaskID;
extern uint8 hciGapTaskID;
extern uint8 hciL2capTaskID;
extern uint8 hciSmpTaskID;
/// @endcond //NODOC

/*******************************************************************************
 * MACROS
 */

/// @brief HCI Assert
#define HCI_ASSERT(condition)  HAL_ASSERT(condition)

/** @} */ // end of HCI

/*******************************************************************************
 * CONSTANTS
 */

/**
 *  @addtogroup HCI_Constants
 *  @{
*/

// OSAL Task Events
#define HCI_TX_PROCESS_EVENT                              0x0001		//!< TX Process Event
#define HCI_TEST_UART_SEND_EVENT                          0x0002		//!< Test UART Send Event
#define HCI_ADV_NOTICE_EVENT                              0x0004		//!< Scan Notice Event
#define HCI_SCAN_NOTICE_EVENT                             0x0008		//!< Scan Notice Event
#define HCI_CONN_NOTICE_EVENT                             0x0010		//!< Connection Notice Event
#define HCI_BDADDR_UPDATED_EVENT                          0x4000		//!< BD Address Updated Event

// OSAL Message Header Events
#define HCI_CTRL_TO_HOST_EVENT                            0x01			//!< Controller to Host Event
#define HCI_HOST_TO_CTRL_CMD_EVENT                        0x02			//!< Host to Controller Command
#define HCI_HOST_TO_CTRL_DATA_EVENT                       0x03			//!< Host to Controller Data

#define HCI_BDADDR_LEN                                    6				//!< BD Address Length

// Max Allowed HCI Packet
#define HCI_MAX_CMD_PKT_SIZE                              0xFF			//!< Max Command Packet Size
#define HCI_MAX_DATA_PKT_SIZE                             0xFFFF		//!< Max Data Pakcet Size

// Max Data Length in Packet
#define HCI_DATA_MAX_DATA_LENGTH                          27			//!< Max Data Length

/**
 * @brief Minimum length for command packet
 *
 * Minimum length for CMD packet is 1+2+1
 * | Packet Type (1) | OPCode(2) | Length(1) |
 */
#define HCI_CMD_MIN_LENGTH                                4

/**
 * @brief Minimum length for event packet
 *
 * Minimum length for EVENT packet is 1+1+1
 * | Packet Type (1) | Event Code(1) | Length(1) |
 */
#define HCI_EVENT_MIN_LENGTH                              3


/**
 * @brief Minimum length for data packet
 *
 * Minimum length for DATA packet is 1+2+2
 * | Packet Type (1) | Handler(2) | Length(2) |
 */
#define HCI_DATA_MIN_LENGTH                               5

#define HCI_MAX_NUM_CONNECTIONS                           0x03		//!< Max Number of Connections

#define HCI_TX_DATA_ANY_CONNECTION                        0xFF		//!< TX Data Any Connection

// HCI Packet Types
#define HCI_CMD_PACKET                                    0x01		//!< Command Packet
#define HCI_ACL_DATA_PACKET                               0x02		//!< ACL Data Packet
#define HCI_SCO_DATA_PACKET                               0x03		//!< SCO Data Packet
#define HCI_EVENT_PACKET                                  0x04		//!< Event Packet


// States for Command and Data packet parser
#define HCI_PARSER_STATE_PKT_TYPE                         0			//!< Packet Type Parser State
#define HCI_CMD_PARSER_STATE_OPCODE                       1			//!< Command Opcode Parser State
#define HCI_CMD_PARSER_STATE_LENGTH                       2			//!< Command Length Parser State
#define HCI_CMD_PARSER_STATE_DATA                         3			//!< Command Data Parser State
#define HCI_DATA_PARSER_STATE_HANDLE                      4			//!< Data Handle Parser State
#define HCI_DATA_PARSER_STATE_LENGTH                      5			//!< Data Parser State
#define HCI_DATA_PARSER_STATE_DATA                        6			//!< Data Data Parser State

// HCI Command Subgroup
#define HCI_OPCODE_CSG_LINK_LAYER                         0			//!< Link Layer Command Subgroup
#define HCI_OPCODE_CSG_CSG_L2CAP                          1			//!< L2CAP Command Subgroup
#define HCI_OPCODE_CSG_CSG_ATT                            2			//!< ATT Command Subgroup
#define HCI_OPCODE_CSG_CSG_GATT                           3			//!< GATT Command Subgroup
#define HCI_OPCODE_CSG_CSG_GAP                            4			//!< GAP Command Subgroup
#define HCI_OPCODE_CSG_CSG_SM                             5			//!< SM Command Subgroup
#define HCI_OPCODE_CSG_CSG_Reserved                       6			//!< Reserved
#define HCI_OPCODE_CSG_CSG_USER_PROFILE                   7			//!< User Profile Command Subgroup

// Vendor Specific OGF
#define VENDOR_SPECIFIC_OGF                               0x3F		//!< Vendor Specific

/*
** HCI Command Opcodes
*/

// Link Control Commands
#define HCI_DISCONNECT                                    0x0406	//!< opcode of @ref HCI_DisconnectCmd
#define HCI_READ_REMOTE_VERSION_INFO                      0x041D	//!< opcode of @ref HCI_ReadRemoteVersionInfoCmd

// Controller and Baseband Commands
#define HCI_SET_EVENT_MASK                                0x0C01	//!< opcode of @ref HCI_SetEventMaskCmd
#define HCI_RESET                                         0x0C03	//!< opcode of@ref HCI_ResetCmd
#define HCI_READ_TRANSMIT_POWER                           0x0C2D	//!< opcode of@ref HCI_ReadTransmitPowerLevelCmd
/// @cond NODOC
#define HCI_SET_CONTROLLER_TO_HOST_FLOW_CONTROL           0x0C31	//!< opcode of @ref HCI_SetControllerToHostFlowCtrlCmd
#define HCI_HOST_BUFFER_SIZE                              0x0C33	//!< opcode of @ref HCI_HostBufferSizeCmd
/// @endcond //NODOC
#define HCI_HOST_NUM_COMPLETED_PACKETS                    0x0C35	//!< opcode of @ref HCI_HostNumCompletedPktCmd
#define HCI_SET_EVENT_MASK_PAGE_2                         0x0C63	//!< opcode of @ref HCI_SetEventMaskPage2Cmd
#define HCI_READ_AUTH_PAYLOAD_TIMEOUT                     0x0C7B	//!< opcode of @ref HCI_ReadAuthPayloadTimeoutCmd
#define HCI_WRITE_AUTH_PAYLOAD_TIMEOUT                    0x0C7C	//!< opcode of @ref HCI_WriteAuthPayloadTimeoutCmd

// Information Parameters
#define HCI_READ_LOCAL_VERSION_INFO                       0x1001	//!< opcode of @ref HCI_ReadLocalVersionInfoCmd
#define HCI_READ_LOCAL_SUPPORTED_COMMANDS                 0x1002	//!< opcode of @ref HCI_ReadLocalSupportedCommandsCmd
#define HCI_READ_LOCAL_SUPPORTED_FEATURES                 0x1003	//!< opcode of @ref HCI_ReadLocalSupportedFeaturesCmd
#define HCI_READ_BDADDR                                   0x1009	//!< opcode of @ref HCI_ReadBDADDRCmd

// Status Parameters
#define HCI_READ_RSSI                                     0x1405	//!< opcode of @ref HCI_ReadRssiCmd

// LE Commands
// V4.0
#define HCI_LE_SET_EVENT_MASK                             0x2001	//!< opcode of @ref HCI_LE_SetEventMaskCmd
#define HCI_LE_READ_BUFFER_SIZE                           0x2002	//!< opcode of @ref HCI_LE_ReadBufSizeCmd
#define HCI_LE_READ_LOCAL_SUPPORTED_FEATURES              0x2003	//!< opcode of @ref HCI_LE_ReadLocalSupportedFeaturesCmd
#define HCI_LE_SET_RANDOM_ADDR                            0x2005	//!< opcode of @ref HCI_LE_SetRandomAddressCmd
#define HCI_LE_SET_ADV_PARAM                              0x2006	//!< opcode of @ref HCI_LE_SetAdvParamCmd
#define HCI_LE_READ_ADV_CHANNEL_TX_POWER                  0x2007	//!< opcode of @ref HCI_LE_ReadAdvChanTxPowerCmd
#define HCI_LE_SET_ADV_DATA                               0x2008	//!< opcode of @ref HCI_LE_SetAdvDataCmd
#define HCI_LE_SET_SCAN_RSP_DATA                          0x2009	//!< opcode of @ref HCI_LE_SetScanRspDataCmd
#define HCI_LE_SET_ADV_ENABLE                             0x200A	//!< opcode of @ref HCI_LE_SetAdvEnableCmd
#define HCI_LE_SET_SCAN_PARAM                             0x200B	//!< opcode of @ref HCI_LE_SetScanParamCmd
#define HCI_LE_SET_SCAN_ENABLE                            0x200C	//!< opcode of @ref HCI_LE_SetScanEnableCmd
#define HCI_LE_CREATE_CONNECTION                          0x200D	//!< opcode of @ref HCI_LE_CreateConnCmd
#define HCI_LE_CREATE_CONNECTION_CANCEL                   0x200E	//!< opcode of @ref HCI_LE_CreateConnCancelCmd
#define HCI_LE_READ_WHITE_LIST_SIZE                       0x200F	//!< opcode of @ref HCI_LE_ReadWhiteListSizeCmd
#define HCI_LE_CLEAR_WHITE_LIST                           0x2010	//!< opcode of @ref HCI_LE_ClearWhiteListCmd
#define HCI_LE_ADD_WHITE_LIST                             0x2011	//!< opcode of @ref HCI_LE_AddWhiteListCmd
#define HCI_LE_REMOVE_WHITE_LIST                          0x2012	//!< opcode of @ref HCI_LE_RemoveWhiteListCmd
#define HCI_LE_CONNECTION_UPDATE                          0x2013	//!< opcode of @ref HCI_LE_ConnUpdateCmd
#define HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION            0x2014	//!< opcode of @ref HCI_LE_SetHostChanClassificationCmd
#define HCI_LE_READ_CHANNEL_MAP                           0x2015	//!< opcode of @ref HCI_LE_ReadChannelMapCmd
#define HCI_LE_READ_REMOTE_USED_FEATURES                  0x2016	//!< opcode of @ref HCI_LE_ReadRemoteUsedFeaturesCmd
#define HCI_LE_ENCRYPT                                    0x2017	//!< opcode of @ref HCI_LE_EncryptCmd
#define HCI_LE_RAND                                       0x2018	//!< opcode of @ref HCI_LE_RandCmd
#define HCI_LE_START_ENCRYPTION                           0x2019	//!< opcode of @ref HCI_LE_StartEncyptCmd
#define HCI_LE_LTK_REQ_REPLY                              0x201A	//!< opcode of @ref HCI_LE_LtkReqReplyCmd
#define HCI_LE_LTK_REQ_NEG_REPLY                          0x201B	//!< opcode of @ref HCI_LE_LtkReqNegReplyCmd
#define HCI_LE_READ_SUPPORTED_STATES                      0x201C	//!< opcode of @ref HCI_LE_ReadSupportedStatesCmd
#define HCI_LE_RECEIVER_TEST                              0x201D	//!< opcode of @ref HCI_LE_ReceiverTestCmd
#define HCI_LE_TRANSMITTER_TEST                           0x201E	//!< opcode of @ref HCI_LE_TransmitterTestCmd
#define HCI_LE_TEST_END                                   0x201F	//!< opcode of @ref HCI_LE_TestEndCmd
// V4.1
#define HCI_LE_REMOTE_CONN_PARAM_REQ_REPLY                0x2020	//!< opcode of @ref HCI_LE_RemoteConnParamReqReplyCmd
#define HCI_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY            0x2021	//!< opcode of @ref HCI_LE_RemoteConnParamReqNegReplyCmd
// V4.2
#define HCI_LE_SET_DATA_LENGTH                            0x2022	//!< opcode of @ref HCI_LE_SetDataLenCmd
#define HCI_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH         0x2023	//!< opcode of @ref HCI_LE_ReadSuggestedDefaultDataLenCmd
#define HCI_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH        0x2024	//!< opcode of @ref HCI_LE_WriteSuggestedDefaultDataLenCmd
#define HCI_LE_READ_LOCAL_P256_PUBLIC_KEY                 0x2025	//!< opcode of @ref HCI_LE_ReadLocalP256PublicKeyCmd
#define HCI_LE_GENERATE_DHKEY                             0x2026	//!< opcode of @ref HCI_LE_GenerateDHKeyCmd
#define HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST               0x2027	//!< opcode of @ref HCI_LE_AddDeviceToResolvingListCmd
#define HCI_LE_REMOVE_DEVICE_FROM_RESOLVING_LIST          0x2028	//!< opcode of @ref HCI_LE_RemoveDeviceFromResolvingListCmd
#define HCI_LE_CLEAR_RESOLVING_LIST                       0x2029	//!< opcode of @ref HCI_LE_ClearResolvingListCmd
#define HCI_LE_READ_RESOLVING_LIST_SIZE                   0x202A	//!< opcode of @ref HCI_LE_ReadResolvingListSizeCmd
#define HCI_LE_READ_PEER_RESOLVABLE_ADDRESS               0x202B	//!< opcode of @ref HCI_LE_ReadPeerResolvableAddressCmd
#define HCI_LE_READ_LOCAL_RESOLVABLE_ADDRESS              0x202C	//!< opcode of @ref HCI_LE_ReadLocalResolvableAddressCmd
#define HCI_LE_SET_ADDRESS_RESOLUTION_ENABLE              0x202D	//!< opcode of @ref HCI_LE_SetAddressResolutionEnableCmd
#define HCI_LE_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT     0x202E	//!< opcode of @ref HCI_LE_SetResolvablePrivateAddressTimeoutCmd
#define HCI_LE_READ_MAX_DATA_LENGTH                       0x202F	//!< opcode of @ref HCI_LE_ReadMaxDataLenCmd

/// @cond BLE_5
// V5.0
#define HCI_LE_READ_PHY                                   0x2030	//!< opcode of @ref HCI_LE_ReadPhyCmd
#define HCI_LE_SET_DEFAULT_PHY                            0x2031	//!< opcode of @ref HCI_LE_SetDefaultPhyCmd
#define HCI_LE_SET_PHY                                    0x2032	//!< opcode of @ref HCI_LE_SetPhyCmd
#define HCI_LE_ENHANCED_RECEIVER_TEST                     0x2033	//!< opcode of @ref HCI_LE_EnhancedRxTestCmd
#define HCI_LE_ENHANCED_TRANSMITTER_TEST                  0x2034	//!< opcode of @ref HCI_LE_EnhancedTxTestCmd
/// @endcond //BLE_5
// LE Vendor Specific LL Extension Commands
#define HCI_EXT_SET_RX_GAIN                               0xFC00	//!< opcode of @ref HCI_EXT_SetRxGainCmd
#define HCI_EXT_SET_TX_POWER                              0xFC01	//!< opcode of @ref HCI_EXT_SetTxPowerCmd
#define HCI_EXT_ONE_PKT_PER_EVT                           0xFC02	//!< opcode of @ref HCI_EXT_OnePktPerEvtCmd
/// @cond CC254X
#define HCI_EXT_CLK_DIVIDE_ON_HALT                        0xFC03	//!< opcode of @ref HCI_EXT_ClkDivOnHaltCmd
#define HCI_EXT_DECLARE_NV_USAGE                          0xFC04	//!< opcode of @ref HCI_EXT_DeclareNvUsageCmd
/// @endcond // CC254X
#define HCI_EXT_DECRYPT                                   0xFC05	//!< opcode of @ref HCI_EXT_DecryptCmd
#define HCI_EXT_SET_LOCAL_SUPPORTED_FEATURES              0xFC06	//!< opcode of @ref HCI_EXT_SetLocalSupportedFeaturesCmd
#define HCI_EXT_SET_FAST_TX_RESP_TIME                     0xFC07	//!< opcode of @ref HCI_EXT_SetFastTxResponseTimeCmd
#define HCI_EXT_MODEM_TEST_TX                             0xFC08	//!< opcode of @ref HCI_EXT_ModemTestTxCmd
#define HCI_EXT_MODEM_HOP_TEST_TX                         0xFC09	//!< opcode of @ref HCI_EXT_ModemHopTestTxCmd
#define HCI_EXT_MODEM_TEST_RX                             0xFC0A	//!< opcode of @ref HCI_EXT_ModemTestRxCmd
#define HCI_EXT_END_MODEM_TEST                            0xFC0B	//!< opcode of @ref HCI_EXT_EndModemTestCmd
#define HCI_EXT_SET_BDADDR                                0xFC0C	//!< opcode of @ref HCI_EXT_SetBDADDRCmd
#define HCI_EXT_SET_SCA                                   0xFC0D	//!< opcode of @ref HCI_EXT_SetSCACmd
#define HCI_EXT_ENABLE_PTM                                0xFC0E	//!< opcode of @ref HCI_EXT_EnablePTMCmd
#define HCI_EXT_SET_FREQ_TUNE                             0xFC0F	//!< opcode of @ref HCI_EXT_SetFreqTuneCmd
#define HCI_EXT_SAVE_FREQ_TUNE                            0xFC10	//!< opcode of @ref HCI_EXT_SaveFreqTuneCmd
#define HCI_EXT_SET_MAX_DTM_TX_POWER                      0xFC11	//!< opcode of @ref HCI_EXT_SetMaxDtmTxPowerCmd
/// @cond CC254X
#define HCI_EXT_MAP_PM_IO_PORT                            0xFC12	//!< opcode of @ref HCI_EXT_MapPmIoPortCmd
/// @endcond //CC254X
#define HCI_EXT_DISCONNECT_IMMED                          0xFC13	//!< opcode of @ref HCI_EXT_DisconnectImmedCmd
#define HCI_EXT_PER                                       0xFC14	//!< opcode of @ref HCI_EXT_PacketErrorRateCmd
#define HCI_EXT_PER_BY_CHAN                               0xFC15	//!< opcode of @ref HCI_EXT_PERbyChanCmd
/// @cond CC254X
#define HCI_EXT_EXTEND_RF_RANGE                           0xFC16	//!< opcode of  @ref HCI_EXT_ExtendRfRangeCmd
/// @endcond // CC254X
#define HCI_EXT_ADV_EVENT_NOTICE                          0xFC17	//!< opcode of @ref HCI_EXT_AdvEventNoticeCmd
#define HCI_EXT_CONN_EVENT_NOTICE                         0xFC18	//!< opcode of @ref HCI_EXT_ConnEventNoticeCmd
/// @cond CC254X
#define HCI_EXT_HALT_DURING_RF                            0xFC19	//!< opcode of @ref HCI_EXT_HaltDuringRfCmd
#define HCI_EXT_OVERRIDE_SL                               0xFC1A	//!< opcode of @ref HCI_EXT_SetSlaveLatencyOverrideCmd
/// @endcond // CC254X
#define HCI_EXT_BUILD_REVISION                            0xFC1B	//!< opcode of @ref HCI_EXT_BuildRevisionCmd
/// @cond CC254X
#define HCI_EXT_DELAY_SLEEP                               0xFC1C	//!< opcode of @ref HCI_EXT_DelaySleepCmd
/// @endcond //CC254X
#define HCI_EXT_RESET_SYSTEM                              0xFC1D	//!< opcode of @ref HCI_EXT_ResetSystemCmd
/// @cond CC254X
#define HCI_EXT_OVERLAPPED_PROCESSING                     0xFC1E	//!< opcode of @ref HCI_EXT_OverlappedProcessingCmd
/// @endcond //CC254X
#define HCI_EXT_NUM_COMPLETED_PKTS_LIMIT                  0xFC1F	//!< opcode of @ref HCI_EXT_NumComplPktsLimitCmd
#define HCI_EXT_GET_CONNECTION_INFO                       0xFC20	//!< opcode of @ref HCI_EXT_GetConnInfoCmd
#define HCI_EXT_SET_MAX_DATA_LENGTH                       0xFC21	//!< opcode of @ref HCI_EXT_SetMaxDataLenCmd
#define HCI_EXT_SCAN_EVENT_NOTICE                         0xFC22	//!< opcode of @ref HCI_EXT_ScanEventNoticeCmd
#define HCI_EXT_SCAN_REQ_REPORT                           0xFC23	//!< opcode of @ref HCI_EXT_ScanReqRptCmd
#define HCI_EXT_GET_ACTIVE_CONNECTION_INFO                0xFC24	//!< opcode of @ref HCI_EXT_GetActiveConnInfoCmd

/// @cond NODOC
#define HCI_EXT_LL_TEST_MODE                              0xFC70	//!< opcode of @ref HCI_EXT_LLTestModeCmd
/// @endcond //NODOC

/*
** HCI Event Codes
*/

// BT Events
#define HCI_DISCONNECTION_COMPLETE_EVENT_CODE             0x05		//!< event of type @ref hciEvt_DisconnComplete_t
#define HCI_ENCRYPTION_CHANGE_EVENT_CODE                  0x08		//!< event of type @ref hciEvt_EncryptChange_t
#define HCI_READ_REMOTE_INFO_COMPLETE_EVENT_CODE          0x0C		//!< event of type @ref hciPacket_t
#define HCI_COMMAND_COMPLETE_EVENT_CODE                   0x0E		//!< event of type @ref hciEvt_CmdComplete_t
#define HCI_COMMAND_STATUS_EVENT_CODE                     0x0F		//!< event of type @ref hciEvt_CommandStatus_t
#define HCI_BLE_HARDWARE_ERROR_EVENT_CODE                 0x10		//!< event of type @ref hciEvt_HardwareError_t
#define HCI_NUM_OF_COMPLETED_PACKETS_EVENT_CODE           0x13		//!< event of type @ref hciEvt_NumCompletedPkt_t
#define HCI_DATA_BUFFER_OVERFLOW_EVENT                    0x1A		//!< event of type @ref hciEvt_BufferOverflow_t
#define HCI_KEY_REFRESH_COMPLETE_EVENT_CODE               0x30		//!< event of type @ref hciPacket_t
#define HCI_APTO_EXPIRED_EVENT_CODE                       0x57		//!< event of type @ref hciEvt_AptoExpired_t

// LE Event Code (for LE Meta Events)
#define HCI_LE_EVENT_CODE                                 0x3E		//!< LE Event

// LE Meta Event Codes
#define HCI_BLE_CONNECTION_COMPLETE_EVENT                 0x01		//!< event of type @ref hciEvt_BLEConnComplete_t
#define HCI_BLE_ADV_REPORT_EVENT                          0x02		//!< event of type @ref hciEvt_DevInfo_t
#define HCI_BLE_CONN_UPDATE_COMPLETE_EVENT                0x03		//!< event of type @ref hciEvt_BLEConnUpdateComplete_t
#define HCI_BLE_READ_REMOTE_FEATURE_COMPLETE_EVENT        0x04		//!< event of type @ref hciEvt_BLEReadRemoteFeatureComplete_t
#define HCI_BLE_LTK_REQUESTED_EVENT                       0x05		//!< event of type @ref hciEvt_BLELTKReq_t
// V4.1
#define HCI_BLE_REMOTE_CONN_PARAM_REQUEST_EVENT           0x06		//!< event of type @ref hciEvt_BLERemoteConnParamReq_t
// V4.2
#define HCI_BLE_DATA_LENGTH_CHANGE_EVENT                  0x07		//!< event of type @ref hciEvt_BLEDataLengthChange_t
#define HCI_BLE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_EVENT 0x08		//!< event of type @ref hciEvt_BLEReadP256PublicKeyComplete_t
#define HCI_BLE_GENERATE_DHKEY_COMPLETE_EVENT             0x09		//!< event of type @ref hciEvt_BLEGenDHKeyComplete_t
#define HCI_BLE_ENHANCED_CONNECTION_COMPLETE_EVENT        0x0A		//!< event of type @ref hciEvt_BLEEnhConnComplete_t
#define HCI_BLE_DIRECT_ADVERTISING_REPORT_EVENT           0x0B		//!< direct advertising report...not used
// V5.0
#define HCI_BLE_PHY_UPDATE_COMPLETE_EVENT                 0x0C		//!< event of type @ref hciEvt_BLEPhyUpdateComplete_t
// VS Meta Event Codes - Texas Instruments Inc specific!
#define HCI_BLE_SCAN_REQ_REPORT_EVENT                     0x80		//!< event of type @ref hciEvt_BLEScanReqReport_t

#define HCI_VE_EVENT_CODE                                 0xFF		//!< event of type @ref hciEvt_VSCmdComplete_t

// LE Vendor Specific LL Extension Events
#define HCI_EXT_SET_RX_GAIN_EVENT                         0x0400	//!< event from @ref HCI_EXT_SetRxGainCmd
#define HCI_EXT_SET_TX_POWER_EVENT                        0x0401	//!< event from @ref HCI_EXT_SetTxPowerCmd
#define HCI_EXT_ONE_PKT_PER_EVT_EVENT                     0x0402	//!< event from @ref HCI_EXT_OnePktPerEvtCmd
/// @cond CC254X
#define HCI_EXT_CLK_DIVIDE_ON_HALT_EVENT                  0x0403	//!< event from @ref HCI_EXT_ClkDivOnHaltCmd
#define HCI_EXT_DECLARE_NV_USAGE_EVENT                    0x0404	//!< event from @ref HCI_EXT_DeclareNvUsageCmd
/// @endcond //CC254X
#define HCI_EXT_DECRYPT_EVENT                             0x0405	//!< event from @ref HCI_EXT_DecryptCmd
#define HCI_EXT_SET_LOCAL_SUPPORTED_FEATURES_EVENT        0x0406	//!< event from @ref HCI_EXT_SetLocalSupportedFeaturesCmd
#define HCI_EXT_SET_FAST_TX_RESP_TIME_EVENT               0x0407	//!< event from @ref HCI_EXT_SetFastTxResponseTimeCmd
#define HCI_EXT_MODEM_TEST_TX_EVENT                       0x0408	//!< event from @ref HCI_EXT_ModemTestTxCmd
#define HCI_EXT_MODEM_HOP_TEST_TX_EVENT                   0x0409	//!< event from @ref HCI_EXT_ModemHopTestTxCmd
#define HCI_EXT_MODEM_TEST_RX_EVENT                       0x040A	//!< event from @ref HCI_EXT_ModemTestRxCmd
#define HCI_EXT_END_MODEM_TEST_EVENT                      0x040B	//!< event from @ref HCI_EXT_EndModemTestCmd
#define HCI_EXT_SET_BDADDR_EVENT                          0x040C	//!< event from @ref HCI_EXT_SetBDADDRCmd
#define HCI_EXT_SET_SCA_EVENT                             0x040D	//!< event from @ref HCI_EXT_SetSCACmd
#define HCI_EXT_ENABLE_PTM_EVENT                          0x040E	//!< event from @ref HCI_EXT_EnablePTMCmd
#define HCI_EXT_SET_FREQ_TUNE_EVENT                       0x040F	//!< event from @ref HCI_EXT_SetFreqTuneCmd
#define HCI_EXT_SAVE_FREQ_TUNE_EVENT                      0x0410	//!< event from @ref HCI_EXT_SaveFreqTuneCmd
#define HCI_EXT_SET_MAX_DTM_TX_POWER_EVENT                0x0411	//!< event from @ref HCI_EXT_SetMaxDtmTxPowerCmd
/// @cond CC254X
#define HCI_EXT_MAP_PM_IO_PORT_EVENT                      0x0412	//!< event from @ref HCI_EXT_MapPmIoPortCmd
/// @endcond //CC254X
#define HCI_EXT_DISCONNECT_IMMED_EVENT                    0x0413	//!< event from @ref HCI_EXT_DisconnectImmedCmd
#define HCI_EXT_PER_EVENT                                 0x0414	//!< event from @ref HCI_EXT_PacketErrorRateCmd
#define HCI_EXT_PER_BY_CHAN_EVENT                         0x0415	//!< event from @ref HCI_EXT_PERbyChanCmd
/// @cond CC254X
#define HCI_EXT_EXTEND_RF_RANGE_EVENT                     0x0416	//!< event from @ref HCI_EXT_ExtendRfRangeCmd
/// @endcond //CC254X
#define HCI_EXT_ADV_EVENT_NOTICE_EVENT                    0x0417	//!< event from @ref HCI_EXT_AdvEventNoticeCmd
#define HCI_EXT_CONN_EVENT_NOTICE_EVENT                   0x0418	//!< event from @ref HCI_EXT_ConnEventNoticeCmd
/// @cond CC254X
#define HCI_EXT_HALT_DURING_RF_EVENT                      0x0419	//!< event from @ref HCI_EXT_HaltDuringRfCmd
#define HCI_EXT_OVERRIDE_SL_EVENT                         0x041A	//!< event from @ref HCI_EXT_SetSlaveLatencyOverrideCmd
/// @endcond //CC254X
#define HCI_EXT_BUILD_REVISION_EVENT                      0x041B	//!< event from @ref HCI_EXT_BuildRevisionCmd
/// @cond CC254X
#define HCI_EXT_DELAY_SLEEP_EVENT                         0x041C	//!< event from @ref HCI_EXT_DelaySleepCmd
/// @endcond //CC254X
#define HCI_EXT_RESET_SYSTEM_EVENT                        0x041D	//!< event from @ref HCI_EXT_ResetSystemCmd
/// @cond CC254X
#define HCI_EXT_OVERLAPPED_PROCESSING_EVENT               0x041E	//!< event from @ref HCI_EXT_OverlappedProcessingCmd
/// @endcond //CC254X
#define HCI_EXT_NUM_COMPLETED_PKTS_LIMIT_EVENT            0x041F	//!< event from @ref HCI_EXT_NumComplPktsLimitCmd
#define HCI_EXT_GET_CONNECTION_INFO_EVENT                 0x0420	//!< event from @ref HCI_EXT_GetConnInfoCmd
#define HCI_EXT_SET_MAX_DATA_LENGTH_EVENT                 0x0421	//!< event from @ref HCI_EXT_SetMaxDataLenCmd
#define HCI_EXT_SCAN_EVENT_NOTICE_EVENT                   0x0422	//!< event from @ref HCI_EXT_ScanEventNoticeCmd
#define HCI_EXT_SCAN_REQ_REPORT_EVENT                     0x0423	//!< event from @ref HCI_EXT_ScanReqRptCmd
#define HCI_EXT_GET_ACTIVE_CONNECTION_INFO_EVENT          0x0424	//!< event from @ref HCI_EXT_GetActiveConnInfoCmd
/// @cond NODOC
#define HCI_EXT_LL_TEST_MODE_EVENT                        0x0470	//!< LL Test Mode
/// @endcond // NODOC


/** @} */ // end of HCI_Constants

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*
** HCI OSAL API
*/

/// @cond NODOC

/**
 * @fn          HCI_Init
 *
 * @brief       This is the HCI OSAL task initialization routine.
 *
 * @param       taskID The HCI OSAL task identifier.
 */
extern void HCI_Init( uint8 taskID );


/**
 * @fn          HCI_TL_getCmdResponderID
 *
 * @brief       Used by application to intercept and handle HCI TL events and
 *              messages
 *
 * @param       taskID The HCI OSAL task identifier.
 */
extern void HCI_TL_getCmdResponderID( uint8 taskID );


/**
 * @fn          HCI_ProcessEvent
 *
 * @brief       This is the HCI OSAL task process event handler.
 *
 * @param       taskID The HCI OSAL task identifier.
 * @param       events HCI OSAL task events.
 *
 * @return      Unprocessed events.
 */
extern uint16 HCI_ProcessEvent( uint8  task_id,
                                uint16 events );


/// @endcond //NODOC

#ifdef __cplusplus
}
#endif

#endif /* HCI_TL_H */

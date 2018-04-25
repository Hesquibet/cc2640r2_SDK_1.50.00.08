/******************************************************************************

 @file       hci_event.c

 @brief This file send HCI events for the controller. It implements all the
        LL event callback and HCI events send.

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

/*******************************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "hci_event.h"

#if defined( CC26XX ) || defined( CC13XX )
#include "rom_jt.h"
#endif // CC26XX/CC13XX

#include "rom_jt.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

/*
** Bluetooth Event Mask
** Core Specification, Vol. 2, Part E, Section 7.3.1
*/

// Byte 0
#define BT_EVT_MASK_INQUIRY_COMPLETE                                   0x01
#define BT_EVT_MASK_INQUIRY_RESULT                                     0x02
#define BT_EVT_MASK_CONNECTION_COMPLETE                                0x04
#define BT_EVT_MASK_CONNECTION_REQUEST                                 0x08
#define BT_EVT_MASK_DISCONNECTION_COMPLETE                             0x10
#define BT_EVT_MASK_AUTHENTICATION_COMPLETE                            0x20
#define BT_EVT_MASK_REMOTE_NAME_REQUEST_COMPLETE                       0x40
#define BT_EVT_MASK_ENCRYPTION_CHANGE                                  0x80
// Byte 1
#define BT_EVT_MASK_CHANGE_CONNECTION_LINK_KEY_COMPLETE                0x01
#define BT_EVT_MASK_MASTER_LINK_KEY_COMPLETE                           0x02
#define BT_EVT_MASK_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE            0x04
#define BT_EVT_MASK_READ_REMOTE_VERSION_INFORMATION_COMPLETE           0x08
#define BT_EVT_MASK_QOS_SETUP_COMPLETE                                 0x10
#define BT_EVT_MASK_RESERVED01                                         0x20
#define BT_EVT_MASK_RESERVED02                                         0x40
#define BT_EVT_MASK_HARDWARE_ERROR                                     0x80
// Byte 2
#define BT_EVT_MASK_FLUSH_OCCURRED                                     0x01
#define BT_EVT_MASK_ROLE_CHANGE                                        0x02
#define BT_EVT_MASK_RESERVED03                                         0x04
#define BT_EVT_MASK_MODE_CHANGE                                        0x08
#define BT_EVT_MASK_RETURN_LINK_KEYS                                   0x10
#define BT_EVT_MASK_PIN_CODE_REQUEST                                   0x20
#define BT_EVT_MASK_LINK_KEY_REQUEST                                   0x40
#define BT_EVT_MASK_LINK_KEY_NOTIFICATION                              0x80
// Byte 3
#define BT_EVT_MASK_LOOPBACK_COMMAND                                   0x01
#define BT_EVT_MASK_DATA_BUFFER_OVERFLOW                               0x02
#define BT_EVT_MASK_MAX_SLOTS_CHANGE                                   0x04
#define BT_EVT_MASK_READ_CLOCK_OFFSET_COMPLETE                         0x08
#define BT_EVT_MASK_CONNECTION_PACKET_TYPE_CHANGED                     0x10
#define BT_EVT_MASK_QOS_VIOLATION                                      0x20
#define BT_EVT_MASK_PAGE_SCAN_MODE_CHANGE                              0x40 // deprecated
#define BT_EVT_MASK_PAGE_SCAN_REPETITION_MODE_CHANGE                   0x80
// Byte 4
#define BT_EVT_MASK_FLOW_SPECIFICATION_COMPLETE                        0x01
#define BT_EVT_MASK_INQUIRY_RESULT_WITH_RSSI                           0x02
#define BT_EVT_MASK_READ_REMOTE_EXTENDED_FEATURES_COMPLETE             0x04
#define BT_EVT_MASK_RESERVED04                                         0x08
#define BT_EVT_MASK_RESERVED05                                         0x10
#define BT_EVT_MASK_RESERVED06                                         0x20
#define BT_EVT_MASK_RESERVED07                                         0x40
#define BT_EVT_MASK_RESERVED08                                         0x80
// Byte 5
#define BT_EVT_MASK_RESERVED09                                         0x01
#define BT_EVT_MASK_RESERVED10                                         0x02
#define BT_EVT_MASK_RESERVED11                                         0x04
#define BT_EVT_MASK_SYNCHRONOUS_CONNECTION_COMPLETE                    0x08
#define BT_EVT_MASK_SYNCHRONOUS_CONNECTION_CHANGED                     0x10
#define BT_EVT_MASK_SNIFF_SUBRATING                                    0x20
#define BT_EVT_MASK_EXTENDED_INQUIRY_RESULT                            0x40
#define BT_EVT_MASK_ENCRYPTION_KEY_REFRESH_COMPLETE                    0x80
// Byte 6
#define BT_EVT_MASK_IO_CAPABILITY_REQUEST                              0x01
#define BT_EVT_MASK_IO_CAPABILITY_REQUEST_REPLY                        0x02
#define BT_EVT_MASK_USER_CONFIRMATION_REQUEST                          0x04
#define BT_EVT_MASK_USER_PASSKEY_REQUEST                               0x08
#define BT_EVT_MASK_REMOTE_OOB_DATA_REQUEST                            0x10
#define BT_EVT_MASK_SIMPLE_PAIRING_COMPLETE                            0x20
#define BT_EVT_MASK_RESERVED12                                         0x40
#define BT_EVT_MASK_LINK_SUPERVISION_TIMEOUT_CHANGED                   0x80
// Byte 7
#define BT_EVT_MASK_ENHANCED_FLUSH_COMPLETE                            0x01
#define BT_EVT_MASK_RESERVED13                                         0x02
#define BT_EVT_MASK_USER_PASSKEY_NOTIFICATION                          0x04
#define BT_EVT_MASK_KEYPRESS_NOTIFICATION                              0x08
#define BT_EVT_MASK_REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION        0x10
#define BT_EVT_MASK_LE_META_EVENT                                      0x20
#define BT_EVT_MASK_RESERVED14                                         0x40
#define BT_EVT_MASK_RESERVED15                                         0x80

// No Bluetooth Event Mask in Byte
#define BT_EVT_MASK_NONE                                               0x00

// Bluetooth Event Mask Index
#define BT_EVT_INDEX_DISCONNECT_COMPLETE                               0
#define BT_EVT_INDEX_ENCRYPTION_CHANGE                                 0
#define BT_EVT_INDEX_READ_REMOTE_VERSION_INFO                          1
#define BT_EVT_INDEX_HARDWARE_ERROR                                    1
#define BT_EVT_INDEX_FLUSH_OCCURRED                                    2
#define BT_EVT_INDEX_BUFFER_OVERFLOW                                   3
#define BT_EVT_INDEX_KEY_REFRESH_COMPLETE                              5
#define BT_EVT_INDEX_LE_META_EVENT                                     7

// Event Mask Default Values
#define BT_EVT_MASK_BYTE0   (BT_EVT_MASK_ENCRYPTION_CHANGE | BT_EVT_MASK_DISCONNECTION_COMPLETE)
#define BT_EVT_MASK_BYTE1   (BT_EVT_MASK_HARDWARE_ERROR | BT_EVT_MASK_READ_REMOTE_VERSION_INFORMATION_COMPLETE)
#define BT_EVT_MASK_BYTE2   (BT_EVT_MASK_FLUSH_OCCURRED)
#define BT_EVT_MASK_BYTE3   (BT_EVT_MASK_DATA_BUFFER_OVERFLOW)
#define BT_EVT_MASK_BYTE4   (BT_EVT_MASK_NONE)
#define BT_EVT_MASK_BYTE5   (BT_EVT_MASK_ENCRYPTION_KEY_REFRESH_COMPLETE)
#define BT_EVT_MASK_BYTE6   (BT_EVT_MASK_NONE)
#define BT_EVT_MASK_BYTE7   (BT_EVT_MASK_LE_META_EVENT)

/*
** Bluetooth Event Mask 2
** Core Specification, Vol. 2, Part E, Section 7.3.69
*/

// Byte 0
#define BT_EVT_MASK2_PHYSICAL_LINK_COMPLETE                            0x01
#define BT_EVT_MASK2_CHANNEL_SELECTED                                  0x02
#define BT_EVT_MASK2_DISCONNECTION_PHYSICAL_LINK                       0x04
#define BT_EVT_MASK2_PHYSICAL_LINK_LOW_EARLY_WARNING                   0x08
#define BT_EVT_MASK2_PHYSICAL_LINK_RECOVERY                            0x10
#define BT_EVT_MASK2_LOGICAL_LINK_COMPLETE                             0x20
#define BT_EVT_MASK2_DISCONNECTION_LOGICAL_LINK_COMPLETE               0x40
#define BT_EVT_MASK2_FLOW_SPEC_MODIFY_COMPLETE                         0x80
// Byte 1
#define BT_EVT_MASK2_NUMBER_OF_COMPLETE_DATA_BLOCKS                    0x01
#define BT_EVT_MASK2_AMP_START_TEST                                    0x02
#define BT_EVT_MASK2_AMP_TEST_END                                      0x04
#define BT_EVT_MASK2_AMP_RECIEVER_REPORT                               0x08
#define BT_EVT_MASK2_SHORT_RANGE_MODE_CHANGE_COMPLETE                  0x10
#define BT_EVT_MASK2_AMP_STATUS_CHANGE                                 0x20
#define BT_EVT_MASK2_TRIGGERED_CLOCK_CAPTURE                           0x40
#define BT_EVT_MASK2_SYNCHRONIZATION_TRAIN_COMPLETE                    0x80
// Byte 2
#define BT_EVT_MASK2_SYNCHRONIZATION_TRAIN_RECEIVED                    0x01
#define BT_EVT_MASK2_CONNECTIONLESS_SLAVE_BROADCAST_RECEIVE            0x02
#define BT_EVT_MASK2_CONNECTIONLESS_SLAVE_BROADCAST_TIMEOUT            0x04
#define BT_EVT_MASK2_TRUNCATED_PAGE_COMPLETE                           0x08
#define BT_EVT_MASK2_SLAVE_PAGE_RESPONSE_TIMEOUT                       0x10
#define BT_EVT_MASK2_CONNECTIONLESS_SLAVE_BROADCAST_CHANNEL_MAP_CHANGE 0x20
#define BT_EVT_MASK2_INQUIRY_RESPONSE_NOTIFICATION                     0x40
#define BT_EVT_MASK2_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED             0x80
// Byte 3 - Byte 7
#define BT_EVT_MASK2_RESERVED01                                        0x01
#define BT_EVT_MASK2_RESERVED02                                        0x02
#define BT_EVT_MASK2_RESERVED03                                        0x04
#define BT_EVT_MASK2_RESERVED04                                        0x08
#define BT_EVT_MASK2_RESERVED05                                        0x10
#define BT_EVT_MASK2_RESERVED06                                        0x20
#define BT_EVT_MASK2_RESERVED07                                        0x40
#define BT_EVT_MASK2_RESERVED08                                        0x80

// No Bluetooth Event Mask 2 in Byte
#define BT_EVT_MASK2_NONE                                              0x00

// Bluetooth Event Mask Page 2 Index
#define BT_EVT_INDEX2_APTO_EXPIRED                                     2

// Event Mask 2 Default Values
#define BT_EVT_MASK2_BYTE0  (BT_EVT_MASK2_NONE)
#define BT_EVT_MASK2_BYTE1  (BT_EVT_MASK2_NONE)
#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & PING_CFG)
#define BT_EVT_MASK2_BYTE2  (BT_EVT_MASK2_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED)
#else // !PING_CFG
#define BT_EVT_MASK2_BYTE2  (BT_EVT_MASK2_NONE)
#endif // PING_CFG
#define BT_EVT_MASK2_BYTE3  (BT_EVT_MASK2_NONE)
#define BT_EVT_MASK2_BYTE4  (BT_EVT_MASK2_NONE)
#define BT_EVT_MASK2_BYTE5  (BT_EVT_MASK2_NONE)
#define BT_EVT_MASK2_BYTE6  (BT_EVT_MASK2_NONE)
#define BT_EVT_MASK2_BYTE7  (BT_EVT_MASK2_NONE)

/*
** Bluetooth LE Event Mask
** Core Specification, Vol. 2, Part E, Section 7.8.1
*/

// Byte 0
#define LE_EVT_MASK_CONN_COMPLETE                                      0x01
#define LE_EVT_MASK_ADV_REPORT                                         0x02
#define LE_EVT_MASK_CONN_UPDATE_COMPLETE                               0x04
#define LE_EVT_MASK_READ_REMOTE_FEATURE                                0x08
#define LE_EVT_MASK_LTK_REQUEST                                        0x10
#define LE_EVT_MASK_REMOTE_CONN_PARAM_REQUEST                          0x20
#define LE_EVT_MASK_DATA_LENGTH_CHANGE                                 0x40
#define LE_EVT_MASK_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE                0x80
// Byte 1
#define LE_EVT_MASK_GENERATE_DHKEY_COMPLETE                            0x01
#define LE_EVT_MASK_ENH_CONN_COMPLETE                                  0x02
#define LE_EVT_MASK_DIRECT_ADVERTISING_REPORT                          0x04
#define LE_EVT_MASK_PHY_UPDATE_COMPLETE                                0x08
#define LE_EVT_MASK_RESERVED01                                         0x10
#define LE_EVT_MASK_RESERVED02                                         0x20
#define LE_EVT_MASK_RESERVED03                                         0x40
#define LE_EVT_MASK_RESERVED04                                         0x80
// Byte 2 - Byte 7
#define LE_EVT_MASK_RESERVED05                                         0x01
#define LE_EVT_MASK_RESERVED06                                         0x02
#define LE_EVT_MASK_RESERVED07                                         0x04
#define LE_EVT_MASK_RESERVED08                                         0x08
#define LE_EVT_MASK_RESERVED09                                         0x10
#define LE_EVT_MASK_RESERVED10                                         0x20
#define LE_EVT_MASK_RESERVED11                                         0x40
#define LE_EVT_MASK_RESERVED12                                         0x80

// No Bluetooth LE Event Mask in Byte
#define LE_EVT_MASK_NONE                                               0x00

// Bluetooth LE Event Mask Index
#define LE_EVT_INDEX_CONN_COMPLETE                                     0
#define LE_EVT_INDEX_ADV_REPORT                                        0
#define LE_EVT_INDEX_CONN_UPDATE_COMPLETE                              0
#define LE_EVT_INDEX_READ_REMOTE_FEATURE                               0
#define LE_EVT_INDEX_LTK_REQUEST                                       0
#define LE_EVT_INDEX_REMOTE_CONN_PARAM_REQUEST                         0
#define LE_EVT_INDEX_DATA_LENGTH_CHANGE                                0
#define LE_EVT_INDEX_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE               0
//
#define LE_EVT_INDEX_GENERATE_DHKEY_COMPLETE                           1
#define LE_EVT_INDEX_ENH_CONN_COMPLETE                                 1
#define LE_EVT_INDEX_DIRECT_ADVERTISING_REPORT                         1
#define LE_EVT_INDEX_PHY_UPDATE_COMPLETE                               1

// Bluetooth LE Event Mask Default Values
#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
  #if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & EXT_DATA_LEN_CFG)
    #if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & SECURE_CONNS_CFG)
      #define LE_EVT_MASK_BYTE0   (LE_EVT_MASK_CONN_COMPLETE             |     \
                                   LE_EVT_MASK_ADV_REPORT                |     \
                                   LE_EVT_MASK_CONN_UPDATE_COMPLETE      |     \
                                   LE_EVT_MASK_READ_REMOTE_FEATURE       |     \
                                   LE_EVT_MASK_LTK_REQUEST               |     \
                                   LE_EVT_MASK_REMOTE_CONN_PARAM_REQUEST |     \
                                   LE_EVT_MASK_DATA_LENGTH_CHANGE        |     \
                                   LE_EVT_MASK_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE)
    #else // !SECURE_CONNS_CFG
      #define LE_EVT_MASK_BYTE0   (LE_EVT_MASK_CONN_COMPLETE             |     \
                                   LE_EVT_MASK_ADV_REPORT                |     \
                                   LE_EVT_MASK_CONN_UPDATE_COMPLETE      |     \
                                   LE_EVT_MASK_READ_REMOTE_FEATURE       |     \
                                   LE_EVT_MASK_LTK_REQUEST               |     \
                                   LE_EVT_MASK_REMOTE_CONN_PARAM_REQUEST |     \
                                   LE_EVT_MASK_DATA_LENGTH_CHANGE)
    #endif // SECURE_CONNS_CFG
  #else // !EXT_DATA_LEN_CFG
    #if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & SECURE_CONNS_CFG)
      #define LE_EVT_MASK_BYTE0   (LE_EVT_MASK_CONN_COMPLETE             |     \
                                   LE_EVT_MASK_ADV_REPORT                |     \
                                   LE_EVT_MASK_CONN_UPDATE_COMPLETE      |     \
                                   LE_EVT_MASK_READ_REMOTE_FEATURE       |     \
                                   LE_EVT_MASK_LTK_REQUEST               |     \
                                   LE_EVT_MASK_REMOTE_CONN_PARAM_REQUEST |     \
                                   LE_EVT_MASK_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE)
    #else // !SECURE_CONNS_CFG
      #define LE_EVT_MASK_BYTE0   (LE_EVT_MASK_CONN_COMPLETE             |     \
                                   LE_EVT_MASK_ADV_REPORT                |     \
                                   LE_EVT_MASK_CONN_UPDATE_COMPLETE      |     \
                                   LE_EVT_MASK_READ_REMOTE_FEATURE       |     \
                                   LE_EVT_MASK_LTK_REQUEST               |     \
                                   LE_EVT_MASK_REMOTE_CONN_PARAM_REQUEST)
    #endif // SECURE_CONNS_CFG
  #endif // EXT_DATA_LEN_CFG
#else // !CONN_PARAM_REQ_CFG
  #if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & EXT_DATA_LEN_CFG)
    #if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & SECURE_CONNS_CFG)
      #define LE_EVT_MASK_BYTE0   (LE_EVT_MASK_CONN_COMPLETE             |     \
                                   LE_EVT_MASK_ADV_REPORT                |     \
                                   LE_EVT_MASK_CONN_UPDATE_COMPLETE      |     \
                                   LE_EVT_MASK_READ_REMOTE_FEATURE       |     \
                                   LE_EVT_MASK_LTK_REQUEST               |     \
                                   LE_EVT_MASK_DATA_LENGTH_CHANGE        |     \
                                   LE_EVT_MASK_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE)
    #else // !SECURE_CONNS_CFG
      #define LE_EVT_MASK_BYTE0   (LE_EVT_MASK_CONN_COMPLETE             |     \
                                   LE_EVT_MASK_ADV_REPORT                |     \
                                   LE_EVT_MASK_CONN_UPDATE_COMPLETE      |     \
                                   LE_EVT_MASK_READ_REMOTE_FEATURE       |     \
                                   LE_EVT_MASK_LTK_REQUEST               |     \
                                   LE_EVT_MASK_DATA_LENGTH_CHANGE)
    #endif // SECURE_CONNS_CFG
  #else // !EXT_DATA_LEN_CFG
    #if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & SECURE_CONNS_CFG)
      #define LE_EVT_MASK_BYTE0   (LE_EVT_MASK_CONN_COMPLETE             |     \
                                   LE_EVT_MASK_ADV_REPORT                |     \
                                   LE_EVT_MASK_CONN_UPDATE_COMPLETE      |     \
                                   LE_EVT_MASK_READ_REMOTE_FEATURE       |     \
                                   LE_EVT_MASK_LTK_REQUEST               |     \
                                   LE_EVT_MASK_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE)
    #else // !SECURE_CONNS_CFG
      #define LE_EVT_MASK_BYTE0   (LE_EVT_MASK_CONN_COMPLETE             |     \
                                   LE_EVT_MASK_ADV_REPORT                |     \
                                   LE_EVT_MASK_CONN_UPDATE_COMPLETE      |     \
                                   LE_EVT_MASK_READ_REMOTE_FEATURE       |     \
                                   LE_EVT_MASK_LTK_REQUEST)
    #endif // SECURE_CONNS_CFG
  #endif // EXT_DATA_LEN_CFG
#endif // CONN_PARAM_REQ_CFG

#if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & SECURE_CONNS_CFG)
  #if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & PRIVACY_1_2_CFG)
    #if defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & (PHY_2MBPS_CFG | PHY_LR_CFG))
      #define LE_EVT_MASK_BYTE1 (LE_EVT_MASK_GENERATE_DHKEY_COMPLETE      |    \
                                 LE_EVT_MASK_ENH_CONN_COMPLETE            |    \
                                 LE_EVT_MASK_DIRECT_ADVERTISING_REPORT    |    \
                                 LE_EVT_MASK_PHY_UPDATE_COMPLETE)
    #else // !(PHY_2MBPS_CFG | PHY_LR_CFG)
      #define LE_EVT_MASK_BYTE1 (LE_EVT_MASK_GENERATE_DHKEY_COMPLETE      |    \
                                 LE_EVT_MASK_ENH_CONN_COMPLETE            |    \
                                 LE_EVT_MASK_DIRECT_ADVERTISING_REPORT)
    #endif // PHY_2MBPS_CFG | PHY_LR_CFG
  #else // !PRIVACY_1_2_CFG
    #if defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & (PHY_2MBPS_CFG | PHY_LR_CFG))
      #define LE_EVT_MASK_BYTE1 (LE_EVT_MASK_GENERATE_DHKEY_COMPLETE      |    \
                                 LE_EVT_MASK_PHY_UPDATE_COMPLETE)
    #else // !(PHY_2MBPS_CFG | PHY_LR_CFG)
      #define LE_EVT_MASK_BYTE1 (LE_EVT_MASK_GENERATE_DHKEY_COMPLETE)
    #endif // PHY_2MBPS_CFG | PHY_LR_CFG
  #endif // PRIVACY_1_2_CFG
#else // !SECURE_CONNS_CFG
  #if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & PRIVACY_1_2_CFG)
    #if defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & (PHY_2MBPS_CFG | PHY_LR_CFG))
      #define LE_EVT_MASK_BYTE1 (LE_EVT_MASK_ENH_CONN_COMPLETE            |    \
                                 LE_EVT_MASK_DIRECT_ADVERTISING_REPORT    |    \
                                 LE_EVT_MASK_PHY_UPDATE_COMPLETE)
    #else // !(PHY_2MBPS_CFG | PHY_LR_CFG)
      #define LE_EVT_MASK_BYTE1 (LE_EVT_MASK_ENH_CONN_COMPLETE            |    \
                                 LE_EVT_MASK_DIRECT_ADVERTISING_REPORT)
    #endif // PHY_2MBPS_CFG | PHY_LR_CFG
  #else // !PRIVACY_1_2_CFG
    #if defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & (PHY_2MBPS_CFG | PHY_LR_CFG))
      #define LE_EVT_MASK_BYTE1 (LE_EVT_MASK_PHY_UPDATE_COMPLETE)
    #else // !(PHY_2MBPS_CFG  | PHY_LR_CFG)
      #define LE_EVT_MASK_BYTE1 (LE_EVT_MASK_NONE)
    #endif // PHY_2MBPS_CFG | PHY_LR_CFG
  #endif // PRIVACY_1_2_CFG
#endif // SECURE_CONNS_CFG

// LE Event Lengths
#define HCI_CMD_COMPLETE_EVENT_LEN                                     3
#define HCI_CMD_VS_COMPLETE_EVENT_LEN                                  2
#define HCI_CMD_STATUS_EVENT_LEN                                       4
#define HCI_NUM_COMPLETED_PACKET_EVENT_LEN                             5
#define HCI_FLUSH_OCCURRED_EVENT_LEN                                   2
#define HCI_REMOTE_VERSION_INFO_EVENT_LEN                              8
#define HCI_CONNECTION_COMPLETE_EVENT_LEN                              19
#define HCI_ENH_CONNECTION_COMPLETE_EVENT_LEN                          31
#define HCI_DISCONNECTION_COMPLETE_LEN                                 4
#define HCI_CONN_UPDATE_COMPLETE_LEN                                   10
#define HCI_ADV_REPORT_EVENT_LEN                                       12
#define HCI_READ_REMOTE_FEATURE_COMPLETE_EVENT_LEN                     12
#define HCI_REMOTE_CONNECTION_PARAMETER_REQUEST_LEN                    11
#define HCI_NUM_COMPLETED_PACKET_EVENT_LEN                             5
#define HCI_APTO_EXPIRED_EVENT_LEN                                     2
#define HCI_LTK_REQUESTED_EVENT_LEN                                    13
#define HCI_DATA_BUF_OVERFLOW_EVENT_LEN                                1
#define HCI_ENCRYPTION_CHANGE_EVENT_LEN                                4
#define HCI_KEY_REFRESH_COMPLETE_EVENT_LEN                             3
#define HCI_BUFFER_OVERFLOW_EVENT_LEN                                  1
#define HCI_DATA_LENGTH_CHANGE_EVENT_LEN                               11
#define HCI_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_EVENT_LEN              66
#define HCI_GENERATE_DHKEY_COMPLETE_EVENT_LEN                          34
#define HCI_PHY_UPDATE_COMPLETE_EVENT_LEN                              6
// Vendor Specific LE Events
#define HCI_SCAN_REQ_REPORT_EVENT_LEN                                  11

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

uint8 pBleEvtMask[B_EVENT_MASK_LEN];
uint8 pHciEvtMask[B_EVENT_MASK_LEN];
uint8 pHciEvtMask2[B_EVENT_MASK_LEN];

/*******************************************************************************
 * EXTERNS
 */

extern uint8 hciPTMenabled;

/*
** Internal Functions
*/

/*******************************************************************************
 * @fn          hciInitEventMasks
 *
 * @brief       This routine initializes Bluetooth and BLE event makss to their
 *              default values.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void hciInitEventMasks( void )
{
  // set default Bluetooth event mask bits
  pHciEvtMask[0] = BT_EVT_MASK_BYTE0;
  pHciEvtMask[1] = BT_EVT_MASK_BYTE1;
  pHciEvtMask[2] = BT_EVT_MASK_BYTE2;
  pHciEvtMask[3] = BT_EVT_MASK_BYTE3;
  pHciEvtMask[4] = BT_EVT_MASK_BYTE4;
  pHciEvtMask[5] = BT_EVT_MASK_BYTE5;
  pHciEvtMask[6] = BT_EVT_MASK_BYTE6;
  pHciEvtMask[7] = BT_EVT_MASK_BYTE7;

  // set default Bluetooth event mask page 2 bits
  pHciEvtMask2[0] = BT_EVT_MASK2_BYTE0;
  pHciEvtMask2[1] = BT_EVT_MASK2_BYTE1;
  pHciEvtMask2[2] = BT_EVT_MASK2_BYTE2;
  pHciEvtMask2[3] = BT_EVT_MASK2_BYTE3;
  pHciEvtMask2[4] = BT_EVT_MASK2_BYTE4;
  pHciEvtMask2[5] = BT_EVT_MASK2_BYTE5;
  pHciEvtMask2[6] = BT_EVT_MASK2_BYTE6;
  pHciEvtMask2[7] = BT_EVT_MASK2_BYTE7;

  // set default BLE event mask bits
  pBleEvtMask[0] = LE_EVT_MASK_BYTE0;
  pBleEvtMask[1] = LE_EVT_MASK_BYTE1;
  pBleEvtMask[2] = LE_EVT_MASK_NONE;
  pBleEvtMask[3] = LE_EVT_MASK_NONE;
  pBleEvtMask[4] = LE_EVT_MASK_NONE;
  pBleEvtMask[5] = LE_EVT_MASK_NONE;
  pBleEvtMask[6] = LE_EVT_MASK_NONE;
  pBleEvtMask[7] = LE_EVT_MASK_NONE;

  return;
}


/*
** HCI Events
*/

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
/*******************************************************************************
 * This function sends the Data Buffer Overflow Event to the Host.
 *
 * Public function defined in hci_c_event.h.
 */
void HCI_DataBufferOverflowEvent( uint8 linkType )
{
  hciPacket_t *msg;
  uint8       totalLength;

  // OSAL message header + HCI event header + parameters
  totalLength = sizeof( hciPacket_t ) +
                HCI_EVENT_MIN_LENGTH +
                HCI_BUFFER_OVERFLOW_EVENT_LEN;

  msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

  if (msg)
  {
    // create message header
    msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
    msg->hdr.status = 0xFF;

    // create event header
    msg->pData    = (uint8*)(msg+1);
    msg->pData[0] = HCI_EVENT_PACKET;
    msg->pData[1] = HCI_DATA_BUFFER_OVERFLOW_EVENT;
    msg->pData[2] = HCI_BUFFER_OVERFLOW_EVENT_LEN;

    // Link Type
    msg->pData[3] = linkType;

    // send message
    (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
  }
}
#endif // ADV_CONN_CFG | INIT_CFG


#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
/*******************************************************************************
 * This function sends the Number of Completed Packets Event to the Host.
 *
 * Public function defined in hci_c_event.h.
 */
void HCI_NumOfCompletedPacketsEvent( uint8   numHandles,
                                     uint16 *handles,
                                     uint16 *numCompletedPkts )
{
  // check if this is for the Host
  if ( hciL2capTaskID != 0 )
  {
    hciEvt_NumCompletedPkt_t *pkt =
      (hciEvt_NumCompletedPkt_t *)MAP_osal_msg_allocate( sizeof(hciEvt_NumCompletedPkt_t) +
                                                     (numHandles * 2 * sizeof(uint16)) );
    if ( pkt )
    {
      pkt->hdr.event = HCI_DATA_EVENT; // packet type
      pkt->hdr.status = HCI_NUM_OF_COMPLETED_PACKETS_EVENT_CODE; // event code
      pkt->numHandles = numHandles;
      pkt->pConnectionHandle = (uint16 *)(pkt+1);
      pkt->pNumCompletedPackets = (uint16 *)( (uint8 *)(pkt+1) + ( numHandles * sizeof(uint16) ) );

      // for each handle, there's a handle number and a number of
      // completed packets for that handle
      for ( uint8 i = 0; i < numHandles; i++ )
      {
        pkt->pConnectionHandle[i] = handles[i];
        pkt->pNumCompletedPackets[i] = numCompletedPkts[i];
      }

      (void)MAP_osal_msg_send( hciL2capTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // the data length
    // ALT: Use 1+(numHandles * HCI_NUM_COMPLETED_PACKET_EVENT_LEN-1).
    dataLength = HCI_NUM_COMPLETED_PACKET_EVENT_LEN +
      ((numHandles-1) * (HCI_NUM_COMPLETED_PACKET_EVENT_LEN-1));

    // OSAL message header + HCI event header + data
    totalLength = sizeof(hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLength;

    // allocate memory for OSAL hdr + packet
    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if ( msg )
    {
      uint8 i;

      // OSAL header
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // build event packet
      msg->pData    = (uint8 *)(msg+1);
      msg->pData[0] = HCI_EVENT_PACKET;                        // packet type
      msg->pData[1] = HCI_NUM_OF_COMPLETED_PACKETS_EVENT_CODE; // event code
      msg->pData[2] = dataLength;
      msg->pData[3] = numHandles;

      // for each handle, there's a number handle number and a number of
      // completed packets for that handle
      for (i=0; i<numHandles; i++)
      {
        msg->pData[4+(4*i)] = LO_UINT16(handles[i]);
        msg->pData[5+(4*i)] = HI_UINT16(handles[i]);
        msg->pData[6+(4*i)] = LO_UINT16(numCompletedPkts[i]);
        msg->pData[7+(4*i)] = HI_UINT16(numCompletedPkts[i]);
      }

      // send message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}
#endif // ADV_CONN_CFG | INIT_CFG


/*******************************************************************************
 * This function sends a Command Complete Event to the Host.
 *
 * Public function defined in hci_c_event.h.
 */
void HCI_CommandCompleteEvent( uint16 opcode,
                               uint8  numParam,
                               uint8  *param )
{
  // check if this is for the Host
  if ( ((hciGapTaskID != 0) || (hciSmpTaskID != 0)) && (hciPTMenabled == FALSE) )
  {
    hciEvt_CmdComplete_t *pkt =
      (hciEvt_CmdComplete_t *)MAP_osal_msg_allocate( sizeof(hciEvt_CmdComplete_t) +
                                                 numParam );

    if ( pkt )
    {
      uint8 taskID;

      if ( (opcode == HCI_LE_RAND || opcode == HCI_LE_ENCRYPT) && (hciSmpTaskID) )
      {
        taskID         = hciSmpTaskID;
        pkt->hdr.event = HCI_SMP_EVENT_EVENT;
      }
      else
      {
        taskID         = hciGapTaskID;
        pkt->hdr.event = HCI_GAP_EVENT_EVENT;
      }
      pkt->hdr.status   = HCI_COMMAND_COMPLETE_EVENT_CODE;
      pkt->numHciCmdPkt = 1;
      pkt->cmdOpcode    = opcode;
      pkt->pReturnParam = (uint8 *)(pkt+1);

      (void)MAP_osal_memcpy( pkt->pReturnParam, param, numParam );

      (void)MAP_osal_msg_send( taskID, (uint8 *)pkt );
    }
  }
  else
  {
    MAP_HCI_SendCommandCompleteEvent( HCI_COMMAND_COMPLETE_EVENT_CODE,
                                      opcode,
                                      numParam,
                                      param );
  }
}


#if defined(BLE_VS_FEATURES) && (BLE_VS_FEATURES & SCAN_REQ_RPT_CFG) &&        \
    defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
/*******************************************************************************
 * @fn          LL_EXT_ScanReqReportCback Callback
 *
 * @brief       This LL callback is used to generate a vendor specific Scan
 *              Request Report meta event when an Scan Request is received by
 *              an Advertiser.
 *
 * input parameters
 *
 * @param       peerAddrType - Peer address type.
 * @param       peerAddr     - Peer address.
 * @param       chan         - BLE channel of report.
 * @param       rssi         - The RSSI of received packet.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_EXT_ScanReqReportCback( uint8  peerAddrType,
                                uint8 *peerAddr,
                                uint8  chan,
                                int8   rssi )
{
  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_BLEScanReqReport_t *pkt;

    pkt = (hciEvt_BLEScanReqReport_t *)MAP_osal_msg_allocate( sizeof(hciEvt_BLEScanReqReport_t) );

    if ( pkt )
    {
      pkt->hdr.event  = HCI_GAP_EVENT_EVENT;
      pkt->hdr.status = HCI_LE_EVENT_CODE;

      pkt->BLEEventCode = HCI_BLE_SCAN_REQ_REPORT_EVENT;
      pkt->eventType    = 0; // Scan Request is the only packet supported

      pkt->peerAddrType = peerAddrType;
      (void)MAP_osal_memcpy( pkt->peerAddr, peerAddr, B_ADDR_LEN );

      pkt->bleChan = chan;
      pkt->rssi    = rssi;

      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;

    // OSAL message header + HCI event header + data
    msg = (hciPacket_t *)MAP_osal_msg_allocate( sizeof(hciPacket_t)  +
                                                HCI_EVENT_MIN_LENGTH +
                                                HCI_SCAN_REQ_REPORT_EVENT_LEN );
    if (msg)
    {
      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData    = (uint8*)(msg+1);
      msg->pData[0] = HCI_EVENT_PACKET;
      msg->pData[1] = HCI_LE_EVENT_CODE;
      msg->pData[2] = HCI_SCAN_REQ_REPORT_EVENT_LEN;

      // populate event
      msg->pData[3]  = HCI_BLE_SCAN_REQ_REPORT_EVENT;                 // event code
      msg->pData[4]  = LL_ADV_RPT_SCAN_REQ;                           // advertisement event type
      msg->pData[5]  = peerAddrType;                                  // peer address type
      (void)MAP_osal_memcpy (&msg->pData[6], peerAddr, B_ADDR_LEN);   // peer address
      msg->pData[12] = chan;                                          // channel
      msg->pData[13] = rssi;                                          // RSSI

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}
#endif // SCAN_REQ_RPT_CFG & (ADV_NCONN_CFG | ADV_CONN_CFG)


/*******************************************************************************
 * This function sends a Vendor Specific Command Complete Event to the Host.
 *
 * Public function defined in hci_c_event.h.
 */
void HCI_VendorSpecifcCommandCompleteEvent( uint16  opcode,
                                            uint8   numParam,
                                            uint8  *param )
{
  // check if this is for the Host
  if ( (hciGapTaskID != 0) && (hciPTMenabled == FALSE) )
  {
    hciEvt_VSCmdComplete_t *pkt =
      (hciEvt_VSCmdComplete_t *)MAP_osal_msg_allocate( sizeof(hciEvt_VSCmdComplete_t) +
                                                   numParam );

    if ( pkt )
    {
      pkt->hdr.event   = HCI_GAP_EVENT_EVENT;
      pkt->hdr.status  = HCI_VE_EVENT_CODE;
      pkt->length      = numParam;
      pkt->cmdOpcode   = opcode;
      pkt->pEventParam = (uint8 *)(pkt+1);

      (void)MAP_osal_memcpy( pkt->pEventParam, param, numParam );

      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    MAP_HCI_SendCommandCompleteEvent( HCI_VE_EVENT_CODE,
                                      opcode,
                                      numParam,
                                      param );
  }
}


/*******************************************************************************
 * This function sends a Command Status Event to the Host.
 *
 * Public function defined in hci_c_event.h.
 */
void HCI_CommandStatusEvent( hciStatus_t status,
                             uint16      opcode )
{
  // check if this is for the Host
  if ( (hciGapTaskID != 0) && (hciPTMenabled == FALSE) )
  {
    hciEvt_CommandStatus_t *pMsg;
    uint8 totalLength;

    totalLength = sizeof(hciEvt_CommandStatus_t);

    pMsg = (hciEvt_CommandStatus_t *)MAP_osal_msg_allocate( totalLength );

    if ( pMsg )
    {
      // message type, HCI event type
      pMsg->hdr.event = HCI_GAP_EVENT_EVENT;

      // use the OSAL status field for HCI event code
      pMsg->hdr.status   = HCI_COMMAND_STATUS_EVENT_CODE;
      pMsg->cmdStatus    = status;
      pMsg->numHciCmdPkt = 1;
      pMsg->cmdOpcode    = opcode;

      // send the message
      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)pMsg );
    }
  }
  else
  {
    MAP_HCI_SendCommandStatusEvent( HCI_COMMAND_STATUS_EVENT_CODE,
                                    status,
                                    opcode );
  }
}


/*******************************************************************************
 * This function sends a Hardware Error Event to the Host.
 *
 * Public function defined in hci_c_event.h.
 */
void HCI_HardwareErrorEvent( uint8 hwErrorCode )
{
  // check the event mask to see if this event has been masked by Host
  if ( (BT_EVT_MASK_HARDWARE_ERROR & pHciEvtMask[BT_EVT_INDEX_HARDWARE_ERROR]) == 0 )
  {
    return;
  }

  // check if this is for the Host
  if ( (hciGapTaskID != 0) && (hciPTMenabled == FALSE) )
  {
    hciEvt_HardwareError_t *pMsg;
    uint8 totalLength;

    totalLength = sizeof(hciEvt_HardwareError_t);

    pMsg = (hciEvt_HardwareError_t *)MAP_osal_msg_allocate( totalLength );

    if ( pMsg )
    {
      // message type, HCI event type
      pMsg->hdr.event = HCI_GAP_EVENT_EVENT;

      // use the OSAL status field for HCI event code
      pMsg->hdr.status   = HCI_BLE_HARDWARE_ERROR_EVENT_CODE;
      pMsg->hardwareCode = hwErrorCode;

      // send the message
      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)pMsg );
    }
  }
  else
  {
    hciPacket_t *msg;

    // OSAL message header + HCI event header + data length of one
    msg = (hciPacket_t *)MAP_osal_msg_allocate( sizeof(hciPacket_t)  +
                                            HCI_EVENT_MIN_LENGTH +
                                            1 );

    // send event (if we have the memory)
    if (msg)
    {
      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData    = (uint8*)(msg+1);
      msg->pData[0] = HCI_EVENT_PACKET;
      msg->pData[1] = HCI_BLE_HARDWARE_ERROR_EVENT_CODE;
      msg->pData[2] = 1; // data length

      // error code
      msg->pData[3] = hwErrorCode;

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }

  return;
}


/*******************************************************************************
 * This generic function sends a Command Complete or a Vendor Specific Command
 * Complete Event to the Host.
 *
 * Public function defined in hci_c_event.h.
 */
void HCI_SendCommandStatusEvent ( uint8  eventCode,
                                  uint16 status,
                                  uint16 opcode )
{
  uint8 data[4];

  data[0] = status;
  data[1] = 1;                 // number of HCI command packets
  data[2] = LO_UINT16(opcode); // opcode (LSB)
  data[3] = HI_UINT16(opcode); // opcode (MSB)

  MAP_HCI_SendControllerToHostEvent( eventCode, 4, data );
}


/*******************************************************************************
 * This generic function sends a Command Complete or a Vendor Specific Command
 * Complete Event to the Host.
 *
 * Public function defined in hci_c_event.h.
 */
void HCI_SendCommandCompleteEvent ( uint8  eventCode,
                                    uint16 opcode,
                                    uint8  numParam,
                                    uint8  *param )
{
  hciPacket_t *msg;
  uint8        totalLength;

  // The initial length will be:
  // OSAL message header(4) - not part of packet sent to HCI Host!
  // Minimum Event Data: Packet Type(1) + Event Code(1) + Length(1)
  // Return Parameters (0..N)
  totalLength = sizeof(hciPacket_t) + HCI_EVENT_MIN_LENGTH + numParam;

  // adjust the size of the event packet based on event code
  // Note: If not a vendor specific event, then the event includes:
  //       Command Complete Data: Number of HCI Commands Allowed(1) + Command Opcode(2)
  // Note: If a vendor specific event, then the event includes:
  //       Vendor Specific Command Complete Data: Vendor Specific Event Opcode(2)
  totalLength += ( (eventCode != HCI_VE_EVENT_CODE)  ?
                   HCI_CMD_COMPLETE_EVENT_LEN        :
                   HCI_CMD_VS_COMPLETE_EVENT_LEN );

  // allocate memory for OSAL hdr + packet
  msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

  if ( msg )
  {
    // OSAL message event, status, and pointer to packet
    msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
    msg->hdr.status = 0xFF;
    msg->pData      = (uint8*)(msg+1);

    // fill in Command Complete Event data
    msg->pData[0] = HCI_EVENT_PACKET;
    msg->pData[1] = eventCode;

    // check if this isn't a vendor specific event
    if ( eventCode != HCI_VE_EVENT_CODE )
    {
      msg->pData[2] = numParam + HCI_CMD_COMPLETE_EVENT_LEN;
      msg->pData[3] = 1;                   // event parameter 1
      msg->pData[4] = LO_UINT16( opcode ); // event parameter 2
      msg->pData[5] = HI_UINT16( opcode ); // event parameter 2

      // remaining event parameters
      (void)MAP_osal_memcpy (&msg->pData[6], param, numParam);
    }
    else // it is a vendor specific event
    {
      // less one byte as number of complete packets not used in vendor specific event
      msg->pData[2] = numParam + HCI_CMD_VS_COMPLETE_EVENT_LEN;
      msg->pData[3] = param[0];            // event parameter 0: event opcode LSB
      msg->pData[4] = param[1];            // event parameter 1: event opcode MSB
      msg->pData[5] = param[2];            // event parameter 2: status
      msg->pData[6] = LO_UINT16( opcode ); // event parameter 3: command opcode LSB
      msg->pData[7] = HI_UINT16( opcode ); // event parameter 3: command opcode MSB

      // remaining event parameters
      // Note: The event opcode and status were already placed in the msg packet.
      (void)MAP_osal_memcpy (&msg->pData[8], &param[3], numParam-HCI_EVENT_MIN_LENGTH);
    }

    // send the message
    (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
  }
}


/*******************************************************************************
 * This is a generic function used to send events from the Controller to the
 * Host.
 *
 * Public function defined in hci_c_event.h.
 */
void HCI_SendControllerToHostEvent( uint8 eventCode,
                                    uint8 dataLen,
                                    uint8 *pData )
{
  hciPacket_t *msg;
  uint8 totalLength;

  // OSAL message header + HCI event header + data
  totalLength = sizeof(hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLen;

  // allocate memory for OSAL hdr + packet
  msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

  if ( msg )
  {
    // message type, HCI event type
    msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
    msg->hdr.status = 0xFF;

    // packet
    msg->pData    = (uint8*)(msg+1);
    msg->pData[0] = HCI_EVENT_PACKET;
    msg->pData[1] = eventCode;
    msg->pData[2] = dataLen;

    // copy data
    if ( dataLen )
    {
      (void)MAP_osal_memcpy( &(msg->pData[3]), pData, dataLen );
    }

    // send message
    (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
  }
}


/*******************************************************************************
 * @fn          LL_AuthPayloadTimeoutExpiredCback Callback
 *
 * @brief       This LL callback is used to generate an Authenticated Payload
 *              Timeout event when the APTO expires.
 *
 * input parameters
 *
 * @param       connHandle - Connection handle.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_AuthPayloadTimeoutExpiredCback( uint16 connHandle )
{
  // check if LE Meta-Events are enabled and this event is enabled
  if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
       ((pHciEvtMask2[BT_EVT_INDEX2_APTO_EXPIRED] & BT_EVT_MASK2_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED) == 0) )
  {
    // the event mask is not enabled for this event
    return;
  }

  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_AptoExpired_t *pkt =
      (hciEvt_AptoExpired_t *)MAP_osal_msg_allocate( sizeof(hciEvt_AptoExpired_t) +
                                                     sizeof(uint16) );
    if ( pkt )
    {
      pkt->hdr.event  = HCI_DATA_EVENT;              // packet type
      pkt->hdr.status = HCI_APTO_EXPIRED_EVENT_CODE; // event code
      pkt->connHandle = connHandle;

      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;

    // OSAL message header + HCI event header + data

    // allocate memory for OSAL hdr + packet
    msg = (hciPacket_t *)MAP_osal_msg_allocate( sizeof(hciPacket_t)  +
                                            HCI_EVENT_MIN_LENGTH +
                                            HCI_APTO_EXPIRED_EVENT_LEN );

    if ( msg )
    {
      // OSAL header
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // build event packet
      msg->pData    = (uint8 *)(msg+1);
      msg->pData[0] = HCI_EVENT_PACKET;            // packet type
      msg->pData[1] = HCI_APTO_EXPIRED_EVENT_CODE; // event code
      msg->pData[2] = HCI_APTO_EXPIRED_EVENT_LEN;
      msg->pData[3] = LO_UINT16( connHandle );
      msg->pData[4] = HI_UINT16( connHandle );

      // send message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}


#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
/*******************************************************************************
 * @fn          LL_RemoteConnParamReqCback Callback
 *
 * @brief       This LL callback is used to generate a Remote Connection
 *              Parameter Request meta event to provide to the Host the peer's
 *              connection parameter request parameters (min connection
 *              interval, max connection interval, slave latency, and connection
 *              timeout), and to request the Host's acceptance or rejection of
 *              this parameters.
 *
 * input parameters
 *
 * @param       connHandle   - Connection handle.
 * @param       Interval_Min - Lower limit for connection interval.
 * @param       Interval_Max - Upper limit for connection interval.
 * @param       Latency      - Slave latency.
 * @param       Timeout      - Connection timeout.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_RemoteConnParamReqCback( uint16 connHandle,
                                 uint16 Interval_Min,
                                 uint16 Interval_Max,
                                 uint16 Latency,
                                 uint16 Timeout )
{
  // check if LE Meta-Events are enabled and this event is enabled
  if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
       ((pBleEvtMask[LE_EVT_INDEX_REMOTE_CONN_PARAM_REQUEST] & LE_EVT_MASK_REMOTE_CONN_PARAM_REQUEST) == 0) )
  {
    // the event mask is not set for this event, so reject immediately
    MAP_LL_RemoteConnParamReqNegReply( connHandle,
                                       HCI_ERROR_CODE_UNSUPPORTED_REMOTE_FEATURE );

    // the event mask is not enabled for this event
    return;
  }

  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_BLERemoteConnParamReq_t *msg;
    uint8 totalLength;

    totalLength = sizeof( hciEvt_BLERemoteConnParamReq_t );

    msg = (hciEvt_BLERemoteConnParamReq_t *)MAP_osal_msg_allocate(totalLength);

    if( msg )
    {
      // message header
      msg->hdr.event  = HCI_GAP_EVENT_EVENT;
      msg->hdr.status = HCI_LE_EVENT_CODE; // use status field to pass the HCI Event code

      // event packet
      msg->BLEEventCode = HCI_BLE_REMOTE_CONN_PARAM_REQUEST_EVENT;
      msg->status       = HCI_SUCCESS;
      msg->connHandle   = connHandle;
      msg->Interval_Min = Interval_Min;
      msg->Interval_Max = Interval_Max;
      msg->Latency      = Latency;
      msg->Timeout      = Timeout;

      // send the message
      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)msg );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // data length
    dataLength = HCI_REMOTE_CONNECTION_PARAMETER_REQUEST_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof(hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 *pBuf;

      // message type
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // point to the byte following the hciPacket_t structure
      msg->pData = (uint8*)(msg+1);
      pBuf       = msg->pData;

      *pBuf++ = HCI_EVENT_PACKET;
      *pBuf++ = HCI_LE_EVENT_CODE;
      *pBuf++ = dataLength;

      // populate event
      *pBuf++ = HCI_BLE_REMOTE_CONN_PARAM_REQUEST_EVENT;  // event code
      *pBuf++ = LO_UINT16(connHandle);           // connection handle (LSB)
      *pBuf++ = HI_UINT16(connHandle);           // connection handle (MSB)
      *pBuf++ = LO_UINT16(Interval_Min);         // connection interval (LSB)
      *pBuf++ = HI_UINT16(Interval_Min);         // connection interval (MSB)
      *pBuf++ = LO_UINT16(Interval_Max);         // connection interval (LSB)
      *pBuf++ = HI_UINT16(Interval_Max);         // connection interval (MSB)
      *pBuf++ = LO_UINT16(Latency);              // slave latency (LSB)
      *pBuf++ = HI_UINT16(Latency);              // slave latency (MSB)
      *pBuf++ = LO_UINT16(Timeout);              // connection timeout (LSB)
      *pBuf++ = HI_UINT16(Timeout);              // connection timeout (MSB)

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}
#endif // ADV_CONN_CFG | INIT_CFG
#endif // CONN_PARAM_REQ_CFG


#if defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & (PHY_2MBPS_CFG | PHY_LR_CFG))
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
/*******************************************************************************
 * @fn          LL_PhyUpdateCompleteEventCback Callback
 *
 * @brief       This LL callback is used to indicate that the Controller has
 *              has changed the transmitter or receiver PHY in use or that the
 *              LL_SetPhy command has failed.
 *
 * input parameters
 *
 * @param       status     - Status of LL_SetPhy command.
 * @param       connHandle - Connection handle.
 * @param       txPhy      - Bit map of PHY used for Tx.
 * @param       rxPhy      - Bit map of PHY used for Rx.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_PhyUpdateCompleteEventCback( llStatus_t status,
                                     uint16     connHandle,
                                     uint8      txPhy,
                                     uint8      rxPhy )
{
  // check if LE Meta-Events are enabled and this event is enabled
  if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
       ((pBleEvtMask[LE_EVT_INDEX_PHY_UPDATE_COMPLETE] & LE_EVT_MASK_PHY_UPDATE_COMPLETE) == 0) )
  {
    // the event mask is not enabled for this event
    return;
  }

  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_BLEPhyUpdateComplete_t *msg =
      (hciEvt_BLEPhyUpdateComplete_t *)MAP_osal_msg_allocate(sizeof( hciEvt_BLEPhyUpdateComplete_t ));

    if( msg )
    {
      // message header
      msg->hdr.event  = HCI_GAP_EVENT_EVENT;
      msg->hdr.status = HCI_LE_EVENT_CODE; // use status field to pass the HCI Event code

      // event packet
      msg->BLEEventCode = HCI_BLE_PHY_UPDATE_COMPLETE_EVENT;
      msg->status       = status;
      msg->connHandle   = connHandle;
      msg->txPhy        = txPhy;
      msg->rxPhy        = rxPhy;

      // send the message
      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)msg );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // data length
    dataLength = HCI_PHY_UPDATE_COMPLETE_EVENT_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof(hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 *pBuf;

      // message type
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // point to the byte following the hciPacket_t structure
      msg->pData = (uint8*)(msg+1);
      pBuf       = msg->pData;

      *pBuf++ = HCI_EVENT_PACKET;
      *pBuf++ = HCI_LE_EVENT_CODE;
      *pBuf++ = dataLength;

      // populate event
      *pBuf++ = HCI_BLE_PHY_UPDATE_COMPLETE_EVENT;  // event code
      *pBuf++ = status;                             // status
      *pBuf++ = LO_UINT16(connHandle);              // connection handle (LSB)
      *pBuf++ = HI_UINT16(connHandle);              // connection handle (MSB)
      *pBuf++ = txPhy;                              // TX PHY
      *pBuf++ = rxPhy;                              // RX PHY

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}
#endif // ADV_CONN_CFG | INIT_CFG
#endif // PHY_2MBPS_CFG | PHY_LR_CFG


#if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & EXT_DATA_LEN_CFG)
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
/*******************************************************************************
 * @fn          LL_DataLengthChangeEventCback Callback
 *
 * @brief       This LL callback is used to indicate that the Controller has
 *              has changed the either the maximum payload length or the
 *              maximum transmit time of data channel PDUs in either direction.
 *              The values reported are the maximum taht will actually be used
 *              on the connection following the change.
 *
 * input parameters
 *
 * @param       connHandle  - Connection handle.
 * @param       maxTxOctets - Maximum number of transmit payload bytes.
 * @param       maxTxTime   - Maximum transmit time.
 * @param       maxRxOctets - Maximum number of receive payload bytes.
 * @param       maxRxTime   - Maximum receive time.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_DataLengthChangeEventCback( uint16 connHandle,
                                    uint16 maxTxOctets,
                                    uint16 maxTxTime,
                                    uint16 maxRxOctets,
                                    uint16 maxRxTime )
{
  // check if LE Meta-Events are enabled and this event is enabled
  if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
       ((pBleEvtMask[LE_EVT_INDEX_DATA_LENGTH_CHANGE] & LE_EVT_MASK_DATA_LENGTH_CHANGE) == 0) )
  {
    // the event mask is not enabled for this event
    return;
  }

  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_BLEDataLengthChange_t *msg =
      (hciEvt_BLEDataLengthChange_t *)MAP_osal_msg_allocate(sizeof( hciEvt_BLEDataLengthChange_t ));

    if( msg )
    {
      // message header
      msg->hdr.event  = HCI_GAP_EVENT_EVENT;
      msg->hdr.status = HCI_LE_EVENT_CODE; // use status field to pass the HCI Event code

      // event packet
      msg->BLEEventCode = HCI_BLE_DATA_LENGTH_CHANGE_EVENT;
      msg->connHandle   = connHandle;
      msg->maxTxOctets  = maxTxOctets;
      msg->maxTxTime    = maxTxTime;
      msg->maxRxOctets  = maxRxOctets;
      msg->maxRxTime    = maxRxTime;

      // send the message
      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)msg );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // data length
    dataLength = HCI_DATA_LENGTH_CHANGE_EVENT_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof(hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 *pBuf;

      // message type
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // point to the byte following the hciPacket_t structure
      msg->pData = (uint8*)(msg+1);
      pBuf       = msg->pData;

      *pBuf++ = HCI_EVENT_PACKET;
      *pBuf++ = HCI_LE_EVENT_CODE;
      *pBuf++ = dataLength;

      // populate event
      *pBuf++ = HCI_BLE_DATA_LENGTH_CHANGE_EVENT;   // event code
      *pBuf++ = LO_UINT16(connHandle);              // connection handle (LSB)
      *pBuf++ = HI_UINT16(connHandle);              // connection handle (MSB)
      *pBuf++ = LO_UINT16(maxTxOctets);             // max Tx bytes (LSB)
      *pBuf++ = HI_UINT16(maxTxOctets);             // max Tx bytes (MSB)
      *pBuf++ = LO_UINT16(maxTxTime);               // max Tx time (LSB)
      *pBuf++ = HI_UINT16(maxTxTime);               // max Tx time (MSB)
      *pBuf++ = LO_UINT16(maxRxOctets);             // max Rx bytes (LSB)
      *pBuf++ = HI_UINT16(maxRxOctets);             // max Rx bytes (MSB)
      *pBuf++ = LO_UINT16(maxRxTime);               // max Rx time (LSB)
      *pBuf++ = HI_UINT16(maxRxTime);               // max Rx time (MSB)

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}
#endif // ADV_CONN_CFG | INIT_CFG
#endif // EXT_DATA_LEN_CFG


#if 0 // Note: This event is not supported by the Host.
#if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & PRIVACY_1_2_CFG)
/*******************************************************************************
 * @fn          LL_DirectAdvReportCback Callback
 *
 * @brief       This LL callback is used to
 *
 * input parameters
 *
 * @param       advEvt      - Advertise event type, or Scan Response event type.
 * @param       advAddrType - Public or Random address type.
 * @param       advAddr     - Pointer to device address.
 * @param       dataLen     - Length of data in bytes.
 * @param       advData     - Pointer to data.
 * @param       rssi        - The RSSI of received packet.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_DirectAdvReportCback( uint8  eventType,
                              uint8  directAddrType,
                              uint8 *directAddr,
                              uint8  advAddrType,
                              uint8 *advAddr,
                              int8   rssi )
{
  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
  }
  else
  {
    hciPacket_t *msg;
    uint8 totalLength;
    uint8 dataLength;

    // check if LE Meta-Events are enabled and this event is enabled
    if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
        (((bleEvtMask & LE_EVT_MASK_ADV_REPORT) == 0 )) )
    {
      // the event mask is not set for this event
      return;
    }

    // data length
    dataLength = HCI_ADV_REPORT_EVENT_LEN + dataLen;

    // OSAL message header + HCI event header + data
    totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData    = (uint8*)(msg+1);
      msg->pData[0] = HCI_EVENT_PACKET;
      msg->pData[1] = HCI_LE_EVENT_CODE;
      msg->pData[2] = dataLength;

      // populate event
      msg->pData[3] = HCI_BLE_ADV_REPORT_EVENT;                // event code
      msg->pData[4] = 1;                                       // number of devices; assume 1 for now
      msg->pData[5] = advEvt;                                  // advertisement event type
      msg->pData[6] = advAddrType;                             // address type
      (void)MAP_osal_memcpy (&msg->pData[7], advAddr, B_ADDR_LEN); // address
      msg->pData[13] = dataLen;                                // data length
      (void)MAP_osal_memcpy (&msg->pData[14], advData, dataLen);   // data
      msg->pData[14 + dataLen] = rssi;                         // RSSI

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}
#endif // PRIVACY_1_2_CFG
#endif // 0


#if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & SECURE_CONNS_CFG)
/*******************************************************************************
 * @fn          LL_ReadLocalP256PublicKeyCompleteEventCback Callback
 *
 * @brief       This LL callback is used to indicate the Controller has
 *              completed the generation of the P256 public key.
 *
 * @param       None.
 *
 * input parameters
 *
 * output parameters
 *
 * @param       status   - Operation status.
 * @param       p256KeyX - P256 public key (first 32 bytes X=0..31).
 * @param       p256KeyY - P256 public key (second 32 bytes Y=32..63).
 *
 * @return      None.
 */
void LL_ReadLocalP256PublicKeyCompleteEventCback( uint8  status,
                                                  uint8 *p256KeyX,
                                                  uint8 *p256KeyY )
{
  // check if LE Meta-Events are enabled and this event is enabled
  if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
       ((pBleEvtMask[LE_EVT_INDEX_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE] & LE_EVT_MASK_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE) == 0) )
  {
    // the event mask is not enabled for this event
    return;
  }

  // check if this is for the Host
  if ( hciSmpTaskID != 0 )
  {
    hciEvt_BLEReadP256PublicKeyComplete_t *msg =
      (hciEvt_BLEReadP256PublicKeyComplete_t *)MAP_osal_msg_allocate(sizeof( hciEvt_BLEReadP256PublicKeyComplete_t ));

    if( msg )
    {
      // message header
      msg->hdr.event  = HCI_SMP_META_EVENT_EVENT;
      msg->hdr.status = HCI_LE_EVENT_CODE;

      // event packet
      msg->BLEEventCode = HCI_BLE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_EVENT;
      msg->status       = status;

      if ( status == HCI_SUCCESS )
      {
        // copy the P256 key
        (void)MAP_osal_memcpy( &msg->p256Key[0],  p256KeyX, LL_SC_P256_KEY_LEN/2 );
        (void)MAP_osal_memcpy( &msg->p256Key[32], p256KeyY, LL_SC_P256_KEY_LEN/2 );
      }
      else // clear the key to avoid confusion
      {
        (void)MAP_osal_memset( msg->p256Key, 0, LL_SC_P256_KEY_LEN );
      }

      // send the message
      (void)MAP_osal_msg_send( hciSmpTaskID, (uint8 *)msg );
    }
  }
  else // Host not present
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // data length
    dataLength = HCI_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_EVENT_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof(hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 *pBuf;

      // message type
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // point to the byte following the hciPacket_t structure
      msg->pData = (uint8*)(msg+1);
      pBuf       = msg->pData;

      *pBuf++ = HCI_EVENT_PACKET;
      *pBuf++ = HCI_LE_EVENT_CODE;
      *pBuf++ = dataLength;

      // populate event
      *pBuf++ = HCI_BLE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_EVENT;   // event code
      *pBuf++ = status;

      if ( status == HCI_SUCCESS )
      {
        // copy the P256 key
        (void)MAP_osal_memcpy( &pBuf[0],  p256KeyX, LL_SC_P256_KEY_LEN/2 );
        (void)MAP_osal_memcpy( &pBuf[32], p256KeyY, LL_SC_P256_KEY_LEN/2 );
      }
      else // clear the key to avoid confusion
      {
        (void)MAP_osal_memset( pBuf, 0, LL_SC_P256_KEY_LEN );
      }

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
    else // out of heap!
    {
      HCI_HardwareErrorEvent( HCI_ERROR_CODE_MEM_CAP_EXCEEDED );
    }
  }
}
#endif // SECURE_CONNS_CFG


#if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & SECURE_CONNS_CFG)
/*******************************************************************************
 * @fn          LL_GenerateDHKeyCompleteEventCback Callback
 *
 * @brief       This LL callback is used to indicate teh Controller has
 *              completed the generation of the Diffie Hellman key.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       status - Operation status.
 * @param       dhKey  - Diffie Hellman key (32 bytes).
 *
 * @return      None.
 */
void LL_GenerateDHKeyCompleteEventCback( uint8  status,
                                         uint8 *dhKey )
{
  // check if LE Meta-Events are enabled and this event is enabled
  if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
       ((pBleEvtMask[LE_EVT_INDEX_GENERATE_DHKEY_COMPLETE] & LE_EVT_MASK_GENERATE_DHKEY_COMPLETE) == 0) )
  {
    // the event mask is not enabled for this event
    return;
  }

  // check if this is for the Host
  if ( hciSmpTaskID != 0 )
  {
    hciEvt_BLEGenDHKeyComplete_t *msg =
      (hciEvt_BLEGenDHKeyComplete_t *)MAP_osal_msg_allocate(sizeof( hciEvt_BLEGenDHKeyComplete_t ));

    if( msg )
    {
      // message header
      msg->hdr.event  = HCI_SMP_META_EVENT_EVENT;
      msg->hdr.status = HCI_LE_EVENT_CODE;

      // event packet
      msg->BLEEventCode = HCI_BLE_GENERATE_DHKEY_COMPLETE_EVENT;
      msg->status       = status;

      if ( status == HCI_SUCCESS )
      {
        // copy the DH key
        (void)MAP_osal_memcpy( &msg->dhKey, dhKey, LL_SC_DHKEY_LEN );
      }
      else // clear the key to avoid confusion
      {
        (void)MAP_osal_memset( &msg->dhKey, 0, LL_SC_DHKEY_LEN );
      }

      // send the message
      (void)MAP_osal_msg_send( hciSmpTaskID, (uint8 *)msg );
    }
  }
  else // Host not present
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // data length
    dataLength = HCI_GENERATE_DHKEY_COMPLETE_EVENT_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof(hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 *pBuf;

      // message type
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // point to the byte following the hciPacket_t structure
      msg->pData = (uint8*)(msg+1);
      pBuf       = msg->pData;

      *pBuf++ = HCI_EVENT_PACKET;
      *pBuf++ = HCI_LE_EVENT_CODE;
      *pBuf++ = dataLength;

      // populate event
      *pBuf++ = HCI_BLE_GENERATE_DHKEY_COMPLETE_EVENT;   // event code
      *pBuf++ = status;

      if ( status == HCI_SUCCESS )
      {
        // copy the DH key
        (void)MAP_osal_memcpy( pBuf, dhKey, LL_SC_DHKEY_LEN );
      }
      else // clear the key to avoid confusion
      {
        (void)MAP_osal_memset( pBuf, 0, LL_SC_DHKEY_LEN );
      }

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
    else // out of heap!
    {
      HCI_HardwareErrorEvent( HCI_ERROR_CODE_MEM_CAP_EXCEEDED );
    }
  }
}
#endif // SECURE_CONNS_CFG


/*******************************************************************************
 * @fn          LL_EnhancedConnectionCompleteCback Callback
 *
 * @brief       This LL callback is used to generate an Enhanced Connection
 *              Complete meta event when a connection is established by either
 *              an Advertiser or an Initiator.
 *
 * input parameters
 *
 * @param       reasonCode    - Status of connection complete.
 * @param       connHandle    - Connection handle.
 * @param       role          - Connection formed as Master or Slave.
 * @param       peerAddrType  - Peer address as Public or Random.
 * @param       peerAddr      - Pointer to peer device address.
 * @param       localRPA      - Pointer to local RPA.
 * @param       peerRPA       - Pointer to peer RPA.
 * @param       connInterval  - Connection interval.
 * @param       slaveLatency  - Slave latency.
 * @param       connTimeout   - Connection timeout.
 * @param       clockAccuracy - Sleep clock accuracy (from Master only).
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_EnhancedConnectionCompleteCback( uint8   reasonCode,
                                         uint16  connHandle,
                                         uint8   role,
                                         uint8   peerAddrType,
                                         uint8  *peerAddr,
                                         uint8  *localRPA,
                                         uint8  *peerRPA,
                                         uint16  connInterval,
                                         uint16  slaveLatency,
                                         uint16  connTimeout,
                                         uint8   clockAccuracy )
{
  // check if LE Meta-Events are enabled and this event is enabled
  if ( !(pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) ||
       (!(pBleEvtMask[LE_EVT_INDEX_CONN_COMPLETE] & LE_EVT_MASK_CONN_COMPLETE) &&
        !(pBleEvtMask[LE_EVT_INDEX_ENH_CONN_COMPLETE] & LE_EVT_MASK_ENH_CONN_COMPLETE)) )
  {
    // the event mask is not enabled for this event
    return;
  }

  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_BLEEnhConnComplete_t *pkt;

    pkt = (hciEvt_BLEEnhConnComplete_t *)MAP_osal_msg_allocate( sizeof(hciEvt_BLEEnhConnComplete_t) );

    if ( pkt )
    {
      pkt->hdr.event  = HCI_GAP_EVENT_EVENT;
      pkt->hdr.status = HCI_LE_EVENT_CODE;

      if ( reasonCode == LL_STATUS_SUCCESS )
      {
        pkt->status = HCI_SUCCESS;
        (void)MAP_osal_memcpy( pkt->peerAddr, peerAddr, B_ADDR_LEN );
      }
      else
      {
        pkt->status = bleGAPConnNotAcceptable;
        (void)MAP_osal_memset( pkt->peerAddr, 0, B_ADDR_LEN );
      }

      if ( pBleEvtMask[LE_EVT_INDEX_ENH_CONN_COMPLETE] & LE_EVT_MASK_ENH_CONN_COMPLETE )
      {
        pkt->BLEEventCode = HCI_BLE_ENHANCED_CONNECTION_COMPLETE_EVENT;

        // local and peer RPA
        (localRPA != NULL) ? MAP_osal_memcpy( pkt->localRPA, localRPA, B_ADDR_LEN ) :
                             MAP_osal_memset( pkt->localRPA, 0, B_ADDR_LEN );

        (peerRPA != NULL)  ? MAP_osal_memcpy( pkt->peerRPA, peerRPA, B_ADDR_LEN ) :
                             MAP_osal_memset( pkt->peerRPA, 0, B_ADDR_LEN );

        pkt->peerAddrType = peerAddrType;
      }
      else // LE_EVT_MASK_CONN_COMPLETE
      {
        pkt->BLEEventCode = HCI_BLE_CONNECTION_COMPLETE_EVENT;
        MAP_osal_memset( pkt->localRPA, 0, B_ADDR_LEN );
        MAP_osal_memset( pkt->peerRPA, 0, B_ADDR_LEN );
        pkt->peerAddrType = peerAddrType & LL_DEV_ADDR_TYPE_MASK;
      }

      pkt->connectionHandle = connHandle;
      pkt->role             = role;
      pkt->connInterval     = connInterval;
      pkt->connLatency      = slaveLatency;
      pkt->connTimeout      = connTimeout;
      pkt->clockAccuracy    = clockAccuracy;

      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // data length
    if ( pBleEvtMask[LE_EVT_INDEX_ENH_CONN_COMPLETE] & LE_EVT_MASK_ENH_CONN_COMPLETE )
    {
      dataLength = HCI_ENH_CONNECTION_COMPLETE_EVENT_LEN;
    }
    else // LE_EVT_MASK_CONN_COMPLETE
    {
      dataLength = HCI_CONNECTION_COMPLETE_EVENT_LEN;
    }

    // OSAL message header + HCI event header + data
    totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 i = 0;

      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData    = (uint8*)(msg+1);
      msg->pData[i++] = HCI_EVENT_PACKET;
      msg->pData[i++] = HCI_LE_EVENT_CODE;
      msg->pData[i++] = dataLength;

      // populate event code
      if ( pBleEvtMask[LE_EVT_INDEX_ENH_CONN_COMPLETE] & LE_EVT_MASK_ENH_CONN_COMPLETE )
      {
        msg->pData[i++] = HCI_BLE_ENHANCED_CONNECTION_COMPLETE_EVENT;
      }
      else // LE_EVT_MASK_CONN_COMPLETE
      {
        msg->pData[i++] = HCI_BLE_CONNECTION_COMPLETE_EVENT;
      }

      msg->pData[i++] = reasonCode;                                 // reason code
      msg->pData[i++] = LO_UINT16 (connHandle);                     // connection handle (LSB)
      msg->pData[i++] = HI_UINT16 (connHandle);                     // connection handle (MSB)
      msg->pData[i++] = role;                                       // role (master/slave)

      if ( pBleEvtMask[LE_EVT_INDEX_ENH_CONN_COMPLETE] & LE_EVT_MASK_ENH_CONN_COMPLETE )
      {
        msg->pData[i++] = peerAddrType;                               // address type
      }
      else // Connection Complete
      {
        msg->pData[i++] = peerAddrType & LL_DEV_ADDR_TYPE_MASK;       // address type
      }

      // copy address
      (peerAddr != NULL) ? MAP_osal_memcpy( &msg->pData[i], peerAddr, B_ADDR_LEN ) :
      MAP_osal_memset( &msg->pData[i], 0, B_ADDR_LEN );

      i += B_ADDR_LEN;

      // populate event code
      if ( pBleEvtMask[LE_EVT_INDEX_ENH_CONN_COMPLETE] & LE_EVT_MASK_ENH_CONN_COMPLETE )
      {
        // local and peer RPA
        (localRPA != NULL) ? MAP_osal_memcpy( &msg->pData[i], localRPA, B_ADDR_LEN ) :
                             MAP_osal_memset( &msg->pData[i], 0, B_ADDR_LEN );

        i += B_ADDR_LEN;

        (peerRPA != NULL)  ? MAP_osal_memcpy( &msg->pData[i], peerRPA, B_ADDR_LEN ) :
                             MAP_osal_memset( &msg->pData[i], 0, B_ADDR_LEN );

        i += B_ADDR_LEN;
      }

      msg->pData[i++] = LO_UINT16 (connInterval);                  // connection interval (LSB)
      msg->pData[i++] = HI_UINT16 (connInterval);                  // connection interval (MSB)
      msg->pData[i++] = LO_UINT16 (slaveLatency);                  // slave latency (LSB)
      msg->pData[i++] = HI_UINT16 (slaveLatency);                  // slave latency (LSB)
      msg->pData[i++] = LO_UINT16 (connTimeout);                   // connectin timeout (LSB)
      msg->pData[i++] = HI_UINT16 (connTimeout);                   // connection timeout (MSB)
      msg->pData[i++] = clockAccuracy;                             // clock accuracy

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}


/*
** LL Callbacks for LE Meta-Events
*/

/*******************************************************************************
 * @fn          LL_AdvReportCback Callback
 *
 * @brief       This LL callback is used to generate a Advertisment Report meta
 *              event when an Advertisment or Scan Response is received by a
 *              Scanner.
 *
 * input parameters
 *
 * @param       advEvt      - Advertise event type, or Scan Response event type.
 * @param       advAddrType - Public or Random address type.
 * @param       advAddr     - Pointer to device address.
 * @param       dataLen     - Length of data in bytes.
 * @param       advData     - Pointer to data.
 * @param       rssi        - The RSSI of received packet.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_AdvReportCback( uint8 advEvt,
                        uint8 advAddrType,
                        uint8 *advAddr,
                        uint8 dataLen,
                        uint8 *advData,
                        int8  rssi )
{
  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_BLEAdvPktReport_t *pkt;
    hciEvt_DevInfo_t *devInfo;
    uint8 x;

    pkt = (hciEvt_BLEAdvPktReport_t *)MAP_osal_msg_allocate( sizeof( hciEvt_BLEAdvPktReport_t ) +
                                                             sizeof( hciEvt_DevInfo_t ) );

    if ( pkt )
    {
      pkt->hdr.event = HCI_GAP_EVENT_EVENT;
      pkt->hdr.status = HCI_LE_EVENT_CODE;
      pkt->BLEEventCode = HCI_BLE_ADV_REPORT_EVENT;
      pkt->numDevices = 1;  // assume one device for now
      pkt->devInfo = devInfo = (hciEvt_DevInfo_t *)(pkt+1);

      for ( x = 0; x < pkt->numDevices; x++, devInfo++ )
      {
        /* Fill in the device info */
        devInfo->eventType = advEvt;
        devInfo->addrType = advAddrType;
        (void)MAP_osal_memcpy( devInfo->addr, advAddr, B_ADDR_LEN );
        devInfo->dataLen = dataLen;
        (void)MAP_osal_memcpy( devInfo->rspData, advData, dataLen );
        devInfo->rssi = rssi;
      }

      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 totalLength;
    uint8 dataLength;

    // check if LE Meta-Events are enabled and this event is enabled
    if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
         (((pBleEvtMask[LE_EVT_INDEX_ADV_REPORT] & LE_EVT_MASK_ADV_REPORT) == 0 )) )
    {
      // the event mask is not set for this event
      return;
    }

    // data length
    dataLength = HCI_ADV_REPORT_EVENT_LEN + dataLen;

    // OSAL message header + HCI event header + data
    totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData    = (uint8*)(msg+1);
      msg->pData[0] = HCI_EVENT_PACKET;
      msg->pData[1] = HCI_LE_EVENT_CODE;
      msg->pData[2] = dataLength;

      // populate event
      msg->pData[3] = HCI_BLE_ADV_REPORT_EVENT;                // event code
      msg->pData[4] = 1;                                       // number of devices; assume 1 for now
      msg->pData[5] = advEvt;                                  // advertisement event type
      msg->pData[6] = advAddrType;                             // address type
      (void)MAP_osal_memcpy (&msg->pData[7], advAddr, B_ADDR_LEN); // address
      msg->pData[13] = dataLen;                                // data length
      (void)MAP_osal_memcpy (&msg->pData[14], advData, dataLen);   // data
      msg->pData[14 + dataLen] = rssi;                         // RSSI

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}


/*******************************************************************************
 * @fn          LL_ConnectionCompleteCback Callback
 *
 * @brief       This LL callback is used to generate a Connection Complete meta
 *              event when a connection is established by either an Advertiser
 *              or an Initiator.
 *
 * input parameters
 *
 * @param       reasonCode    - Status of connection complete.
 * @param       connHandle    - Connection handle.
 * @param       role          - Connection formed as Master or Slave.
 * @param       peerAddrType  - Peer address as Public or Random.
 * @param       peerAddr      - Pointer to peer device address.
 * @param       connInterval  - Connection interval.
 * @param       slaveLatency  - Slave latency.
 * @param       connTimeout   - Connection timeout.
 * @param       clockAccuracy - Sleep clock accuracy (from Master only).
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_ConnectionCompleteCback( uint8  reasonCode,
                                 uint16 connHandle,
                                 uint8  role,
                                 uint8  peerAddrType,
                                 uint8  *peerAddr,
                                 uint16 connInterval,
                                 uint16 slaveLatency,
                                 uint16 connTimeout,
                                 uint8  clockAccuracy )
{
  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_BLEConnComplete_t *pkt;

    pkt = (hciEvt_BLEConnComplete_t *)MAP_osal_msg_allocate( sizeof( hciEvt_BLEConnComplete_t ) );
    if ( pkt )
    {
      pkt->hdr.event    = HCI_GAP_EVENT_EVENT;
      pkt->hdr.status   = HCI_LE_EVENT_CODE;
      pkt->BLEEventCode = HCI_BLE_CONNECTION_COMPLETE_EVENT;

      if ( reasonCode == LL_STATUS_SUCCESS )
      {
        pkt->status = HCI_SUCCESS;
        (void)MAP_osal_memcpy( pkt->peerAddr, peerAddr, B_ADDR_LEN );
      }
      else
      {
        pkt->status = bleGAPConnNotAcceptable;
        (void)MAP_osal_memset( pkt->peerAddr, 0, B_ADDR_LEN );
      }
      pkt->connectionHandle = connHandle;
      pkt->role             = role;
      pkt->peerAddrType     = peerAddrType;
      pkt->connInterval     = connInterval;
      pkt->connLatency      = slaveLatency;
      pkt->connTimeout      = connTimeout;
      pkt->clockAccuracy    = clockAccuracy;

      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // check if LE Meta-Events are enabled and this event is enabled
    if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
         (((pBleEvtMask[LE_EVT_INDEX_CONN_COMPLETE] & LE_EVT_MASK_CONN_COMPLETE) == 0 )) )
    {
      // the event mask is not enabled for this event
      return;
    }

    // data length
    dataLength = HCI_CONNECTION_COMPLETE_EVENT_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData    = (uint8*)(msg+1);
      msg->pData[0] = HCI_EVENT_PACKET;
      msg->pData[1] = HCI_LE_EVENT_CODE;
      msg->pData[2] = dataLength;

      // populate event
      msg->pData[3] = HCI_BLE_CONNECTION_COMPLETE_EVENT;          // event code
      msg->pData[4] = reasonCode;                                 // reason code
      msg->pData[5] = LO_UINT16 (connHandle);                     // connection handle (LSB)
      msg->pData[6] = HI_UINT16 (connHandle);                     // connection handle (MSB)
      msg->pData[7] = role;                                       // role (master/slave)
      msg->pData[8] = peerAddrType;                               // address type

      // copy address
      (void)MAP_osal_memcpy (&msg->pData[9], peerAddr, B_ADDR_LEN);

      msg->pData[15] = LO_UINT16 (connInterval);                  // connection interval (LSB)
      msg->pData[16] = HI_UINT16 (connInterval);                  // connection interval (MSB)
      msg->pData[17] = LO_UINT16 (slaveLatency);                  // slave latency (LSB)
      msg->pData[18] = HI_UINT16 (slaveLatency);                  // slave latency (LSB)
      msg->pData[19] = LO_UINT16 (connTimeout);                   // connectin timeout (LSB)
      msg->pData[20] = HI_UINT16 (connTimeout);                   // connection timeout (MSB)
      msg->pData[21] = clockAccuracy;                             // clock accuracy

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}


/*******************************************************************************
 * @fn          LL_DisconnectCback Callback
 *
 * @brief       This LL callback is used to generate a Disconnect Complete meta
 *              event when a connection is disconnected by either a Master or
 *              a Slave.
 *
 * input parameters
 *
 * @param       connHandle - Connection handle.
 * @param       reasonCode - Status of connection complete.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_DisconnectCback( uint16 connHandle,
                         uint8  reasonCode )
{
  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_DisconnComplete_t *pkt;

    pkt = (hciEvt_DisconnComplete_t *)MAP_osal_msg_allocate( sizeof( hciEvt_DisconnComplete_t ) );
    if ( pkt )
    {
      pkt->hdr.event  = HCI_GAP_EVENT_EVENT;
      pkt->hdr.status = HCI_DISCONNECTION_COMPLETE_EVENT_CODE;
      pkt->status     = HCI_SUCCESS;
      pkt->connHandle = connHandle;
      pkt->reason     = reasonCode;

      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // check the event mask
    if( ( BT_EVT_MASK_DISCONNECTION_COMPLETE &
          pHciEvtMask[BT_EVT_INDEX_DISCONNECT_COMPLETE]) == 0 )
    {
      // event mask is not set for this event, do not send to host
      return;
    }

    // data length
    dataLength = HCI_DISCONNECTION_COMPLETE_LEN;

    // the length will be OSAL message header + HCI event header + data
    totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData    = (uint8*)(msg+1);
      msg->pData[0] = HCI_EVENT_PACKET;
      msg->pData[1] = HCI_DISCONNECTION_COMPLETE_EVENT_CODE;
      msg->pData[2] = dataLength;

      msg->pData[3] = HCI_SUCCESS;
      msg->pData[4] = LO_UINT16(connHandle);      // connection handle (LSB)
      msg->pData[5] = HI_UINT16(connHandle);      // connection handle (MSB)
      msg->pData[6] = reasonCode;                 // reason code

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}


/*******************************************************************************
 * @fn          LL_ConnParamUpdateCback Callback
 *
 * @brief       This LL callback is used to generate a Connection Update
 *              Complete meta event when a connection's parameters are updated
 *              by the Master, or if an error occurs (e.g. during the
 *              Connection Parameter Request control procedure).
 *
 * input parameters
 *
 * @param       status       - Status of update complete event.
 * @param       connHandle   - Connection handle.
 * @param       connInterval - Connection interval.
 * @param       slaveLatency - Slave latency.
 * @param       connTimeout  - Connection timeout.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
void LL_ConnParamUpdateCback( llStatus_t status,
#else // CONN_PARAM_REQ_CFG
void LL_ConnParamUpdateCback(
#endif // CONN_PARAM_REQ_CFG
                              uint16     connHandle,
                              uint16     connInterval,
                              uint16     connLatency,
                              uint16     connTimeout )
{
  // check if LE Meta-Events are enabled and this event is enabled
  if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
       (((pBleEvtMask[LE_EVT_INDEX_CONN_UPDATE_COMPLETE] & LE_EVT_MASK_CONN_UPDATE_COMPLETE) == 0 )) )
  {
    // the event mask is not enabled for this event
    return;
  }

  // check if this is for the Host
  if ( hciGapTaskID != 0 )
  {
    hciEvt_BLEConnUpdateComplete_t *msg;
    uint8 totalLength;

    totalLength = sizeof( hciEvt_BLEConnUpdateComplete_t );

    msg = (hciEvt_BLEConnUpdateComplete_t *)MAP_osal_msg_allocate(totalLength);

    if( msg )
    {
      // message header
      msg->hdr.event  = HCI_GAP_EVENT_EVENT;
      msg->hdr.status = HCI_LE_EVENT_CODE; // use status field to pass the HCI Event code

      // event packet
      msg->BLEEventCode     = HCI_BLE_CONN_UPDATE_COMPLETE_EVENT;
#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
      msg->status           = status;
#else // !CONN_PARAM_REQ_CFG
      msg->status           = HCI_SUCCESS;
#endif // CONN_PARAM_REQ_CFG
      msg->connectionHandle = connHandle;
      msg->connInterval     = connInterval;
      msg->connLatency      = connLatency;
      msg->connTimeout      = connTimeout;

      // send the message
      (void)MAP_osal_msg_send( hciGapTaskID, (uint8 *)msg );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // data length
    dataLength = HCI_CONN_UPDATE_COMPLETE_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 *pBuf;

      // message type
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // point to the byte following the hciPacket_t structure
      msg->pData = (uint8*)(msg+1);
      pBuf       = msg->pData;

      *pBuf++ = HCI_EVENT_PACKET;
      *pBuf++ = HCI_LE_EVENT_CODE;
      *pBuf++ = dataLength;

      // populate event
      *pBuf++ = HCI_BLE_CONN_UPDATE_COMPLETE_EVENT;  // event code
#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
      *pBuf++ = status;                              // status
#else // !CONN_PARAM_REQ_CFG
      *pBuf++ = HCI_SUCCESS;                         // status
#endif // CONN_PARAM_REQ_CFG
      *pBuf++ = LO_UINT16(connHandle);               // connection handle (LSB)
      *pBuf++ = HI_UINT16(connHandle);               // connection handle (MSB)
      *pBuf++ = LO_UINT16(connInterval);             // connection interval (LSB)
      *pBuf++ = HI_UINT16(connInterval);             // connection interval (MSB)
      *pBuf++ = LO_UINT16(connLatency);              // slave latency (LSB)
      *pBuf++ = HI_UINT16(connLatency);              // slave latency (MSB)
      *pBuf++ = LO_UINT16(connTimeout);              // connection timeout (LSB)
      *pBuf++ = HI_UINT16(connTimeout);              // connection timeout (MSB)

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}


#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
/*******************************************************************************
 * @fn          LL_ReadRemoteUsedFeaturesCompleteCback Callback
 *
 * @brief       This LL callback is used to generate a Read Remote Used Features
 *              Complete meta event when a Master makes this request of a Slave.
 *
 * input parameters
 *
 * @param       status     - HCI status.
 * @param       connHandle - Connection handle.
 * @param       featureSet - Pointer to eight byte bit mask of LE features.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_ReadRemoteUsedFeaturesCompleteCback( hciStatus_t  status,
                                             uint16       connHandle,
                                             uint8       *featureSet )
{
  hciPacket_t *msg;
  uint8 dataLength;
  uint8 totalLength;

  // check if LE Meta-Events are enabled and this event is enabled
  if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
       (((pBleEvtMask[LE_EVT_INDEX_READ_REMOTE_FEATURE] & LE_EVT_MASK_READ_REMOTE_FEATURE) == 0 )) )
  {
    // the event mask is not enabled for this event
    return;
  }

  // data length
  dataLength = HCI_READ_REMOTE_FEATURE_COMPLETE_EVENT_LEN;

  // OSAL message header + HCI event header + data
  totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

  msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

  if (msg)
  {
    // message type, length
    msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
    msg->hdr.status = 0xFF;

    // create message
    msg->pData    = (uint8*)(msg+1);
    msg->pData[0] = HCI_EVENT_PACKET;
    msg->pData[1] = HCI_LE_EVENT_CODE;
    msg->pData[2] = dataLength;

    // event code
    msg->pData[3] = HCI_BLE_READ_REMOTE_FEATURE_COMPLETE_EVENT;
    msg->pData[4] = status;
    msg->pData[5] = LO_UINT16(connHandle);  // connection handle (LSB)
    msg->pData[6] = HI_UINT16(connHandle);  // connection handle (MSB)

    // feature set
    (void)MAP_osal_memcpy (&msg->pData[7], featureSet, B_FEATURE_SUPPORT_LENGTH);

    // send the message
    (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
  }
}
#endif // ADV_CONN_CFG | INIT_CFG


/*******************************************************************************
 * @fn          LL_ReadRemoteVersionInfoCback Callback
 *
 * @brief       This LL callback is used to generate a Read Remote Version
 *              Information Complete meta event when a Master makes this request
 *              of a Slave.
 *
 * input parameters
 *
 * @param       status     - Status of callback.
 * @param       verNum     - Version of the Bluetooth Controller specification.
 * @param       connHandle - Company identifier of the manufacturer of the
 *                           Bluetooth Controller.
 * @param       subverNum  - A unique value for each implementation or revision
 *                           of an implementation of the Bluetooth Controller.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_ReadRemoteVersionInfoCback( hciStatus_t status,
                                    uint16      connHandle,
                                    uint8       verNum,
                                    uint16      comId,
                                    uint16      subverNum )
{
  hciPacket_t *msg;
  uint8 dataLength;
  uint8 totalLength;

  // check the event mask
  if( ( BT_EVT_MASK_READ_REMOTE_VERSION_INFORMATION_COMPLETE &
        pHciEvtMask[BT_EVT_INDEX_READ_REMOTE_VERSION_INFO]) == 0 )
  {
    // event mask is not set for this event, do not send to host
    return;
  }

  // data length
  dataLength = HCI_REMOTE_VERSION_INFO_EVENT_LEN;

  // OSAL message header + HCI event header + data
  totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

  msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

  if (msg)
  {
    // message type, length
    msg->hdr.event   = HCI_CTRL_TO_HOST_EVENT;
    msg->hdr.status = 0xFF;

    // create message
    msg->pData     = (uint8*)(msg+1);
    msg->pData[0]  = HCI_EVENT_PACKET;
    msg->pData[1]  = HCI_READ_REMOTE_INFO_COMPLETE_EVENT_CODE;
    msg->pData[2]  = dataLength;
    msg->pData[3]  = status;
    msg->pData[4]  = LO_UINT16( connHandle );
    msg->pData[5]  = HI_UINT16( connHandle );
    msg->pData[6]  = verNum;
    msg->pData[7]  = LO_UINT16( comId );       // company ID (LSB)
    msg->pData[8]  = HI_UINT16( comId );       // company ID (MSB)
    msg->pData[9]  = LO_UINT16( subverNum );
    msg->pData[10] = HI_UINT16( subverNum );

    // send the message
    (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
  }
}


/*******************************************************************************
 * @fn          LL_EncLtkReqCback Callback
 *
 * @brief       This LL callback is used to generate a Encryption LTK Request
 *              meta event to provide to the Host the Master's random number
 *              and encryption diversifier, and to request the Host's Long Term
 *              Key (LTK).
 *
 * input parameters
 *
 * @param       connHandle - The LL connection ID for new connection.
 * @param       randNum    - Random vector used in device identification.
 * @param       encDiv     - Encrypted diversifier.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_EncLtkReqCback( uint16  connHandle,
                        uint8  *randNum,
                        uint8  *encDiv )
{
  // check if this is for the Host
  if ( hciSmpTaskID != 0 )
  {
    hciEvt_BLELTKReq_t *pkt;

    pkt = (hciEvt_BLELTKReq_t *)MAP_osal_msg_allocate( sizeof( hciEvt_BLELTKReq_t ) );
    if ( pkt )
    {
      pkt->hdr.event    = HCI_SMP_EVENT_EVENT;
      pkt->hdr.status   = HCI_LE_EVENT_CODE;
      pkt->BLEEventCode = HCI_BLE_LTK_REQUESTED_EVENT;
      pkt->connHandle   = connHandle;

      (void)MAP_osal_memcpy( pkt->random, randNum, B_RANDOM_NUM_SIZE );
      pkt->encryptedDiversifier = BUILD_UINT16( encDiv[0], encDiv[1] );

      (void)MAP_osal_msg_send( hciSmpTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // check if LE Meta-Events are enabled and this event is enabled
    if ( ((pHciEvtMask[BT_EVT_INDEX_LE_META_EVENT] & BT_EVT_MASK_LE_META_EVENT) == 0) ||
         (((pBleEvtMask[LE_EVT_INDEX_LTK_REQUEST] & LE_EVT_MASK_LTK_REQUEST) == 0 )) )
    {
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
      // the event mask is not set for this event, so reject immediately
      MAP_LL_EncLtkNegReply( connHandle );
#endif // CTRL_CONFIG=ADV_CONN_CFG

      return;
    }

    // data length
    dataLength = HCI_LTK_REQUESTED_EVENT_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 *pBuf;

      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData = (uint8*)(msg+1);
      pBuf       = msg->pData;

      *pBuf++ = HCI_EVENT_PACKET;
      *pBuf++ = HCI_LE_EVENT_CODE;
      *pBuf++ = dataLength;
      *pBuf++ = HCI_BLE_LTK_REQUESTED_EVENT;                 // event code
      *pBuf++ = LO_UINT16(connHandle);                       // connection handle (LSB)
      *pBuf++ = HI_UINT16(connHandle);                       // connection handle (MSB)
      (void)MAP_osal_memcpy (pBuf, randNum, B_RANDOM_NUM_SIZE);  // random number
      pBuf += B_RANDOM_NUM_SIZE;                             // size of random number
      *pBuf++ = *encDiv++;                                   // encryption diversifier (LSB)
      *pBuf   = *encDiv;                                     // encryption diversifier (MSB)

      /* Send the message */
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}


#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
/*******************************************************************************
 * @fn          LL_EncChangeCback Callback
 *
 * @brief       This Callback is used by the LL to indicate to the Host that
 *              an encryption change has taken place. This results when
 *              the host performs a LL_StartEncrypt when encryption is not
 *              already enabled.
 *
 *              Note: If the key request was rejected, then encryption will
 *                    remain off.
 *
 * input parameters
 *
 * @param       connHandle - The LL connection ID for new connection.
 * @param       reason     - LL_ENC_KEY_REQ_ACCEPTED or LL_ENC_KEY_REQ_REJECTED.
 * @param       encEnab    - LL_ENCRYPTION_OFF or LL_ENCRYPTION_ON.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_EncChangeCback( uint16 connHandle,
                        uint8  reason,
                        uint8  encEnab )
{
  // check if this is for the Host
  if ( hciSmpTaskID != 0 )
  {
    hciEvt_EncryptChange_t *pkt;

    pkt = (hciEvt_EncryptChange_t *)MAP_osal_msg_allocate( sizeof( hciEvt_EncryptChange_t ) );
    if ( pkt )
    {
      pkt->hdr.event    = HCI_SMP_EVENT_EVENT;
      pkt->hdr.status   = HCI_LE_EVENT_CODE;
      pkt->BLEEventCode = HCI_ENCRYPTION_CHANGE_EVENT_CODE;
      pkt->connHandle   = connHandle;
      pkt->reason       = reason;
      pkt->encEnable    = encEnab;

      (void)MAP_osal_msg_send( hciSmpTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // check the event mask
    if( ( BT_EVT_MASK_ENCRYPTION_CHANGE &
          pHciEvtMask[BT_EVT_INDEX_ENCRYPTION_CHANGE]) == 0 )
    {
      // event mask is not set for this event, do not send to host
      return;
    }

    // data length
    dataLength = HCI_ENCRYPTION_CHANGE_EVENT_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 *pBuf;

      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData = (uint8*)(msg+1);
      pBuf       = msg->pData;

      // populate event
      *pBuf++    = HCI_EVENT_PACKET;
      *pBuf++    = HCI_ENCRYPTION_CHANGE_EVENT_CODE;
      *pBuf++    = dataLength;
      *pBuf++    = reason;
      *pBuf++    = LO_UINT16(connHandle);
      *pBuf++    = HI_UINT16(connHandle);
      *pBuf      = encEnab;

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}
#endif // ADV_CONN_CFG | INIT_CFG


#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
/*******************************************************************************
 * @fn          LL_EncKeyRefreshCback Callback
 *
 * @brief       This Callback is used by the LL to indicate to the Host that
 *              an encryption key refresh has taken place. This results when
 *              the host performs a LL_StartEncrypt when encryption is already
 *              enabled.
 *
 * input parameters
 *
 * @param       connHandle - The LL connection ID for new connection.
 * @param       reason    - LL_ENC_KEY_REQ_ACCEPTED, LL_CTRL_PKT_TIMEOUT_TERM
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void LL_EncKeyRefreshCback( uint16 connHandle,
                            uint8  reason )
{
  // check if this is for the Host
  if ( hciSmpTaskID != 0 )
  {
    hciEvt_EncryptChange_t *pkt;

    pkt = (hciEvt_EncryptChange_t *)MAP_osal_msg_allocate( sizeof( hciEvt_EncryptChange_t ) );
    if ( pkt )
    {
      pkt->hdr.event    = HCI_SMP_EVENT_EVENT;
      pkt->hdr.status   = HCI_LE_EVENT_CODE;
      pkt->BLEEventCode = HCI_ENCRYPTION_CHANGE_EVENT_CODE;
      pkt->connHandle   = connHandle;
      pkt->reason       = reason;
      pkt->encEnable    = TRUE;

      (void)MAP_osal_msg_send( hciSmpTaskID, (uint8 *)pkt );
    }
  }
  else
  {
    hciPacket_t *msg;
    uint8 dataLength;
    uint8 totalLength;

    // check the event mask
    if( ( BT_EVT_MASK_ENCRYPTION_KEY_REFRESH_COMPLETE &
          pHciEvtMask[BT_EVT_INDEX_KEY_REFRESH_COMPLETE]) == 0 )
    {
      /* Event mask is not set for this event, do not send to host */
      return;
    }

    // data length
    dataLength = HCI_KEY_REFRESH_COMPLETE_EVENT_LEN;

    // OSAL message header + HCI event header + data
    totalLength = sizeof( hciPacket_t ) + HCI_EVENT_MIN_LENGTH + dataLength;

    msg = (hciPacket_t *)MAP_osal_msg_allocate(totalLength);

    if (msg)
    {
      uint8 *pBuf;

      // message type, length
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;

      // create message
      msg->pData = (uint8*)(msg+1);
      pBuf       = msg->pData;

      // populate event
      *pBuf++    = HCI_EVENT_PACKET;
      *pBuf++    = HCI_KEY_REFRESH_COMPLETE_EVENT_CODE;
      *pBuf++    = dataLength;
      *pBuf++    = reason;
      *pBuf++    = LO_UINT16(connHandle);
      *pBuf++    = HI_UINT16(connHandle);

      // send the message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
  }
}
#endif // ADV_CONN_CFG | INIT_CFG


/*******************************************************************************
 * @fn          LL_DirectTestEndDone Callback
 *
 * @brief       This Callback is used by the LL to notify the HCI that the
 *              Direct Test End command has completed.
 *
 *
 * input parameters
 *
 * @param       numPackets - The number of packets received. Zero for transmit.
 * @param       mode       - LL_DIRECT_TEST_MODE_TX or LL_DIRECT_TEST_MODE_RX.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      LL_STATUS_SUCCESS
 */
void LL_DirectTestEndDoneCback( uint16 numPackets,
                                uint8  mode )
{
  uint8 rtnParam[3];

  // unused input parameter; PC-Lint error 715.
  (void)mode;

  rtnParam[0] = HCI_SUCCESS;

  rtnParam[1] = LO_UINT16( numPackets );
  rtnParam[2] = HI_UINT16( numPackets );

  MAP_HCI_CommandCompleteEvent( HCI_LE_TEST_END, 3, rtnParam );
}


/*******************************************************************************
 */


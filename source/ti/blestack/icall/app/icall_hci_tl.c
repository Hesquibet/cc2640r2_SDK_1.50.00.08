/******************************************************************************

 @file       icall_hci_tl.c

 @brief This file contains the HCI TL implementation to transpose HCI serial
        packets into HCI function calls to the Stack.

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

/*********************************************************************
 * INCLUDES
 */

#include <string.h>

#include "comdef.h"

#include <icall.h>
#include "util.h"
/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include "icall_hci_tl.h"

/*********************************************************************
 * MACROS
 */

/**
 * 32 bits of data are used to describe the arguments of each HCI command.
 * a total of 8 arguments are allowed and the bits are divided equally between
 * each argument, thus allocating 4 bits to describe all possible rules for
 * interpreting the serialized arguments of an HCI buffer into a parameterized
 * list for calling the variadic function macro which maps a serial packet's
 * opcode to the corresponding HCI function in the Stack. 4 bits allow for 16
 * different rules, with the used bit permutations described below.
 *
 * Multi-octet values are little-endian unless otherwise specified.
 *
 * Value  | Rule
 *********|*********************************************************************
 * 0b0000 | No Parameter, signifies end of parsing
 * 0b0001 | 1 byte parameter
 * 0b0010 | 2 byte parameter
 * 0b0011 | 4 byte parameter (not used)
 * 0b0100 | reserved
 * 0b0101 | pointer to a single byte parameter
 * 0b0110 | pointer to 2 byte parameter
 * 0b0111 | reserved
 * 0b1000 | reserved
 * 0b1001 | 1 byte parameter implying the len (in units specified by the rule)
 *          of upcoming buffer(s)
 * 0b1010 | reserved
 * 0b1011 | reserved
 * 0b1100 | pointer to offset 0 of a 2 byte buffer
 * 0b1101 | pointer to offset 0 of a 6 byte buffer (BLE Address)
 * 0b1110 | pointer to offset 0 of a 8 byte buffer
 * 0b1111 | pointer to offset 0 of a 16 byte buffer (key buffer)
 *******************************************************************************
 *
 * Alternate meanings:
 *                     The command index field is a 16 bit field which may
 * have greater than 256 indexes, but would never require the full range.  The
 * highest order bit of this field can be used to switch the rules parameter
 * from specifying a pointer to an extended array of rules for large commands.
 */

// Number of parameters supported
#define HCI_MAX_NUM_ARGS                          12
#define HCI_DEFAULT_NUM_RULES                      8

// number of bits used to describe each parameter
#define PARAM_BIT_WIDTH                           4
#define PARAM_BIT_MASK                            0x0000000F

// Macro for condensing rules into a 32 bit value.
#define PARAM(x, i)                               ((x) << ((i)))
#define PARAM0(x)                                 PARAM(x, 0)
#define PARAM1(x)                                 PARAM(x, 1 * PARAM_BIT_WIDTH)
#define PARAM2(x)                                 PARAM(x, 2 * PARAM_BIT_WIDTH)
#define PARAM3(x)                                 PARAM(x, 3 * PARAM_BIT_WIDTH)
#define PARAM4(x)                                 PARAM(x, 4 * PARAM_BIT_WIDTH)
#define PARAM5(x)                                 PARAM(x, 5 * PARAM_BIT_WIDTH)
#define PARAM6(x)                                 PARAM(x, 6 * PARAM_BIT_WIDTH)
#define PARAM7(x)                                 PARAM(x, 7 * PARAM_BIT_WIDTH)

#define HPARAMS(x0, x1, x2, x3, x4, x5, x6, x7)   (PARAM7(x7) | \
                                                   PARAM6(x6) | \
                                                   PARAM5(x5) | \
                                                   PARAM4(x4) | \
                                                   PARAM3(x3) | \
                                                   PARAM2(x2) | \
                                                   PARAM1(x1) | \
                                                   PARAM0(x0))

#define HPARAMS_DUPLE(x0, x1)                     (PARAM1(x1) | PARAM0(x0))

#ifdef STACK_LIBRARY
#define EXTENDED_PARAMS_FLAG                      0x80000000
#else
#define EXTENDED_PARAMS_FLAG                      0x8000
#endif
#define PARAM_FLAGS                               (EXTENDED_PARAMS_FLAG)
#define EXTENDED_PARAMS_CMD_IDX_GROUP(idx)        ((EXTENDED_PARAMS_FLAG) | (idx))
#define IS_EXTENDED_PARAMS(cmdIdxGroup)           ((cmdIdxGroup) & (EXTENDED_PARAMS_FLAG))
#define GET_CMD_IDX(cmdIdxGroup)                  ((cmdIdxGroup) & ~(PARAM_FLAGS))
#define GET_RULE(pRules, arg_i)                   ((uint8_t)(((pRules)[(arg_i)/2] >> ((arg_i) % 2) * PARAM_BIT_WIDTH) & PARAM_BIT_MASK))
#define HOST_OPCODE(csg, opcode)                  ((((csg) & 0x7) << 7) | ((opcode) & 0x7F))


#ifdef STACK_LIBRARY
#define HCI_EXTENDED_ENTRY(op, idx, ruleBuffer) \
{(op), 1, ((uint32_t) idx), (uint32_t)(ruleBuffer)}
// Abstraction of a HCI translation table entry.
#define HCI_TRANSLATION_ENTRY(op, idx, r0, r1, r2, r3, r4, r5, r6, r7) \
{ (op), 0,  ((uint32_t)idx), HPARAMS(r0, r1, r2, r3, r4, r5, r6, r7) }

#else

#define HCI_EXTENDED_ENTRY(op, idx, ruleBuffer) \
{(op), EXTENDED_PARAMS_CMD_IDX_GROUP(idx), (uint32_t)(ruleBuffer)}
// Abstraction of a HCI translation table entry.
#define HCI_TRANSLATION_ENTRY(op, idx, r0, r1, r2, r3, r4, r5, r6, r7) \
{ (op), (idx), HPARAMS(r0, r1, r2, r3, r4, r5, r6, r7) }

#endif

// HCI Rule Codes.
#define HLEN                                      0x8 // buffer len type.
#define HPTR                                      0x4 // pointer type.
#define HU32                                      0x3 // 4 octets. Intentionally uses 2 bits.
#define HU16                                      0x2 // 2 octets.
#define HU8                                       0x1 // 1 octet.
#define HNP                                       0x0 // No Parameter. Encountering this rule ends the
                                                      // translation a call to the stack is made.

// Pointers.
#define HU8PTR                                    (HPTR | HU8)  // Pointer to buffer of 1 byte values.
#define HU16PTR                                   (HPTR | HU16) // Pointer to buffer of 2 byte values.

// Pointer mask.
#define HPTRMASK                                  0xB

// Buf counters.
#define HU8LEN                                    (HLEN | HU8)         // 1 byte parameter buffer length counter.
#define H2B                                       (HLEN | HPTR)        // implicit  2 byte count.
#define HAB                                       (HLEN | HPTR | HU8)  // implicit  6 byte count.
#define H8B                                       (HLEN | HPTR | HU16) // implicit  8 byte count.
#define HKB                                       (HLEN | HPTR | HU32) // implicit 16 byte count.

#define HCI_TL_LE_CREATE_CONN_NUM_PARAMS          12

#define RSP_PAYLOAD_IDX                           6
#define MAX_RSP_DATA_LEN                          50
#define MAX_RSP_BUF                               (RSP_PAYLOAD_IDX + MAX_RSP_DATA_LEN)

#if !defined(HCI_EXT_APP_OUT_BUF)
  #define HCI_EXT_APP_OUT_BUF                     44
#endif // !HCI_EXT_APP_OUT_BUF

#define KEYDIST_SENC                              0x01
#define KEYDIST_SID                               0x02
#define KEYDIST_SSIGN                             0x04
#define KEYDIST_SLINK                             0x08
#define KEYDIST_MENC                              0x10
#define KEYDIST_MID                               0x20
#define KEYDIST_MSIGN                             0x40
#define KEYDIST_MLINK                             0x80

// Maximum number of reliable writes supported by Attribute Client
#define GATT_MAX_NUM_RELIABLE_WRITES              5

#if !defined(STACK_REVISION)
  #define STACK_REVISION                          0x030100
#endif // STACK_REVISION

#define PROXY_ID(id) (ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, (id)))

/*********************************************************************
 * TYPEDEFS
 */
#ifdef STACK_LIBRARY
typedef struct
{
  uint16_t opcode;
  uint8_t  extFlag;
  uint32_t cmdIdx;
  uint32_t paramRules;
} hciTranslatorEntry_t;
#define IDX_CAST (uint32_t)

#else
typedef struct
{
  uint16_t opcode;
  uint16_t cmdIdx;
  uint32_t paramRules;
} hciTranslatorEntry_t;
#define IDX_CAST
#endif

typedef const hciTranslatorEntry_t hciEntry_t;

typedef uint32_t hci_arg_t;

typedef struct
{
  uint8_t  pktType;
  uint16_t opCode;
  uint8_t  len;
  uint8_t  *pData;
} hciExtCmd_t;

// Structure containing the parameter for the passcode Event callback
typedef struct
{
  uint8_t connectionHandle;
} passcodeEvtCallback_t;

/*********************************************************************
 * EXTERNS
 */

extern uint32 lastAppOpcodeIdxSent;
extern const uint16_t ll_buildRevision;

/*********************************************************************
 * LOCAL VARIABLES
 */

#if defined(HCI_TL_FULL) || defined(HCI_TL_PTM)
#if defined(GAP_BOND_MGR)
uint32_t host_tl_defaultPasscode;
// Passcode.
static void host_tl_passcodeCB(uint8_t *deviceAddr, uint16_t connectionHandle,
                                     uint8_t uiInputs, uint8_t uiOutputs);
// Bond Manager Callbacks
static const gapBondCBs_t host_tl_bondCB =
{
  (pfnPasscodeCB_t)host_tl_passcodeCB, //  Passcode callback
  NULL                  //  Pairing state callback
};
#endif //GAP_BOND_MGR
#endif // (HCI_TL_FULL || HCI_TL_PTM)

#if defined(HCI_TL_FULL)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
static uint8_t hci_tl_le_create_conn_params[] =
{
  HCI_TL_LE_CREATE_CONN_NUM_PARAMS,
  HPARAMS_DUPLE(HU16, HU16),
  HPARAMS_DUPLE(HU8, HU8),
  HPARAMS_DUPLE(HAB, HU8),
  HPARAMS_DUPLE(HU16, HU16),
  HPARAMS_DUPLE(HU16, HU16),
  HPARAMS_DUPLE(HU16, HU16)
};
#endif /* defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG) */

static hciEntry_t hciTranslationTable[] =
{
  // Opcode                                                            Command function index                      Param 0        1        2        3        4        5        6        7
  HCI_TRANSLATION_ENTRY(HCI_READ_RSSI,                                 IDX_CAST IDX_HCI_ReadRssiCmd,                              HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_SET_EVENT_MASK,                            IDX_CAST IDX_HCI_SetEventMaskCmd,                          HU8PTR,  HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_SET_EVENT_MASK_PAGE_2,                     IDX_CAST IDX_HCI_SetEventMaskPage2Cmd,                     HU8PTR,  HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_RESET,                                     IDX_CAST IDX_HCI_ResetCmd,                                 HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_READ_LOCAL_VERSION_INFO,                   IDX_CAST IDX_HCI_ReadLocalVersionInfoCmd,                  HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_READ_LOCAL_SUPPORTED_COMMANDS,             IDX_CAST IDX_HCI_ReadLocalSupportedCommandsCmd,            HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_READ_LOCAL_SUPPORTED_FEATURES,             IDX_CAST IDX_HCI_ReadLocalSupportedFeaturesCmd,            HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_READ_BDADDR,                               IDX_CAST IDX_HCI_ReadBDADDRCmd,                            HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_RECEIVER_TEST,                          IDX_CAST IDX_HCI_LE_ReceiverTestCmd,                       HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_TRANSMITTER_TEST,                       IDX_CAST IDX_HCI_LE_TransmitterTestCmd,                    HU8,     HU8,     HU8,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_TEST_END,                               IDX_CAST IDX_HCI_LE_TestEndCmd,                            HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_ENCRYPT,                                IDX_CAST IDX_HCI_LE_EncryptCmd,                            HKB,     HKB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_RAND,                                   IDX_CAST IDX_HCI_LE_RandCmd,                               HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_SUPPORTED_STATES,                  IDX_CAST IDX_HCI_LE_ReadSupportedStatesCmd,                HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_WHITE_LIST_SIZE,                   IDX_CAST IDX_HCI_LE_ReadWhiteListSizeCmd,                  HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_CLEAR_WHITE_LIST,                       IDX_CAST IDX_HCI_LE_ClearWhiteListCmd,                     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_ADD_WHITE_LIST,                         IDX_CAST IDX_HCI_LE_AddWhiteListCmd,                       HU8,     HAB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_REMOVE_WHITE_LIST,                      IDX_CAST IDX_HCI_LE_RemoveWhiteListCmd,                    HU8,     HAB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_EVENT_MASK,                         IDX_CAST IDX_HCI_LE_SetEventMaskCmd,                       HU8PTR,  HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_LOCAL_SUPPORTED_FEATURES,          IDX_CAST IDX_HCI_LE_ReadLocalSupportedFeaturesCmd,         HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_BUFFER_SIZE,                       IDX_CAST IDX_HCI_LE_ReadBufSizeCmd,                        HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_RANDOM_ADDR,                        IDX_CAST IDX_HCI_LE_SetRandomAddressCmd,                   HU8PTR,  HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),

#if defined(CTRL_CONFIG) && ((CTRL_CONFIG & ADV_NCONN_CFG) || (CTRL_CONFIG & ADV_CONN_CFG))
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_ADV_CHANNEL_TX_POWER,              IDX_CAST IDX_HCI_LE_ReadAdvChanTxPowerCmd,                 HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && ((CTRL_CONFIG & ADV_NCONN_CFG) || (CTRL_CONFIG & ADV_CONN_CFG)))

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  HCI_TRANSLATION_ENTRY(HCI_DISCONNECT,                                IDX_CAST IDX_HCI_DisconnectCmd,                            HU16,    HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_CHANNEL_MAP,                       IDX_CAST IDX_HCI_LE_ReadChannelMapCmd,                     HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_REMOTE_USED_FEATURES,              IDX_CAST IDX_HCI_LE_ReadRemoteUsedFeaturesCmd,             HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_READ_REMOTE_VERSION_INFO,                  IDX_CAST IDX_HCI_ReadRemoteVersionInfoCmd,                 HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_READ_TRANSMIT_POWER,                       IDX_CAST IDX_HCI_ReadTransmitPowerLevelCmd,                HU16,    HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_SET_CONTROLLER_TO_HOST_FLOW_CONTROL,       IDX_CAST IDX_HCI_SetControllerToHostFlowCtrlCmd,           HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_HOST_BUFFER_SIZE,                          IDX_CAST IDX_HCI_HostBufferSizeCmd,                        HU16,    HU8,     HU16,    HU16,    HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_HOST_NUM_COMPLETED_PACKETS,                IDX_CAST IDX_HCI_HostNumCompletedPktCmd,                   HU8LEN,  HU16PTR, HU16PTR, HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG)))

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (INIT_CFG))
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION,        IDX_CAST IDX_HCI_LE_SetHostChanClassificationCmd,          HU8PTR,  HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && (CTRL_CONFIG & (INIT_CFG)))


  // V4.2 - Extended Data Length
#if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & EXT_DATA_LEN_CFG) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_MAX_DATA_LENGTH,                   IDX_CAST IDX_HCI_LE_ReadMaxDataLenCmd,                     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_DATA_LENGTH,                        IDX_CAST IDX_HCI_LE_SetDataLenCmd,                         HU16,    HU16,    HU16,    HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH,    IDX_CAST IDX_HCI_LE_WriteSuggestedDefaultDataLenCmd,       HU16,    HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH,     IDX_CAST IDX_HCI_LE_ReadSuggestedDefaultDataLenCmd,        HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & EXT_DATA_LEN_CFG) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG)))

  // V4.1
#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  HCI_TRANSLATION_ENTRY(HCI_READ_AUTH_PAYLOAD_TIMEOUT,                 IDX_CAST IDX_HCI_ReadAuthPayloadTimeoutCmd,                HU16,    HU16PTR, HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_WRITE_AUTH_PAYLOAD_TIMEOUT,                IDX_CAST IDX_HCI_WriteAuthPayloadTimeoutCmd,               HU16,    HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_REMOTE_CONN_PARAM_REQ_REPLY,            IDX_CAST IDX_HCI_LE_RemoteConnParamReqReplyCmd,            HU16,    HU16,    HU16,    HU16,    HU16,    HU16,    HU16,    HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY,        IDX_CAST IDX_HCI_LE_RemoteConnParamReqNegReplyCmd,         HU16,    HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG)))

  // V4.2 - Privacy 1.2
#if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & PRIVACY_1_2_CFG)
  HCI_TRANSLATION_ENTRY(HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST,           IDX_CAST IDX_HCI_LE_AddDeviceToResolvingListCmd,           HU8,     HAB,     HKB,     HKB,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_REMOVE_DEVICE_FROM_RESOLVING_LIST,      IDX_CAST IDX_HCI_LE_RemoveDeviceFromResolvingListCmd,      HU8,     HAB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_CLEAR_RESOLVING_LIST,                   IDX_CAST IDX_HCI_LE_ClearResolvingListCmd,                 HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_RESOLVING_LIST_SIZE,               IDX_CAST IDX_HCI_LE_ReadResolvingListSizeCmd,              HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_PEER_RESOLVABLE_ADDRESS,           IDX_CAST IDX_HCI_LE_ReadPeerResolvableAddressCmd,          HU8,     HAB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_LOCAL_RESOLVABLE_ADDRESS,          IDX_CAST IDX_HCI_LE_ReadLocalResolvableAddressCmd,         HU8,     HAB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_ADDRESS_RESOLUTION_ENABLE,          IDX_CAST IDX_HCI_LE_SetAddressResolutionEnableCmd,         HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT, IDX_CAST IDX_HCI_LE_SetResolvablePrivateAddressTimeoutCmd, HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & PRIVACY_1_2_CFG))

  // V4.2 - Secure Connections
#if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & SECURE_CONNS_CFG)
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_LOCAL_P256_PUBLIC_KEY,             IDX_CAST IDX_HCI_LE_ReadLocalP256PublicKeyCmd,             HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_GENERATE_DHKEY,                         IDX_CAST IDX_HCI_LE_GenerateDHKeyCmd,                      HU8PTR,  HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & SECURE_CONNS_CFG))

  // V5.0 - 2Mbps PHY
#if defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & (PHY_2MBPS_CFG | PHY_LR_CFG)) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  HCI_TRANSLATION_ENTRY(HCI_LE_READ_PHY,                               IDX_CAST IDX_HCI_LE_ReadPhyCmd,                            HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_DEFAULT_PHY,                        IDX_CAST IDX_HCI_LE_SetDefaultPhyCmd,                      HU8,     HU8,     HU8,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_PHY,                                IDX_CAST IDX_HCI_LE_SetPhyCmd,                             HU16,    HU8,     HU8,     HU8,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_ENHANCED_RECEIVER_TEST,                 IDX_CAST IDX_HCI_LE_EnhancedRxTestCmd,                     HU8,     HU8,     HU8,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_ENHANCED_TRANSMITTER_TEST,              IDX_CAST IDX_HCI_LE_EnhancedTxTestCmd,                     HU8,     HU8,     HU8,     HU8,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & (PHY_2MBPS_CFG | PHY_LR_CFG)) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG)))

  // Advertiser
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_ADV_PARAM,                          IDX_CAST IDX_HCI_LE_SetAdvParamCmd,                        HU16,    HU16,    HU8,     HU8,     HU8,     HAB,     HU8,     HU8),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_ADV_DATA,                           IDX_CAST IDX_HCI_LE_SetAdvDataCmd,                         HU8,     HU8PTR,  HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_SCAN_RSP_DATA,                      IDX_CAST IDX_HCI_LE_SetScanRspDataCmd,                     HU8,     HU8PTR,  HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_ADV_ENABLE,                         IDX_CAST IDX_HCI_LE_SetAdvEnableCmd,                       HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG)))

  // Scanner
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_SCAN_PARAM,                         IDX_CAST IDX_HCI_LE_SetScanParamCmd,                       HU8,     HU16,    HU16,    HU8,     HU8,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_SET_SCAN_ENABLE,                        IDX_CAST IDX_HCI_LE_SetScanEnableCmd,                      HU8,     HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG))

  // Initiator
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  HCI_EXTENDED_ENTRY(HCI_LE_CREATE_CONNECTION,                         IDX_CAST IDX_HCI_LE_CreateConnCmd,                         hci_tl_le_create_conn_params),
  HCI_TRANSLATION_ENTRY(HCI_LE_CREATE_CONNECTION_CANCEL,               IDX_CAST IDX_HCI_LE_CreateConnCancelCmd,                   HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_START_ENCRYPTION,                       IDX_CAST IDX_HCI_LE_StartEncyptCmd,                        HU16,    H8B,     H2B,     HKB,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG))

  // Connection Updates
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  HCI_TRANSLATION_ENTRY(HCI_LE_CONNECTION_UPDATE,                      IDX_CAST IDX_HCI_LE_ConnUpdateCmd,                         HU16,    HU16,    HU16,    HU16,    HU16,    HU16,    HU16,    HNP),
#endif // (defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG)))

  // Security Responder
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  HCI_TRANSLATION_ENTRY(HCI_LE_LTK_REQ_REPLY,                          IDX_CAST IDX_HCI_LE_LtkReqReplyCmd,                        HU16,    HKB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_LTK_REQ_NEG_REPLY,                      IDX_CAST IDX_HCI_LE_LtkReqNegReplyCmd,                     HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG))

  // Vendor Specific HCI Commands
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_TX_POWER,                          IDX_CAST IDX_HCI_EXT_SetTxPowerCmd,                        HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_BUILD_REVISION,                        IDX_CAST IDX_HCI_EXT_BuildRevisionCmd,                     HU8,     HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_DELAY_SLEEP,                           IDX_CAST IDX_HCI_EXT_DelaySleepCmd,                        HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_DECRYPT,                               IDX_CAST IDX_HCI_EXT_DecryptCmd,                           HKB,     HKB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_ENABLE_PTM,                            IDX_CAST IDX_HCI_EXT_EnablePTMCmd,                         HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_MODEM_TEST_TX,                         IDX_CAST IDX_HCI_EXT_ModemTestTxCmd,                       HU8,     HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_MODEM_HOP_TEST_TX,                     IDX_CAST IDX_HCI_EXT_ModemHopTestTxCmd,                    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_MODEM_TEST_RX,                         IDX_CAST IDX_HCI_EXT_ModemTestRxCmd,                       HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_END_MODEM_TEST,                        IDX_CAST IDX_HCI_EXT_EndModemTestCmd,                      HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_BDADDR,                            IDX_CAST IDX_HCI_EXT_SetBDADDRCmd,                         HAB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_RESET_SYSTEM,                          IDX_CAST IDX_HCI_EXT_ResetSystemCmd,                       HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_LOCAL_SUPPORTED_FEATURES,          IDX_CAST IDX_HCI_EXT_SetLocalSupportedFeaturesCmd,         H8B,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_MAX_DTM_TX_POWER,                  IDX_CAST IDX_HCI_EXT_SetMaxDtmTxPowerCmd,                  HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_RX_GAIN,                           IDX_CAST IDX_HCI_EXT_SetRxGainCmd,                         HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_EXTEND_RF_RANGE,                       IDX_CAST IDX_HCI_EXT_ExtendRfRangeCmd,                     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_HALT_DURING_RF,                        IDX_CAST IDX_HCI_EXT_HaltDuringRfCmd,                      HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_CLK_DIVIDE_ON_HALT,                    IDX_CAST IDX_HCI_EXT_ClkDivOnHaltCmd,                      HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_DECLARE_NV_USAGE,                      IDX_CAST IDX_HCI_EXT_DeclareNvUsageCmd,                    HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_MAP_PM_IO_PORT,                        IDX_CAST IDX_HCI_EXT_MapPmIoPortCmd,                       HU8,     HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_FREQ_TUNE,                         IDX_CAST IDX_HCI_EXT_SetFreqTuneCmd,                       HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SAVE_FREQ_TUNE,                        IDX_CAST IDX_HCI_EXT_SaveFreqTuneCmd,                      HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  HCI_TRANSLATION_ENTRY(HCI_EXT_CONN_EVENT_NOTICE,                     IDX_CAST IDX_HCI_EXT_ConnEventNoticeCmd,                   HU16,    HU8,     HU16,    HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_DISCONNECT_IMMED,                      IDX_CAST IDX_HCI_EXT_DisconnectImmedCmd,                   HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_PER,                                   IDX_CAST IDX_HCI_EXT_PacketErrorRateCmd,                   HU16,    HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_NUM_COMPLETED_PKTS_LIMIT,              IDX_CAST IDX_HCI_EXT_NumComplPktsLimitCmd,                 HU8,     HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_ONE_PKT_PER_EVT,                       IDX_CAST IDX_HCI_EXT_OnePktPerEvtCmd,                      HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_SCA,                               IDX_CAST IDX_HCI_EXT_SetSCACmd,                            HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_GET_CONNECTION_INFO,                   IDX_CAST IDX_HCI_EXT_GetConnInfoCmd,                       HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_GET_ACTIVE_CONNECTION_INFO,            IDX_CAST IDX_HCI_EXT_GetActiveConnInfoCmd,                 HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_OVERLAPPED_PROCESSING,                 IDX_CAST IDX_HCI_EXT_OverlappedProcessingCmd,              HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG)))

#if defined(CTRL_CONFIG) && ((CTRL_CONFIG & ADV_NCONN_CFG) || (CTRL_CONFIG & ADV_CONN_CFG))
  HCI_TRANSLATION_ENTRY(HCI_EXT_ADV_EVENT_NOTICE,                      IDX_CAST IDX_HCI_EXT_AdvEventNoticeCmd,                    HU8,     HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && ((CTRL_CONFIG & ADV_NCONN_CFG) || (CTRL_CONFIG & ADV_CONN_CFG)))

#if (defined(BLE_VS_FEATURES) && (BLE_VS_FEATURES & SCAN_EVT_NOTICE_CFG) && (defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)))
  HCI_TRANSLATION_ENTRY(HCI_EXT_SCAN_EVENT_NOTICE,                     IDX_CAST IDX_HCI_EXT_ScanEventNoticeCmd,                   HU8,     HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(BLE_VS_FEATURES) && (BLE_VS_FEATURES & SCAN_EVT_NOTICE_CFG) && (defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)))

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_FAST_TX_RESP_TIME,                 IDX_CAST IDX_HCI_EXT_SetFastTxResponseTimeCmd,             HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_OVERRIDE_SL,                           IDX_CAST IDX_HCI_EXT_SetSlaveLatencyOverrideCmd,           HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG)))

  //Scan Request Report
#if defined(BLE_VS_FEATURES) && (BLE_VS_FEATURES & SCAN_REQ_RPT_CFG) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  HCI_TRANSLATION_ENTRY(HCI_EXT_SCAN_REQ_REPORT,                       IDX_CAST IDX_HCI_EXT_ScanReqRptCmd,                        HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
#endif // (defined(BLE_VS_FEATURES) && (BLE_VS_FEATURES & SCAN_REQ_RPT_CFG) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG)))

  // Set max data len
#if defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & EXT_DATA_LEN_CFG) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_MAX_DATA_LENGTH,                   IDX_CAST IDX_HCI_EXT_SetMaxDataLenCmd,                     HU16,    HU16,    HU16,    HU16,    HNP,     HNP,     HNP,     HNP),
#endif // (defined(CTRL_V42_CONFIG) && (CTRL_V42_CONFIG & EXT_DATA_LEN_CFG) && defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG)))

  // LL Test Mode
#ifdef LL_TEST_MODE
  HCI_TRANSLATION_ENTRY(HCI_EXT_LL_TEST_MODE,                          IDX_CAST IDX_HCI_EXT_LLTestModeCmd,                        HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP)
#endif
};

#elif defined (HCI_TL_PTM)
static hciEntry_t hciTranslationTable[] =
{
  // Controller and Baseband Commands
  HCI_TRANSLATION_ENTRY(HCI_RESET,                                     IDX_CAST IDX_HCI_ResetCmd,                                 HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),

  // LE Commands - Direct Test Mode
  HCI_TRANSLATION_ENTRY(HCI_LE_RECEIVER_TEST,                          IDX_CAST IDX_HCI_LE_ReceiverTestCmd,                       HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_TRANSMITTER_TEST,                       IDX_CAST IDX_HCI_LE_TransmitterTestCmd,                    HU8,     HU8,     HU8,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_LE_TEST_END,                               IDX_CAST IDX_HCI_LE_TestEndCmd,                            HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_MODEM_TEST_TX,                         IDX_CAST IDX_HCI_EXT_ModemTestTxCmd,                       HU8,     HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_MODEM_HOP_TEST_TX,                     IDX_CAST IDX_HCI_EXT_ModemHopTestTxCmd,                    HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_MODEM_TEST_RX,                         IDX_CAST IDX_HCI_EXT_ModemTestRxCmd,                       HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_END_MODEM_TEST,                        IDX_CAST IDX_HCI_EXT_EndModemTestCmd,                      HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),

  // LE Commands - General
  HCI_TRANSLATION_ENTRY(HCI_READ_BDADDR,                               IDX_CAST IDX_HCI_ReadBDADDRCmd,                            HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_BDADDR,                            IDX_CAST IDX_HCI_EXT_SetBDADDRCmd,                         HAB,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_TX_POWER,                          IDX_CAST IDX_HCI_EXT_SetTxPowerCmd,                        HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_MAX_DTM_TX_POWER,                  IDX_CAST IDX_HCI_EXT_SetMaxDtmTxPowerCmd,                  HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_EXTEND_RF_RANGE,                       IDX_CAST IDX_HCI_EXT_ExtendRfRangeCmd,                     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_HALT_DURING_RF,                        IDX_CAST IDX_HCI_EXT_HaltDuringRfCmd,                      HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_READ_TRANSMIT_POWER,                       IDX_CAST IDX_HCI_ReadTransmitPowerLevelCmd,                HU16,    HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_BUILD_REVISION,                        IDX_CAST IDX_HCI_EXT_BuildRevisionCmd,                     HU8,     HU16,    HNP,     HNP,     HNP,     HNP,     HNP,     HNP),

  // LE Commands - Production Test Mode
  HCI_TRANSLATION_ENTRY(HCI_EXT_ENABLE_PTM,                            IDX_CAST IDX_HCI_EXT_EnablePTMCmd,                         HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SET_FREQ_TUNE,                         IDX_CAST IDX_HCI_EXT_SetFreqTuneCmd,                       HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_SAVE_FREQ_TUNE,                        IDX_CAST IDX_HCI_EXT_SaveFreqTuneCmd,                      HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),
  HCI_TRANSLATION_ENTRY(HCI_EXT_RESET_SYSTEM,                          IDX_CAST IDX_HCI_EXT_ResetSystemCmd,                       HU8,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP,     HNP),

};
#else // !defined (HCI_TL_PTM) or !defined (HCI_TL_FULL)
static hciEntry_t hciTranslationTable[] = { 0 };
#endif // defined (HCI_TL_PTM) or defined (HCI_TL_FULL)

// Callback for overriding contents of serial buffer.
HCI_TL_ParameterOverwriteCB_t HCI_TL_ParameterOverwriteCB = NULL;

#if ( defined(HCI_TL_FULL) || defined(HCI_TL_PTM))  && defined(HOST_CONFIG)
static ICall_EntityID appTaskID;

// Outgoing response
static uint8_t rspBuf[MAX_RSP_BUF];

// The device's local keys
static uint8_t IRK[KEYLEN] = {0};
static uint8_t SRK[KEYLEN] = {0};

// Sign counter
static uint32_t hciExtSignCounter = 0;

// Outgoing event
static uint8_t out_msg[HCI_EXT_APP_OUT_BUF];
#endif // (HCI_TL_FULL || HCI_TL_PTM)  && HOST_CONFIG

// Callback for sending command status.
static HCI_TL_CommandStatusCB_t HCI_TL_CommandStatusCB = NULL;

// Callback for posting callback event (switching from SWI to Thread).
static HCI_TL_CalllbackEvtProcessCB_t HCI_TL_CallbackEvtProcessCB = NULL;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
#if (defined(HCI_TL_FULL) || defined(HCI_TL_PTM) )
static void HCI_TL_SendCommandPkt(hciPacket_t *pMsg);
static void HCI_TL_SendDataPkt(uint8_t *pMsg);

//HOST
#ifdef HOST_CONFIG
static void HCI_TL_SendExtensionCmdPkt(hciPacket_t *pMsg);
static uint8_t processExtMsg(hciPacket_t *pMsg);
static uint8_t processExtMsgL2CAP(uint8_t cmdID, hciExtCmd_t *pCmd, uint8_t *pRspDataLen);
static uint8_t processExtMsgATT(uint8_t cmdID, hciExtCmd_t *pCmd);
static uint8_t processExtMsgGATT(uint8_t cmdID, hciExtCmd_t *pCmd, uint8_t *pRspDataLen);
static uint8_t processExtMsgGAP(uint8_t cmdID, hciExtCmd_t *pCmd, uint8_t *pRspDataLen);
static uint8_t processExtMsgUTIL(uint8_t cmdID, hciExtCmd_t *pCmd, uint8_t *pRspDataLen);

static uint8_t *createMsgPayload(uint8_t *pBuf, uint16_t len);
static uint8_t *createSignedMsgPayload(uint8_t sig, uint8_t cmd, uint8_t *pBuf, uint16_t len);
static uint8_t *createPayload(uint8_t *pBuf, uint16_t len, uint8_t sigLen);
static uint8_t checkNVLen(osalSnvId_t id, osalSnvLen_t len);
static uint8_t mapATT2BLEStatus(uint8_t status);
static uint8_t buildHCIExtHeader(uint8_t *pBuf, uint16_t event, uint8_t status, uint16_t connHandle);

static uint8_t processEvents(ICall_Hdr *pMsg);
static uint8_t *processEventsGAP(gapEventHdr_t *pMsg, uint8_t *pOutMsg,
                               uint8_t *pMsgLen, uint8_t *pAllocated,
                               uint8_t *pDeallocate);
static uint8_t *processEventsSM(smEventHdr_t *pMsg, uint8_t *pOutMsg,
                               uint8_t *pMsgLen, uint8_t *pAllocated);
static uint8_t *processEventsL2CAP(l2capSignalEvent_t *pPkt, uint8_t *pOutMsg,
                                  uint8_t *pMsgLen);
static uint8_t *processDataL2CAP(l2capDataEvent_t *pPkt, uint8_t *pOutMsg,
                                uint8_t *pMsgLen, uint8_t *pAllocated);
static uint8_t *processEventsGATT(gattMsgEvent_t *pPkt, uint8_t *pOutMsg,
                                 uint8_t *pMsgLen, uint8_t *pAllocated);
#if !defined(GATT_DB_OFF_CHIP)
static uint8_t *processEventsGATTServ(gattEventHdr_t *pPkt, uint8_t *pMsg,
                                     uint8_t *pMsgLen);
#endif

#if defined(BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
static uint16_t l2capVerifySecCB(uint16_t connHandle, uint8_t id, l2capConnectReq_t *pReq);
static uint8_t buildCoChannelInfo(uint16_t CID, l2capCoCInfo_t *pInfo, uint8_t *pRspBuf);
#endif /* BLE_V41_FEATURES & L2CAP_COC_CFG */

#if defined(GATT_DB_OFF_CHIP)
static uint8_t addAttrRec(gattService_t *pServ, uint8_t *pUUID, uint8_t len,
                         uint8_t permissions, uint16_t *pTotalAttrs, uint8_t *pRspDataLen);
static const uint8_t *findUUIDRec(uint8_t *pUUID, uint8_t len);
static void freeAttrRecs(gattService_t *pServ);
#endif // GATT_DB_OFF_CHIP

#if ( HOST_CONFIG & ( PERIPHERAL_CFG | CENTRAL_CFG ) )
#if defined(GAP_BOND_MGR)
// Passcode.
static void host_tl_passcodeCB(uint8_t *deviceAddr, uint16_t connectionHandle,
                               uint8_t uiInputs, uint8_t uiOutputs);
static void host_tl_passcodeCBProcess(passcodeEvtCallback_t *pData);
#endif //GAP_BOND_MGR
#endif // PERIPHERAL_CFG | CENTRAL_CFG

#endif /* HOST_CONFIG */
#endif /* (defined(HCI_TL_FULL) || defined(HCI_TL_PTM) ) && defined(HOST_CONFIG) */

/*********************************************************************
 * @fn      HCI_TL_Init
 *
 * @brief   Initialize HCI TL.
 *
 * @param   overwriteCB     - callback used to allow custom override the contents of the
 *                            serial buffer.
 *          csCB            - Callback to process command status
 *          evtCB           - Callback to post event related to Callback from LL
 *          taskID - Task ID of task to process and forward messages to the TL.
 *
 * @return  none.
 */
void HCI_TL_Init(HCI_TL_ParameterOverwriteCB_t overwriteCB,
                 HCI_TL_CommandStatusCB_t csCB,
                 HCI_TL_CalllbackEvtProcessCB_t evtCB,
                 ICall_EntityID taskID)
{
  HCI_TL_ParameterOverwriteCB = overwriteCB;

  HCI_TL_CommandStatusCB = csCB;
  (void) HCI_TL_CommandStatusCB;

  HCI_TL_CallbackEvtProcessCB = evtCB;
  (void) HCI_TL_CallbackEvtProcessCB;
  
#if ( HOST_CONFIG & ( CENTRAL_CFG | PERIPHERAL_CFG ) )
  #if (defined(HCI_TL_FULL) || defined(HCI_TL_PTM) ) && defined(HOST_CONFIG)
  appTaskID = taskID;

  // Register for GATT local events and ATT Responses pending for transmission
  GATT_RegisterForMsgs(appTaskID);

  // Register to receive incoming ATT Indications/Notifications
  GATT_RegisterForInd(appTaskID);

    #if !defined(GATT_DB_OFF_CHIP)
    // Register with GATT Server App for event messages
    GATTServApp_RegisterForMsg(appTaskID);
    #else
    // Register with GATT Server for GATT messages
    GATT_RegisterForReq(appTaskID);
    #endif // GATT_DB_OFF_CHIP

  // Do not use SNV drivers with FPGA. Unverified behavior
    #ifndef USE_FPGA
  VOID osal_snv_read(BLE_NVID_IRK, KEYLEN, IRK);
  VOID osal_snv_read(BLE_NVID_CSRK, KEYLEN, SRK);
  VOID osal_snv_read(BLE_NVID_SIGNCOUNTER, sizeof(uint32_t), &hciExtSignCounter);
    #endif // USE_FPGA
  #endif // (HCI_TL_FULL || HCI_TL_PTM) && HOST_CONFIG
#endif // (CENTRAL_CFG | PERIPHERAL_CFG)
}

/*********************************************************************
 * @fn      HCI_TL_compareAppLastOpcodeSent
 *
 * @brief   check if the opcode of an event received matches the last
 *          opcode of an HCI command called from the embedded application.
 *
 * @param   opcode - opcode of the received Stack event.
 *
 * @return  TRUE if opcode matches, FALSE otherwise.
 */
uint8_t HCI_TL_compareAppLastOpcodeSent(uint16_t opcode)
{
  uint8_t isMatch = FALSE;

  // See if the opcode received matches the ICall LITE ID last sent from the stack.
  // Last last ICall ID sent from the stack should be invalid if it was already matched.

  // If the application is looking for an event
  if (lastAppOpcodeIdxSent != 0xFFFFFFFF)
  {
    // Iterate over the hci table.
    for (uint8_t idx = 0;
         idx < sizeof(hciTranslationTable)/sizeof(hciEntry_t);
         idx++)
    {
      // If an entry matching the opcode event is found and the ICall Lite ID matches that of the last we sent
      if (hciTranslationTable[idx].opcode == opcode &&
          GET_CMD_IDX(hciTranslationTable[idx].cmdIdx) == lastAppOpcodeIdxSent)
      {
        // A match is found.
        isMatch = TRUE;
        break;
      }
    }
  }

  return(isMatch);
}

/*********************************************************************
 * @fn      HCI_TL_SendToStack
 *
 * @brief   Translate seriall buffer into it's corresponding function and
 *          parameterize the arguments to send to the Stack.
 *
 * @param   msgToParse - pointer to a serialized HCI command or data packet.
 *
 * @return  none.
 */
void HCI_TL_SendToStack(uint8_t *msgToParse)
{
#if (defined(HCI_TL_FULL) || defined(HCI_TL_PTM) )
  hciPacket_t *pMsg;

  pMsg = (hciPacket_t *)msgToParse;

  // if there is a host configuration
#ifdef HOST_CONFIG
  if (pMsg->hdr.event == HCI_EXT_CMD_EVENT)
  {
    HCI_TL_SendExtensionCmdPkt(pMsg);
  }
  else
#endif /* HOST_CONFIG */
  // What type of packet is this?
  if (pMsg->hdr.event == HCI_HOST_TO_CTRL_CMD_EVENT)
  {
    // Command.  Interpret the command and send to Host.
    HCI_TL_SendCommandPkt(pMsg);
  }
  else if (pMsg->hdr.event == HCI_HOST_TO_CTRL_DATA_EVENT)
  {
    HCI_TL_SendDataPkt(msgToParse);
  }
#endif
}

#if (defined(HCI_TL_FULL) || defined(HCI_TL_PTM) )
/*********************************************************************
 * @fn      HCI_TL_SendCommandPkt
 *
 * @brief   Interprets an HCI command packet based off the translation table and
 *          calls the corresponding BLE API with the arguments provided.
 *
 * @param   pMsg - HCI Command serial packet to translate.
 *
 * @return  none.
 */
static void HCI_TL_SendCommandPkt(hciPacket_t *pMsg)
{
  uint16_t cmdOpCode;
  uint32_t  i = 0;

  // retrieve opcode
  cmdOpCode = BUILD_UINT16(pMsg->pData[1], pMsg->pData[2]);

  // If this is a command which the embedded "Host" must modify the parameters
  // to handle properly (e.g. taskEvent, taskID), it is handled here.
  if (HCI_TL_ParameterOverwriteCB)
  {
    HCI_TL_ParameterOverwriteCB(pMsg->pData);
  }

  // Loop over table to find a matching opcode
  for(; i < sizeof(hciTranslationTable) / sizeof(hciEntry_t) ; i++)
  {
    // Search for opcode that matches cmdOpCode
    if (hciTranslationTable[i].opcode == cmdOpCode)
    {
      // Found it.
      break;
    }
  }

  // If we found a matching opcode
  if (i != sizeof(hciTranslationTable) / sizeof(hciEntry_t))
  {
    uint8_t numRules;
    uint8_t *pRules;
    hci_arg_t args[HCI_MAX_NUM_ARGS] = {0,};
    uint8_t bufCount = 1;
    uint8_t *param = &pMsg->pData[4]; // pointer into HCI command parameters.

    // Determine where the rules are and number of rules
#ifdef STACK_LIBRARY
    if (hciTranslationTable[i].extFlag)
#else
    if (IS_EXTENDED_PARAMS(hciTranslationTable[i].cmdIdx))
#endif
    {
      pRules = ((uint8_t *)(hciTranslationTable[i].paramRules)) + 1;
      numRules = *(pRules - 1);
    }
    else
    {
      pRules = (uint8_t *)(&hciTranslationTable[i].paramRules);
      numRules = HCI_DEFAULT_NUM_RULES;
    }

    // Parse the arguments according to the rules of the matching opcode
    for (uint8_t arg_i = 0; arg_i < numRules; arg_i++)
    {
      // Get this parameter's rule.
      uint8_t rule  = GET_RULE(pRules, arg_i);

      // If this a buffer counter
      if (rule & HLEN)
      {
        // What type of buffer counter is this?
        switch(rule)
        {
          // Check implicit counts.
          case H2B:
            bufCount = 2;
            break;

          case HAB:
            bufCount = 6;
            break;

          case H8B:
            bufCount = 8;
            break;

          case HKB:
            bufCount = 16;
            break;

          // Default to explicit count.
          default:
            bufCount = *param;
            break;
        }
      }

      // If this is a pointer
      if (rule & HPTR)
      {
        // Note that a pointer means the argument is the address of the byte at
        // offset 0 from param.

        // Store the address.
        args[arg_i] = (hci_arg_t)param;

        // Because the argument is stored in the buffer, param is moved forward
        // by the byte width of the argument stored there.
        switch (rule & HPTRMASK)
        {
          case HU32:
            param += 4 * bufCount;
            break;

          case HU16:
            param += 2 * bufCount;
            break;

          case HU8:
            param += 1 * bufCount;
            break;

          // Implicit buffer counters fall into here.
          default:
            param += bufCount;
            break;
        }
      }
      else if (rule == HU32)
      {
        // 32 bit value
        args[arg_i] = BUILD_UINT32(param[0], param[1], param[2], param[3]);
        param += 4;
      }
      else if (rule == HU16)
      {
        // 16 bit value
        args[arg_i] = BUILD_UINT16(param[0], param[1]);
        param += 2;
      }
      else if (rule == HU8)
      {
        // 8 bit value
        args[arg_i] = *param++;
      }
      else if (rule == HNP)
      {
        // No parameters remaining.
        break;
      }
    }

    // Call the function using the translated parameters.
    icall_directAPI(ICALL_SERVICE_CLASS_BLE,
                    (uint32_t) GET_CMD_IDX(hciTranslationTable[i].cmdIdx),
                    args[0],
                    args[1],
                    args[2],
                    args[3],
                    args[4],
                    args[5],
                    args[6],
                    args[7],
                    args[8],
                    args[9],
                    args[10],
                    args[11]);
  }
  else
  {
    // Opcode not found, return error.
    uint8_t status = HCI_ERROR_CODE_UNKNOWN_HCI_CMD;

    HCI_CommandCompleteEvent (cmdOpCode, 1, &status);
  }
}

/*********************************************************************
 * @fn      HCI_TL_SendDataPkt
 *
 * @brief   Interprets an HCI data packet and sends to the Stack.
 *
 * @param   pMsg - HCI data serial packet to translate.
 *
 * @return  none.
 */
static void HCI_TL_SendDataPkt(uint8_t *pMsg)
{
  hciDataPacket_t *pDataPkt = (hciDataPacket_t *) pMsg;

  // LE only accepts Data packets of type ACL.
  if (pDataPkt->pktType == HCI_ACL_DATA_PACKET)
  {
    uint8_t *pData = pDataPkt->pData;

    // Replace data with bm data
    pDataPkt->pData = (uint8_t *) HCI_bm_alloc(pDataPkt->pktLen);

    if (pDataPkt->pData)
    {
      memcpy(pDataPkt->pData, pData, pDataPkt->pktLen);

      HCI_SendDataPkt(pDataPkt->connHandle,
                      pDataPkt->pbFlag,
                      pDataPkt->pktLen,
                      pDataPkt->pData);
    }
  }
}


/************************** Host Command Translation **************************/
#ifdef HOST_CONFIG

/*********************************************************************
 * @fn      HCI_TL_processStructuredEvent
 *
 * @brief   Interprets a structured Event from the BLE Host and serializes it.
 *
 * @param   pEvt - structured event to serialize.
 *
 * @return  TRUE to deallocate pEvt message, False otherwise.
 */
uint8_t HCI_TL_processStructuredEvent(ICall_Hdr *pEvt)
{
  return(processEvents(pEvt));
}

/*********************************************************************
 * @fn      HCI_TL_SendExtensionCmdPkt
 *
 * @brief   Interprets an HCI extension command packet and sends to the Stack.
 *
 * @param   pMsg - HCI command serial packet to translate.
 *
 * @return  none.
 */
static void HCI_TL_SendExtensionCmdPkt(hciPacket_t *pMsg)
{
  // Process Extension messages not bound for the HCI layer.
  processExtMsg(pMsg);
}

static void HCI_TL_SendVSEvent(uint8_t *pBuf, uint8_t dataLen)
{
  hciPacket_t *msg;
  uint8_t totalLength;

  // OSAL message header + HCI event header + data
  totalLength = sizeof(hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLen;

  // allocate memory for OSAL hdr + packet
  msg = (hciPacket_t *)ICall_allocMsg(totalLength);

  if (msg)
  {
    // message type, HCI event type
    msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
    msg->hdr.status = 0xFF;

    // packet
    msg->pData    = (uint8_t*)(msg+1);
    msg->pData[0] = HCI_EVENT_PACKET;
    msg->pData[1] = HCI_VE_EVENT_CODE;
    msg->pData[2] = dataLen;

    // copy data
    if (dataLen)
    {
      memcpy(&(msg->pData[3]), pBuf, dataLen);
    }

    // Send to High Layer.
    if (HCI_TL_CommandStatusCB)
    {
      HCI_TL_CommandStatusCB(msg->pData, HCI_EVENT_MIN_LENGTH + msg->pData[2]);
    }

    // We're done with this message.
    ICall_freeMsg(msg);
  }
}

/*********************************************************************
 * @fn      processExtMsg
 *
 * @brief   Parse and process incoming HCI extension messages.
 *
 * @param   pMsg - incoming HCI extension message.
 *
 * @return  none
 */
static uint8_t processExtMsg(hciPacket_t *pMsg)
{
  uint8_t deallocateIncoming;
  bStatus_t stat = SUCCESS;
  uint8_t rspDataLen = 0;
  hciExtCmd_t msg;
  uint8_t *pBuf = pMsg->pData;

  // Parse the header
  msg.pktType = *pBuf++;
  msg.opCode = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += 2;

  msg.len = *pBuf++;
  msg.pData = pBuf;

  switch(msg.opCode >> 7)
  {
#if defined(HOST_CONFIG)
    case HCI_EXT_L2CAP_SUBGRP:
      stat = processExtMsgL2CAP((msg.opCode & 0x007F), &msg, &rspDataLen);
      break;

    case HCI_EXT_ATT_SUBGRP:
      stat = processExtMsgATT((msg.opCode & 0x007F), &msg);
      break;

    case HCI_EXT_GATT_SUBGRP:
      stat = processExtMsgGATT((msg.opCode & 0x007F), &msg, &rspDataLen);
      break;

    case HCI_EXT_GAP_SUBGRP:
      stat = processExtMsgGAP((msg.opCode & 0x007F), &msg, &rspDataLen);
      break;
#endif // HOST_CONFIG

    case HCI_EXT_UTIL_SUBGRP:
      stat = processExtMsgUTIL((msg.opCode & 0x007F), &msg, &rspDataLen);
      break;

    default:
      stat = FAILURE;
      break;
  }

  deallocateIncoming = FALSE;

  // Send back an immediate response
  rspBuf[0] = LO_UINT16(HCI_EXT_GAP_CMD_STATUS_EVENT);
  rspBuf[1] = HI_UINT16(HCI_EXT_GAP_CMD_STATUS_EVENT);
  rspBuf[2] = stat;
  rspBuf[3] = LO_UINT16(0xFC00 | msg.opCode);
  rspBuf[4] = HI_UINT16(0xFC00 | msg.opCode);
  rspBuf[5] = rspDataLen;

  // IMPORTANT!! Fill in Payload (if needed) in case statement

  HCI_TL_SendVSEvent(rspBuf, (6 + rspDataLen));

  return(deallocateIncoming);
}

/*********************************************************************
 * @fn      processExtMsgUTIL
 *
 * @brief   Parse and process incoming HCI extension UTIL messages.
 *
 * @param   cmdID - incoming HCI extension command ID.
 * @param   pCmd - incoming HCI extension message.
 * @param   pRspDataLen - response data length to be returned.
 *
 * @return  SUCCESS, INVALIDPARAMETER and FAILURE.
 */
static uint8_t processExtMsgUTIL(uint8_t cmdID, hciExtCmd_t *pCmd, uint8_t *pRspDataLen)
{
  bStatus_t stat = SUCCESS;

  *pRspDataLen = 0;

  switch(cmdID)
  {
  // Do not use SNV drivers with FPGA. Unverified behavior
#ifndef USE_FPGA
    case HCI_EXT_UTIL_NV_READ:
      {
        uint8_t *pBuf = pCmd->pData;
        osalSnvId_t id  = pBuf[0];
        osalSnvLen_t len = pBuf[1];

        // This has a limitation of only allowing a max data length because of the fixed buffer.
        if ((len < MAX_RSP_DATA_LEN) && (checkNVLen(id, len) == SUCCESS))
        {
          stat = osal_snv_read(id, len, &rspBuf[RSP_PAYLOAD_IDX]);
          if (stat == SUCCESS)
          {
            *pRspDataLen = pBuf[1];
          }
        }
        else
        {
          stat = INVALIDPARAMETER;
        }
      }
      break;

    case HCI_EXT_UTIL_NV_WRITE:
      {
        uint8_t *pBuf = pCmd->pData;
        osalSnvId_t id  = pBuf[0];
        osalSnvLen_t len = pBuf[1];
        if (checkNVLen(id, len) == SUCCESS)
        {
          stat = osal_snv_write(id, len, &pBuf[2]);

#if defined(HCI_TL_FULL) && defined(HOST_CONFIG)
          if (id == BLE_NVID_SIGNCOUNTER)
          {
            hciExtSignCounter = BUILD_UINT32(pBuf[2], pBuf[3], pBuf[4], pBuf[5]);
          }
#endif // HCI_TL_FULL && HOST_CONFIG
        }
        else
        {
          stat = INVALIDPARAMETER;
        }
      }
      break;
#endif // USE_FPGA

    case HCI_EXT_UTIL_BUILD_REV:
      {
        ICall_BuildRevision buildRev;

        VOID buildRevision(&buildRev);

        // Stack revision
        //  Byte 0: Major
        //  Byte 1: Minor
        //  Byte 2: Patch
        rspBuf[RSP_PAYLOAD_IDX]   = BREAK_UINT32(buildRev.stackVersion, 0);
        rspBuf[RSP_PAYLOAD_IDX+1] = BREAK_UINT32(buildRev.stackVersion, 1);
        rspBuf[RSP_PAYLOAD_IDX+2] = BREAK_UINT32(buildRev.stackVersion, 2);

        // Build revision
        rspBuf[RSP_PAYLOAD_IDX+3] = LO_UINT16(buildRev.buildVersion);
        rspBuf[RSP_PAYLOAD_IDX+4] = HI_UINT16(buildRev.buildVersion);

        // Stack info (Byte 5)
        rspBuf[RSP_PAYLOAD_IDX+5] = buildRev.stackInfo;

        // Controller info - part 1 (Byte 6)
        rspBuf[RSP_PAYLOAD_IDX+6] = LO_UINT16(buildRev.ctrlInfo);

        // Controller info - part 2 (Byte 7)
        rspBuf[RSP_PAYLOAD_IDX+7] = 0; // reserved

        // Host info - part 1 (Byte 8)
        rspBuf[RSP_PAYLOAD_IDX+8] = LO_UINT16(buildRev.hostInfo);

        // Host info - part 2 (Byte 9)
        rspBuf[RSP_PAYLOAD_IDX+9] = 0; // reserved

        *pRspDataLen = 10;
      }
      break;

    default:
      stat = FAILURE;
      break;
  }

  return(stat);
}

/*********************************************************************
 * @fn      processExtMsgL2CAP
 *
 * @brief   Parse and process incoming HCI extension L2CAP messages.
 *
 * @param   pCmd - incoming HCI extension message.
 *
 * @return  SUCCESS or FAILURE
 */
static uint8_t processExtMsgL2CAP(uint8_t cmdID, hciExtCmd_t *pCmd, uint8_t *pRspDataLen)
{
  uint8_t *pBuf = pCmd->pData;
  uint16_t connHandle = BUILD_UINT16(pBuf[0], pBuf[1]); // connHandle, CID or PSM
  l2capSignalCmd_t cmd;
  bStatus_t stat;

  switch(cmdID)
  {
#if defined(BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
    case HCI_EXT_L2CAP_DATA:
      if (pCmd->len > 2)
      {
        uint8_t *pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
        if (pPayload != NULL)
        {
          l2capPacket_t pkt;

          pkt.CID = connHandle; // connHandle is CID here
          pkt.pPayload = pPayload;
          pkt.len = pCmd->len-2;

          // Send SDU over dynamic channel
          stat = L2CAP_SendSDU(&pkt);
          if (stat != SUCCESS)
          {
            VOID ICall_free(pPayload);
          }
        }
        else
        {
          stat = bleMemAllocError;
        }
      }
      else
      {
        stat = FAILURE;
      }
      break;

    case HCI_EXT_L2CAP_REGISTER_PSM:
      if (pCmd->len == 10)
      {
        l2capPsm_t psm;

        psm.psm = connHandle; // connHandle is PSM here
        psm.mtu = BUILD_UINT16(pBuf[2], pBuf[3]);
        psm.initPeerCredits = BUILD_UINT16(pBuf[4], pBuf[5]);
        psm.peerCreditThreshold = BUILD_UINT16(pBuf[6], pBuf[7]);
        psm.maxNumChannels = pBuf[8];
        psm.pfnVerifySecCB = pBuf[9] ? l2capVerifySecCB : NULL;
        psm.taskId = PROXY_ID(appTaskID);

        stat = L2CAP_RegisterPsm(&psm);
      }
      else
      {
        stat = FAILURE;
      }
      break;

    case HCI_EXT_L2CAP_DEREGISTER_PSM:
      if (pCmd->len == 2)
      {
        stat = L2CAP_DeregisterPsm(appTaskID, connHandle); // connHandle is PSM here
      }
      else
      {
        stat = FAILURE;
      }
      break;

    case HCI_EXT_L2CAP_PSM_INFO:
      if (pCmd->len == 2)
      {
        l2capPsmInfo_t info;

        stat = L2CAP_PsmInfo(connHandle, &info); // connHandle is PSM here
        if (stat == SUCCESS)
        {
          *pRspDataLen = 10;
          rspBuf[RSP_PAYLOAD_IDX]   = LO_UINT16(info.mtu);
          rspBuf[RSP_PAYLOAD_IDX+1] = HI_UINT16(info.mtu);

          rspBuf[RSP_PAYLOAD_IDX+2] = LO_UINT16(info.mps);
          rspBuf[RSP_PAYLOAD_IDX+3] = HI_UINT16(info.mps);

          rspBuf[RSP_PAYLOAD_IDX+4] = LO_UINT16(info.initPeerCredits);
          rspBuf[RSP_PAYLOAD_IDX+5] = HI_UINT16(info.initPeerCredits);

          rspBuf[RSP_PAYLOAD_IDX+6] = LO_UINT16(info.peerCreditThreshold);
          rspBuf[RSP_PAYLOAD_IDX+7] = HI_UINT16(info.peerCreditThreshold);

          rspBuf[RSP_PAYLOAD_IDX+8] = info.maxNumChannels;
          rspBuf[RSP_PAYLOAD_IDX+9] = info.numActiveChannels;
        }
      }
      else
      {
        stat = FAILURE;
      }
      break;

    case HCI_EXT_L2CAP_PSM_CHANNELS:
      if (pCmd->len == 2)
      {
        l2capPsmInfo_t info;

        stat = L2CAP_PsmInfo(connHandle, &info); // connHandle is PSM here
        if ((stat == SUCCESS) && (info.numActiveChannels > 0))
        {
          uint8_t numCIDs = info.numActiveChannels;

          uint16_t *pCIDs = (uint16_t *)ICall_malloc(sizeof(uint16_t) * numCIDs);
          if (pCIDs != NULL)
          {
            stat = L2CAP_PsmChannels(connHandle, numCIDs, pCIDs);
            if (stat == SUCCESS)
            {
              uint8_t *pRspBuf = &rspBuf[RSP_PAYLOAD_IDX];
              uint8_t i;

              for (i = 0; i < numCIDs; i++)
              {
                *pRspBuf++ = LO_UINT16(pCIDs[i]);
                *pRspBuf++ = HI_UINT16(pCIDs[i]);
              }

              *pRspDataLen = numCIDs * 2;
            }

            VOID ICall_free(pCIDs);
          }
          else
          {
            stat = bleMemAllocError;
          }
        }
      }
      else
      {
        stat = FAILURE;
      }
      break;

    case HCI_EXT_L2CAP_CHANNEL_INFO:
      if (pCmd->len == 2)
      {
        l2capChannelInfo_t channelInfo;

        stat = L2CAP_ChannelInfo(connHandle, &channelInfo); // connHandle is CID here
        if (stat == SUCCESS)
        {
          rspBuf[RSP_PAYLOAD_IDX] = channelInfo.state;

          *pRspDataLen = 1 + buildCoChannelInfo(connHandle, &channelInfo.info,
                                                 &rspBuf[RSP_PAYLOAD_IDX+1]);
        }
      }
      else
      {
        stat = FAILURE;
      }
      break;

    case L2CAP_CONNECT_REQ:
      if (pCmd->len == 6)
      {
        uint16_t psm = BUILD_UINT16(pBuf[2], pBuf[3]);
        uint16_t peerPsm = BUILD_UINT16(pBuf[4], pBuf[5]);

        stat = L2CAP_ConnectReq(connHandle, psm, peerPsm);
      }
      else
      {
        stat = FAILURE;
      }
      break;

    case L2CAP_CONNECT_RSP:
      if (pCmd->len == 5)
      {
        uint16_t result = BUILD_UINT16(pBuf[3], pBuf[4]);

        stat = L2CAP_ConnectRsp(connHandle, pBuf[2], result);
      }
      else
      {
        stat = FAILURE;
      }
      break;

    case L2CAP_DISCONNECT_REQ:
      if (pCmd->len == 2)
      {
        stat = L2CAP_DisconnectReq(connHandle); // connHandle is CID here
      }
      else
      {
        stat = FAILURE;
      }
      break;

    case L2CAP_FLOW_CTRL_CREDIT:
      stat = L2CAP_ParseFlowCtrlCredit(&cmd, pBuf, pCmd->len);
      if (stat == SUCCESS)
      {
        stat = L2CAP_FlowCtrlCredit(cmd.credit.CID, cmd.credit.credits);
      }
      break;
#endif //(BLE_V41_FEATURES & L2CAP_COC_CFG)

    case L2CAP_PARAM_UPDATE_REQ:
      stat = L2CAP_ParseParamUpdateReq(&cmd, &pBuf[2], pCmd->len-2);
      if (stat == SUCCESS)
      {
        stat =  L2CAP_ConnParamUpdateReq(connHandle, &cmd.updateReq, appTaskID);
      }
      break;

    case L2CAP_INFO_REQ:
      stat = L2CAP_ParseInfoReq(&cmd, &pBuf[2], pCmd->len-2);
      if (stat == SUCCESS)
      {
        stat = L2CAP_InfoReq(connHandle, &cmd.infoReq, appTaskID);
      }
      break;

    default:
      stat = FAILURE;
      break;
  }

  return(stat);
}

/*********************************************************************
 * @fn      processExtMsgATT
 *
 * @brief   Parse and process incoming HCI extension ATT messages.
 *
 * @param   pCmd - incoming HCI extension message.
 *
 * @return  SUCCESS, INVALIDPARAMETER, FAILURE,
 *          bleInvalidPDU, bleInsufficientAuthen,
 *          bleInsufficientKeySize, bleInsufficientEncrypt or bleMemAllocError
 */
static uint8_t processExtMsgATT(uint8_t cmdID, hciExtCmd_t *pCmd)
{
  static uint8_t numPrepareWrites = 0;
  static attPrepareWriteReq_t *pPrepareWrites = NULL;
  uint8_t *pBuf, *pPayload = NULL, safeToDealloc = TRUE;
  uint16_t connHandle;
  attMsg_t msg;
  bStatus_t stat = bleInvalidPDU;

  // Make sure received buffer contains at lease connection handle (2 otects)
  if (pCmd->len < 2)
  {
    return(stat);
  }

  pBuf = pCmd->pData;
  connHandle = BUILD_UINT16(pBuf[0], pBuf[1]);

  switch(cmdID)
  {
    case ATT_ERROR_RSP:
      if (ATT_ParseErrorRsp(&pBuf[2], pCmd->len-2, &msg) == SUCCESS)
      {
        stat = ATT_ErrorRsp(connHandle, &msg.errorRsp);
      }
      break;

    case ATT_EXCHANGE_MTU_REQ:
      if (ATT_ParseExchangeMTUReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                    pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_ExchangeMTU(connHandle, &msg.exchangeMTUReq, appTaskID);
      }
      break;

    case ATT_EXCHANGE_MTU_RSP:
      if (ATT_ParseExchangeMTURsp(&pBuf[2], pCmd->len-2, &msg) == SUCCESS)
      {
        stat = ATT_ExchangeMTURsp(connHandle, &msg.exchangeMTURsp);
      }
      break;

    case ATT_FIND_INFO_REQ:
      if (ATT_ParseFindInfoReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                 pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_DiscAllCharDescs(connHandle, msg.findInfoReq.startHandle,
                                      msg.findInfoReq.endHandle, appTaskID);
      }
      break;

    case ATT_FIND_INFO_RSP:
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseFindInfoRsp(pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          stat = ATT_FindInfoRsp(connHandle, &msg.findInfoRsp);
          if (stat == SUCCESS)
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_FIND_BY_TYPE_VALUE_REQ:
      if (ATT_ParseFindByTypeValueReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                        pCmd->len-2, &msg) == SUCCESS)
      {
        attFindByTypeValueReq_t *pReq = &msg.findByTypeValueReq;

        // Find out what's been requested
        if (gattPrimaryServiceType(pReq->type)     &&
             (pReq->startHandle == GATT_MIN_HANDLE) &&
             (pReq->endHandle   == GATT_MAX_HANDLE))
        {
          // Discover primary service by service UUID
          stat = GATT_DiscPrimaryServiceByUUID(connHandle, pReq->pValue,
                                                pReq->len, appTaskID);
        }
        else
        {
          stat = INVALIDPARAMETER;
        }
      }
      break;

    case ATT_FIND_BY_TYPE_VALUE_RSP:
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseFindByTypeValueRsp(pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          stat = ATT_FindByTypeValueRsp(connHandle, &msg.findByTypeValueRsp);
          if (stat == SUCCESS)
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_READ_BY_TYPE_REQ:
      if (ATT_ParseReadByTypeReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                   pCmd->len-2, &msg) == SUCCESS)
      {
        attReadByTypeReq_t *pReq = &msg.readByTypeReq;

        // Find out what's been requested
        if (gattIncludeType(pReq->type))
        {
          // Find included services
          stat = GATT_FindIncludedServices(connHandle, pReq->startHandle,
                                            pReq->endHandle, appTaskID);
        }
        else if (gattCharacterType(pReq->type))
        {
          // Discover all characteristics of a service
          stat = GATT_DiscAllChars(connHandle, pReq->startHandle,
                                    pReq->endHandle, appTaskID);
        }
        else
        {
          // Read using characteristic UUID
          stat = GATT_ReadUsingCharUUID(connHandle, pReq, appTaskID);
        }
      }
      break;

    case ATT_READ_BY_TYPE_RSP:
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseReadByTypeRsp(pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          stat = ATT_ReadByTypeRsp(connHandle, &msg.readByTypeRsp);
          if (stat == SUCCESS)
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_READ_REQ:
      if (ATT_ParseReadReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                             pCmd->len-2, &msg) == SUCCESS)
      {
        // Read Characteristic Value or Read Characteristic Descriptor
        stat = GATT_ReadCharValue(connHandle, &msg.readReq, appTaskID);
      }
      break;

    case ATT_READ_RSP:
      // Create payload if attribute value is present
      if (pCmd->len > 2)
      {
        pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      }

      if ((pCmd->len == 2) || (pPayload != NULL))
      {
        if (ATT_ParseReadRsp(pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          stat = ATT_ReadRsp(connHandle, &msg.readRsp);
          if  (stat == SUCCESS)
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_READ_BLOB_REQ:
      if (ATT_ParseReadBlobReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                 pCmd->len-2, &msg) == SUCCESS)
      {
        // Read long characteristic value
        stat = GATT_ReadLongCharValue(connHandle, &msg.readBlobReq, appTaskID);
      }
      break;

    case ATT_READ_BLOB_RSP:
      // Create payload if attribute value is present
      if (pCmd->len > 2)
      {
        pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      }

      if ((pCmd->len == 2) || (pPayload != NULL))
      {
        if (ATT_ParseReadBlobRsp(pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          stat = ATT_ReadBlobRsp(connHandle, &msg.readBlobRsp);
          if (stat == SUCCESS)
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_READ_MULTI_REQ:
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseReadMultiReq(ATT_SIG_NOT_INCLUDED, FALSE, pPayload,
                                    pCmd->len-2, &msg) == SUCCESS)
        {
          stat = GATT_ReadMultiCharValues(connHandle, &msg.readMultiReq, appTaskID);
          if (stat == SUCCESS)
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_READ_MULTI_RSP:
      // Create payload if attribute value is present
      if (pCmd->len > 2)
      {
        pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      }

      if ((pCmd->len == 2) || (pPayload != NULL))
      {
        if (ATT_ParseReadMultiRsp(pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          stat = ATT_ReadMultiRsp(connHandle, &msg.readMultiRsp);
          if (stat == SUCCESS)
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_READ_BY_GRP_TYPE_REQ:
      if (ATT_ParseReadByTypeReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                   pCmd->len-2, &msg) == SUCCESS)
      {
        attReadByGrpTypeReq_t *pReq = &msg.readByGrpTypeReq;

        // Find out what's been requested
        if (gattPrimaryServiceType(pReq->type)     &&
             (pReq->startHandle == GATT_MIN_HANDLE) &&
             (pReq->endHandle   == GATT_MAX_HANDLE))
        {
          // Discover all primary services
          stat = GATT_DiscAllPrimaryServices(connHandle, appTaskID);
        }
        else
        {
          stat = INVALIDPARAMETER;
        }
      }
      break;

    case ATT_READ_BY_GRP_TYPE_RSP:
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseReadByGrpTypeRsp(pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          stat = ATT_ReadByGrpTypeRsp(connHandle, &msg.readByGrpTypeRsp);
          if (stat == SUCCESS)
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_WRITE_REQ:
      pPayload = createSignedMsgPayload(pBuf[2], pBuf[3], &pBuf[4], pCmd->len-4);
      if (pPayload != NULL)
      {
        if (ATT_ParseWriteReq(pBuf[2], pBuf[3], pPayload,
                                pCmd->len-4, &msg) == SUCCESS)
        {
          attWriteReq_t *pReq = &msg.writeReq;

          if (pReq->cmd == FALSE)
          {
            // Write Characteristic Value or Write Characteristic Descriptor
            stat = GATT_WriteCharValue(connHandle, pReq, appTaskID);
          }
          else
          {
            if (pReq->sig == FALSE)
            {
              // Write Without Response
              stat = GATT_WriteNoRsp(connHandle, pReq);
            }
            else
            {
              // Signed Write Without Response
              stat = GATT_SignedWriteNoRsp(connHandle, pReq);
            }
          }

          if ((stat == SUCCESS) && (pReq->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_WRITE_RSP:
      if (ATT_ParseWriteRsp(&pBuf[2], pCmd->len-2, &msg) == SUCCESS)
      {
        stat = ATT_WriteRsp(connHandle);
      }
      break;

    case ATT_PREPARE_WRITE_REQ:
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParsePrepareWriteReq(ATT_SIG_NOT_INCLUDED, FALSE, pPayload,
                                       pCmd->len-2, &msg) == SUCCESS)
        {
          attPrepareWriteReq_t *pReq = &msg.prepareWriteReq;

#if !defined(GATT_DB_OFF_CHIP) && defined(TESTMODES)
          if (GATTServApp_GetParamValue() == GATT_TESTMODE_PREPARE_WRITE)
          {
            // Send the Prepare Write Request right away - needed for GATT testing
            stat = GATT_PrepareWriteReq(connHandle, pReq, appTaskID);
            if ((stat == SUCCESS) && (pReq->pValue != NULL))
            {
              safeToDealloc = FALSE; // payload passed to GATT
            }
          }
          else
#endif // !GATT_DB_OFF_CHIP && TESTMODE
          {
            // GATT Reliable Writes
            if (pPrepareWrites == NULL)
            {
              // First allocated buffer for the Prepare Write Requests
              pPrepareWrites = ICall_malloc(GATT_MAX_NUM_RELIABLE_WRITES * sizeof(attPrepareWriteReq_t));
            }

            if (pPrepareWrites != NULL)
            {
              if (numPrepareWrites < GATT_MAX_NUM_RELIABLE_WRITES)
              {
                // Save the Prepare Write Request for now
                VOID memcpy(&(pPrepareWrites[numPrepareWrites++]),
                                  pReq, sizeof(attPrepareWriteReq_t));

                // Request was parsed and saved successfully
                stat = SUCCESS;
                safeToDealloc = FALSE; // payload saved for later
              }
              else
              {
                stat = INVALIDPARAMETER;
              }
            }
            else
            {
              stat = bleMemAllocError;
            }
          }
        }
      }
      break;

    case ATT_PREPARE_WRITE_RSP:
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParsePrepareWriteRsp(pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          attPrepareWriteRsp_t *pRsp = &msg.prepareWriteRsp;

          stat = ATT_PrepareWriteRsp(connHandle, pRsp);
          if ((stat == SUCCESS) && (pRsp->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_EXECUTE_WRITE_REQ:
      if (ATT_ParseExecuteWriteReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                     pCmd->len-2, &msg) == SUCCESS)
      {
#if !defined(GATT_DB_OFF_CHIP) && defined(TESTMODES)
        if (GATTServApp_GetParamValue() == GATT_TESTMODE_PREPARE_WRITE)
        {
          // Send the Execute Write Request right away - needed for GATT testing
          stat = GATT_ExecuteWriteReq(connHandle, &msg.executeWriteReq, appTaskID);
        }
        else
#endif // !GATT_DB_OFF_CHIP && TESTMODE
        if (pPrepareWrites != NULL)
        {
          // GATT Reliable Writes - send all saved Prepare Write Requests
          stat = GATT_ReliableWrites(connHandle, pPrepareWrites, numPrepareWrites,
                                      msg.executeWriteReq.flags, appTaskID);
          if (stat != SUCCESS)
          {
        	uint8_t i;

            // Free saved payload buffers
            for (i = 0; i < numPrepareWrites; i++)
            {
              if (pPrepareWrites[i].pValue != NULL)
              {
                BM_free(pPrepareWrites[i].pValue);
              }
            }

            ICall_free(pPrepareWrites);
          }
          // else pPrepareWrites will be freed by GATT Client

          // Reset GATT Reliable Writes variables
          pPrepareWrites = NULL;
          numPrepareWrites = 0;
        }
        else
        {
          stat = INVALIDPARAMETER;
        }
      }
      break;

    case ATT_EXECUTE_WRITE_RSP:
      if (ATT_ParseExecuteWriteRsp(&pBuf[2], pCmd->len-2, &msg) == SUCCESS)
      {
        stat = ATT_ExecuteWriteRsp(connHandle);
      }
      break;

    case ATT_HANDLE_VALUE_NOTI:
      pPayload = createMsgPayload(&pBuf[3], pCmd->len-3);
      if (pPayload != NULL)
      {
        if (ATT_ParseHandleValueInd(ATT_SIG_NOT_INCLUDED, FALSE, pPayload,
                                      pCmd->len-3, &msg) == SUCCESS)
        {
          attHandleValueNoti_t *pNoti = &msg.handleValueNoti;

          stat = GATT_Notification(connHandle, pNoti, pBuf[2]);
          if ((stat == SUCCESS) && (pNoti->pValue!= NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_HANDLE_VALUE_IND:
      pPayload = createMsgPayload(&pBuf[3], pCmd->len-3);
      if (pPayload != NULL)
      {
        if (ATT_ParseHandleValueInd(ATT_SIG_NOT_INCLUDED, FALSE, pPayload,
                                      pCmd->len-3, &msg) == SUCCESS)
        {
          attHandleValueInd_t *pInd = &msg.handleValueInd;

          stat = GATT_Indication(connHandle, pInd, pBuf[2], appTaskID);
          if ((stat == SUCCESS) && (pInd->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_HANDLE_VALUE_CFM:
      if (ATT_ParseHandleValueCfm(&pBuf[2], pCmd->len-2, &msg) == SUCCESS)
      {
        stat = ATT_HandleValueCfm(connHandle);
      }
      break;

    default:
      stat = FAILURE;
      break;
  }

  if ((pPayload != NULL) && safeToDealloc)
  {
    // Free payload
    BM_free(pPayload);
  }

  return(mapATT2BLEStatus(stat));
}

/*********************************************************************
 * @fn      processExtMsgGATT
 *
 * @brief   Parse and process incoming HCI extension GATT messages.
 *
 * @param   cmdID - incoming HCI extension command ID.
 * @param   pCmd - incoming HCI extension message.
 * @param   pRspDataLen - response data length to be returned.
 *
 * @return  SUCCESS, INVALIDPARAMETER, FAILURE,
 *          bleInvalidPDU or bleMemAllocError
 */
static uint8_t processExtMsgGATT(uint8_t cmdID, hciExtCmd_t *pCmd, uint8_t *pRspDataLen)
{
#if defined(GATT_DB_OFF_CHIP)
  static uint16_t totalAttrs = 0;
  static gattService_t service = { 0, NULL };
#endif // GATT_DB_OFF_CHIP
  uint8_t *pBuf, *pPayload = NULL, safeToDealloc = TRUE;
  uint16_t connHandle;
  attMsg_t msg;
  bStatus_t stat = bleInvalidPDU;

  // Make sure received buffer is at lease 2-otect long
  if (pCmd->len < 2)
  {
    return(stat);
  }

  pBuf = pCmd->pData;
  connHandle = BUILD_UINT16(pBuf[0], pBuf[1]);

  switch(cmdID)
  {
    case ATT_EXCHANGE_MTU_REQ: // GATT Exchange MTU
      if (ATT_ParseExchangeMTUReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                    pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_ExchangeMTU(connHandle, &msg.exchangeMTUReq, appTaskID);
      }
      break;

    case ATT_READ_BY_GRP_TYPE_REQ: // GATT Discover All Primary Services
      stat = GATT_DiscAllPrimaryServices(connHandle, appTaskID);
      break;

    case ATT_FIND_BY_TYPE_VALUE_REQ: // GATT Discover Primary Service By UUID
      stat = GATT_DiscPrimaryServiceByUUID(connHandle, &pBuf[2],
                                            pCmd->len-2, appTaskID);
      break;

    case GATT_FIND_INCLUDED_SERVICES: // GATT Find Included Services
    case GATT_DISC_ALL_CHARS: // GATT Discover All Characteristics
      if ((pCmd->len-2) == ATT_READ_BY_TYPE_REQ_FIXED_SIZE)
      {
        // First requested handle number
        uint16_t startHandle = BUILD_UINT16(pBuf[2], pBuf[3]);

        // Last requested handle number
        uint16_t endHandle = BUILD_UINT16(pBuf[4], pBuf[5]);

        if (cmdID == GATT_FIND_INCLUDED_SERVICES)
        {
          stat = GATT_FindIncludedServices(connHandle, startHandle,
                                            endHandle, appTaskID);
        }
        else
        {
          stat = GATT_DiscAllChars(connHandle, startHandle,
                                    endHandle, appTaskID);
        }
      }
      break;

    case ATT_READ_BY_TYPE_REQ: // GATT Discover Characteristics by UUID
      if (ATT_ParseReadByTypeReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                   pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_DiscCharsByUUID(connHandle, &msg.readByTypeReq, appTaskID);
      }
      break;

    case ATT_FIND_INFO_REQ: // GATT Discover All Characteristic Descriptors
      if (ATT_ParseFindInfoReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                 pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_DiscAllCharDescs(connHandle, msg.findInfoReq.startHandle,
                                      msg.findInfoReq.endHandle, appTaskID);
      }
      break;

    case ATT_READ_REQ: // GATT Read Characteristic Value
      if (ATT_ParseReadReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                             pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_ReadCharValue(connHandle, &msg.readReq, appTaskID);
      }
      break;

    case GATT_READ_USING_CHAR_UUID: // GATT Read Using Characteristic UUID
      if (ATT_ParseReadByTypeReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                   pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_ReadUsingCharUUID(connHandle, &msg.readByTypeReq, appTaskID);
      }
      break;

    case ATT_READ_BLOB_REQ: // GATT Read Long Characteristic Value
      if (ATT_ParseReadBlobReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                 pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_ReadLongCharValue(connHandle, &msg.readBlobReq, appTaskID);
      }
      break;

    case ATT_READ_MULTI_REQ: // GATT Read Multiple Characteristic Values
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseReadMultiReq(ATT_SIG_NOT_INCLUDED, FALSE, pPayload,
                                    pCmd->len-2, &msg) == SUCCESS)
        {
          stat = GATT_ReadMultiCharValues(connHandle, &msg.readMultiReq, appTaskID);
          if (stat == SUCCESS)
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case GATT_WRITE_NO_RSP: // GATT Write Without Response
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseWriteReq(FALSE, TRUE, pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          attWriteReq_t *pReq = &msg.writeReq;

          stat = GATT_WriteNoRsp(connHandle, pReq);
          if ((stat == SUCCESS) && (pReq->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case GATT_SIGNED_WRITE_NO_RSP: // GATT Signed Write Without Response
      pPayload = createSignedMsgPayload(TRUE, TRUE, &pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseWriteReq(TRUE, TRUE, pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          attWriteReq_t *pReq = &msg.writeReq;

          stat = GATT_SignedWriteNoRsp(connHandle, pReq);
          if ((stat == SUCCESS) && (pReq->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_WRITE_REQ: // GATT Write Characteristic Value
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseWriteReq(FALSE, FALSE, pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          attWriteReq_t *pReq = &msg.writeReq;

          stat = GATT_WriteCharValue(connHandle, pReq, appTaskID);
          if ((stat == SUCCESS) && (pReq->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_PREPARE_WRITE_REQ: // GATT Write Long Characteristic Value
    case GATT_WRITE_LONG_CHAR_DESC: // GATT Write Long Characteristic Descriptor
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParsePrepareWriteReq(ATT_SIG_NOT_INCLUDED, FALSE, pPayload,
                                       pCmd->len-2, &msg) == SUCCESS)
        {
          attPrepareWriteReq_t *pReq = &msg.prepareWriteReq;
          if (cmdID == ATT_PREPARE_WRITE_REQ)
          {
            stat = GATT_WriteLongCharValue(connHandle, pReq, appTaskID);
          }
          else
          {
            stat = GATT_WriteLongCharDesc(connHandle, pReq, appTaskID);
          }

          if ((stat == SUCCESS) && (pReq->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case GATT_RELIABLE_WRITES: // GATT Reliable Writes
      if (pCmd->len-2 > 0)
      {
        uint8_t numReqs = pBuf[2];

        if ((numReqs > 0) && (numReqs <= GATT_MAX_NUM_RELIABLE_WRITES))
        {
          // First allocated buffer for the Prepare Write Requests
          attPrepareWriteReq_t *pReqs = ICall_malloc(numReqs * sizeof(attPrepareWriteReq_t));
          if (pReqs != NULL)
          {
            uint8_t i;

            VOID memset(pReqs, 0, numReqs * sizeof(attPrepareWriteReq_t));

            pBuf += 3; // pass connHandle and numReqs

            // Create payload buffer for each Prepare Write Request
            for (i = 0; i < numReqs; i++)
            {
              // length of request is length of attribute value plus fixed fields.
              // request format: length (1) + handle (2) + offset (2) + attribute value
              uint8_t reqLen = ATT_PREPARE_WRITE_REQ_FIXED_SIZE;

              if (*pBuf > 0)
              {
                reqLen += *pBuf++;
                pPayload = createMsgPayload(pBuf, reqLen);
                if (pPayload == NULL)
                {
                  stat = bleMemAllocError;
                  break;
                }

                VOID ATT_ParsePrepareWriteReq(ATT_SIG_NOT_INCLUDED, FALSE, pPayload,
                                               reqLen, (attMsg_t *)&(pReqs[i]));
              }
              else // no attribute value
              {
                pBuf++;
                VOID ATT_ParsePrepareWriteReq(ATT_SIG_NOT_INCLUDED, FALSE, pBuf,
                                               reqLen, (attMsg_t *)&(pReqs[i]));
              }

              // Next request
              pBuf += reqLen;
            }

            // See if all requests were parsed successfully
            if (i == numReqs)
            {
              // Send all saved Prepare Write Requests
              stat = GATT_ReliableWrites(connHandle, pReqs, numReqs,
                                          ATT_WRITE_PREPARED_VALUES, appTaskID);
            }

            if (stat != SUCCESS)
            {
              // Free payload buffers first
              for (i = 0; i < numReqs; i++)
              {
                if (pReqs[i].pValue != NULL)
                {
                  BM_free(pReqs[i].pValue);
                }
              }

              ICall_free(pReqs);
            }
            // else pReqs will be freed by GATT Client
          }
          else
          {
            stat = bleMemAllocError;
          }
        }
        else
        {
          stat = INVALIDPARAMETER;
        }
      }
      break;

    case GATT_READ_CHAR_DESC: // GATT Read Characteristic Descriptor
      if (ATT_ParseReadReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                             pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_ReadCharDesc(connHandle, &msg.readReq, appTaskID);
      }
      break;

    case GATT_READ_LONG_CHAR_DESC: // GATT Read Long Characteristic Descriptor
      if (ATT_ParseReadBlobReq(ATT_SIG_NOT_INCLUDED, FALSE, &pBuf[2],
                                 pCmd->len-2, &msg) == SUCCESS)
      {
        stat = GATT_ReadLongCharDesc(connHandle, &msg.readBlobReq, appTaskID);
      }
      break;

    case GATT_WRITE_CHAR_DESC: // GATT Write Characteristic Descriptor
      pPayload = createMsgPayload(&pBuf[2], pCmd->len-2);
      if (pPayload != NULL)
      {
        if (ATT_ParseWriteReq(FALSE, FALSE, pPayload, pCmd->len-2, &msg) == SUCCESS)
        {
          attWriteReq_t *pReq = &msg.writeReq;

          stat = GATT_WriteCharDesc(connHandle, pReq, appTaskID);
          if ((stat == SUCCESS) && (pReq->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_HANDLE_VALUE_NOTI:
      pPayload = createMsgPayload(&pBuf[3], pCmd->len-3);
      if (pPayload != NULL)
      {
        if (ATT_ParseHandleValueInd(ATT_SIG_NOT_INCLUDED, FALSE, pPayload,
                                      pCmd->len-3, &msg) == SUCCESS)
        {
          attHandleValueNoti_t *pNoti = &msg.handleValueNoti;

          stat = GATT_Notification(connHandle, pNoti, pBuf[2]);
          if ((stat == SUCCESS) && (pNoti->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

    case ATT_HANDLE_VALUE_IND:
      pPayload = createMsgPayload(&pBuf[3], pCmd->len-3);
      if (pPayload != NULL)
      {
        if (ATT_ParseHandleValueInd(ATT_SIG_NOT_INCLUDED, FALSE, pPayload,
                                      pCmd->len-3, &msg) == SUCCESS)
        {
          attHandleValueInd_t *pInd = &msg.handleValueInd;

          stat = GATT_Indication(connHandle, pInd, pBuf[2], appTaskID);
          if ((stat == SUCCESS) && (pInd->pValue != NULL))
          {
            safeToDealloc = FALSE; // payload passed to GATT
          }
        }
      }
      break;

#if defined(GATT_DB_OFF_CHIP) // These GATT commands don't include connHandle field
    case HCI_EXT_GATT_ADD_SERVICE:
      if (service.attrs == NULL)
      {
        // Service type must be 2 octets (Primary or Secondary)
        if (pCmd->len-3 == ATT_BT_UUID_SIZE)
        {
          uint16_t uuid = BUILD_UINT16(pBuf[0], pBuf[1]);
          uint16_t numAttrs = BUILD_UINT16(pBuf[2], pBuf[3]);

          if (((uuid == GATT_PRIMARY_SERVICE_UUID)     ||
                 (uuid == GATT_SECONDARY_SERVICE_UUID)) &&
               (numAttrs > 0))
          {
            uint8_t encKeySize = pBuf[4];

            if ((encKeySize >= GATT_MIN_ENCRYPT_KEY_SIZE) &&
                 (encKeySize <= GATT_MAX_ENCRYPT_KEY_SIZE))
            {
              // Allocate buffer for the attribute table
              service.attrs = ICall_malloc(numAttrs * sizeof(gattAttribute_t));
              if (service.attrs != NULL)
              {
                // Zero out all attribute fields
                VOID memset(service.attrs, 0, numAttrs * sizeof(gattAttribute_t));

                totalAttrs = numAttrs;
                service.encKeySize = encKeySize;

                // Set up service record
                stat = addAttrRec(&service, pBuf, ATT_BT_UUID_SIZE,
                                   GATT_PERMIT_READ, &totalAttrs, pRspDataLen);
              }
              else
              {
                stat = bleMemAllocError;
              }
            }
            else
            {
              stat = bleInvalidRange;
            }
          }
          else
          {
            stat = INVALIDPARAMETER;
          }
        }
      }
      else
      {
        stat = blePending;
      }
      break;

    case HCI_EXT_GATT_DEL_SERVICE:
      {
        uint16_t handle = BUILD_UINT16(pBuf[0], pBuf[1]);

        if (handle == 0x0000)
        {
          // Service is not registered with GATT yet
          freeAttrRecs(&service);

          totalAttrs = 0;
        }
        else
        {
          gattService_t serv;

          // Service is already registered with the GATT Server
          stat = GATT_DeregisterService(handle, &serv);
          if (stat == SUCCESS)
          {
            freeAttrRecs(&serv);
          }
        }

        stat = SUCCESS;
      }
      break;

    case HCI_EXT_GATT_ADD_ATTRIBUTE:
      if (service.attrs != NULL)
      {
        if ((pCmd->len-1 == ATT_UUID_SIZE) ||
             (pCmd->len-1 == ATT_BT_UUID_SIZE))
        {
          // Add attribute record to the service being added
          stat = addAttrRec(&service, pBuf, pCmd->len-1,
                             pBuf[pCmd->len-1], &totalAttrs, pRspDataLen);
        }
      }
      else // no corresponding service
      {
        stat = INVALIDPARAMETER;
      }
      break;
#endif // GATT_DB_OFF_CHIP

    default:
      stat = FAILURE;
      break;
  }

  if ((pPayload != NULL) && safeToDealloc)
  {
    // Free payload
    BM_free(pPayload);
  }

  return(mapATT2BLEStatus(stat));
}

/*********************************************************************
 * @fn      processExtMsgGAP
 *
 * @brief   Parse and process incoming HCI extension GAP messages.
 *
 * @param   cmdID - incoming HCI extension command ID.
 * @param   pCmd - incoming HCI extension message.
 * @param   pRspDataLen - response data length to be returned.
 *
 * @return  SUCCESS, INVALIDPARAMETER, FAILURE,
 *          or bleMemAllocError
 */
static uint8_t processExtMsgGAP(uint8_t cmdID, hciExtCmd_t *pCmd, uint8_t *pRspDataLen)
{
  uint8_t *pBuf = pCmd->pData;
  bStatus_t stat = SUCCESS;

  switch(cmdID)
  {
    case HCI_EXT_GAP_DEVICE_INIT:
      {
        uint32_t signCounter;
        uint8_t profileRole = pBuf[0];

        // Copy the IRK, SRK and sign counter from the command if they aren't all "0",
        // otherwise use what's in NV
        if (Util_isBufSet(&pBuf[2], 0, KEYLEN) == FALSE)
        {
          VOID memcpy(IRK, &pBuf[2], KEYLEN);
        }

        if (Util_isBufSet(&pBuf[2+KEYLEN], 0, KEYLEN) == FALSE)
        {
          VOID memcpy(SRK, &pBuf[2+KEYLEN], KEYLEN);
        }

        signCounter = BUILD_UINT32(pBuf[2+KEYLEN+KEYLEN+0], pBuf[2+KEYLEN+KEYLEN+1],
                                        pBuf[2+KEYLEN+KEYLEN+2], pBuf[2+KEYLEN+KEYLEN+3]);
        if (signCounter > 0)
        {
          hciExtSignCounter = signCounter;
        }

        stat = GAP_DeviceInit(appTaskID, profileRole, pBuf[1], IRK, SRK, &hciExtSignCounter);

#if defined(GAP_BOND_MGR)
       // Register with bond manager after starting device.
       GAPBondMgr_Register((gapBondCBs_t *) &host_tl_bondCB);
#endif

        // Take over the processing of Authentication messages
        VOID GAP_SetParamValue(TGAP_AUTH_TASK_ID, PROXY_ID(appTaskID));
      }
      break;

#if (HOST_CONFIG & (CENTRAL_CFG | OBSERVER_CFG))
    case HCI_EXT_GAP_DEVICE_DISC_REQ:
      {
        gapDevDiscReq_t discReq;

        discReq.taskID = PROXY_ID(appTaskID);
        discReq.mode = *pBuf++;
        discReq.activeScan = *pBuf++;
        discReq.whiteList = *pBuf;

        stat = GAP_DeviceDiscoveryRequest(&discReq);
      }
      break;

    case HCI_EXT_GAP_DEVICE_DISC_CANCEL:
      stat = GAP_DeviceDiscoveryCancel(appTaskID);
      break;
#endif // OBSERVER_CFG | CENTRAL_CFG

#if (HOST_CONFIG & CENTRAL_CFG)
    case HCI_EXT_GAP_EST_LINK_REQ:
      {
        gapEstLinkReq_t linkReq;

        linkReq.taskID = PROXY_ID(appTaskID);
        linkReq.highDutyCycle = *pBuf++;
        linkReq.whiteList = *pBuf++;
        linkReq.addrTypePeer = *pBuf++;
        VOID memcpy(linkReq.peerAddr, pBuf, B_ADDR_LEN);

        stat =  GAP_EstablishLinkReq(&linkReq);
      }
      break;
#endif // CENTRAL_CFG

#if (HOST_CONFIG & (PERIPHERAL_CFG | BROADCASTER_CFG))
    case HCI_EXT_GAP_MAKE_DISCOVERABLE:
      {
        gapAdvertisingParams_t adParams;

        adParams.eventType = *pBuf++;
        adParams.initiatorAddrType = *pBuf++;
        VOID memcpy(adParams.initiatorAddr, pBuf, B_ADDR_LEN);
        pBuf += B_ADDR_LEN;
        adParams.channelMap = *pBuf++;
        adParams.filterPolicy = *pBuf;
        stat = GAP_MakeDiscoverable(appTaskID, &adParams);
      }
      break;

    case HCI_EXT_GAP_UPDATE_ADV_DATA:
      stat = GAP_UpdateAdvertisingData(appTaskID, pBuf[0], pBuf[1], &pBuf[2]);
      break;

    case HCI_EXT_GAP_END_DISC:
      stat = GAP_EndDiscoverable(appTaskID);
      break;
#endif // PERIPHERAL_CFG | BROADCASTER_CFG

#if (HOST_CONFIG & (CENTRAL_CFG | PERIPHERAL_CFG))
    case HCI_EXT_GAP_CONFIG_DEVICE_ADDR:
      {
        uint8_t *pStaticAddr = NULL;
        uint8_t nullAddr[B_ADDR_LEN] = {0};

        if (memcmp(&pBuf[1], nullAddr, B_ADDR_LEN))
        {
          pStaticAddr = &pBuf[1];
        }

        stat = GAP_ConfigDeviceAddr(pBuf[0], pStaticAddr);
      }
      break;

    case HCI_EXT_GAP_TERMINATE_LINK:
      stat = GAP_TerminateLinkReq(appTaskID, BUILD_UINT16(pBuf[0], pBuf[1]), pBuf[2]);
      break;

    case HCI_EXT_GAP_UPDATE_LINK_PARAM_REQ:
      {
        gapUpdateLinkParamReq_t updateLinkReq;

        updateLinkReq.connectionHandle = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        updateLinkReq.intervalMin = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        updateLinkReq.intervalMax = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        updateLinkReq.connLatency = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        updateLinkReq.connTimeout = BUILD_UINT16(pBuf[0], pBuf[1]);

        stat = GAP_UpdateLinkParamReq(&updateLinkReq);
      }
      break;

    case HCI_EXT_GAP_UPDATE_LINK_PARAM_REQ_REPLY:
      {
        gapUpdateLinkParamReqReply_t updateLinkReply;

        updateLinkReply.connectionHandle = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        updateLinkReply.intervalMin = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        updateLinkReply.intervalMax = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        updateLinkReply.connLatency = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        updateLinkReply.connTimeout = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        updateLinkReply.accepted    = *pBuf;

        stat = GAP_UpdateLinkParamReqReply(&updateLinkReply);
      }
      break;

#ifndef NO_BLE_SECURITY
    case HCI_EXT_GAP_AUTHENTICATE:
      {
        uint8_t tmp;
        gapAuthParams_t params;
        gapPairingReq_t pairReq;
        gapPairingReq_t *pPairReq = NULL;

        VOID memset(&params, 0, sizeof (gapAuthParams_t));

        params.connectionHandle = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;

        params.secReqs.ioCaps = *pBuf++;
        params.secReqs.oobAvailable = *pBuf++;
        VOID memcpy(params.secReqs.oob, pBuf, KEYLEN);
        pBuf += KEYLEN;

#if defined(BLE_V42_FEATURES) && (BLE_V42_FEATURES & SECURE_CONNS_CFG)
        VOID memcpy(params.secReqs.oobConfirm, pBuf, KEYLEN);
        pBuf += KEYLEN;
        params.secReqs.localOobAvailable = *pBuf++;
        VOID memcpy(params.secReqs.localOob, pBuf, KEYLEN);
        pBuf += KEYLEN;
        params.secReqs.isSCOnlyMode = *pBuf++;
        params.secReqs.eccKeys.isUsed = *pBuf++;
        VOID memcpy(params.secReqs.eccKeys.sK, pBuf, SM_ECC_KEY_LEN);
        pBuf += SM_ECC_KEY_LEN;
        VOID memcpy(params.secReqs.eccKeys.pK_x, pBuf, SM_ECC_KEY_LEN);
        pBuf += SM_ECC_KEY_LEN;
        VOID memcpy(params.secReqs.eccKeys.pK_y, pBuf, SM_ECC_KEY_LEN);
        pBuf += SM_ECC_KEY_LEN;
#else
        // Skip the Secure Connection bits.
        pBuf += 131;
#endif //(BLE_V42_FEATURES & SECURE_CONNS_CFG)

        params.secReqs.authReq = *pBuf++;
        params.secReqs.maxEncKeySize = *pBuf++;

        tmp = *pBuf++;
        params.secReqs.keyDist.sEncKey = (tmp & KEYDIST_SENC) ? TRUE : FALSE;
        params.secReqs.keyDist.sIdKey = (tmp & KEYDIST_SID) ? TRUE : FALSE;
        params.secReqs.keyDist.sSign = (tmp & KEYDIST_SSIGN) ? TRUE : FALSE;
        params.secReqs.keyDist.sLinkKey = (tmp & KEYDIST_SLINK) ? TRUE : FALSE;
        params.secReqs.keyDist.sReserved = 0;
        params.secReqs.keyDist.mEncKey = (tmp & KEYDIST_MENC) ? TRUE : FALSE;
        params.secReqs.keyDist.mIdKey = (tmp & KEYDIST_MID) ? TRUE : FALSE;
        params.secReqs.keyDist.mSign = (tmp & KEYDIST_MSIGN) ? TRUE : FALSE;
        params.secReqs.keyDist.mLinkKey = (tmp & KEYDIST_MLINK) ? TRUE : FALSE;
        params.secReqs.keyDist.mReserved = 0;

        tmp = *pBuf++;
        if (tmp)
        {
          pairReq.ioCap = *pBuf++;
          pairReq.oobDataFlag = *pBuf++;
          pairReq.authReq = *pBuf++;
          pairReq.maxEncKeySize = *pBuf++;
          tmp = *pBuf++;
          pairReq.keyDist.sEncKey = (tmp & KEYDIST_SENC) ? TRUE : FALSE;
          pairReq.keyDist.sIdKey = (tmp & KEYDIST_SID) ? TRUE : FALSE;
          pairReq.keyDist.sSign = (tmp & KEYDIST_SSIGN) ? TRUE : FALSE;
          pairReq.keyDist.sLinkKey = (tmp & KEYDIST_SLINK) ? TRUE : FALSE;
          pairReq.keyDist.sReserved = 0;
          pairReq.keyDist.mEncKey = (tmp & KEYDIST_MENC) ? TRUE : FALSE;
          pairReq.keyDist.mIdKey = (tmp & KEYDIST_MID) ? TRUE : FALSE;
          pairReq.keyDist.mSign = (tmp & KEYDIST_MSIGN) ? TRUE : FALSE;
          pairReq.keyDist.mLinkKey = (tmp & KEYDIST_MLINK) ? TRUE : FALSE;
          pairReq.keyDist.mReserved = 0;
          pPairReq = &pairReq;
        }

        stat = GAP_Authenticate(&params, pPairReq);

      }
      break;

    case HCI_EXT_GAP_TERMINATE_AUTH:
      stat = GAP_TerminateAuth(BUILD_UINT16(pBuf[0], pBuf[1]), pBuf[2]);
      break;

    case HCI_EXT_GAP_BOND:
      {
        uint16_t connectionHandle;
        smSecurityInfo_t securityInfo;
        uint8_t authenticated;
        uint8_t secureConnections;

        // Do Security Information part
        connectionHandle = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        authenticated = *pBuf++;
        secureConnections = *pBuf++;
        VOID memcpy(securityInfo.ltk, pBuf, KEYLEN);
        pBuf += KEYLEN;
        securityInfo.div = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;
        VOID memcpy(securityInfo.rand, pBuf, B_RANDOM_NUM_SIZE);
        pBuf += B_RANDOM_NUM_SIZE;
        securityInfo.keySize = *pBuf++;

        stat = GAP_Bond(connectionHandle, authenticated, secureConnections,
                         &securityInfo, TRUE);
      }
      break;

    case HCI_EXT_GAP_SIGNABLE:
      {
        uint16_t connectionHandle;
        uint8_t authenticated;
        smSigningInfo_t signing;

        connectionHandle = BUILD_UINT16(pBuf[0], pBuf[1]);
        pBuf += 2;

        authenticated = *pBuf++;

        VOID memcpy(signing.srk, pBuf, KEYLEN);
        pBuf += KEYLEN;

        signing.signCounter = BUILD_UINT32(pBuf[0], pBuf[1], pBuf[2], pBuf[3]);

        stat = GAP_Signable(connectionHandle, authenticated, &signing);
      }
      break;

    case HCI_EXT_GAP_PASSKEY_UPDATE:
      stat = GAP_PasskeyUpdate(&pBuf[2], BUILD_UINT16(pBuf[0], pBuf[1]));
      break;

#if !defined (BLE_V42_FEATURES) || !(BLE_V42_FEATURES & PRIVACY_1_2_CFG)
    case HCI_EXT_GAP_RESOLVE_PRIVATE_ADDR:
        stat = GAP_ResolvePrivateAddr(&pBuf[0], &pBuf[KEYLEN]);
      break;
#endif // ! BLE_V42_FEATURES | ! PRIVACY_1_2_CFG
#endif // CENTRAL_CFG | PERIPHERAL_CFG
#endif //NO_BLE_SECURITY

    case HCI_EXT_GAP_SET_PARAM:
      {
        uint16_t id = (uint16_t)pBuf[0];
        uint16_t value = BUILD_UINT16(pBuf[1], pBuf[2]);

        if ((id != TGAP_AUTH_TASK_ID) && (id < TGAP_PARAMID_MAX))
        {
          stat = GAP_SetParamValue(id, value);
        }
#if !defined(GATT_DB_OFF_CHIP) && defined(TESTMODES)
        else if (id == TGAP_GATT_TESTCODE)
        {
          GATTServApp_SetParamValue(value);
        }
        else if (id == TGAP_ATT_TESTCODE)
        {
          ATT_SetParamValue(value);
        }
        else if (id == TGAP_GGS_TESTCODE)
        {
          GGS_SetParamValue(value);
        }
        else if (id == TGAP_L2CAP_TESTCODE)
        {
          L2CAP_SetParamValue(value);
        }
#endif // !GATT_DB_OFF_CHIP && TESTMODES
        else
        {
          stat = INVALIDPARAMETER;
        }
      }
      break;

    case HCI_EXT_GAP_GET_PARAM:
      {
        uint16_t paramValue = 0xFFFF;
        uint16_t param = (uint16_t)pBuf[0];

        if (param < 0x00FF)
        {
          if ((param != TGAP_AUTH_TASK_ID) && (param < TGAP_PARAMID_MAX))
          {
            paramValue = GAP_GetParamValue(param);
          }
#if !defined(GATT_DB_OFF_CHIP) && defined(TESTMODES)
          else if (param == TGAP_GATT_TESTCODE)
          {
            paramValue = GATTServApp_GetParamValue();
          }
          else if (param == TGAP_ATT_TESTCODE)
          {
            paramValue = ATT_GetParamValue();
          }
          else if (param == TGAP_GGS_TESTCODE)
          {
            paramValue = GGS_GetParamValue();
          }
          else if (param == TGAP_L2CAP_TESTCODE)
          {
            paramValue = L2CAP_GetParamValue();
          }
#endif // !GATT_DB_OFF_CHIP && TESTMODES
        }
        if (paramValue != 0xFFFF)
        {
          stat = SUCCESS;
        }
        else
        {
          stat = INVALIDPARAMETER;
        }
        *pRspDataLen = 2;
        rspBuf[RSP_PAYLOAD_IDX] = LO_UINT16(paramValue);
        rspBuf[RSP_PAYLOAD_IDX+1] = HI_UINT16(paramValue);
      }
      break;

#if (HOST_CONFIG & PERIPHERAL_CFG)
#ifndef NO_BLE_SECURITY
    case HCI_EXT_GAP_SLAVE_SECURITY_REQ_UPDATE:
      stat = GAP_SendSlaveSecurityRequest(BUILD_UINT16(pBuf[0], pBuf[1]), pBuf[2]);
      break;
#endif //NO_BLE_SECURITY
#endif // PERIPHERAL_CFG

#if (HOST_CONFIG & (PERIPHERAL_CFG | BROADCASTER_CFG))
    case HCI_EXT_GAP_SET_ADV_TOKEN:
      {
        gapAdvDataToken_t *pToken;
        uint8_t attrLen = pBuf[1];

        pToken = (gapAdvDataToken_t *)ICall_malloc(sizeof (gapAdvDataToken_t) + attrLen);
        if (pToken)
        {
          pToken->adType = pBuf[0];
          pToken->attrLen =attrLen;
          pToken->pAttrData = (uint8_t *)(pToken+1);

          VOID memcpy(pToken->pAttrData, &pBuf[2], attrLen);
          stat = GAP_SetAdvToken(pToken);
          if (stat != SUCCESS)
          {
            ICall_free(pToken);
          }
        }
        else
        {
          stat = bleMemAllocError;
        }
      }
      break;

    case HCI_EXT_GAP_REMOVE_ADV_TOKEN:
      {
        gapAdvDataToken_t *pToken = (gapAdvDataToken_t *)GAP_RemoveAdvToken(pBuf[0]);
        if (pToken)
        {
          ICall_free(pToken);
          stat = SUCCESS;
        }
        else
        {
          stat = INVALIDPARAMETER;
        }
      }
      break;

    case HCI_EXT_GAP_UPDATE_ADV_TOKENS:
      {
        stat = GAP_UpdateAdvTokens();
      }
      break;
#endif // PERIPHERAL_CFG | BROADCASTER_CFG

    case HCI_EXT_GAP_BOND_SET_PARAM:
      {
#if defined(GAP_BOND_MGR)
        uint16_t id = BUILD_UINT16(pBuf[0], pBuf[1]);
        uint8_t *pValue;

        if(id == GAPBOND_DEFAULT_PASSCODE)
        {
            // First build passcode
            if ( pBuf[2] == 2 )
              host_tl_defaultPasscode =  ( BUILD_UINT32( pBuf[3], pBuf[3 + 1], 0L, 0L ) );
            else if ( pBuf[2] == 3 )
              host_tl_defaultPasscode =  ( BUILD_UINT32( pBuf[3], pBuf[3 + 1], pBuf[3 + 2], 0L ) );
            else if ( pBuf[2] == 4 )
              host_tl_defaultPasscode =  ( BUILD_UINT32( pBuf[3], pBuf[3 + 1], pBuf[3 + 2], pBuf[3 + 3] ) );
            else
              host_tl_defaultPasscode =  ( (uint32)pBuf[3] );

            stat = SUCCESS;
        }
        else
        {
            pValue = &pBuf[3];
            stat = GAPBondMgr_SetParameter(id, pBuf[2],  pValue);
        }
#else
        stat = INVALIDPARAMETER;
#endif
      }
      break;

    case HCI_EXT_GAP_BOND_GET_PARAM:
      {
        uint8_t len = 0;
#if defined(GAP_BOND_MGR)
        uint16_t id = BUILD_UINT16(pBuf[0], pBuf[1]);

        switch (id)
        {
          case GAPBOND_INITIATE_WAIT:
            stat = GAPBondMgr_GetParameter(id, &rspBuf[RSP_PAYLOAD_IDX]);
            len = 2;
            break;

          case GAPBOND_OOB_DATA:
            stat = GAPBondMgr_GetParameter(id, &rspBuf[RSP_PAYLOAD_IDX]);
            len = KEYLEN;
            break;

          case GAPBOND_DEFAULT_PASSCODE:
            stat =  SUCCESS;
            len = sizeof(host_tl_defaultPasscode);
            memcpy(&rspBuf[RSP_PAYLOAD_IDX], &host_tl_defaultPasscode, len);
            break;

          default:
            stat = GAPBondMgr_GetParameter(id, &rspBuf[RSP_PAYLOAD_IDX]);
            len = 1;
            break;
        }
#else
        stat = INVALIDPARAMETER;
#endif
        *pRspDataLen = len;
      }
      break;

#if !defined(GATT_NO_SERVICE_CHANGED)
    case HCI_EXT_GAP_BOND_SERVICE_CHANGE:
      {
#if defined(GAP_BOND_MGR)
        stat = GAPBondMgr_ServiceChangeInd(BUILD_UINT16(pBuf[0], pBuf[1]), pBuf[2]);
#else
        stat = INVALIDPARAMETER;
#endif
      }
      break;
#endif // !GATT_NO_SERVICE_CHANGED

#ifndef NO_BLE_SECURITY
#if defined(BLE_V42_FEATURES) && (BLE_V42_FEATURES & SECURE_CONNS_CFG)
    case HCI_EXT_SM_REGISTER_TASK:
      SM_RegisterTask(appTaskID);
      break;

    case HCI_EXT_SM_GET_ECCKEYS:
      stat = SM_GetEccKeys();
      break;

    case HCI_EXT_SM_GET_DHKEY:
      {
        // each field is 32 bytes
        stat = SM_GetDHKey(pBuf, &pBuf[32], &pBuf[64]);
      }
      break;

    case HCI_EXT_SM_GET_CONFIRM_OOB:
      stat = SM_GetScConfirmOob(pBuf,
                                &pBuf[32],
                                &rspBuf[RSP_PAYLOAD_IDX]);

      *pRspDataLen = 16;
      break;
#endif //(BLE_V42_FEATURES & SECURE_CONNS_CFG)
#endif //NO_BLE_SECURITY

    default:
      stat = FAILURE;
      break;
  }

  return(stat);
}

/*********************************************************************
 * @fn      createMsgPayload
 *
 * @brief   Create payload buffer for OTA message to be sent.
 *
 * @param   pBuf - pointer to message data
 * @param   len - length of message data
 *
 * @return  pointer to payload. NULL if alloc fails.
 */
static uint8_t *createMsgPayload(uint8_t *pBuf, uint16_t len)
{
  return(createPayload(pBuf, len, 0));
}

#if defined(GAP_BOND_MGR)
/*********************************************************************
 * @fn      host_tl_passcodeCB
 *
 * @brief   Passcode callback.
 *
 * @return  none
 */
static void host_tl_passcodeCB(uint8_t *deviceAddr, uint16_t connectionHandle,
                               uint8_t uiInputs, uint8_t uiOutputs)
{
  if (HCI_TL_CallbackEvtProcessCB)
  {
    passcodeEvtCallback_t * passcodeEvtCallback = ICall_malloc(sizeof(passcodeEvtCallback_t));

    if (passcodeEvtCallback)
    {
        passcodeEvtCallback->connectionHandle = connectionHandle;

        HCI_TL_CallbackEvtProcessCB( (void*) passcodeEvtCallback, (void*) host_tl_passcodeCBProcess);
     }
  }
}

/*********************************************************************
 * @fn      host_tl_passcodeCBProcess
 *
 * @brief   Process passcode event (translation from the callback).
 *
 * @return  none
 */
static void host_tl_passcodeCBProcess(passcodeEvtCallback_t * pPasscodeEvt)
{
  // Send passcode response right away, without going through an event...
  GAPBondMgr_PasscodeRsp(pPasscodeEvt->connectionHandle, SUCCESS, host_tl_defaultPasscode);
}
#endif //GAP_BOND_MGR
/*********************************************************************
 * @fn      createMsgPayload
 *
 * @brief   Create payload buffer for OTA message to be sent.
 *
 * @param   sig - whether to include authentication signature
 * @param   cmd - whether it's write command
 * @param   pBuf - pointer to message data
 * @param   len - length of message data
 *
 * @return  pointer to payload. NULL if alloc fails.
 */
static uint8_t *createSignedMsgPayload(uint8_t sig, uint8_t cmd, uint8_t *pBuf, uint16_t len)
{
  if (sig && cmd)
  {
    // Need to allocate space for Authentication Signature
    return(createPayload(pBuf, len, ATT_AUTHEN_SIG_LEN));
  }

  return(createPayload(pBuf, len, 0));
}

/*********************************************************************
 * @fn      createPayload
 *
 * @brief   Create payload buffer for OTA message plus authentication signature
 *          to be sent.
 *
 * @param   pBuf - pointer to message data
 * @param   len - length of message data
 * @param   sigLen - length of authentication signature
 *
 * @return  pointer to payload. NULL if alloc fails.
 */
static uint8_t *createPayload(uint8_t *pBuf, uint16_t len, uint8_t sigLen)
{
  // Create payload buffer for OTA message to be sent
  uint8_t *pPayload = (uint8_t *) L2CAP_bm_alloc(len + ATT_OPCODE_SIZE + sigLen);
  if (pPayload != NULL)
  {

    ICall_CSState key;

    /* Critical section is entered to disable interrupts that might cause call
     * to callback due to race condition */
    key = ICall_enterCriticalSection();
    pPayload = osal_bm_adjust_header(pPayload, -ATT_OPCODE_SIZE);
    ICall_leaveCriticalSection(key);

    // Copy received data over
    VOID memcpy(pPayload, pBuf, len);

    return(pPayload);
  }

  return(NULL);
}

// Do not use SNV drivers with FPGA. Unverified behavior
#ifndef USE_FPGA
/*********************************************************************
 * @fn      checkNVLen
 *
 * @brief   Checks the size of NV items.
 *
 * @param   id - NV ID.
 * @param   len - lengths in bytes of item.
 *
 * @return  SUCCESS, INVALIDPARAMETER or FAILURE
 */
static uint8_t checkNVLen(osalSnvId_t id, osalSnvLen_t len)
{
  uint8_t stat = SUCCESS;

  switch (id)
  {
    case BLE_NVID_CSRK:
    case BLE_NVID_IRK:
      if (len != KEYLEN)
      {
        stat = INVALIDPARAMETER;
      }
      break;

    case BLE_NVID_SIGNCOUNTER:
      if (len != sizeof (uint32_t))
      {
        stat = INVALIDPARAMETER;
      }
      break;

    default:
#if defined(GAP_BOND_MGR)
      if ((id >= BLE_NVID_GAP_BOND_START) && (id <= BLE_NVID_GAP_BOND_END))
      {
        stat = GAPBondMgr_CheckNVLen(id, len);
      }
      else
#endif
#if !defined(NO_OSAL_SNV) && !(defined(OSAL_SNV) && (OSAL_SNV == 0))
      if ((id >= BLE_NVID_CUST_START) && (id <= BLE_NVID_CUST_END))
      {
        stat = SUCCESS;
      }
      else
#endif
      {
        stat = INVALIDPARAMETER;  // Initialize status to failure
      }
      break;
  }

  return(stat);
}
#endif // USE_FPGA

/*********************************************************************
 * @fn      mapATT2BLEStatus
 *
 * @brief   Map ATT error code to BLE Generic status code.
 *
 * @param   status - ATT status
 *
 * @return  BLE Generic status
 */
static uint8_t mapATT2BLEStatus(uint8_t status)
{
  uint8_t stat;

  switch (status)
  {
    case ATT_ERR_INSUFFICIENT_AUTHEN:
      // Returned from Send routines
      stat = bleInsufficientAuthen;
      break;

    case ATT_ERR_INSUFFICIENT_ENCRYPT:
      // Returned from Send routines
      stat = bleInsufficientEncrypt;
      break;

    case ATT_ERR_INSUFFICIENT_KEY_SIZE:
      // Returned from Send routines
      stat = bleInsufficientKeySize;
      break;

    default:
      stat = status;
  }

  return(stat);
}

/*********************************************************************
 * @fn      processEvents
 *
 * @brief   Process an incoming Event messages.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE to deallocate, FALSE otherwise
 */
static uint8_t processEvents(ICall_Hdr *pMsg)
{
  uint8_t msgLen = 0;
  uint8_t *pBuf = NULL;
  uint8_t allocated = FALSE;
  uint8_t deallocateIncoming = TRUE;

  VOID memset(out_msg, 0, sizeof (out_msg));

  switch (pMsg->event)
  {
    case GAP_MSG_EVENT:
      pBuf = processEventsGAP((gapEventHdr_t *)pMsg, out_msg, &msgLen, &allocated, &deallocateIncoming);
      break;

    case SM_MSG_EVENT:
      pBuf = processEventsSM((smEventHdr_t *)pMsg, out_msg, &msgLen, &allocated);
      break;

    case L2CAP_SIGNAL_EVENT:
      pBuf = processEventsL2CAP((l2capSignalEvent_t *)pMsg, out_msg, &msgLen);
      break;

    case L2CAP_DATA_EVENT:
      pBuf = processDataL2CAP((l2capDataEvent_t *)pMsg, out_msg, &msgLen, &allocated);
      break;

    case GATT_MSG_EVENT:
      pBuf = processEventsGATT((gattMsgEvent_t *)pMsg, out_msg, &msgLen, &allocated);
      break;
#if !defined(GATT_DB_OFF_CHIP)
    case GATT_SERV_MSG_EVENT:
      pBuf = processEventsGATTServ((gattEventHdr_t *)pMsg, out_msg, &msgLen);
      break;
#endif
    default:
      break; // ignore
  }

  if (msgLen)
  {
    HCI_TL_SendVSEvent(pBuf, msgLen);
    //HCI_SendControllerToHostEvent(HCI_VE_EVENT_CODE,  msgLen, pBuf);
  }

  if ((pBuf != NULL) && (allocated == TRUE))
  {
    ICall_free(pBuf);

    if (!deallocateIncoming)
    {
      return(FALSE);
    }
  }

  return(TRUE);
}

/*********************************************************************
 * @fn      processEventsGAP
 *
 * @brief   Process an incoming GAP Event messages.
 *
 * @param   pMsg - message to process
 * @param   pOutMsg - outgoing message to be built
 * @param   pMsgLen - length of outgoing message
 * @param   pAllocated - whether outgoing message is locally allocated
 * @param   pDeallocate - whether it's safe to deallocate incoming message
 *
 * @return  outgoing message
 */
static uint8_t *processEventsGAP(gapEventHdr_t *pMsg, uint8_t *pOutMsg, uint8_t *pMsgLen,
                                uint8_t *pAllocated, uint8_t *pDeallocate)
{
  uint8_t msgLen = 0;
  uint8_t *pBuf = NULL;

  *pDeallocate = TRUE;

  switch (pMsg->opcode)
  {
    case GAP_DEVICE_INIT_DONE_EVENT:
      {
        gapDeviceInitDoneEvent_t *pPkt = (gapDeviceInitDoneEvent_t *)pMsg;

        pOutMsg[0] = LO_UINT16(HCI_EXT_GAP_DEVICE_INIT_DONE_EVENT);
        pOutMsg[1] = HI_UINT16(HCI_EXT_GAP_DEVICE_INIT_DONE_EVENT);
        pOutMsg[2] = pPkt->hdr.status;
        VOID memcpy(&pOutMsg[3], pPkt->devAddr, B_ADDR_LEN);
        pOutMsg[9]  = LO_UINT16(pPkt->dataPktLen);
        pOutMsg[10] = HI_UINT16(pPkt->dataPktLen);
        pOutMsg[11] = pPkt->numDataPkts;

        // Copy the Device's local keys
        VOID memcpy(&pOutMsg[12], IRK, KEYLEN);
        VOID memcpy(&pOutMsg[12+KEYLEN], SRK, KEYLEN);

        if (pPkt->hdr.status == SUCCESS)
        {
          VOID osal_snv_write(BLE_NVID_IRK, KEYLEN, IRK);
          VOID osal_snv_write(BLE_NVID_CSRK, KEYLEN, SRK);
          VOID osal_snv_write(BLE_NVID_SIGNCOUNTER, sizeof(uint32_t), &hciExtSignCounter);
        }

        pBuf = pOutMsg;
        msgLen = 44;
      }
      break;

    case GAP_DEVICE_DISCOVERY_EVENT:
      {
        gapDevDiscEvent_t *pPkt = (gapDevDiscEvent_t *)pMsg;

        if ((pPkt->hdr.status == SUCCESS) && (pPkt->numDevs > 0))
        {
          uint8_t x;
          gapDevRec_t *devList = pPkt->pDevList;

          // Calculate buffer needed
          msgLen = 4; // Size of opCode, status and numDevs field
          msgLen += (pPkt->numDevs * 8); // Num devices * (eventType, addrType, addr)

          pBuf = ICall_malloc(msgLen);
          if (pBuf)
          {
            uint8_t *buf = pBuf;

            // Fill in header
            *buf++ = LO_UINT16(HCI_EXT_GAP_DEVICE_DISCOVERY_EVENT);
            *buf++ = HI_UINT16(HCI_EXT_GAP_DEVICE_DISCOVERY_EVENT);
            *buf++ = pPkt->hdr.status;
            *buf++ = pPkt->numDevs;

            devList = pPkt->pDevList;
            for (x = 0; x < pPkt->numDevs; x++, devList++)
            {
              *buf++ = devList->eventType;
              *buf++ = devList->addrType;
              VOID memcpy(buf, devList->addr, B_ADDR_LEN);
              buf += B_ADDR_LEN;
            }

            *pAllocated = TRUE;
          }
          else
          {
            pPkt->hdr.status = bleMemAllocError;
          }
        }

        if ((pPkt->hdr.status != SUCCESS) || (pPkt->numDevs == 0))
        {
          pOutMsg[0] = LO_UINT16(HCI_EXT_GAP_DEVICE_DISCOVERY_EVENT);
          pOutMsg[1] = HI_UINT16(HCI_EXT_GAP_DEVICE_DISCOVERY_EVENT);
          pOutMsg[2] = pPkt->hdr.status;
          pOutMsg[3] = 0;
          pBuf = pOutMsg;
          msgLen = 4; // Size of opCode, status and numDevs field
        }
      }
      break;

    case GAP_ADV_DATA_UPDATE_DONE_EVENT:
      {
        gapAdvDataUpdateEvent_t *pPkt = (gapAdvDataUpdateEvent_t *)pMsg;

        pOutMsg[0] = LO_UINT16(HCI_EXT_GAP_ADV_DATA_UPDATE_DONE_EVENT);
        pOutMsg[1] = HI_UINT16(HCI_EXT_GAP_ADV_DATA_UPDATE_DONE_EVENT);
        pOutMsg[2] = pPkt->hdr.status;
        pOutMsg[3] = pPkt->adType;
        pBuf = pOutMsg;
        msgLen = 4;
      }
      break;

    case GAP_MAKE_DISCOVERABLE_DONE_EVENT:
      {
        gapMakeDiscoverableRspEvent_t *pPkt = (gapMakeDiscoverableRspEvent_t *)pMsg;

        pOutMsg[0] = LO_UINT16(HCI_EXT_GAP_MAKE_DISCOVERABLE_DONE_EVENT);
        pOutMsg[1] = HI_UINT16(HCI_EXT_GAP_MAKE_DISCOVERABLE_DONE_EVENT);
        pOutMsg[2] = pPkt->hdr.status;
        pBuf = pOutMsg;
        msgLen = 3;
      }
      break;

    case GAP_END_DISCOVERABLE_DONE_EVENT:
      pOutMsg[0] = LO_UINT16(HCI_EXT_GAP_END_DISCOVERABLE_DONE_EVENT);
      pOutMsg[1] = HI_UINT16(HCI_EXT_GAP_END_DISCOVERABLE_DONE_EVENT);
      pOutMsg[2] = pMsg->hdr.status;
      pBuf = pOutMsg;
      msgLen = 3;
      break;

    case GAP_LINK_ESTABLISHED_EVENT:
      {
        gapEstLinkReqEvent_t *pPkt = (gapEstLinkReqEvent_t *)pMsg;

#if defined(GAP_BOND_MGR)
       if (pPkt->hdr.status == SUCCESS)
       {
          // Notify the Bond Manager to the connection
          GAPBondMgr_LinkEst(pPkt->devAddrType, pPkt->devAddr, pPkt->connectionHandle, pPkt->connRole);
       }
#endif

        pOutMsg[0] = LO_UINT16(HCI_EXT_GAP_LINK_ESTABLISHED_EVENT);
        pOutMsg[1] = HI_UINT16(HCI_EXT_GAP_LINK_ESTABLISHED_EVENT);
        pOutMsg[2] = pPkt->hdr.status;
        pOutMsg[3] = pPkt->devAddrType;
        VOID memcpy(&(pOutMsg[4]), pPkt->devAddr, B_ADDR_LEN);
        pOutMsg[10] = LO_UINT16(pPkt->connectionHandle);
        pOutMsg[11] = HI_UINT16(pPkt->connectionHandle);
        pOutMsg[12] = pPkt->connRole;
        pOutMsg[13] = LO_UINT16(pPkt->connInterval);
        pOutMsg[14] = HI_UINT16(pPkt->connInterval);
        pOutMsg[15] = LO_UINT16(pPkt->connLatency);
        pOutMsg[16] = HI_UINT16(pPkt->connLatency);
        pOutMsg[17] = LO_UINT16(pPkt->connTimeout);
        pOutMsg[18] = HI_UINT16(pPkt->connTimeout);
        pOutMsg[19] = pPkt->clockAccuracy;
        pBuf = pOutMsg;
        msgLen = 20;
      }
      break;

    case GAP_LINK_TERMINATED_EVENT:
      {
        gapTerminateLinkEvent_t *pPkt = (gapTerminateLinkEvent_t *)pMsg;

#if defined(GAP_BOND_MGR)
        VOID GAPBondMgr_ProcessGAPMsg((gapEventHdr_t *)pMsg);
#endif

        pOutMsg[0] = LO_UINT16(HCI_EXT_GAP_LINK_TERMINATED_EVENT);
        pOutMsg[1] = HI_UINT16(HCI_EXT_GAP_LINK_TERMINATED_EVENT);
        pOutMsg[2] = pPkt->hdr.status;
        pOutMsg[3] = LO_UINT16(pPkt->connectionHandle);
        pOutMsg[4] = HI_UINT16(pPkt->connectionHandle);
        pOutMsg[5] = pPkt->reason;
        pBuf = pOutMsg;
        msgLen = 6;
      }
      break;

    case GAP_UPDATE_LINK_PARAM_REQ_EVENT:
      {
        gapUpdateLinkParamReqEvent_t *pPkt =
                                           (gapUpdateLinkParamReqEvent_t *)pMsg;

        pOutMsg[0]  = LO_UINT16(HCI_EXT_GAP_LINK_PARAM_UPDATE_REQ_EVENT);
        pOutMsg[1]  = HI_UINT16(HCI_EXT_GAP_LINK_PARAM_UPDATE_REQ_EVENT);
        pOutMsg[2]  = pPkt->hdr.status;
        pOutMsg[3]  = LO_UINT16(pPkt->req.connectionHandle);
        pOutMsg[4]  = HI_UINT16(pPkt->req.connectionHandle);
        pOutMsg[5]  = LO_UINT16(pPkt->req.intervalMin);
        pOutMsg[6]  = HI_UINT16(pPkt->req.intervalMin);
        pOutMsg[7]  = LO_UINT16(pPkt->req.intervalMax);
        pOutMsg[8]  = HI_UINT16(pPkt->req.intervalMax);
        pOutMsg[9]  = LO_UINT16(pPkt->req.connLatency);
        pOutMsg[10] = HI_UINT16(pPkt->req.connLatency);
        pOutMsg[11] = LO_UINT16(pPkt->req.connTimeout);
        pOutMsg[12] = HI_UINT16(pPkt->req.connTimeout);
        pBuf =  pOutMsg;
        msgLen = 13;
      }
      break;

    case GAP_LINK_PARAM_UPDATE_EVENT:
      {
        gapLinkUpdateEvent_t *pPkt = (gapLinkUpdateEvent_t *)pMsg;

        pOutMsg[0]  = LO_UINT16(HCI_EXT_GAP_LINK_PARAM_UPDATE_EVENT);
        pOutMsg[1]  = HI_UINT16(HCI_EXT_GAP_LINK_PARAM_UPDATE_EVENT);
        pOutMsg[2]  = pPkt->hdr.status;
        pOutMsg[3]  = LO_UINT16(pPkt->connectionHandle);
        pOutMsg[4]  = HI_UINT16(pPkt->connectionHandle);
        pOutMsg[5]  = LO_UINT16(pPkt->connInterval);
        pOutMsg[6]  = HI_UINT16(pPkt->connInterval);
        pOutMsg[7]  = LO_UINT16(pPkt->connLatency);
        pOutMsg[8]  = HI_UINT16(pPkt->connLatency);
        pOutMsg[9]  = LO_UINT16(pPkt->connTimeout);
        pOutMsg[10] = HI_UINT16(pPkt->connTimeout);
        pBuf = pOutMsg;
        msgLen = 11;
      }
      break;

    case GAP_RANDOM_ADDR_CHANGED_EVENT:
      {
        gapRandomAddrEvent_t *pPkt = (gapRandomAddrEvent_t *)pMsg;

        pOutMsg[0]  = LO_UINT16(HCI_EXT_GAP_RANDOM_ADDR_CHANGED_EVENT);
        pOutMsg[1]  = HI_UINT16(HCI_EXT_GAP_RANDOM_ADDR_CHANGED_EVENT);
        pOutMsg[2]  = pPkt->hdr.status;
        pOutMsg[3]  = pPkt->addrMode;
        VOID memcpy(&(pOutMsg[4]), pPkt->newRandomAddr, B_ADDR_LEN);
        pBuf = pOutMsg;
        msgLen = 10;
      }
      break;

#ifndef NO_BLE_SECURITY
    case GAP_SIGNATURE_UPDATED_EVENT:
      {
        gapSignUpdateEvent_t *pPkt = (gapSignUpdateEvent_t *)pMsg;

#if defined(GAP_BOND_MGR)
        VOID GAPBondMgr_ProcessGAPMsg((gapEventHdr_t *)pMsg);
#endif

        pOutMsg[0]  = LO_UINT16(HCI_EXT_GAP_SIGNATURE_UPDATED_EVENT);
        pOutMsg[1]  = HI_UINT16(HCI_EXT_GAP_SIGNATURE_UPDATED_EVENT);
        pOutMsg[2]  = pPkt->hdr.status;
        pOutMsg[3]  = pPkt->addrType;
        VOID memcpy(&(pOutMsg[4]), pPkt->devAddr, B_ADDR_LEN);
        pOutMsg[10] = BREAK_UINT32(pPkt->signCounter, 0);
        pOutMsg[11] = BREAK_UINT32(pPkt->signCounter, 1);
        pOutMsg[12] = BREAK_UINT32(pPkt->signCounter, 2);
        pOutMsg[13] = BREAK_UINT32(pPkt->signCounter, 3);
        pBuf = pOutMsg;
        msgLen = 14;
      }
      break;

    case GAP_PASSKEY_NEEDED_EVENT:
      {
        gapPasskeyNeededEvent_t *pPkt = (gapPasskeyNeededEvent_t *)pMsg;

#if defined(GAP_BOND_MGR)
        VOID GAPBondMgr_ProcessGAPMsg((gapEventHdr_t *)pMsg);
#endif
        pOutMsg[0]  = LO_UINT16(HCI_EXT_GAP_PASSKEY_NEEDED_EVENT);
        pOutMsg[1]  = HI_UINT16(HCI_EXT_GAP_PASSKEY_NEEDED_EVENT);
        pOutMsg[2]  = pPkt->hdr.status;
        VOID memcpy(&(pOutMsg[3]), pPkt->deviceAddr, B_ADDR_LEN);
        pOutMsg[9] = LO_UINT16(pPkt->connectionHandle);
        pOutMsg[10] = HI_UINT16(pPkt->connectionHandle);
        pOutMsg[11] = pPkt->uiInputs;
        pOutMsg[12] = pPkt->uiOutputs;
        VOID memcpy(&(pOutMsg[13]), &pPkt->numComparison, 4);
        pBuf = pOutMsg;
        msgLen = 17;
      }
      break;

    case GAP_AUTHENTICATION_COMPLETE_EVENT:
      {
        gapAuthCompleteEvent_t *pPkt = (gapAuthCompleteEvent_t *)pMsg;

        msgLen = 107;

        pBuf = ICall_malloc(msgLen);
        if (pBuf)
        {
          uint8_t *buf = pBuf;

          *pAllocated = TRUE;

          VOID memset(buf, 0, msgLen);

          *buf++  = LO_UINT16(HCI_EXT_GAP_AUTH_COMPLETE_EVENT);
          *buf++  = HI_UINT16(HCI_EXT_GAP_AUTH_COMPLETE_EVENT);
          *buf++  = pPkt->hdr.status;
          *buf++  = LO_UINT16(pPkt->connectionHandle);
          *buf++  = HI_UINT16(pPkt->connectionHandle);
          *buf++  = pPkt->authState;

          if (pPkt->pSecurityInfo)
          {
            *buf++ = TRUE;
            *buf++ = pPkt->pSecurityInfo->keySize;
            VOID memcpy(buf, pPkt->pSecurityInfo->ltk, KEYLEN);
            buf += KEYLEN;
            *buf++ = LO_UINT16(pPkt->pSecurityInfo->div);
            *buf++ = HI_UINT16(pPkt->pSecurityInfo->div);
            VOID memcpy(buf, pPkt->pSecurityInfo->rand, B_RANDOM_NUM_SIZE);
            buf += B_RANDOM_NUM_SIZE;
          }
          else
          {
            // Skip securityInfo
            buf += 1 + KEYLEN + B_RANDOM_NUM_SIZE + 2 + 1;
          }

          if (pPkt->pDevSecInfo)
          {
            *buf++ = TRUE;
            *buf++ = pPkt->pDevSecInfo->keySize;
            VOID memcpy(buf, pPkt->pDevSecInfo->ltk, KEYLEN);
            buf += KEYLEN;
            *buf++ = LO_UINT16(pPkt->pDevSecInfo->div);
            *buf++ = HI_UINT16(pPkt->pDevSecInfo->div);
            VOID memcpy(buf, pPkt->pDevSecInfo->rand, B_RANDOM_NUM_SIZE);
            buf += B_RANDOM_NUM_SIZE;
          }
          else
          {
            // Skip securityInfo
            buf += 1 + KEYLEN + B_RANDOM_NUM_SIZE + 2 + 1;
          }

          if (pPkt->pIdentityInfo)
          {
            *buf++ = TRUE;
            VOID memcpy(buf, pPkt->pIdentityInfo->irk, KEYLEN);
            buf += KEYLEN;
            VOID memcpy(buf, pPkt->pIdentityInfo->bd_addr, B_ADDR_LEN);
            buf += B_ADDR_LEN;
            *buf++ = pPkt->pIdentityInfo->addrType;
          }
          else
          {
            // Skip identityInfo
            buf += KEYLEN + B_ADDR_LEN + 2;
          }

          if (pPkt->pSigningInfo)
          {
            *buf++ = TRUE;
            VOID memcpy(buf, pPkt->pSigningInfo->srk, KEYLEN);
            buf += KEYLEN;

            *buf++ = BREAK_UINT32(pPkt->pSigningInfo->signCounter, 0);
            *buf++ = BREAK_UINT32(pPkt->pSigningInfo->signCounter, 1);
            *buf++ = BREAK_UINT32(pPkt->pSigningInfo->signCounter, 2);
            *buf = BREAK_UINT32(pPkt->pSigningInfo->signCounter, 3);
          }
        }
        else
        {
          pOutMsg[0]  = LO_UINT16(HCI_EXT_GAP_AUTH_COMPLETE_EVENT);
          pOutMsg[1]  = HI_UINT16(HCI_EXT_GAP_AUTH_COMPLETE_EVENT);
          pOutMsg[2]  = bleMemAllocError;
          pOutMsg[3]  = LO_UINT16(pPkt->connectionHandle);
          pOutMsg[4]  = HI_UINT16(pPkt->connectionHandle);
          pBuf = pOutMsg;
          msgLen = 5;
        }

#if defined(GAP_BOND_MGR)
        *pDeallocate = GAPBondMgr_ProcessGAPMsg((gapEventHdr_t *)pMsg);
#endif
      }
      break;

    case GAP_BOND_COMPLETE_EVENT:
      {
        gapBondCompleteEvent_t *pPkt = (gapBondCompleteEvent_t *)pMsg;

#if defined(GAP_BOND_MGR)
        VOID GAPBondMgr_ProcessGAPMsg((gapEventHdr_t *)pMsg);
#endif

        pOutMsg[0] = LO_UINT16(HCI_EXT_GAP_BOND_COMPLETE_EVENT);
        pOutMsg[1] = HI_UINT16(HCI_EXT_GAP_BOND_COMPLETE_EVENT);
        pOutMsg[2] = pPkt->hdr.status;
        pOutMsg[3] = LO_UINT16(pPkt->connectionHandle);
        pOutMsg[4] = HI_UINT16(pPkt->connectionHandle);
        pBuf = pOutMsg;
        msgLen = 5;
      }
      break;

    case GAP_PAIRING_REQ_EVENT:
      {
        gapPairingReqEvent_t *pPkt = (gapPairingReqEvent_t *)pMsg;
        uint8_t tmp = 0;

#if defined(GAP_BOND_MGR)
        VOID GAPBondMgr_ProcessGAPMsg((gapEventHdr_t *)pMsg);
#endif

        pOutMsg[0] = LO_UINT16(HCI_EXT_GAP_PAIRING_REQ_EVENT);
        pOutMsg[1] = HI_UINT16(HCI_EXT_GAP_PAIRING_REQ_EVENT);
        pOutMsg[2] = pPkt->hdr.status;
        pOutMsg[3] = LO_UINT16(pPkt->connectionHandle);
        pOutMsg[4] = HI_UINT16(pPkt->connectionHandle);
        pOutMsg[5] = pPkt->pairReq.ioCap;
        pOutMsg[6] = pPkt->pairReq.oobDataFlag;
        pOutMsg[7] = pPkt->pairReq.authReq;
        pOutMsg[8] = pPkt->pairReq.maxEncKeySize;

        tmp |= (pPkt->pairReq.keyDist.sEncKey) ? KEYDIST_SENC : 0;
        tmp |= (pPkt->pairReq.keyDist.sIdKey) ? KEYDIST_SID : 0;
        tmp |= (pPkt->pairReq.keyDist.sSign) ? KEYDIST_SSIGN : 0;
        tmp |= (pPkt->pairReq.keyDist.sLinkKey) ? KEYDIST_SLINK : 0;
        tmp |= (pPkt->pairReq.keyDist.mEncKey) ? KEYDIST_MENC : 0;
        tmp |= (pPkt->pairReq.keyDist.mIdKey) ? KEYDIST_MID : 0;
        tmp |= (pPkt->pairReq.keyDist.mSign) ? KEYDIST_MSIGN : 0;
        tmp |= (pPkt->pairReq.keyDist.mLinkKey) ? KEYDIST_MLINK : 0;
        pOutMsg[9] = tmp;

        pBuf = pOutMsg;
        msgLen = 10;
      }
      break;

    case GAP_SLAVE_REQUESTED_SECURITY_EVENT:
      {
        gapSlaveSecurityReqEvent_t *pPkt = (gapSlaveSecurityReqEvent_t *)pMsg;

#if defined(GAP_BOND_MGR)
        VOID GAPBondMgr_ProcessGAPMsg((gapEventHdr_t *)pMsg);
#endif

        pOutMsg[0]  = LO_UINT16(HCI_EXT_GAP_SLAVE_REQUESTED_SECURITY_EVENT);
        pOutMsg[1]  = HI_UINT16(HCI_EXT_GAP_SLAVE_REQUESTED_SECURITY_EVENT);
        pOutMsg[2]  = pPkt->hdr.status;
        pOutMsg[3]  = LO_UINT16(pPkt->connectionHandle);
        pOutMsg[4]  = HI_UINT16(pPkt->connectionHandle);
        VOID memcpy(&(pOutMsg[5]), pPkt->deviceAddr, B_ADDR_LEN);
        pOutMsg[11] = pPkt->authReq;
        pBuf = pOutMsg;
        msgLen = 12;
      }
      break;
#endif //NO_BLE_SECURITY

    case GAP_DEVICE_INFO_EVENT:
      {
        gapDeviceInfoEvent_t *pPkt = (gapDeviceInfoEvent_t *)pMsg;

        msgLen = 13 + pPkt->dataLen;

        pBuf = ICall_malloc(msgLen);
        if (pBuf)
        {
          uint8_t *buf = pBuf;

          // Fill in header
          *buf++ = LO_UINT16(HCI_EXT_GAP_DEVICE_INFO_EVENT);
          *buf++ = HI_UINT16(HCI_EXT_GAP_DEVICE_INFO_EVENT);
          *buf++ = pPkt->hdr.status;
          *buf++ = pPkt->eventType;
          *buf++ = pPkt->addrType;

          VOID memcpy(buf, pPkt->addr, B_ADDR_LEN);
          buf += B_ADDR_LEN;

          *buf++ = (uint8_t)pPkt->rssi;
          *buf++ = pPkt->dataLen;
          VOID memcpy(buf, pPkt->pEvtData, pPkt->dataLen);

          *pAllocated = TRUE;
        }
        else
        {
          pPkt->hdr.status = bleMemAllocError;
        }
      }
      break;

    default:
      // Unknown command
      break;
  }

  *pMsgLen = msgLen;

  return(pBuf);
}

/*********************************************************************
 * @fn      processEventsSM
 *
 * @brief   Process an incoming SM Event messages.
 *
 * @param   pMsg       - packet to process
 * @param   pOutMsg    - outgoing message to be built
 * @param   pMsgLen    - length of outgoing message
 * @param   pAllocated - whether outgoing message is locally allocated
 *
 * @return  outgoing message
 */
static uint8_t *processEventsSM(smEventHdr_t *pMsg, uint8_t *pOutMsg,
                               uint8_t *pMsgLen, uint8_t *pAllocated)
{
  uint8_t msgLen = 0;
  uint8_t *pBuf = NULL;

#ifndef NO_BLE_SECURITY
  switch (pMsg->opcode)
  {
    case SM_ECC_KEYS_EVENT:
        {
          smEccKeysEvt_t *keys = (smEccKeysEvt_t *)pMsg;

          msgLen = (SM_ECC_KEY_LEN * 3) + 3;

          pBuf = ICall_malloc(msgLen);
          if (pBuf)
          {
            uint8_t *buf = pBuf;

            *buf++ = LO_UINT16(HCI_EXT_SM_GET_ECC_KEYS_EVENT);
            *buf++ = HI_UINT16(HCI_EXT_SM_GET_ECC_KEYS_EVENT);
            *buf++ = keys->hdr.status;
            memcpy(buf, keys->privateKey, SM_ECC_KEY_LEN);
            buf+=32;
            memcpy(buf, keys->publicKeyX, SM_ECC_KEY_LEN);
            buf+=32;
            memcpy(buf, keys->publicKeyY, SM_ECC_KEY_LEN);

            *pAllocated = TRUE;
          }
          else
          {
            keys->hdr.status = bleMemAllocError;
          }
        }
        break;

      case SM_DH_KEY_EVENT:
        {
          smDhKeyEvt_t *dhKey = (smDhKeyEvt_t *)pMsg;

          msgLen = SM_ECC_KEY_LEN + 3;

          pBuf = ICall_malloc(msgLen);
          if (pBuf)
          {
            uint8_t *buf = pBuf;
            *buf++ = LO_UINT16(HCI_EXT_SM_GET_DH_KEY_EVENT);
            *buf++ = HI_UINT16(HCI_EXT_SM_GET_DH_KEY_EVENT);
            *buf++ = dhKey->hdr.status;
            memcpy(buf, dhKey->dhKey, SM_ECC_KEY_LEN);

            *pAllocated = TRUE;
          }
          else
          {
            dhKey->hdr.status = bleMemAllocError;
          }
        }
        break;

      default:
        break;
  }
#endif //NO_BLE_SECURITY

  *pMsgLen = msgLen;

  return(pBuf);
}


/*********************************************************************
 * @fn      processEventsL2CAP
 *
 * @brief   Process an incoming L2CAP Event messages.
 *
 * @param   pPkt - packet to process
 * @param   pOutMsg - outgoing message to be built
 * @param   pMsgLen - length of outgoing message
 *
 * @return  outgoing message
 */
static uint8_t *processEventsL2CAP(l2capSignalEvent_t *pPkt, uint8_t *pOutMsg, uint8_t *pMsgLen)
{
  uint8_t msgLen;

  // Build the message header first
  msgLen = buildHCIExtHeader(pOutMsg, (HCI_EXT_L2CAP_EVENT | pPkt->opcode),
                              pPkt->hdr.status, pPkt->connHandle);
#if defined(BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  // Build Send SDU Done event regardless of status
  if (pPkt->opcode == L2CAP_SEND_SDU_DONE_EVT)
  {
    l2capSendSduDoneEvt_t *pSduEvt = &(pPkt->cmd.sendSduDoneEvt);

    pOutMsg[msgLen++] = LO_UINT16(pSduEvt->CID);
    pOutMsg[msgLen++] = HI_UINT16(pSduEvt->CID);

    pOutMsg[msgLen++] = LO_UINT16(pSduEvt->credits);
    pOutMsg[msgLen++] = HI_UINT16(pSduEvt->credits);

    pOutMsg[msgLen++] = LO_UINT16(pSduEvt->peerCID);
    pOutMsg[msgLen++] = HI_UINT16(pSduEvt->peerCID);

    pOutMsg[msgLen++] = LO_UINT16(pSduEvt->peerCredits);
    pOutMsg[msgLen++] = HI_UINT16(pSduEvt->peerCredits);

    pOutMsg[msgLen++] = LO_UINT16(pSduEvt->totalLen);
    pOutMsg[msgLen++] = HI_UINT16(pSduEvt->totalLen);

    pOutMsg[msgLen++] = LO_UINT16(pSduEvt->txLen);
    pOutMsg[msgLen++] = HI_UINT16(pSduEvt->txLen);
  }
  else
#endif //(BLE_V41_FEATURES & L2CAP_COC_CFG)
  if (pPkt->hdr.status == SUCCESS) // Build all other messages if status is success
  {
    switch (pPkt->opcode)
    {
      case L2CAP_CMD_REJECT:
        msgLen += L2CAP_BuildCmdReject(&pOutMsg[msgLen], (uint8_t *)&(pPkt->cmd.cmdReject));
        break;

      case L2CAP_PARAM_UPDATE_RSP:
        msgLen += L2CAP_BuildParamUpdateRsp(&pOutMsg[msgLen], (uint8_t *)&(pPkt->cmd.updateRsp));
        break;

      case L2CAP_INFO_RSP:
        msgLen += L2CAP_BuildInfoRsp(&pOutMsg[msgLen], (uint8_t *)&(pPkt->cmd.infoRsp));
        break;

#if defined(BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
      case L2CAP_CONNECT_REQ:
        pOutMsg[msgLen++] = pPkt->id;
        msgLen += L2CAP_BuildConnectReq(&pOutMsg[msgLen], (uint8_t *)&(pPkt->cmd.connectReq));
        break;

      case L2CAP_CHANNEL_ESTABLISHED_EVT:
        {
          l2capChannelEstEvt_t *pEstEvt = &(pPkt->cmd.channelEstEvt);

          pOutMsg[msgLen++] = LO_UINT16(pEstEvt->result);
          pOutMsg[msgLen++] = HI_UINT16(pEstEvt->result);

          if (pEstEvt->result == L2CAP_CONN_SUCCESS)
          {
            msgLen += buildCoChannelInfo(pEstEvt->CID, &pEstEvt->info, &pOutMsg[msgLen]);
          }
        }
        break;

       case L2CAP_CHANNEL_TERMINATED_EVT:
        {
          l2capChannelTermEvt_t *pTermEvt = &(pPkt->cmd.channelTermEvt);

          pOutMsg[msgLen++] = LO_UINT16(pTermEvt->CID);
          pOutMsg[msgLen++] = HI_UINT16(pTermEvt->CID);

          pOutMsg[msgLen++] = LO_UINT16(pTermEvt->peerCID);
          pOutMsg[msgLen++] = HI_UINT16(pTermEvt->peerCID);

          pOutMsg[msgLen++] = LO_UINT16(pTermEvt->reason);
          pOutMsg[msgLen++] = HI_UINT16(pTermEvt->reason);
        }
        break;

      case L2CAP_OUT_OF_CREDIT_EVT:
      case L2CAP_PEER_CREDIT_THRESHOLD_EVT:
        {
          l2capCreditEvt_t *pCreditEvt = &(pPkt->cmd.creditEvt);

          pOutMsg[msgLen++] = LO_UINT16(pCreditEvt->CID);
          pOutMsg[msgLen++] = HI_UINT16(pCreditEvt->CID);

          pOutMsg[msgLen++] = LO_UINT16(pCreditEvt->peerCID);
          pOutMsg[msgLen++] = HI_UINT16(pCreditEvt->peerCID);

          pOutMsg[msgLen++] = LO_UINT16(pCreditEvt->credits);
          pOutMsg[msgLen++] = HI_UINT16(pCreditEvt->credits);
        }
        break;
#endif //(BLE_V41_FEATURES & L2CAP_COC_CFG)

      default:
        // Unknown command
        break;
    }
  }

  *pMsgLen = msgLen;

  return(pOutMsg);
}

/*********************************************************************
 * @fn      processDataL2CAP
 *
 * @brief   Process an incoming L2CAP Data message.
 *
 * @param   pPkt - packet to process
 * @param   pOutMsg - outgoing message to be built
 * @param   pMsgLen - length of outgoing message
 *
 * @return  outgoing message
 */
static uint8_t *processDataL2CAP(l2capDataEvent_t *pPkt, uint8_t *pOutMsg,
                                uint8_t *pMsgLen, uint8_t *pAllocated)
{
  uint8_t *pBuf;
  uint16_t msgLen = HCI_EXT_HDR_LEN + 2 + 2; // hdr + CID + packet length;
  uint8_t status = pPkt->hdr.status;

  *pAllocated = FALSE;

  msgLen += pPkt->pkt.len;
  if (msgLen > HCI_EXT_APP_OUT_BUF)
  {
    pBuf = ICall_malloc(msgLen);
    if (pBuf)
    {
      *pAllocated = TRUE;
    }
    else
    {
      pBuf = pOutMsg;
      msgLen -= pPkt->pkt.len;

      status = bleMemAllocError;
    }
  }
  else
  {
    pBuf = pOutMsg;
  }

  // Build the message header first
  VOID buildHCIExtHeader(pBuf, (HCI_EXT_L2CAP_EVENT | HCI_EXT_L2CAP_DATA),
                          status, pPkt->connHandle);
  // Add CID
  pBuf[HCI_EXT_HDR_LEN] = LO_UINT16(pPkt->pkt.CID);
  pBuf[HCI_EXT_HDR_LEN+1] = HI_UINT16(pPkt->pkt.CID);

  // Add data length
  pBuf[HCI_EXT_HDR_LEN+2] = LO_UINT16(pPkt->pkt.len);
  pBuf[HCI_EXT_HDR_LEN+3] = HI_UINT16(pPkt->pkt.len);

  // Add payload
  if (pPkt->pkt.pPayload != NULL)
  {
    if (status == SUCCESS)
    {
      VOID memcpy(&pBuf[HCI_EXT_HDR_LEN+4], pPkt->pkt.pPayload, pPkt->pkt.len);
    }

    // Received buffer is processed so it's safe to free it
    BM_free(pPkt->pkt.pPayload);
  }

  *pMsgLen = msgLen;

  return(pBuf);
}

/*********************************************************************
 * @fn      processEventsGATT
 *
 * @brief   Process an incoming GATT Event messages.
 *
 * @param   pPkt - packet to process
 * @param   pOutMsg - outgoing message to be built
 * @param   pMsgLen - length of outgoing message
 * @param   pAllocated - whether outgoing message is locally allocated
 *
 * @return  outgoing message
 */
static uint8_t *processEventsGATT(gattMsgEvent_t *pPkt, uint8_t *pOutMsg,
                                 uint8_t *pMsgLen, uint8_t *pAllocated)
{
  uint8_t msgLen = 0, attHdrLen = 0, hdrLen = HCI_EXT_HDR_LEN + 1; // hdr + event length
  uint8_t *pBuf, *pPayload = NULL;
  uint8_t status = pPkt->hdr.status;

  *pAllocated = FALSE;

  if ((status == SUCCESS) || (status == blePending))
  {
    // Build the ATT header first
    switch (pPkt->method)
    {
      case ATT_ERROR_RSP:
        attHdrLen = ATT_BuildErrorRsp(&pOutMsg[hdrLen], (uint8_t *)&(pPkt->msg.errorRsp));
        break;

      case ATT_EXCHANGE_MTU_REQ:
      case ATT_MTU_UPDATED_EVENT:
        attHdrLen = ATT_BuildExchangeMTUReq(&pOutMsg[hdrLen], (uint8_t *)&(pPkt->msg.exchangeMTUReq));
        break;

      case ATT_EXCHANGE_MTU_RSP:
        attHdrLen = ATT_BuildExchangeMTURsp(&pOutMsg[hdrLen], (uint8_t *)&(pPkt->msg.exchangeMTURsp));
        break;

      case ATT_FIND_INFO_REQ:
        attHdrLen = ATT_BuildFindInfoReq(&pOutMsg[hdrLen], (uint8_t *)&(pPkt->msg.findInfoReq));
        break;

      case ATT_FIND_INFO_RSP:
        {
          attFindInfoRsp_t *pRsp = &pPkt->msg.findInfoRsp;

          attHdrLen = ATT_FIND_INFO_RSP_FIXED_SIZE;

          // Copy response header over
          msgLen = ATT_BuildFindInfoRsp(&pOutMsg[hdrLen], (uint8_t *)pRsp) - attHdrLen;
          pPayload = pRsp->pInfo;
        }
        break;

      case ATT_FIND_BY_TYPE_VALUE_REQ:
        {
          attFindByTypeValueReq_t *pReq = &pPkt->msg.findByTypeValueReq;

          attHdrLen = ATT_FIND_BY_TYPE_VALUE_REQ_FIXED_SIZE;

          // Copy request header over
          msgLen = ATT_BuildFindByTypeValueReq(&pOutMsg[hdrLen], (uint8_t *)pReq) - attHdrLen;
          pPayload = pReq->pValue;
        }
        break;

      case ATT_FIND_BY_TYPE_VALUE_RSP:
        {
          attFindByTypeValueRsp_t *pRsp = &pPkt->msg.findByTypeValueRsp;

          msgLen = ATT_BuildFindByTypeValueRsp(&pOutMsg[hdrLen], (uint8_t *)pRsp);
          pPayload = pRsp->pHandlesInfo;
        }
        break;

      case ATT_READ_BY_TYPE_REQ:
        attHdrLen = ATT_BuildReadByTypeReq(&pOutMsg[hdrLen], (uint8_t *)&(pPkt->msg.readByTypeReq));
        break;

      case ATT_READ_BY_TYPE_RSP:
        {
          attReadByTypeRsp_t *pRsp = &pPkt->msg.readByTypeRsp;

          attHdrLen = ATT_READ_BY_TYPE_RSP_FIXED_SIZE;

          // Copy request header over
          msgLen = ATT_BuildReadByTypeRsp(&pOutMsg[hdrLen], (uint8_t *)pRsp) - attHdrLen;
          pPayload = pRsp->pDataList;
        }
        break;

      case ATT_READ_REQ:
        attHdrLen = ATT_BuildReadReq(&pOutMsg[hdrLen], (uint8_t *)&(pPkt->msg.readReq));
        break;

      case ATT_READ_RSP:
        {
          attReadRsp_t *pRsp = &pPkt->msg.readRsp;

          msgLen = ATT_BuildReadRsp(&pOutMsg[hdrLen], (uint8_t *)pRsp);
          pPayload = pRsp->pValue;
        }
        break;

      case ATT_READ_BLOB_REQ:
        attHdrLen = ATT_BuildReadBlobReq(&pOutMsg[hdrLen], (uint8_t *)&(pPkt->msg.readBlobReq));
        break;

      case ATT_READ_BLOB_RSP:
        {
          attReadBlobRsp_t *pRsp = &pPkt->msg.readBlobRsp;

          msgLen = ATT_BuildReadBlobRsp(&pOutMsg[hdrLen], (uint8_t *)pRsp);
          pPayload = pRsp->pValue;
        }
        break;

      case ATT_READ_MULTI_REQ:
        {
          attReadMultiReq_t *pReq = &pPkt->msg.readMultiReq;

          msgLen = ATT_BuildReadMultiReq(&pOutMsg[hdrLen], (uint8_t *)pReq);
          pPayload = pReq->pHandles;
        }
        break;

      case ATT_READ_MULTI_RSP:
        {
          attReadMultiRsp_t *pRsp = &pPkt->msg.readMultiRsp;

          msgLen = ATT_BuildReadMultiRsp(&pOutMsg[hdrLen], (uint8_t *)pRsp);
          pPayload = pRsp->pValues;
        }
        break;

      case ATT_READ_BY_GRP_TYPE_REQ:
        attHdrLen = ATT_BuildReadByTypeReq(&pOutMsg[hdrLen], (uint8_t *)&(pPkt->msg.readByGrpTypeReq));
        break;

      case ATT_READ_BY_GRP_TYPE_RSP:
        {
          attReadByGrpTypeRsp_t *pRsp = &pPkt->msg.readByGrpTypeRsp;

          attHdrLen = ATT_READ_BY_GRP_TYPE_RSP_FIXED_SIZE;

          // Copy request header over
          msgLen = ATT_BuildReadByGrpTypeRsp(&pOutMsg[hdrLen], (uint8_t *)pRsp) - attHdrLen;
          pPayload = pRsp->pDataList;
        }
        break;

      case ATT_WRITE_REQ:
        {
          attWriteReq_t *pReq = &pPkt->msg.writeReq;

          pOutMsg[hdrLen] = pReq->sig;
          pOutMsg[hdrLen+1] = pReq->cmd;

          attHdrLen = ATT_WRITE_REQ_FIXED_SIZE;

          // Copy request header over
          msgLen = ATT_BuildWriteReq(&pOutMsg[hdrLen+2], (uint8_t *)pReq) - attHdrLen;
          pPayload = pReq->pValue;

          attHdrLen += 2; // sig + cmd
        }
        break;

      case ATT_PREPARE_WRITE_REQ:
        {
          attPrepareWriteReq_t *pReq = &pPkt->msg.prepareWriteReq;

          attHdrLen = ATT_PREPARE_WRITE_REQ_FIXED_SIZE;

          // Copy request header over
          msgLen = ATT_BuildPrepareWriteReq(&pOutMsg[hdrLen], (uint8_t *)pReq) - attHdrLen;
          pPayload = pReq->pValue;
        }
        break;

      case ATT_PREPARE_WRITE_RSP:
        {
          attPrepareWriteRsp_t *pRsp = &pPkt->msg.prepareWriteRsp;

          attHdrLen = ATT_PREPARE_WRITE_RSP_FIXED_SIZE;

          // Copy request header over
          msgLen = ATT_BuildPrepareWriteRsp(&pOutMsg[hdrLen], (uint8_t *)pRsp) - attHdrLen;
          pPayload = pRsp->pValue;
        }
        break;

      case ATT_EXECUTE_WRITE_REQ:
        attHdrLen = ATT_BuildExecuteWriteReq(&pOutMsg[hdrLen], (uint8_t *)&(pPkt->msg.executeWriteReq));
        break;

      case ATT_HANDLE_VALUE_IND:
      case ATT_HANDLE_VALUE_NOTI:
        {
          attHandleValueInd_t *pInd = &pPkt->msg.handleValueInd;

          attHdrLen = ATT_HANDLE_VALUE_IND_FIXED_SIZE;

          // Copy request header over
          msgLen = ATT_BuildHandleValueInd(&pOutMsg[hdrLen], (uint8_t *)pInd) - attHdrLen;
          pPayload = pInd->pValue;
        }
        break;

      case ATT_FLOW_CTRL_VIOLATED_EVENT:
        {
          attFlowCtrlViolatedEvt_t *pEvt = &pPkt->msg.flowCtrlEvt;

          pOutMsg[hdrLen]   = pEvt->opcode;
          pOutMsg[hdrLen+1] = pEvt->pendingOpcode;

          attHdrLen = 2;
        }
        break;

      default:
        // Unknown command
        break;
    }
  }

  // Event format: HCI Ext hdr + event len + ATT hdr + ATT PDU
  if ((hdrLen + attHdrLen + msgLen) > HCI_EXT_APP_OUT_BUF)
  {
    pBuf = ICall_malloc(hdrLen + attHdrLen + msgLen);
    if (pBuf)
    {
      *pAllocated = TRUE;

      // Copy the ATT header over
      if (attHdrLen > 0)
      {
        VOID memcpy(&pBuf[hdrLen], &pOutMsg[hdrLen], attHdrLen);
      }
    }
    else
    {
      pBuf = pOutMsg;
      msgLen = 0;

      status = bleMemAllocError;
    }
  }
  else
  {
    pBuf = pOutMsg;
  }

  // Build the message PDU
  if (pPayload != NULL)
  {
    if (msgLen > 0)
    {
      // Copy the message payload over
      VOID memcpy(&pBuf[hdrLen+attHdrLen], pPayload, msgLen);
    }

    // Free the payload buffer
    BM_free(pPayload);
  }

  // Build the message header
  VOID buildHCIExtHeader(pBuf, (HCI_EXT_ATT_EVENT | pPkt->method), status, pPkt->connHandle);

  // Add the event (PDU) length for GATT events for now!
  pBuf[HCI_EXT_HDR_LEN] = attHdrLen + msgLen;

  *pMsgLen = hdrLen + attHdrLen + msgLen;

  return(pBuf);
}

#if !defined(GATT_DB_OFF_CHIP)
/*********************************************************************
 * @fn      processEventsGATTServ
 *
 * @brief   Process an incoming GATT Server Event messages.
 *
 * @param   pPkt - packet to process
 * @param   pMsg - outgoing message to be built
 * @param   pMsgLen - length of outgoing message
 *
 * @return  outgoing message
 */
static uint8_t *processEventsGATTServ(gattEventHdr_t *pPkt, uint8_t *pMsg, uint8_t *pMsgLen)
{
  uint8_t hdrLen = HCI_EXT_HDR_LEN + 1; // hdr + event length
  uint8_t msgLen = 0;
  uint8_t *pBuf = pMsg;

  if (pPkt->hdr.status == SUCCESS)
  {
    // Build the message first
    switch (pPkt->method)
    {
      case GATT_CLIENT_CHAR_CFG_UPDATED_EVENT:
        {
          gattClientCharCfgUpdatedEvent_t *pEvent = (gattClientCharCfgUpdatedEvent_t *)pPkt;

#if defined(GAP_BOND_MGR)
          VOID GAPBondMgr_UpdateCharCfg(pEvent->connHandle, pEvent->attrHandle, pEvent->value);
#endif
          // Attribute handle
          pMsg[hdrLen]   = LO_UINT16(pEvent->attrHandle);
          pMsg[hdrLen+1] = HI_UINT16(pEvent->attrHandle);

          // Attribute value
          pMsg[hdrLen+2] = LO_UINT16(pEvent->value);
          pMsg[hdrLen+3] = HI_UINT16(pEvent->value);

          msgLen = 4;
        }
        break;

      default:
        // Unknown command
        break;
    }
  }

  // Build the message header
  VOID buildHCIExtHeader(pBuf, (HCI_EXT_GATT_EVENT | pPkt->method), pPkt->hdr.status, pPkt->connHandle);

  // Add the event (PDU) length for GATT events for now!
  pBuf[HCI_EXT_HDR_LEN] = msgLen;

  *pMsgLen = hdrLen + msgLen;

  return(pBuf);
}
#endif // !GATT_DB_OFF_CHIP

/*********************************************************************
 * @fn      buildHCIExtHeader
 *
 * @brief   Build an HCI Extension header.
 *
 * @param   pBuf - header to be built
 * @param   event - event id
 * @param   status - event status
 * @param   connHandle - connection handle
 *
 * @return  header length
 */
static uint8_t buildHCIExtHeader(uint8_t *pBuf, uint16_t event, uint8_t status, uint16_t connHandle)
{
  pBuf[0] = LO_UINT16(event);
  pBuf[1] = HI_UINT16(event);
  pBuf[2] = status;
  pBuf[3] = LO_UINT16(connHandle);
  pBuf[4] = HI_UINT16(connHandle);

  return(HCI_EXT_HDR_LEN);
}

#if defined(BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
/*********************************************************************
  * @fn      l2capVerifySecCB
 *
 * @brief   Callback function to verify security when a Connection
 *          Request is received.
 *
 * @param   connHandle - connection handle request was received on
 * @param   id - identifier matches responses with requests
 * @param   pReq - received connection request
 *
 * @return  See L2CAP Connection Response: Result values
 */
static uint16_t l2capVerifySecCB(uint16_t connHandle, uint8_t id, l2capConnectReq_t *pReq)
{
  uint8_t msgLen = 0;

  // Build the message header first
  msgLen = buildHCIExtHeader(out_msg, (HCI_EXT_L2CAP_EVENT | L2CAP_CONNECT_REQ),
                              SUCCESS, connHandle);

  out_msg[msgLen++] = id;
  msgLen += L2CAP_BuildConnectReq(&out_msg[msgLen], (uint8_t *)pReq);

  // Send out the Connection Request
  HCI_SendControllerToHostEvent(HCI_VE_EVENT_CODE,  msgLen, out_msg);

  return(L2CAP_CONN_PENDING_SEC_VERIFY);
}

/*********************************************************************
 * @fn      buildCoChannelInfo
 *
 * @brief   Build Connection Oriented Channel info.
 *
 * @param   CID - local CID
 * @param   pInfo - pointer to CoC info
 * @param   pRspBuf - buffer to copy CoC info into
 *
 * @return  length of data copied
 */
static uint8_t buildCoChannelInfo(uint16_t CID, l2capCoCInfo_t *pInfo, uint8_t *pRspBuf)
{
  uint8_t msgLen = 0;

  pRspBuf[msgLen++] = LO_UINT16(pInfo->psm);
  pRspBuf[msgLen++] = HI_UINT16(pInfo->psm);

  pRspBuf[msgLen++] = LO_UINT16(CID);
  pRspBuf[msgLen++] = HI_UINT16(CID);

  pRspBuf[msgLen++] = LO_UINT16(pInfo->mtu);
  pRspBuf[msgLen++] = HI_UINT16(pInfo->mtu);

  pRspBuf[msgLen++] = LO_UINT16(pInfo->mps);
  pRspBuf[msgLen++] = HI_UINT16(pInfo->mps);

  pRspBuf[msgLen++] = LO_UINT16(pInfo->credits);
  pRspBuf[msgLen++] = HI_UINT16(pInfo->credits);

  pRspBuf[msgLen++] = LO_UINT16(pInfo->peerCID);
  pRspBuf[msgLen++] = HI_UINT16(pInfo->peerCID);

  pRspBuf[msgLen++] = LO_UINT16(pInfo->peerMtu);
  pRspBuf[msgLen++] = HI_UINT16(pInfo->peerMtu);

  pRspBuf[msgLen++] = LO_UINT16(pInfo->peerMps);
  pRspBuf[msgLen++] = HI_UINT16(pInfo->peerMps);

  pRspBuf[msgLen++] = LO_UINT16(pInfo->peerCredits);
  pRspBuf[msgLen++] = HI_UINT16(pInfo->peerCredits);

  pRspBuf[msgLen++] = LO_UINT16(pInfo->peerCreditThreshold);
  pRspBuf[msgLen++] = HI_UINT16(pInfo->peerCreditThreshold);

  return(msgLen);
}


#endif //(BLE_V41_FEATURES & L2CAP_COC_CFG)

#if defined(GATT_DB_OFF_CHIP)
/*********************************************************************
 * @fn      addAttrRec
 *
 * @brief   Add attribute record to its service.
 *
 * @param   pServ - GATT service
 * @param   pUUID - attribute UUID
 * @param   len - length of UUID
 * @param   permissions - attribute permissions
 * @param   pTotalAttrs - total number of attributes
 * @param   pRspDataLen - response data length to be returned
 *
 * @return  status
 */
static uint8_t addAttrRec(gattService_t *pServ, uint8_t *pUUID, uint8_t len,
                         uint8_t permissions, uint16_t *pTotalAttrs, uint8_t *pRspDataLen)
{
  gattAttribute_t *pAttr = &(pServ->attrs[pServ->numAttrs]);
  uint8_t stat = SUCCESS;

  // Set up attribute record
  pAttr->type.uuid = findUUIDRec(pUUID, len);
  if (pAttr->type.uuid != NULL)
  {
    pAttr->type.len = len;
    pAttr->permissions = permissions;

    // Are all attributes added to the service yet?
    if (++pServ->numAttrs == *pTotalAttrs)
    {
      // Register the service with the GATT Server
      stat = GATT_RegisterService(pServ);
      if (stat == SUCCESS)
      {
        *pRspDataLen = 4;

        // Service startHandle
        uint16_t handle = pServ->attrs[0].handle;
        rspBuf[RSP_PAYLOAD_IDX] = LO_UINT16(handle);
        rspBuf[RSP_PAYLOAD_IDX+1] = HI_UINT16(handle);

        // Service endHandle
        handle = pServ->attrs[pServ->numAttrs-1].handle;
        rspBuf[RSP_PAYLOAD_IDX+2] = LO_UINT16(handle);
        rspBuf[RSP_PAYLOAD_IDX+3] = HI_UINT16(handle);

        // Service is registered with GATT; clear its info
        pServ->attrs = NULL;
        pServ->numAttrs = 0;
      }
      else
      {
        freeAttrRecs(pServ);
      }

      // We're done with this service
      *pTotalAttrs = 0;
    }
  }
  else
  {
    stat = INVALIDPARAMETER;
  }

  return(stat);
}

/*********************************************************************
 * @fn      freeAttrRecs
 *
 * @brief   Free attribute records. Also, free UUIDs that were
 *          allocated dynamically.
 *
 * @param   pServ - GATT service
 *
 * @return  none
 */
static void freeAttrRecs(gattService_t *pServ)
{
  if (pServ->attrs != NULL)
  {
    for (uint8_t i = 0; i < pServ->numAttrs; i++)
    {
      gattAttrType_t *pType = &pServ->attrs[i].type;
      if (pType->uuid != NULL)
      {
        if (GATT_FindUUIDRec((uint8_t *)pType->uuid, pType->len) == NULL)
        {
          // UUID was dynamically allocated; free it
          ICall_free((uint8_t *)pType->uuid);
        }
      }
    }

    ICall_free(pServ->attrs);

    pServ->attrs = NULL;
    pServ->numAttrs = 0;
  }
}

/*********************************************************************
 * @fn      findUUIDRec
 *
 * @brief   Find UUID record. If the record is not found, create one
 *          dynamically.
 *
 * @param   pUUID - UUID to look for
 * @param   len - length of UUID
 *
 * @return  UUID record
 */
static const uint8_t *findUUIDRec(uint8_t *pUUID, uint8_t len)
{
  const uint8_t *pUuid = (uint8_t*) GATT_FindUUIDRec(pUUID, len);
  if (pUuid == NULL)
  {
    // UUID not found; allocate space for it
    pUuid = ICall_malloc(len);
    if (pUuid != NULL)
    {
      VOID memcpy((uint8_t *)pUuid, pUUID, len);
    }
    else
    {
#ifdef EXT_HAL_ASSERT
      HAL_ASSERT( HAL_ASSERT_CAUSE_OUT_OF_MEMORY );
#else /* !EXT_HAL_ASSERT */
      HAL_ASSERT( FALSE );
#endif /* EXT_HAL_ASSERT */
    }
  }

  return(pUuid);
}
#endif // GATT_DB_OFF_CHIP
#endif // HOST_CONFIG
#endif // #endif /* (defined(HCI_TL_FULL) || defined(HCI_TL_PTM) ) */



/*********************************************************************
*********************************************************************/

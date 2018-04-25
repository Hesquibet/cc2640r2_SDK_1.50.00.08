/******************************************************************************

 @file       gap_internal.h

 @brief This file contains internal interfaces for the GAP.

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

/*********************************************************************
 *
 * WARNING!!!
 *
 * THE API'S FOUND IN THIS FILE ARE FOR INTERNAL STACK USE ONLY!
 * FUNCTIONS SHOULD NOT BE CALLED DIRECTLY FROM APPLICATIONS, AND ANY
 * CALLS TO THESE FUNCTIONS FROM OUTSIDE OF THE STACK MAY RESULT IN
 * UNEXPECTED BEHAVIOR
 *
 */

#ifndef GAP_INTERNAL_H
#define GAP_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "hci.h"
#include "l2cap.h"
#include "gap.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// GAP OSAL Events
#define GAP_OSAL_TIMER_SCAN_DURATION_EVT        0x0001
#define GAP_END_ADVERTISING_EVT                 0x0002
#define GAP_CHANGE_RESOLVABLE_PRIVATE_ADDR_EVT  0x0004

#define GAP_PRIVATE_ADDR_CHANGE_RESOLUTION      0xEA60 // Timer resolution is 1 minute

#define ADV_TOKEN_HDR                           2

// Own Address Types understood by Controller for Enhanced Privacy
#define OWN_ADDRTYPE_PUBLIC                     0x00  //!< Use the BD_ADDR
#define OWN_ADDRTYPE_RANDOM                     0x01  //!< Use LL Random Address
#define OWN_ADDRTYPE_RPA_DEF_PUB                0x02  //!< LL Generates RPA, defaults to BD_ADDR if no local IRK
#define OWN_ADDRTYPE_RPA_DEF_RAND               0x03  //!< LL Generates RPA, defaults to Random Addr if no local IRK

#if defined ( TESTMODES )
  // GAP TestModes
  #define GAP_TESTMODE_OFF                      0 // No Test mode
  #define GAP_TESTMODE_NO_RESPONSE              1 // Don't respond to any GAP message
#endif  // TESTMODES

// L2CAP Connection Parameters Update Request event
#define L2CAP_PARAM_UPDATE                      0xFFFF

/*********************************************************************
 * TYPEDEFS
 */

typedef struct gapAdvToken
{
  struct gapAdvToken *pNext;     // Pointer to next item in link list
  gapAdvDataToken_t  *pToken;    // Pointer to data token
} gapAdvToken_t;

/** Advertising and Scan Response Data **/
typedef struct
{
  uint8   dataLen;                  // Number of bytes used in "dataField"
  uint8   dataField[B_MAX_ADV_LEN]; // Data field of the advertisement or SCAN_RSP
} gapAdvertisingData_t;

typedef struct
{
  uint8   dataLen;       // length (in bytes) of "dataField"
  uint8   dataField[1];  // This is just a place holder size
                         // The dataField will be allocated bigger
} gapAdvertRecData_t;

// Temporary advertising record
typedef struct
{
  uint8  eventType;               // Avertisement or SCAN_RSP
  uint8  addrType;                // Advertiser's address type
  uint8  addr[B_ADDR_LEN];        // Advertiser's address
  gapAdvertRecData_t *pAdData;    // Advertising data field. This space is allocated.
  gapAdvertRecData_t *pScanData;  // SCAN_RSP data field. This space is allocated.
} gapAdvertRec_t;

typedef enum
{
  GAP_ADSTATE_SET_PARAMS,     // Setting the advertisement parameters
  GAP_ADSTATE_SET_MODE,       // Turning on advertising
  GAP_ADSTATE_ADVERTISING,    // Currently Advertising
  GAP_ADSTATE_ENDING          // Turning off advertising
} gapAdvertStatesIDs_t;

// Advertising State Information
typedef struct
{
  uint8 taskID;                   // App that started an advertising period
  gapAdvertStatesIDs_t state;     // Make Discoverable state
  gapAdvertisingParams_t params;  // Advertisement parameters
} gapAdvertState_t;

typedef struct
{
  uint8                state;            // Authentication states
  uint16               connectionHandle; // Connection Handle from controller,
  smLinkSecurityReq_t  secReqs;          // Pairing Control info

  // The following are only used if secReqs.bondable == BOUND, which means that
  // the device is already bound and we should use the security information and
  // keys
  smSecurityInfo_t     *pSecurityInfo;    // BOUND - security information
  smIdentityInfo_t     *pIdentityInfo;    // BOUND - identity information
  smSigningInfo_t      *pSigningInfo;     // Signing information
} gapAuthStateParams_t;

// Callback when an HCI Command Event has been received on the Central.
typedef uint8 (*gapProcessHCICmdEvt_t)( uint16 cmdOpcode, hciEvt_CmdComplete_t *pMsg );

// Callback when an Scanning Report has been received on the Central.
typedef void (*gapProcessScanningEvt_t)( hciEvt_BLEAdvPktReport_t *pMsg );

// Callback to cancel a connection initiation on the Central.
typedef bStatus_t (*gapCancelLinkReq_t)( uint8 taskID, uint16 connectionHandle );

// Callback when a connection-related event has been received on the Central.
typedef uint8(*gapProcessConnEvt_t)( uint16 cmdOpcode, hciEvt_CommandStatus_t *pMsg );

// Callback when an HCI Command Command Event on the Peripheral.
typedef uint8 (*gapProcessHCICmdCompleteEvt_t)( hciEvt_CmdComplete_t *pMsg );

// Callback when an Advertising Event has been received on the Peripheral.
typedef void (*gapProcessAdvertisingEvt_t)( uint8 timeout );

// Callback when a Set Advertising Params has been received on the Peripheral.
typedef bStatus_t (*gapSetAdvParams_t)( void );

// Central callback structure - must be setup by the Central.
typedef struct
{
  gapProcessHCICmdEvt_t   pfnProcessHCICmdEvt;   // When HCI Command Event received
  gapProcessScanningEvt_t pfnProcessScanningEvt; // When Scanning Report received
} gapCentralCBs_t;

// Central connection-related callback structure - must be setup by the Central.
typedef struct
{
  gapCancelLinkReq_t  pfnCancelLinkReq;  // When cancel connection initiation requested
  gapProcessConnEvt_t pfnProcessConnEvt; // When connection-related event received
} gapCentralConnCBs_t;

// Peripheral callback structure - must be setup by the Peripheral.
typedef struct
{
  gapProcessHCICmdCompleteEvt_t pfnProcessHCICmdCompleteEvt; // When HCI Command Complete Event received
  gapProcessAdvertisingEvt_t    pfnProcessAdvertisingEvt;    // When Advertising Event received
  gapSetAdvParams_t             pfnSetAdvParams;             // When Set Advertising Params received
} gapPeripheralCBs_t;

// Peripheral connection-related callback structure - must be setup by the Peripheral.
typedef struct
{
  gapProcessConnEvt_t pfnProcessConnEvt;   // When connection-related event received
} gapPeripheralConnCBs_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

extern uint8 gapTaskID;

extern uint8 gapUnwantedTaskID;

extern uint8 gapAppTaskID;        // default task ID to send events

extern uint8 gapProfileRole;      // device GAP Profile Role(s)

extern uint8 gapDeviceAddrMode;   //  ADDRMODE_PUBLIC, ADDRMODE_STATIC,
                                  //  ADDRMODE_PRIVATE_NONRESOLVE
                                  //  or ADDRMODE_PRIVATE_RESOLVE

// Central variables
extern gapDevDiscReq_t *pGapDiscReq;

extern gapEstLinkReq_t *pEstLink;

extern gapCentralConnCBs_t *pfnCentralConnCBs;

// Peripheral variables
extern gapAdvertState_t *pGapAdvertState;

extern gapPeripheralCBs_t *pfnPeripheralCBs;

extern gapPeripheralConnCBs_t *pfnPeripheralConnCBs;

// Common variables
extern gapAuthStateParams_t *pAuthLink;

extern uint16 slaveConnHandle;

extern l2capParamUpdateReq_t slaveUpdateReq;

#if !defined (BLE_V42_FEATURES) || !(BLE_V42_FEATURES & PRIVACY_1_2_CFG)
extern uint16 gapPrivateAddrChangeTimeout;

extern uint8 gapAutoAdvPrivateAddrChange;
#endif // ! BLE_V42_FEATURES | ! PRIVACY_1_2_CFG

/*********************************************************************
 * GAP Task Functions
 */

extern uint8 gapProcessBLEEvents( osal_event_hdr_t *pMsg );
extern uint8 gapProcessCommandStatusEvt( hciEvt_CommandStatus_t *pMsg );
extern uint8 gapProcessConnEvt( uint16 connHandle, uint16 cmdOpcode, hciEvt_CommandStatus_t *pMsg );
extern uint8 gapProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t *pMsg );
extern uint8 gapProcessOSALMsg( osal_event_hdr_t *pMsg );
extern void gapRegisterCentral( gapCentralCBs_t *pfnCBs );
extern void gapRegisterPeripheral( gapPeripheralCBs_t *pfnCBs );

/*********************************************************************
 * GAP Link Manager Functions
 */

extern bStatus_t disconnectNext( uint8 taskID, uint8 reason );
extern void gapFreeAuthLink( void );
extern void gapFreeEstLink( void );
extern void gapPairingCompleteCB( uint8 status, uint8 initiatorRole, uint16 connectionHandle, uint8 authState,
                                  smSecurityInfo_t *pEncParams, smSecurityInfo_t *pDevEncParams,
                                  smIdentityInfo_t *pIdInfo, smSigningInfo_t  *pSigningInfo );
extern void gapPasskeyNeededCB( uint16 connectionHandle, uint8 type, uint32 numComparison );
extern void gapProcessConnectionCompleteEvt( uint8 eventCode, hciEvt_BLEConnComplete_u *pPkt );
extern void gapProcessDisconnectCompleteEvt( hciEvt_DisconnComplete_t *pPkt );
extern void gapProcessRemoteConnParamReqEvt( hciEvt_BLERemoteConnParamReq_t *pPkt, uint8 accepted );
extern void gapRegisterCentralConn( gapCentralConnCBs_t *pfnCBs);
extern void gapRegisterPeripheralConn( gapPeripheralConnCBs_t *pfnCBs);
extern void gapSendBondCompleteEvent( uint8 status, uint16 connectionHandle );
extern void gapSendLinkUpdateEvent( uint8 status, uint16 connectionHandle, uint16 connInterval,
                                    uint16 connLatency, uint16 connTimeout );
extern void gapSendPairingReqEvent( uint8 status, uint16 connectionHandle, uint8 ioCap, uint8 oobDataFlag,
                                    uint8 authReq, uint8 maxEncKeySize, keyDist_t keyDist );
extern void gapSendSignUpdateEvent( uint8 taskID, uint8 addrType, uint8 *pDevAddr, uint32 signCounter );
extern void gapSendSlaveSecurityReqEvent( uint8 taskID, uint16 connHandle, uint8 *pDevAddr, uint8 authReq );
extern void gapUpdateConnSignCounter( uint16 connHandle, uint32 newSignCounter );
extern void sendAuthEvent( uint8 status, uint16 connectionHandle, uint8 authState, smSecurityInfo_t *pDevEncParams );
extern void sendEstLinkEvent( uint8 status, uint8 taskID, uint8 devAddrType, uint8 *pDevAddr,
                              uint16 connectionHandle, uint8 connRole, uint16 connInterval,
                              uint16 connLatency, uint16 connTimeout, uint16 clockAccuracy );
extern void sendTerminateEvent( uint8 status, uint8 taskID, uint16 connectionHandle, uint8 reason );

/*********************************************************************
 * GAP Configuration Manager Functions
 */

extern void gapAddAddrAdj( uint8 addrMode, uint8 *pAddr );
extern uint8 *gapGetDevAddress( uint8 real );
extern uint8 gapGetDevAddressMode( void );
extern uint32 gapGetSignCounter( void );
extern uint8 *gapGetSRK( void );
extern uint8 gapHost2CtrlOwnAddrType( uint8 addrMode );
extern  void gapIncSignCounter( void );
extern bStatus_t gapProcessNewAddr( uint8 *pNewAddr );
extern void gapProcessRandomAddrComplete( uint8 status );
extern uint8 gapReadBD_ADDRStatus( uint8 status, uint8 *pBdAddr );
extern uint8 gapReadBufSizeCmdStatus( uint8 *pCmdStat );
extern void gapSendDeviceInitDoneEvent( uint8 status );
extern void gapSendRandomAddrChangeEvent( uint8 status, uint8 *pNewAddr );

/*********************************************************************
 * GAP Device Manager Functions
 */

extern void gapClrState( uint8 state_flag );
extern uint8 *gapFindADType( uint8 adType, uint8 *pAdLen, uint8 dataLen, uint8 *pDataField );
extern uint8 gapIsAdvertising( void );
extern uint8 gapIsScanning( void );
extern void gapSetState( uint8 state_flag );
extern uint8 gapValidADType( uint8 adType );

/*********************************************************************
 * GAP Peripheral Link Manager Functions
 */

extern void gapL2capConnParamUpdateReq( void );
extern uint8 gapPeriProcessConnEvt( uint16 cmdOpcode, hciEvt_CommandStatus_t *pMsg );
extern void gapPeriProcessConnUpdateCmdStatus( uint8 status );
extern void gapPeriProcessConnUpdateCompleteEvt( hciEvt_BLEConnUpdateComplete_t *pPkt );
extern uint8 gapPeriProcessSignalEvt( l2capSignalEvent_t *pCmd );

/*********************************************************************
 * GAP Peripheral Device Manager Functions
 */

extern bStatus_t gapAddAdvToken( gapAdvDataToken_t *pToken );
extern bStatus_t gapAllocAdvRecs( void );
extern bStatus_t gapBuildADTokens( void );
extern void gapCalcAdvTokenDataLen( uint8 *pAdLen, uint8 *pSrLen );
extern void gapConnectedCleanUpAdvertising( void );
extern gapAdvDataToken_t *gapDeleteAdvToken( uint8 ADType );
extern gapAdvToken_t *gapFindAdvToken( uint8 ADType );
extern void gapFreeAdvertState( void );
extern uint8 gapPeriProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t *pMsg );
extern void gapProcessAdvertisingEvt( uint8 timeout );
extern void gapProcessAdvertisingTimeout( void );
extern void gapSendAdDataUpdateEvent( uint8 adType, uint8 status );
extern void gapSendEndDiscoverableEvent( uint8 status );
extern void gapSendMakeDiscEvent( bStatus_t status );
extern bStatus_t gapSetAdvParams( void );
extern uint8 gapSetAdvParamsStatus( uint8 status );
extern void gapWriteAdvDataStatus( uint8 adType, uint8 status );
extern uint8 gapWriteAdvEnableStatus( uint8 status );
extern uint8 isLimitedDiscoverableMode( void );

/*********************************************************************
 * GAP Central Link Manager Functions
 */

extern bStatus_t gapCancelLinkReq( uint8 taskID, uint16 connectionHandle );
extern uint8 gapCentProcessConnEvt( uint16 cmdOpcode, hciEvt_CommandStatus_t *pMsg );
extern void gapCentProcessConnUpdateCmdStatus( uint8 status );
extern void gapCentProcessConnUpdateCompleteEvt( hciEvt_BLEConnUpdateComplete_t *pPkt );
extern void gapCentProcessSignalEvt( l2capSignalEvent_t *pCmd );
extern void gapProcessCreateLLConnCmdStatus( uint8 status );
extern void gapTerminateConnComplete( void );

/*********************************************************************
 * GAP Central Device Manager Functions
 */

extern bStatus_t gapAllocScanRecs( void );
extern uint8 gapCentProcessHCICmdEvt( uint16 cmdOpcode, hciEvt_CmdComplete_t *pMsg );
extern gapAdvertRec_t *gapFindEmptyScanRec( void );
extern gapAdvertRec_t *gapFindScanRec( uint8 addrType, uint8 *pAddr );
extern void gapFreeScanRecs( uint8 numScanRecs, uint8 freeRecs );
extern uint8 gapProcessAdvertDevInfo( hciEvt_DevInfo_t *pDevInfo );
extern void gapProcessAdvertisementReport( hciEvt_BLEAdvPktReport_t *pPkt );
extern void gapProcessScanDurationTimeout( void );
extern void gapProcessScanningEvt( hciEvt_BLEAdvPktReport_t *pPkt );
extern void gapSendDevDiscEvent( bStatus_t status );
extern void gapSendDeviceInfoEvent( hciEvt_DevInfo_t *pDevInfo );
extern bStatus_t gapSendScanEnable( uint8 enable );
extern uint8 gapSetScanParamStatus( uint8 status );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* GAP_INTERNAL_H */

/******************************************************************************

 @file       oad.h

 @brief This file contains OAD Profile header file.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2017-2017, Texas Instruments Incorporated
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
#ifndef OAD_H
#define OAD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "hal_types.h"
#include <ti/sysbios/knl/Queue.h>

#include "oad_image_header.h"

/*********************************************************************
 * CONSTANTS
 */

// This define controls whether OAD requires BLE security
// By default, this is off
#ifdef OAD_BLE_SECURITY
    #define OAD_WRITE_PERMIT     GATT_PERMIT_AUTHEN_WRITE
#else
    #define OAD_WRITE_PERMIT     GATT_PERMIT_WRITE
#endif //OAD_SECURITY

// Number of failed image IDs are allowed before OAD terminates the cxn
#define OAD_IMG_ID_RETRIES                  3

// Events the OAD process will post to the app
#define OAD_QUEUE_EVT                       Event_Id_01
#define OAD_DL_COMPLETE_EVT                 Event_Id_02

/*
 * OAD Service UUIDs:
 * Warning! The image count and image status chars are deprecated
 * their functionality has been moved to the control point
 */
#define OAD_SERVICE_UUID                    0xFFC0
#define OAD_IMG_IDENTIFY_UUID               0xFFC1
#define OAD_IMG_BLOCK_UUID                  0xFFC2
#define OAD_IMG_COUNT_UUID                  0xFFC3  // Deprecated!
#define OAD_IMG_STATUS_UUID                 0xFFC4  // Deprecated!
#define OAD_EXT_CTRL_UUID                   0xFFC5

// OAD Characteristic Indices
#define OAD_IDX_IMG_IDENTIFY                0
#define OAD_IDX_IMG_BLOCK                   1
#define OAD_IDX_EXT_CTRL                    2

// OAD payload overheads
#define OAD_ATT_OVERHEAD                    3
#define OAD_BLK_NUM_HDR_SZ                  4

/*
 * OAD Block size can range from [OAD_DEFAULT_BLOCK_SIZE,OAD_MAX_BLOCK_SIZE]
 * Note: OAD block size must be 4 Octet aligned
 */
#define OAD_DEFAULT_BLOCK_SIZE              20
#define OAD_MAX_BLOCK_SIZE                  244

#define OAD_IMAGE_ID_RSP_LEN                0x01

// OAD EXT CTRL CMD OP CODES
#define OAD_EXT_CTRL_GET_BLK_SZ             0x01
#define OAD_EXT_CTRL_IMG_CNT                0x02
#define OAD_EXT_CTRL_START_OAD              0x03
#define OAD_EXT_CTRL_ENABLE_IMG             0x04
#define OAD_EXT_CTRL_CANCEL_OAD             0x05
#define OAD_EXT_CTRL_DISABLE_BLK_NOTIF      0x06
#define OAD_EXT_CTRL_GET_SW_VER             0x07
#define OAD_EXT_CTRL_GET_IMG_STAT           0x08
#define OAD_EXT_CTRL_GET_PROF_VER           0x09
#define OAD_EXT_CTRL_GET_DEV_TYPE           0x10
#define OAD_EXT_CTRL_GET_IMG_INFO           0x11
#define OAD_EXT_CTRL_BLK_RSP_NOTIF          0x12
#define OAD_EXT_CTRL_ERASE_BONDS            0x13

// OAD EXT CTRL Error code for unsupported command
#define OAD_EXT_CTRL_CMD_NOT_SUPPORTED      0xFF

#define OAD_IMG_INFO_ONCHIP                 0x01

// OAD has a 10s inactivity timeout (measured in ms)
#define OAD_DEFAULT_INACTIVITY_TIME         10000
#define OAD_MIN_INACTIVITY_TIME             4000

// Each Char in the service has a CCCD
#define OAD_CHAR_CNT                        3
#define OAD_CCCD_CNT                        OAD_CHAR_CNT

// OAD Status codes
#define OAD_SUCCESS                         0   //!< OAD succeeded
#define OAD_CRC_ERR                         1   //!< Downloaded image CRC doesn't match
#define OAD_FLASH_ERR                       2   //!< Flash function failure (int, ext)
#define OAD_BUFFER_OFL                      3   //!< Block Number doesn't match requested.
#define OAD_ALREADY_STARTED                 4   //!< OAD is already is progress
#define OAD_NOT_STARTED                     5   //!< OAD has not yet started
#define OAD_DL_NOT_COMPLETE                 6   //!< An OAD is ongoing
#define OAD_NO_RESOURCES                    7   //!< If memory allocation fails
#define OAD_IMAGE_TOO_BIG                   8   //!< Candidate image is too big
#define OAD_INCOMPATIBLE_IMAGE              9   //!< Image signing failure, boundary mismatch
#define OAD_INVALID_FILE                    10  //!< If Invalid image ID received
#define OAD_INCOMPATIBLE_FILE               11  //!< BIM or FW mismatch
#define OAD_AUTH_FAIL                       12  //!< Authorization failed
#define OAD_EXT_NOT_SUPPORTED               13  //!< Ctrl point command not supported
#define OAD_DL_COMPLETE                     14  //!< OAD image payload download complete
#define OAD_CCCD_NOT_ENABLED                15  //!< CCCD is not enabled, notif can't be sent
#define OAD_IMG_ID_TIMEOUT                  16  //!< Image identify timed out, too many failures

//!< OAD Image Types
#define OAD_IMG_TYPE_PERSIST                0
#define OAD_IMG_TYPE_APP_ONLY               1
#define OAD_IMG_TYPE_STACK_ONLY             2
#define OAD_IMG_TYPE_APP_STACK              3
#define OAD_IMG_TYPE_BLE_FACTORY            4
#define OAD_IMG_TYPE_BIM                    5
#define OAD_IMG_TYPE_USR_BEGIN              16
#define OAD_IMG_TYPE_USR_END                31
#define OAD_IMG_TYPE_HOST_BEGIN             32
#define OAD_IMG_TYPE_HOST_END               63
#define OAD_IMG_TYPE_RSVD_BEGIN             64
#define OAD_IMG_TYPE_RSVD_END               255

#define BIM_START                           0x1F000

//!< Segment types
#define OAD_SEG_BOUNDARY                    0x00
#define OAD_SEG_CONTIGUOUS                  0x01
#define OAD_SEG_NON_CONTIGUOUS              0x02
#define OAD_SEG_RSVD                        0xFF

//!< Wireless Technology Types
#define WIRELESS_TECH_BLE                   0xFE
#define WIRELESS_TECH_TIMAC_SUBG            0xFD
#define WIRELESS_TECH_TIMAC_2_4G            0xFB
#define WIRELESS_TECH_ZIGBEE                0xF7
#define WIRELESS_TECH_RF4CE                 0xEF
#define WIRELESS_TECH_THREAD                0xDF
#define WIRELESS_TECH_EASYLINK              0xBF

#define OAD_IMG_PG_INVALID                  0xFF

/*********************************************************************
 * MACROS
 */

#define EXT_CTRL_OP_CODE(pData)(pData[0])

/*********************************************************************
 * TYPEDEFS
 */

/* Image Identify Payload */
PACKED_TYPEDEF_STRUCT
{
    uint8_t   imgID[8];       //!< User-defined Image Identification bytes. */
    uint8_t   bimVer;         //!< BIM version */
    uint8_t   metaVer;        //!< Metadata version */
    uint8_t   imgCpStat;      //!< Image copy status bytes */
    uint8_t   crcStat;        //!< CRC status */
    uint8_t   imgType;        //!< Image Type */
    uint8_t   imgNo;          //!< Image number of 'image type' */
    uint32_t  len;            //!< Image length in octets
    uint8_t   softVer[4];     //!< Software version of the image */
} imgIdentifyPld_t;

PACKED_TYPEDEF_STRUCT
{
    uint8_t   cmdID;
    uint8_t   prevBlkStat;
    uint32_t  requestedBlk;
}blockReqPld_t;

PACKED_TYPEDEF_STRUCT
{
    uint8_t   imgCpStat;
    uint8_t   crcStat;
    uint8_t   imgType;
    uint8_t   imgNo;
} imageInfo_t;

typedef enum
{
    OAD_WRITE_IDENTIFY_REQ,
    OAD_WRITE_BLOCK_REQ,
    OAD_IMAGE_COMPLETE,
    OAD_EXT_CTRL_WRITE_CMD,
    OAD_TIMEOUT
}oadEvent_e;

/* OAD Profile Write Event struct */
typedef struct
{
    Queue_Elem _elem;
    oadEvent_e  event;
    uint16_t connHandle;
    uint16_t len;
    uint16_t offset;
    uint8_t  *pData;
} oadTargetWrite_t;

typedef enum
{
    OAD_IDLE,
    OAD_VALIDATE_IMG,
    OAD_CONFIG,
    OAD_DOWNLOAD,
    OAD_CHECK_DL,
    OAD_COMPLETE
}oadState_e;


/*********************************************************************
 * Ext Control Command Payloads
 */

PACKED_TYPEDEF_STRUCT
{
    uint8_t   cmdID;
    uint8_t   status;
} genericExtCtrlRsp_t;

PACKED_TYPEDEF_STRUCT
{
    uint8_t   cmdID;
    uint8_t   profVer;
} profileVerRsp_t;

PACKED_TYPEDEF_STRUCT
{
    uint8_t     cmdID;
    uint16_t    oadBlkSz;
} blockSizeRspPld_t;

PACKED_TYPEDEF_STRUCT
{
    uint8_t       cmdID;
    uint8_t       swVer[OAD_SW_VER_LEN];
} swVersionPld_t;

PACKED_TYPEDEF_STRUCT
{
    uint8_t   cmdID;
    uint8_t   chipType;
    uint8_t   chipFamily;
    uint8_t   hardwareRev;
    uint8_t   rsvd;
} deviceTypeRspPld_t;

PACKED_TYPEDEF_STRUCT
{
    uint8_t   cmdID;
    uint8_t   numImages;
    uint8_t   imgCpStat;
    uint8_t   crcStat;
    uint8_t   imgType;
    uint8_t   imgNo;
} imageInfoRspPld_t;

PACKED_TYPEDEF_STRUCT
{
    uint8_t     cmdID;
    uint8_t     imgType;
    uint8_t     imgNo;
    uint16_t    techType;
}extImgEnableReq_t;


/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*oadWriteCB_t)(uint8_t event, uint16_t arg);

typedef struct
{
    oadWriteCB_t       pfnOadWrite; // Called when characteristic value changes.
} oadTargetCBs_t;

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      OAD_open
 *
 * @brief   Initializes the OAD Service by registering GATT attributes
 *          with the GATT server. Only call this function once.
 *
 * @param   oadTimeout - the timeout on for a pending OAD operation (in ms)
 *
 * @return  Success or Failure
 */
extern uint8_t OAD_open(uint32_t oadTimeout);

/*********************************************************************
 * @fn      OAD_cancel
 *
 * @brief   Cancel the OAD in progress and reset the state
 *
 * @param   None.
 *
 * @return  None.
 */
extern void OAD_cancel(void);


/*********************************************************************
 * @fn      OAD_close
 *
 * @brief   Close the OAD and its flash_interface, reset the state
 *
 * @param   None.
 *
 * @return  None.
 */
extern void OAD_close(void);

/*********************************************************************
 * @fn      OAD_isUsrAppValid
 *
 * @brief   Returns whether or not the user application is valid
 *
 * @param   None.
 *
 * @return  user app status - true if the user app hans't been erased.
 */
extern bool OAD_isUsrAppValid(void);

/*********************************************************************
 * @fn      OAD_register
 *
 * @brief   Register a callback function with the OAD Target Profile.
 *
 * @param   pfnOadCBs - struct holding function pointers to OAD application
 *                      callbacks.
 *
 * @return  None.
 */
extern void OAD_register(oadTargetCBs_t *pfnOadCBs);

/*********************************************************************
 * @fn      OAD_getSWVersion
 *
 * @brief   Returns the currently running SW version field
 *
 * @param   swVer - pointer to array to copy version into
 * @param   len - number of bytes to copy
 *
 * @return  status - true if copy succeeded
 *                 - false if copy failed (invalid param)
 */
extern bool OAD_getSWVersion(uint8_t *swVer, uint8_t len);

/*********************************************************************
 * @fn      OAD_getBlockSize
 *
 * @brief   Gets the block size currently used
 *
 * @param   None.
 *
 * @return  The block size being used.
 */
extern uint16_t OAD_getBlockSize(void);

/*********************************************************************
 * @fn      OAD_setBlockSize
 *
 * @brief   Sets the block size to be used during OAD. Will choose the
 *          largest possible value out of 16, 32, 64, and 128.
 *
 * @param   None.
 *
 * @return  TRUE if operation is successful.
 */
extern uint8_t OAD_setBlockSize(uint16_t mtuSize);

/*********************************************************************
 * @fn      OAD_processQueue
 *
 * @brief   Process the OAD message queue.
 *
 * @param   none
 *
 * @return  none
 */
extern uint8_t OAD_processQueue(void);

/*********************************************************************
 * @fn      OAD_getactiveCxnHandle
 *
 * @brief   Returns the active connection handle of the OAD session
 *
 * @param   none
 *
 * @return  handle - the 16bit connection handle of the OAD
 */
extern uint16_t OAD_getactiveCxnHandle();

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OAD_H */

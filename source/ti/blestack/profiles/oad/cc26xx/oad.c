/******************************************************************************

 @file       oad.c

 @brief This file contains OAD profile implementation.

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

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include "hal_flash.h"
#include <driverlib/rom.h>
#include <driverlib/vims.h>

#include <icall.h>
#include "util.h"
/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include "flash_interface.h"
#include "oad.h"
#include "crc32.h"
#include "oad_image_header.h"
#include "ext_flash_layout.h"

#include <driverlib/chipinfo.h>

/*********************************************************************
 * CONSTANTS
 */

#define OAD_PROFILE_VERSION     0x01

#define OAD_BIM_START           0x1F000

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * GLOBAL VARIABLES
 */

// OAD Service UUID
static const uint8_t oadServUUID[ATT_UUID_SIZE] =
{
    TI_BASE_UUID_128(OAD_SERVICE_UUID)
};

static const uint8_t oadCharUUID[OAD_CHAR_CNT][ATT_UUID_SIZE] =
{
    // OAD Image Identify UUID
    TI_BASE_UUID_128(OAD_IMG_IDENTIFY_UUID),

    // OAD Image Block Request/Response UUID
    TI_BASE_UUID_128(OAD_IMG_BLOCK_UUID),

    // OAD Extended Control UUID
    TI_BASE_UUID_128(OAD_EXT_CTRL_UUID)
};


// The current image's header is initialized in oad_image_header_app.c
extern const imgHdr_t _imgHdr;


// The stack's image header is located in the main function
extern const imgHdr_t *stackImageHeader;

/*********************************************************************
 * Profile Attributes - variables
 */

// OAD Service attribute
static const gattAttrType_t oadService = { ATT_UUID_SIZE, oadServUUID };

// Place holders for the GATT Server App to be able to lookup handles.
static uint8_t oadCharVals[OAD_CHAR_CNT] = {0, 0 , OAD_SUCCESS};

// OAD Characteristic Properties
static uint8_t oadCharProps = GATT_PROP_WRITE_NO_RSP | GATT_PROP_WRITE
                              | GATT_PROP_NOTIFY;

static uint8_t oadCharExtCtrlProps = GATT_PROP_WRITE_NO_RSP | GATT_PROP_NOTIFY;

// OAD Client Characteristic Configs
static gattCharCfg_t *oadImgIdentifyConfig;
static gattCharCfg_t *oadImgBlockConfig;
static gattCharCfg_t *oadExtCtrlConfig;

static gattCharCfg_t *oadCCCDTable;

// OAD Characteristic user descriptions
static const uint8_t oadImgIdentifyDesc[] = "Img Identify";
static const uint8_t oadImgBlockDesc[] = "Img Block";
static const uint8_t oadExtCtrlDesc[] = "OAD Extended Control";

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t oadAttrTbl[] =
{
  // OAD Service
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID },
    GATT_PERMIT_READ,
    0,
    (uint8_t *)&oadService
  },

    // OAD Image Identify Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &oadCharProps
    },

        // OAD Image Identify Characteristic Value
        {
            { ATT_UUID_SIZE, oadCharUUID[OAD_IDX_IMG_IDENTIFY] },
            OAD_WRITE_PERMIT,
            0,
            oadCharVals
        },

        // Characteristic configuration
        {
            { ATT_BT_UUID_SIZE, clientCharCfgUUID },
            GATT_PERMIT_READ | OAD_WRITE_PERMIT,
            0,
            (uint8_t *)&oadImgIdentifyConfig
        },

        // OAD Image Identify User Description
        {
            { ATT_BT_UUID_SIZE, charUserDescUUID },
            GATT_PERMIT_READ,
            0,
            (uint8_t *)oadImgIdentifyDesc
        },

    // OAD Image Block Request/Response Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &oadCharProps
    },

        // OAD Image Block Request/Response Characteristic Value
        {
            { ATT_UUID_SIZE, oadCharUUID[OAD_IDX_IMG_BLOCK] },
            OAD_WRITE_PERMIT,
            0,
            oadCharVals + OAD_IDX_IMG_BLOCK,
        },

        // Characteristic configuration
        {
            { ATT_BT_UUID_SIZE, clientCharCfgUUID },
            GATT_PERMIT_READ | OAD_WRITE_PERMIT,
            0,
            (uint8_t *)&oadImgBlockConfig
        },

        // OAD Image Block Request/Response User Description
        {
            { ATT_BT_UUID_SIZE, charUserDescUUID },
            GATT_PERMIT_READ,
            0,
            (uint8_t *)oadImgBlockDesc
        },

    // OAD Extended Control Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &oadCharExtCtrlProps
    },

        // OAD Extended Control Characteristic Value
        {
            { ATT_UUID_SIZE, oadCharUUID[OAD_IDX_EXT_CTRL] },
            OAD_WRITE_PERMIT,
            0,
            oadCharVals + OAD_IDX_EXT_CTRL
        },

        // Characteristic configuration
        {
            { ATT_BT_UUID_SIZE, clientCharCfgUUID },
            GATT_PERMIT_READ | OAD_WRITE_PERMIT,
            0,
            (uint8_t *)&oadExtCtrlConfig
        },

        // OAD Extended Control User Description
        {
            { ATT_BT_UUID_SIZE, charUserDescUUID },
            GATT_PERMIT_READ,
            0,
            (uint8_t *)oadExtCtrlDesc
        }
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static imgHdr_t candidateImageHeader;

static oadWriteCB_t oadTargetWriteCB = NULL;

static uint32_t oadBlkNum = 0;
static uint32_t oadBlkTot = 0xFFFFFFFF;
static uint16_t oadBlkSize = OAD_DEFAULT_BLOCK_SIZE;
static uint16_t oadImgBytesPerBlock = OAD_DEFAULT_BLOCK_SIZE - OAD_BLK_NUM_HDR_SZ;
static uint8_t  blkReqActive = true;
static uint8_t numBlksInImgHdr = 0;

/* Information about image that is currently being downloaded */
static uint32_t imageAddress = 0;
static uint16_t imagePage = 0;
static uint32_t candidateImageLength = 0xFFFFFFFF;
static uint8_t  candidateImageType = OAD_IMG_TYPE_APP_ONLY;


static bool useExternalFlash = false;

// Number of retries allowed on image ID
static uint8_t imgIDRetries = OAD_IMG_ID_RETRIES;

// OAD is only allowed via a single connection, store it here
static uint16_t activeOadCxnHandle = NULL;

// Page that metadata is stored on
uint16_t metaPage = 0;

// OAD Queue
static Queue_Struct oadQ;
static Queue_Handle hOadQ;

// OAD Activity Clock
static Clock_Struct oadActivityClk;

static oadState_e state = OAD_IDLE;
static oadState_e nextState = OAD_IDLE;

static bool userAppValid = true;

static uint32_t oadStateTimeout = OAD_DEFAULT_INACTIVITY_TIME;


static uint8_t oadGlobalStatus = OAD_SUCCESS;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static bStatus_t oadReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                               uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                               uint16_t maxLen, uint8_t method);

static bStatus_t oadWriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                uint8_t *pValue, uint16_t len, uint16_t offset,
                                uint8_t method);

static uint8_t oadImgIdentifyWrite(uint16_t connHandle, uint8_t *pValue,
                                   uint16_t len, uint16_t offset);

static uint8_t oadProcessExtControlCmd(uint16_t connHandle, uint8_t  *pData,
                                       uint16_t len);

static uint8_t oadImgBlockWrite(uint16_t connHandle, uint8_t *pValue, uint8_t len);

static void oadGetNextBlockReq(uint16_t connHandle, uint32_t blkNum,
                                uint8_t status);

static uint8_t oadEnqueueMsg(oadEvent_e event, uint16_t connHandle,
                           uint8_t *pData, uint16_t len);

static uint8_t oadSendNotification(uint16_t connHandle, gattCharCfg_t *charCfg,
                                    uint8_t charIdx, uint8_t *pData,
                                    uint8_t len);

static uint8_t oadValidateCandidateHdr(imgHdr_t *receivedHeader);

static uint8_t oadFindCurrentImageHdr(void);

static uint8_t oadCheckDL(void);

static void oadResetState(void);

static void oadInactivityTimeout(UArg param);

static uint8_t oadCheckImageID(imgIdentifyPld_t *idPld);

static uint32_t oadFindExtFlImgAddr(uint8_t imgType);

static uint8_t oadFindExtFlMetaPage(void);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// Note: When an operation on a characteristic requires authorization and
// pfnAuthorizeAttrCB is not defined for that characteristic's service, the
// Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
// operation on a characteristic requires authorization the Stack will call
// pfnAuthorizeAttrCB to check a client's authorization prior to calling
// pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
// made within these functions.
CONST gattServiceCBs_t oadCBs =
{
    oadReadAttrCB,  // Read callback function pointer.
    oadWriteAttrCB, // Write callback function pointer.
    NULL            // Authorization callback function pointer.
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      OAD_open
 *
 * @brief   Initializes the OAD Service by registering GATT attributes
 *          with the GATT server. Only call this function once.
 *
 * @param   oadTimeout - the timeout on for a pending OAD operation (in ms)
 *
 * @return  The return value of GATTServApp_RegisterForMsg().
 */
uint8_t OAD_open(uint32_t oadTimeout)
{
    uint8_t status = OAD_SUCCESS;
    uint8_t oadCCCDTableSize = (sizeof(gattCharCfg_t) * linkDBNumConns) \
                                 * OAD_CCCD_CNT;

    /*
     * Allocate all memory for OAD CCCDs in a single malloc. This is
     * more efficient in terms of heap. The individual pointers will
     * be set as offsets into the table. Each CCCD entry is of size
     * sizeof(gattCharCfg_t)*linkDBNumConns)
     */
    oadCCCDTable = (gattCharCfg_t *)ICall_malloc(oadCCCDTableSize);

    // Ensure there is heap memory available for the CCCD table
    if (oadCCCDTable == NULL)
    {
        status = OAD_NO_RESOURCES;
    }

    // ImgID has no offset into the table
    oadImgIdentifyConfig = oadCCCDTable;
    // ImgBlock is the second entry
    oadImgBlockConfig = oadCCCDTable + linkDBNumConns;
    // ExtCtrl is the third entry
    oadExtCtrlConfig = oadCCCDTable + 2*linkDBNumConns;

    // Initialize Client Characteristic Configuration attributes.
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, oadImgIdentifyConfig);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, oadImgBlockConfig);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, oadExtCtrlConfig);

    //reset state variable
    state = OAD_IDLE;
    nextState = OAD_IDLE;

    // Construct the OAD Queue
    Queue_construct(&oadQ, NULL);

    // Get the handle to the newly constructed Queue
    hOadQ = Queue_handle(&oadQ);

    // Create OAD activity timer
    Util_constructClock(&oadActivityClk, oadInactivityTimeout,
                        oadStateTimeout, 0, false, OAD_QUEUE_EVT);


    bStatus_t registerStat;
    registerStat = GATTServApp_RegisterService(oadAttrTbl,
                                                GATT_NUM_ATTRS(oadAttrTbl),
                                                GATT_MAX_ENCRYPT_KEY_SIZE,
                                                &oadCBs);

    if(registerStat != SUCCESS)
    {
        status = OAD_NO_RESOURCES;
    }

    // Open the flash interface
    bool flashStat = flash_open();

    if(flashStat != true)
    {
        status = OAD_FLASH_ERR;
    }

    // This variable controls whether the OAD module uses internal or external
    // flash memory
    useExternalFlash = hasExternalFlash();

    if(oadTimeout > OAD_MIN_INACTIVITY_TIME)
    {
        // If the user input timeout is sane then use that
        oadStateTimeout = oadTimeout;
    }

    return (status);
}

/*********************************************************************
 * @fn      OAD_register
 *
 * @brief   Register a callback function with the OAD Target Profile.
 *
 * @param   *pfnOadCBs - write callback function container.
 *
 * @return  None.
 */
void OAD_register(oadTargetCBs_t *pfnOadCBs)
{
    // Register a write callback function.
    oadTargetWriteCB = pfnOadCBs->pfnOadWrite;
}

/*********************************************************************
 * @fn      OAD_getactiveCxnHandle
 *
 * @brief   Returns the active connection handle of the OAD session
 *
 * @param   none
 *
 * @return  handle - the 16bit connection handle of the OAD
 */
uint16_t OAD_getactiveCxnHandle()
{
    return (activeOadCxnHandle);
}

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
bool OAD_getSWVersion(uint8_t *swVer, uint8_t len)
{
    bool status = true;

    if(len == OAD_SW_VER_LEN)
    {
        // Set status to true
        status = true;

        // Read in the current image header
        uint8_t currentImgPg = oadFindCurrentImageHdr();
        if(currentImgPg != OAD_IMG_PG_INVALID)
        {
          const imgHdr_t *currentImgHdr = (imgHdr_t *)(currentImgPg * HAL_FLASH_PAGE_SIZE);

          // Populate the software version field
          uint8_t swVerCombined[4] = {stackImageHeader->softVer[0],
                                      stackImageHeader->softVer[1],
                                      currentImgHdr->softVer[2],
                                      currentImgHdr->softVer[3]};

          // Copy into the return buffer
          memcpy(swVer, &swVerCombined, OAD_SW_VER_LEN);
        }
        else
        {
          // Application version not available, populate just the stck version
          uint8_t swVerCombined[4] = {stackImageHeader->softVer[0],
                                      stackImageHeader->softVer[1],
                                      0xFF,
                                      0xFF};
          // Copy into the return buffer
          memcpy(swVer, &swVerCombined, OAD_SW_VER_LEN);
        }
    }
    else
    {
        status = false;
    }

    return (status);
}

/*********************************************************************
 * @fn      OAD_processQueue
 *
 * @brief   Process the OAD message queue.
 *
 * @param   none
 *
 * @return  none
 */
uint8_t OAD_processQueue(void)
{
    // Status of the event
    uint8_t status;

    // Get the message at the Queue head
    oadTargetWrite_t *oadWriteEvt = Queue_get(hOadQ);

    // Iterate over all the messages in the queue, in a threadsafe manner
    while (oadWriteEvt != (oadTargetWrite_t *)hOadQ)
    {
        if(oadWriteEvt != NULL)
        {
            // Extract the event from the message
            oadEvent_e event = oadWriteEvt->event;

            // Timeout is state independent. Always results in OAD state reset
            if(event == OAD_TIMEOUT)
            {
                // Clear Queue and reset variables
                oadResetState();
                // Move to IDLE state
                nextState = OAD_IDLE;
            }
            else if(state == OAD_IDLE)
            {
                // Identify new image.
                if (event == OAD_WRITE_IDENTIFY_REQ)
                {
                    status = oadImgIdentifyWrite(oadWriteEvt->connHandle,
                                                    oadWriteEvt->pData,
                                                    oadWriteEvt->len,
                                                    oadWriteEvt->offset);

                    // Advance the state based on status
                    nextState = (status == OAD_SUCCESS) ? OAD_CONFIG: OAD_IDLE;
                }
                else if (event == OAD_EXT_CTRL_WRITE_CMD)
                {
                    status = oadProcessExtControlCmd(oadWriteEvt->connHandle,
                                                        oadWriteEvt->pData,
                                                        oadWriteEvt->len);
                    nextState = OAD_IDLE;
                }
                else
                {
                    // Send a notification to the peer
                    oadGetNextBlockReq(oadWriteEvt->connHandle, 0x00000000,
                                        OAD_NOT_STARTED);
                    // An error has occured, reset and go back to idle
                    oadResetState();
                    nextState = OAD_IDLE;
                }
            }
            else if (state == OAD_CONFIG)
            {
                if(event == OAD_EXT_CTRL_WRITE_CMD)
                {
                    uint8_t opCode = EXT_CTRL_OP_CODE(oadWriteEvt->pData);

                    if(opCode == OAD_EXT_CTRL_START_OAD)
                    {
                        nextState = OAD_DOWNLOAD;
                    }
                    else
                    {
                        nextState = OAD_CONFIG;
                    }

                    status = oadProcessExtControlCmd(oadWriteEvt->connHandle,
                                                        oadWriteEvt->pData,
                                                        oadWriteEvt->len);


                }
                else
                {
                    // Send a notification to the peer
                    oadGetNextBlockReq(oadWriteEvt->connHandle, 0x00000000,
                                        OAD_NOT_STARTED);
                    // An error has occured, reset and go back to idle
                    oadResetState();
                    nextState = OAD_IDLE;
                }
            }
            else if (state == OAD_DOWNLOAD)
            {
                if(event == OAD_WRITE_BLOCK_REQ)
                {
                    status = oadImgBlockWrite(oadWriteEvt->connHandle,
                                                oadWriteEvt->pData,
                                                oadWriteEvt->len);

                    // Request the next block
                    oadGetNextBlockReq(oadWriteEvt->connHandle, oadBlkNum,
                                        status);

                    if(status == OAD_SUCCESS)
                    {
                        // If the block write was successful but the process
                        // is not complete then stay in download state
                        nextState = OAD_DOWNLOAD;
                    }
                    else if(status == OAD_DL_COMPLETE)
                    {
                        // Image download is complete, now wait for enable command
                        nextState = OAD_COMPLETE;
                    }
                    else
                    {
                        // An error has occured, reset and go back to idle
                        oadResetState();
                        nextState = OAD_IDLE;
                    }
                }
                else if (event == OAD_EXT_CTRL_WRITE_CMD)
                {
                    status = oadProcessExtControlCmd(oadWriteEvt->connHandle,
                                                        oadWriteEvt->pData,
                                                        oadWriteEvt->len);
                }
                else
                {
                    // An error has occured, reset and go back to idle
                    oadResetState();
                    nextState = OAD_IDLE;
                }
            }
            else if (state == OAD_COMPLETE)
            {
                if(event == OAD_EXT_CTRL_WRITE_CMD)
                {
                    status = oadProcessExtControlCmd(oadWriteEvt->connHandle,
                                                        oadWriteEvt->pData,
                                                        oadWriteEvt->len);
                }
                else
                {
                    // An error has occured, reset and go back to idle
                    oadResetState();
                    nextState = OAD_IDLE;
                }
            }


            // Free buffer.
            ICall_free(oadWriteEvt);
        }
        else
        {
            oadResetState();
            nextState = OAD_IDLE;

            status = OAD_NO_RESOURCES;
        }

        // Advance the state machine
        state = nextState;

        // Get the next Queue Element
        oadWriteEvt = Queue_get(hOadQ);
    }

    // Store the global status
    if(status == OAD_CCCD_NOT_ENABLED)
    {
        if(state == OAD_IDLE)
        {
            oadGlobalStatus = OAD_NOT_STARTED;
        }
        else if( state == OAD_DOWNLOAD ||
                 state == OAD_CONFIG   ||
                 state == OAD_CHECK_DL)
        {
            oadGlobalStatus = OAD_ALREADY_STARTED;
        }
    }
    else
    {
        oadGlobalStatus = status;
    }

    return (status);
}

/*********************************************************************
 * @fn      OAD_getBlockSize
 *
 * @brief   Gets the block size used during the OAD
 *          (will not be changed until first block is received)
 *
 * @param   None.
 *
 * @return  The block size used during the last OAD.
 */
uint16_t OAD_getBlockSize(void)
{
    return (oadBlkSize);
}

/*********************************************************************
 * @fn      OAD_cancel
 *
 * @brief   Cancel the OAD in progress and reset the state
 *
 * @param   None.
 *
 * @return  None.
 */
void OAD_cancel(void)
{
    oadResetState();
}

/*********************************************************************
 * @fn      OAD_isUsrAppValid
 *
 * @brief   Returns whether or not the user application is valid
 *
 * @param   None.
 *
 * @return  user app status - true if the user app hans't been erased.
 */
bool OAD_isUsrAppValid(void)
{
    return (userAppValid);
}

/*********************************************************************
 * @fn      OAD_close
 *
 * @brief   Close the OAD and its flash_interface, reset the state
 *
 * @param   None.
 *
 * @return  None.
 */
void OAD_close(void)
{
    // Reset the state machine
    oadResetState();

    // Close the flash interface
    flash_close();
}

/*********************************************************************
 * @fn      OAD_setBlockSize
 *
 * @brief   Sets the block size to use during the OAD transfer. This is
 *          based on the MTU size
 *
 * @param   blkSize The desired OAD block size
 *
 * @return  TRUE if the operation is successful
 */
uint8_t OAD_setBlockSize(uint16_t mtuSize)
{
    uint16_t tempMtuSz = mtuSize - OAD_ATT_OVERHEAD;
    uint8_t status = FALSE;

    if((tempMtuSz > OAD_DEFAULT_BLOCK_SIZE) && (tempMtuSz <= OAD_MAX_BLOCK_SIZE))
    {
        uint16_t tempBlkSz = (tempMtuSz / HAL_FLASH_WORD_SIZE) * HAL_FLASH_WORD_SIZE;
        if(tempBlkSz % HAL_FLASH_WORD_SIZE == 0)
        {
            oadBlkSize = tempBlkSz;
            oadImgBytesPerBlock = oadBlkSize - OAD_BLK_NUM_HDR_SZ;
            status = TRUE;
        }
    }

    return (status);
}

/*********************************************************************
 * PRIVATE FUNCTIONS
 */

/*********************************************************************
 * @fn      oadImgIdentifyWrite
 *
 * @brief   Process the Image Identify Write.  Determined if the image
 *          header identified here should or should not be downloaded by
 *          this application.
 *
 * @param   connHandle - connection message was received on
 * @param   pValue     - pointer to image header data
 *
 * @return  none
 */
uint8_t oadImgIdentifyWrite(uint16_t connHandle, uint8_t *pValue, uint16_t len,
                          uint16_t offset)
{
    uint8_t idStatus;
    uint8_t notifStat;

    // Find the number of blocks in the image header, round up if necessary
    numBlksInImgHdr = sizeof(imgHdr_t) / (oadImgBytesPerBlock)  +  \
                        (sizeof(imgHdr_t) % (oadImgBytesPerBlock) != 0);

    // Cast the pValue byte array to imgIdentifyPld_t
    imgIdentifyPld_t *idPld = (imgIdentifyPld_t *)(pValue);

    // Validate the ID
    idStatus = oadCheckImageID(idPld);

    // If image ID is accepted, set variables and pre-erase flash pages
    if(idStatus == OAD_SUCCESS)
    {

        if(!useExternalFlash)
        {
            imageAddress = 0;
            imagePage = 0;
            metaPage = 0;
        }
        else
        {
            imageAddress = oadFindExtFlImgAddr(idPld->imgType);
            imagePage = imageAddress / EFL_PAGE_SIZE;
            metaPage = oadFindExtFlMetaPage();
        }

        // Calculate total number of OAD blocks, round up if needed
        oadBlkTot = candidateImageLength / (oadImgBytesPerBlock);

        // If there is a remainder after division, round up
        if( 0 != (candidateImageLength % (oadImgBytesPerBlock)))
        {
            oadBlkTot += 1;
        }

        // Image has been accepted, start the inactivity timer
        Util_startClock(&oadActivityClk);
    }
    else
    {
        imgIDRetries--;
        if(imgIDRetries == 0)
        {
            idStatus = OAD_IMG_ID_TIMEOUT;
        }
    }

    // Send a response to the ImgID command
    notifStat  = oadSendNotification(connHandle, oadImgIdentifyConfig,
                                        OAD_IDX_IMG_IDENTIFY,
                                        &idStatus, OAD_IMAGE_ID_RSP_LEN);

    return ((notifStat == OAD_SUCCESS) ? idStatus : notifStat);
}

/*********************************************************************
 * @fn      oadResetState
 *
 * @brief   Reset OAD variables
 *
 * @param   none
 *
 * @return  none
 */
 static void oadResetState(void)
 {
    // Reset the state variable
    state = OAD_IDLE;
    nextState = OAD_IDLE;

    imgIDRetries = OAD_IMG_ID_RETRIES;

    // Get the current link state and check if connected
    if(TRUE != linkDB_State(activeOadCxnHandle , LINK_CONNECTED))
    {
        oadBlkSize = OAD_DEFAULT_BLOCK_SIZE;
        oadImgBytesPerBlock = oadBlkSize - OAD_BLK_NUM_HDR_SZ;
    }

    // Reset the variables used by the OAD state machine's event handlers
    oadBlkNum = 0;
    oadBlkTot = 0xFFFFFFFF;
    numBlksInImgHdr = 0;
    imageAddress = 0;
    candidateImageLength = 0;
    imagePage = 0;
    candidateImageType = OAD_IMG_TYPE_APP_ONLY;
    activeOadCxnHandle = NULL;

    // Remove the element from the head of the Queue
    oadTargetWrite_t *oadWriteEvt = Queue_get(hOadQ);

    // Clear out any remaining messages in the queue
    while (oadWriteEvt != (oadTargetWrite_t *)hOadQ)
    {
        // Free buffer.
        if(oadWriteEvt != NULL)
        {
            ICall_free(oadWriteEvt);
        }

        // Retrieve the next message in the Queue
        oadWriteEvt = Queue_get(hOadQ);
    }

    // Stop the inactivity timer if running
    if(Util_isActive(&oadActivityClk))
    {
        Util_stopClock(&oadActivityClk);
    }
 }

/*********************************************************************
 * @fn      oadImgBlockWrite
 *
 * @brief   Process the Image Block Write.
 *
 * @param   connHandle - connection message was received on
 * @param   pValue - pointer to data to be written
 *
 * @return  none
 */
uint8_t oadImgBlockWrite(uint16_t connHandle, uint8_t *pValue, uint8_t len)
{
    uint8_t status = OAD_SUCCESS;

    if(Util_isActive(&oadActivityClk))
    {
        // Any over the air interaction with the OAD profile will reset the clk
        Util_rescheduleClock(&oadActivityClk, oadStateTimeout);
        Util_startClock(&oadActivityClk);
    }

    // N.B. This must be left volatile.
    volatile uint32_t blkNum = BUILD_UINT32(pValue[0], pValue[1],
                                            pValue[2], pValue[3]);

    // Determine the expected block size, note the last block may be a partial
    uint8_t expectedBlkSz;
    if(blkNum == (oadBlkTot- 1))
    {
        if(candidateImageHeader.len % (oadImgBytesPerBlock) != 0)
        {
            expectedBlkSz = (candidateImageHeader.len % (oadImgBytesPerBlock)) + \
                                OAD_BLK_NUM_HDR_SZ;
        }
        else
        {
            expectedBlkSz = oadBlkSize;
        }
    }
    else
    {
        expectedBlkSz = oadBlkSize;
    }

    // Check that this is the expected block number, and the block size is right
    if ((oadBlkNum == blkNum) && (len == expectedBlkSz))
    {
        uint8_t offset = (oadImgBytesPerBlock)*blkNum;

        // Remaining bytes that are included in this packet that are not
        // part of the image header
        uint8_t nonHeaderBytes = 0;

        // Number of bytes in this packet that need to be copied to header
        uint8_t remainder = 0;

        // The destination in RAM to copy the payload info
        uint8_t *destAddr = (uint8_t *)(&candidateImageHeader) + offset;


        // Don't start store to flash until entire header is received
        if(blkNum < (numBlksInImgHdr - 1))
        {

            memcpy(destAddr, pValue+OAD_BLK_NUM_HDR_SZ,
                    (oadImgBytesPerBlock));
        }
        else if (blkNum == (numBlksInImgHdr - 1))
        {
            if((oadImgBytesPerBlock)  >= sizeof(imgHdr_t))
            {
                // if we can fit the entire header in a single block
                memcpy(destAddr,
                        pValue+OAD_BLK_NUM_HDR_SZ,
                        sizeof(imgHdr_t));

                nonHeaderBytes = (oadImgBytesPerBlock) - sizeof(imgHdr_t);
                remainder = sizeof(imgHdr_t);
            }
            else
            {
                // Find out how much of the block contains header data
                remainder = sizeof(imgHdr_t) % (oadImgBytesPerBlock);
                if(remainder == 0)
                {
                    remainder = oadImgBytesPerBlock;
                }

                nonHeaderBytes = (oadImgBytesPerBlock) - remainder;
                // if this block contains the last part of the header
                memcpy(destAddr,
                        pValue+OAD_BLK_NUM_HDR_SZ,
                        remainder);
            }

            status = oadValidateCandidateHdr((imgHdr_t * )&candidateImageHeader);
            if(status == OAD_SUCCESS)
            {
                // Calculate number of flash pages to pre-erase
                uint8_t numFlashPages = candidateImageHeader.len / HAL_FLASH_PAGE_SIZE;
                if(0 != (candidateImageHeader.len % HAL_FLASH_PAGE_SIZE))
                {
                    numFlashPages += 1;
                }

                // Pre-erase the correct amount of pages before starting OAD
                for(uint8_t page = imagePage; page < (imagePage + numFlashPages); ++page)
                {
                    uint8_t flashStat = eraseFlash(page);
                    if(flashStat == FLASH_FAILURE)
                    {
                        // If we fail to pre-erase, then halt the OAD process
                        status = OAD_FLASH_ERR;
                        break;
                    }

                }

                // at this point we have erased the user app
                if(!useExternalFlash)
                {
                    userAppValid = false;
                }

                // Write a OAD_BLOCK to Flash.
                status = writeFlashPg(imagePage, 0,
                                        (uint8_t * ) &candidateImageHeader,
                                        sizeof(imgHdr_t));

                // Cancel OAD due to flash program error
                if(FLASH_SUCCESS != status)
                {
                    return (OAD_FLASH_ERR);
                }

                // If there are non header (image data) bytes in this packet
                // write them to flash as well
                if(nonHeaderBytes)
                {
                    // Write a OAD_BLOCK to Flash.
                    status = writeFlashPg(imagePage,
                                            sizeof(imgHdr_t),
                                            (pValue+OAD_BLK_NUM_HDR_SZ+remainder),
                                            nonHeaderBytes);

                    // Cancel OAD due to flash program error
                    if(FLASH_SUCCESS != status)
                    {
                        return (OAD_FLASH_ERR);
                    }
                }
            }
            else
            {
                // Cancel OAD due to boundary error
                return (status);
            }
        }
        else
        {
            // Calculate address to write as (start of OAD range) + (offset into range)
            uint32_t blkStartAddr = (oadImgBytesPerBlock)*blkNum + imageAddress;

            uint8_t page = (blkStartAddr >> 12);
            uint32_t offset = (blkStartAddr & 0x00000FFF);

            // Write a OAD_BLOCK to Flash.
            status = writeFlashPg(page, offset, pValue+OAD_BLK_NUM_HDR_SZ,
                                  (len - OAD_BLK_NUM_HDR_SZ));

            // Cancel OAD due to flash program error
            if(FLASH_SUCCESS != status)
            {
                return (OAD_FLASH_ERR);
            }
        }

        // Increment received block count.
        oadBlkNum++;
    }
    else
    {
        // Overflow, abort OAD
        oadBlkNum = 0;

        return (OAD_BUFFER_OFL);
    }

    // Check if the OAD Image is complete.
    if (oadBlkNum == oadBlkTot)
    {
        // Run CRC check on new image.
        if (OAD_SUCCESS != oadCheckDL())
        {
            // CRC error
            return (OAD_CRC_ERR);

        }

        // Set copy status
        if( !useExternalFlash &&
            (candidateImageType == OAD_IMG_TYPE_STACK_ONLY))
        {
            uint8_t copyStatus = NEED_COPY;
            uint8_t flashStatus = OAD_SUCCESS;
            flashStatus = writeFlashPg(imagePage, IMG_COPY_STAT_OFFSET,
                                        &copyStatus,
                                        sizeof(uint8_t));
            if(flashStatus != FLASH_SUCCESS)
            {
                return (OAD_FLASH_ERR);
            }
        }

        if(useExternalFlash)
        {

            // Copy the metadata to the meta page
            imgHdr_t storedImgHdr;
            readFlashPg(imagePage, 0, (uint8_t *)&storedImgHdr,
                            OAD_IMG_HDR_LEN);

            // Populate ext imge info struct
            ExtImageInfo_t extFlMetaHdr;

            // ExtImgInfo and imgHdr are identical for the first
            // EFL_META_COPY_SZ bytes
            memcpy((uint8_t *)&extFlMetaHdr, (uint8_t *)&storedImgHdr,
                    EFL_META_COPY_SZ);


            uint8_t imgIDExtFl[] = OAD_EXTFL_ID_VAL;
            memcpy((uint8_t *)&extFlMetaHdr, imgIDExtFl, OAD_IMG_ID_LEN);

            extFlMetaHdr.extFlAddr = imageAddress;
            extFlMetaHdr.counter =  0x00000000;

            // Store the metadata
            uint8_t flashStatus = OAD_SUCCESS;
            flashStatus =  writeFlashPg(metaPage, 0,
                                            (uint8_t *)&extFlMetaHdr,
                                            sizeof(ExtImageInfo_t));

            if(flashStatus != FLASH_SUCCESS)
            {
                return (OAD_FLASH_ERR);
            }
        }

        // Indicate a successful download and CRC
        oadBlkNum = 0;
        return (OAD_DL_COMPLETE);
    }
    else
    {
        // Return and request the next block
        return (status);
    }
}

/*********************************************************************
 * @fn      oadProcessExtControlCmd
 *
 * @brief   Process an extended control command
 *
 * @param   connHandle - connection message was received on
 * @param   pData - pointer to the extended cmd payload
 *
 * @return  None
 */
uint8_t oadProcessExtControlCmd(uint16_t connHandle, uint8_t  *pData,
                                uint16_t len)
{
    uint8_t *pCmdRsp = NULL;
    uint8_t pRspPldlen;
    uint8_t status = OAD_SUCCESS;

    switch(EXT_CTRL_OP_CODE(pData))
    {
        case OAD_EXT_CTRL_GET_BLK_SZ:
        {
            // Determine the size of the payload
            pRspPldlen = sizeof(blockSizeRspPld_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            blockSizeRspPld_t *rsp = (blockSizeRspPld_t *)pCmdRsp;

            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_GET_BLK_SZ;
            rsp->oadBlkSz = oadBlkSize;

            break;
        }
        case OAD_EXT_CTRL_IMG_CNT:
        {
            pRspPldlen = sizeof(genericExtCtrlRsp_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            genericExtCtrlRsp_t *rsp = (genericExtCtrlRsp_t *)pCmdRsp;


            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_IMG_CNT;

            if(state == OAD_CONFIG)
            {
                rsp->status = OAD_SUCCESS;
            }
            else
            {
                rsp->status = OAD_NOT_STARTED;
            }

            break;
        }
        case OAD_EXT_CTRL_START_OAD:
        {
            if(state == OAD_CONFIG)
            {
                // Reset OAD variables
                oadBlkNum = 0;

                // Send the first block request to kick off the OAD
                oadGetNextBlockReq(connHandle, oadBlkNum, OAD_SUCCESS);

                // This is an exception case where a RSP is not sent
                return (OAD_SUCCESS);
            }
            else
            {
                // Cannot start an OAD before image ID
                pRspPldlen = sizeof(genericExtCtrlRsp_t);
                pCmdRsp = ICall_malloc(pRspPldlen);
                genericExtCtrlRsp_t *rsp = (genericExtCtrlRsp_t *)pCmdRsp;
                rsp->cmdID = OAD_EXT_CTRL_START_OAD;
                rsp->status = OAD_NOT_STARTED;
            }
            break;
        }
        case OAD_EXT_CTRL_ENABLE_IMG:
        {

            pRspPldlen = sizeof(genericExtCtrlRsp_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            genericExtCtrlRsp_t *rsp = (genericExtCtrlRsp_t *)pCmdRsp;

            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_ENABLE_IMG;

            // Setup the payload rsp
            rsp->status = OAD_SUCCESS;

            uint16_t bim_var = 0x0001;

            // An enable command with zero payload means the
            // last downloaded image should be enabled
            if(len == 1)
            {
                if (state == OAD_COMPLETE)
                {

                    if(candidateImageHeader.imgType <= OAD_IMG_TYPE_APP_STACK &&
                        candidateImageHeader.imgType > OAD_IMG_TYPE_PERSIST)
                    {
                        /*
                         * The bimVar is used to switch for on-chip
                         * We can safely enable the active image
                         * image since its boundary has already been validated
                         */
                        if(!useExternalFlash)
                        {
                            if(candidateImageHeader.imgType == OAD_IMG_TYPE_STACK_ONLY)
                            {
                                // Set BIM var to persist app
                                bim_var = 0x0001;
                            }
                            else if(candidateImageHeader.imgType == OAD_IMG_TYPE_APP_ONLY)
                            {
                                // Set BIM var to usr app
                                bim_var = 0x0101;
                            }
                        }
                        else
                        {
                            /*
                             * We can safely enable the active image
                             * image since its boundary has already been validated
                             */
                            ExtImageInfo_t extFlMetaHdr;
                            // Read in the meta header
                            readFlashPg(metaPage, 0, (uint8_t *)&extFlMetaHdr,
                                            sizeof(ExtImageInfo_t));

                            extFlMetaHdr.imgCpStat = NEED_COPY;
                            extFlMetaHdr.crcStat = CRC_VALID;
                            eraseFlash(metaPage);
                            rsp->status = writeFlashPg(metaPage,
                                                        0,
                                                        (uint8_t *)&extFlMetaHdr,
                                                        sizeof(ExtImageInfo_t));
                        }

                        // We're about to reset, close the flash interface
                        flash_close();

                        if (oadTargetWriteCB != NULL)
                        {
                            (*oadTargetWriteCB)(OAD_DL_COMPLETE_EVT, bim_var);
                        }
                    }
                    else
                    {
                        // Image type is not runnable
                        rsp->status = OAD_INCOMPATIBLE_IMAGE;
                    }
                }
                else
                {
                    rsp->status = OAD_DL_NOT_COMPLETE;
                }
            }
            else if (len == sizeof(extImgEnableReq_t))
            {
                if(useExternalFlash)
                {
                    // Extended img enable can happen at any time and is only
                    // supported on external flash
                    extImgEnableReq_t *enablePld = (extImgEnableReq_t *) pData;
                    bool imgFound = false;

                    // Only app, stack, or merged images allowed
                    if(enablePld->imgType <= OAD_IMG_TYPE_APP_STACK &&
                        enablePld->imgType > OAD_IMG_TYPE_PERSIST)
                    {
                        /*
                         * First find the meta page of the image to enable
                         * This search is greedy, it will return the first
                         * meta page that matches the requested params
                         */
                        ExtImageInfo_t extFlMetaHdr;
                        for(uint8_t curPg = 0; curPg < EFL_MAX_META; ++curPg)
                        {
                            // Read in the meta header
                            readFlashPg(curPg, 0, (uint8_t *)&extFlMetaHdr,
                                            sizeof(ExtImageInfo_t));

                            // Check to see if image matches the requested params
                            if(extFlMetaHdr.imgType == enablePld->imgType &&
                                extFlMetaHdr.imgNo == enablePld->imgNo    &&
                                extFlMetaHdr.techType == enablePld->techType)
                            {

                                // Copy the candidate img header into the buffer
                                readFlashPg(extFlMetaHdr.extFlAddr, 0,
                                            (uint8_t *)&candidateImageHeader,
                                            sizeof(imgHdr_t));

                                uint8_t status = oadValidateCandidateHdr((imgHdr_t * )&candidateImageHeader);

                                if(status == OAD_SUCCESS)
                                {
                                    extFlMetaHdr.imgCpStat = NEED_COPY;
                                    extFlMetaHdr.crcStat = CRC_VALID;
                                    eraseFlash(curPg);

                                    // Write the meta page back
                                    rsp->status = writeFlashPg(curPg,
                                                                0,
                                                                (uint8_t *)&extFlMetaHdr,
                                                                sizeof(ExtImageInfo_t));

                                    imgFound = true;
                                    break;
                                }
                            }
                        }
                    }

                    if(!imgFound)
                    {
                        // We didn't find a matching image in ext fl
                        rsp->status = OAD_INCOMPATIBLE_IMAGE;
                    }
                    else
                    {
                        // We're about to reset, close the flash interface
                        flash_close();

                        if (oadTargetWriteCB != NULL)
                        {
                            (*oadTargetWriteCB)(OAD_DL_COMPLETE_EVT, bim_var);
                        }
                    }
                }
                else
                {
                    // Extended imgEnable is not allowed on-chip
                    rsp->status = OAD_EXT_CTRL_CMD_NOT_SUPPORTED;
                }
            }

            break;
        }
        case OAD_EXT_CTRL_CANCEL_OAD:
        {

            pRspPldlen = sizeof(genericExtCtrlRsp_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            genericExtCtrlRsp_t *rsp = (genericExtCtrlRsp_t *)pCmdRsp;

            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_CANCEL_OAD;

            if(state != OAD_IDLE)
            {
                // Setup the payload rsp
                rsp->status = OAD_SUCCESS;
                // Reset the OAD state machine
                oadResetState();
            }
            else
            {
                // Cannot cancel inactive OAD
                rsp->status = OAD_NOT_STARTED;
            }

            break;
        }
        case OAD_EXT_CTRL_DISABLE_BLK_NOTIF:
        {
            // Stop sending block request notifications
            blkReqActive = false;

            pRspPldlen = sizeof(genericExtCtrlRsp_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            genericExtCtrlRsp_t *rsp = (genericExtCtrlRsp_t *)pCmdRsp;

            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_DISABLE_BLK_NOTIF;

            // Setup the payload rsp
            rsp->status = OAD_SUCCESS;

            break;
        }
        case OAD_EXT_CTRL_GET_SW_VER:
        {
            uint8_t swVer[4];

            pRspPldlen = sizeof(swVersionPld_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            swVersionPld_t *rsp = (swVersionPld_t *)pCmdRsp;

            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_GET_SW_VER;

            // This API will get app and stack SW version and combine
            OAD_getSWVersion(swVer, OAD_SW_VER_LEN);

            // Copy combined version string into the response payload
            memcpy(rsp->swVer, &swVer, OAD_SW_VER_LEN);

            break;
        }
        case OAD_EXT_CTRL_GET_IMG_STAT:
        {
            pRspPldlen = sizeof(genericExtCtrlRsp_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            genericExtCtrlRsp_t *rsp = (genericExtCtrlRsp_t *)pCmdRsp;

            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_GET_IMG_STAT;

            // Setup the payload rsp
            rsp->status = oadGlobalStatus;

            break;
        }
        case OAD_EXT_CTRL_GET_PROF_VER:
        {
            pRspPldlen = sizeof(profileVerRsp_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            profileVerRsp_t *rsp = (profileVerRsp_t *)pCmdRsp;

            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_GET_PROF_VER;

            // Setup the payload rsp
            rsp->profVer = OAD_PROFILE_VERSION;

            break;
        }
        case OAD_EXT_CTRL_GET_DEV_TYPE:
        {
            pRspPldlen = sizeof(deviceTypeRspPld_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            deviceTypeRspPld_t *rsp = (deviceTypeRspPld_t *)pCmdRsp;

            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_GET_DEV_TYPE;

            rsp->chipType = (uint8_t )ChipInfo_GetChipType();
            rsp->chipFamily = (uint8_t )ChipInfo_GetChipFamily();
            rsp->hardwareRev = (uint8_t )ChipInfo_GetHwRevision();
            rsp->rsvd = 0xFF;

            break;
        }
        case OAD_EXT_CTRL_GET_IMG_INFO:
        {

            if(pData[1] == OAD_IMG_INFO_ONCHIP)
            {
                // Read the active app's image header
                uint8_t  appImageHdrPage = oadFindCurrentImageHdr();
                uint32_t appImageHeaderAddr = FLASH_ADDRESS(appImageHdrPage, 0);
                imgHdr_t *currentImageHeader = (imgHdr_t *)(appImageHeaderAddr);

                // If on-chip and the user app is erased, use persist app
                if(!useExternalFlash && (appImageHdrPage == OAD_IMG_PG_INVALID))
                {
                    currentImageHeader = (imgHdr_t *)&_imgHdr;
                }

                pRspPldlen = sizeof(imageInfoRspPld_t);

                // Allocate memory for the ext ctrl rsp message
                pCmdRsp = ICall_malloc(pRspPldlen);

                imageInfoRspPld_t *rsp = (imageInfoRspPld_t *)pCmdRsp;

                // Pack up the payload
                rsp->cmdID = OAD_EXT_CTRL_GET_IMG_INFO;
                // There is only image for on-chip
                rsp->numImages = 0x01;
                rsp->imgCpStat = currentImageHeader->imgCpStat;
                rsp->crcStat = currentImageHeader->crcStat;
                rsp->imgType = currentImageHeader->imgType;
                rsp->imgNo = currentImageHeader->imgNo;
            }
            else
            {
                ExtImageInfo_t extFlMetaHdr;

                uint8_t numImages = 0;
                // Create a buffer for all the possible image info
                imageInfo_t extFlInfo[EFL_MAX_META];

                for(uint8_t curPg = 0; curPg < EFL_MAX_META; ++curPg)
                {
                    uint8_t imgIdExpected[OAD_IMG_ID_LEN] = OAD_EXTFL_ID_VAL;

                    // Read in the meta header
                    readFlashPg(curPg, 0, (uint8_t *)&extFlMetaHdr,
                                    sizeof(ExtImageInfo_t));


                    if(memcmp(&extFlMetaHdr.imgID, &imgIdExpected, OAD_IMG_ID_LEN) == 0)
                    {
                        // copy the image info into the array
                        memcpy(&extFlInfo[numImages], &extFlMetaHdr.imgCpStat, sizeof(imageInfo_t));

                        // We found an image, add it to the meta array
                        numImages++;
                    }
                }

                pRspPldlen = (numImages*sizeof(imageInfo_t) + 2);

                pCmdRsp = ICall_malloc(pRspPldlen);

                imageInfoRspPld_t *rsp = (imageInfoRspPld_t *)pCmdRsp;

                rsp->cmdID = OAD_EXT_CTRL_GET_IMG_INFO;
                // There is only image for on-chip
                rsp->numImages = numImages;

                memcpy(&(rsp->imgCpStat), extFlInfo, (pRspPldlen - 2));


            }
            break;
        }
        case OAD_EXT_CTRL_ERASE_BONDS:
        {
            // Ext control commands for erasing bonds
            GAPBondMgr_SetParameter(GAPBOND_ERASE_ALLBONDS, 0, NULL);

            pRspPldlen = sizeof(genericExtCtrlRsp_t);

            // Allocate memory for the ext ctrl rsp message
            pCmdRsp = ICall_malloc(pRspPldlen);

            genericExtCtrlRsp_t *rsp = (genericExtCtrlRsp_t *)pCmdRsp;

            rsp->status = OAD_SUCCESS;

            // Pack up the payload
            rsp->cmdID = OAD_EXT_CTRL_ERASE_BONDS;
            break;
        }
        default:
        {
            pRspPldlen = sizeof(uint8_t);
            pCmdRsp = ICall_malloc(pRspPldlen);
            pCmdRsp[0] = OAD_EXT_CTRL_CMD_NOT_SUPPORTED;
            break;
        }
    }

    if(pCmdRsp != NULL)
    {
        // Send out the populated command structure
         status = oadSendNotification(activeOadCxnHandle, oadExtCtrlConfig,
                                        OAD_IDX_EXT_CTRL, pCmdRsp, pRspPldlen);
        ICall_free(pCmdRsp);

    }
    else
    {
        status = OAD_NO_RESOURCES;
    }

    return (status);
}

/*********************************************************************
 * @fn      oadEnqueueMsg
 *
 * @brief   Create Queue element and add it to the OAD message
 *
 * @param   event      - event type:
 *                       OAD_WRITE_IDENTIFY_REQ
 *                       OAD_WRITE_BLOCK_REQ
 *                       OAD_IMAGE_COMPLETE
 *                       OAD_EXT_CTRL_WRITE_CMD
 * @param   connHandle - the connection Handle this request is from.
 * @param   pData      - pointer to data for processing and/or storing
 * @param   len        - Lenght of payload in pData
 *
 * @return  status - whether or not the operation succeeded.
 */
uint8_t oadEnqueueMsg(oadEvent_e event, uint16_t connHandle,
                           uint8_t *pData, uint16_t len)
{
    uint8_t status = OAD_SUCCESS;

    oadTargetWrite_t *oadWriteEvt = ICall_malloc( sizeof(oadTargetWrite_t) + \
                                             sizeof(uint8_t) * len);

    if ( oadWriteEvt != NULL )
    {
        oadWriteEvt->event = event;
        oadWriteEvt->connHandle = connHandle;

        oadWriteEvt->pData = (uint8_t *)(&oadWriteEvt->pData + 1);
        oadWriteEvt->len = len;
        memcpy(oadWriteEvt->pData, pData, len);

        Queue_put(hOadQ, (Queue_Elem *)oadWriteEvt);
    }
    else
    {
        status = OAD_NO_RESOURCES;
    }

    return (status);
}

/*********************************************************************
 * @fn      oadFindCurrentImageHdr
 *
 * @brief   Search internal flash for the currently running image header
 *
 * @return  headerPg - Page of the current image's header
 */
static uint8_t oadFindCurrentImageHdr(void)
{
    uint8_t headerPage = OAD_IMG_PG_INVALID;
    // When in the persist app, we need to search for the header
    // Start at page 0 and search up to the stack boundary
    uint8_t stackStartPg =  FLASH_PAGE((uint32_t)(stackImageHeader));

    for(uint8_t page = 0; page < stackStartPg; ++page)
    {
        uint8_t imgIdExpected[OAD_IMG_ID_LEN] = OAD_IMG_ID_VAL;
        uint8_t imgIdReceived[OAD_IMG_ID_LEN];
        uint32_t *pgAddr = (uint32_t *)FLASH_ADDRESS(page, 0);
        memcpy(imgIdReceived, pgAddr, OAD_IMG_ID_LEN);

        if(memcmp(&imgIdExpected, &imgIdReceived, OAD_IMG_ID_LEN) == 0)
        {
            uint8_t imgType;
            pgAddr = (uint32_t *)FLASH_ADDRESS(page, IMG_TYPE_OFFSET);
            memcpy(&imgType, pgAddr, sizeof(imgType));
            if(imgType == OAD_IMG_TYPE_APP ||
               imgType == OAD_IMG_TYPE_APP_STACK ||
               imgType == OAD_IMG_TYPE_PERSIST)
            {
                // If the ids match, we found the header, break loop
                headerPage = page;
                break;
            }
        }
    }

    return (headerPage);
}

/*********************************************************************
 * @fn      oadValidateCandidateHdr
 *
 * @brief   Validate the header of the incoming image
 *
 * @param   receivedHeader - pointer to the candidate image's header
 *
 * @return  Status - OAD_SUCCESS if segment valid or not a segment
                     OAD_INCOMPATIBLE_IMAGE - if the boundary check fails
 */
static uint8_t oadValidateCandidateHdr(imgHdr_t *receivedHeader)
{
    uint8_t status = OAD_SUCCESS;

    // Read the active app's image header
    uint8_t  appImageHdrPage = oadFindCurrentImageHdr();
    uint32_t appImageHeaderAddr = FLASH_ADDRESS(appImageHdrPage, 0);
    imgHdr_t *currentImageHeader = (imgHdr_t *)(appImageHeaderAddr);

    // If on-chip and the user app is erased, use persist app
    if(!useExternalFlash && (appImageHdrPage == OAD_IMG_PG_INVALID))
    {
        currentImageHeader = (imgHdr_t *)&_imgHdr;
    }

    /*
     * For on-chip OAD the tech type must match the currently running one
     * for off-chip OAD the tech type must match with the exception of
     * merged images which can be of a different tech type
     */
    if(!(useExternalFlash && (receivedHeader->imgType == OAD_IMG_TYPE_APP_STACK)))
    {
        if( currentImageHeader->techType !=  receivedHeader->techType          ||
            currentImageHeader->wirelessTech1 != receivedHeader->wirelessTech1 ||
            currentImageHeader->wirelessTech2 != receivedHeader->wirelessTech2)
        {
            status = OAD_INCOMPATIBLE_IMAGE;
        }
    }

    // Check that the incoming image is page aligned
    if(receivedHeader->startAddr & (HAL_FLASH_PAGE_SIZE - 1) != 0)
    {
        status = OAD_INCOMPATIBLE_IMAGE;
    }

    if(receivedHeader->imgType == OAD_IMG_TYPE_APP_ONLY)
    {
        uint32_t appEndAddr = receivedHeader->startAddr + receivedHeader->len;

        // Ensure that the candidate image would not overwrite the stack
        if(appEndAddr > stackImageHeader->startAddr)
        {
            status = OAD_IMAGE_TOO_BIG;
        }

        // Ensure that the stack RAM boundary is not violated
        if(receivedHeader->ram0EndAddr >= stackImageHeader->ram0StartAddr)
        {
            status = OAD_INCOMPATIBLE_IMAGE;
        }

        // Be sure that the the app image's RAM doesn't dip into RAM rsvd for ROM
        if(receivedHeader->ram0StartAddr < currentImageHeader->ram0StartAddr)
        {
            status = OAD_INCOMPATIBLE_IMAGE;
        }
    }
    else if (receivedHeader->imgType == OAD_IMG_TYPE_STACK_ONLY)
    {
        uint32_t nvSize = HAL_FLASH_PAGE_SIZE * OAD_NUM_NV_PGS;
        /*
         * In on-chip OAD, stack only OAD requires the app to be udpated as well
         * so the currently app will be wiped out, but the new stack cannot
         * grow into the existing stack (pre-copy in BIM)
         *
         * In off-chip OAD the stack can grow so long as it doesn't overwrite
         * the last address of the current application
         */
        if(!useExternalFlash)
        {
            if(receivedHeader->len > stackImageHeader->startAddr)
            {
                status = OAD_IMAGE_TOO_BIG;
            }

            // Check that the stack will not overwrite the persistent app
            if(receivedHeader->imgEndAddr + nvSize > _imgHdr.startAddr)
            {
                status = OAD_IMAGE_TOO_BIG;
            }
        }
        else
        {
            if(currentImageHeader->imgType != OAD_IMG_TYPE_APP_STACK)
            {
                /*
                 * If the current image is not of merged type then its
                 * end addr is the end of the app. The stack cannot overwrite
                 * the app
                 */
                if(receivedHeader->startAddr <= currentImageHeader->imgEndAddr)
                {
                    status = OAD_IMAGE_TOO_BIG;
                }

                // Check that the stack will not overwrite the BIM
                if(receivedHeader->imgEndAddr + nvSize > BIM_START)
                {
                    status = OAD_IMAGE_TOO_BIG;
                }
            }
            else
            {
                /*
                 * If the current image is of merged type then its not possible
                 * to know exactly where the app ends, just make sure
                 * ICALL_STACK0 isn't violated.
                 */
                if(receivedHeader->startAddr < stackImageHeader->stackStartAddr)
                {
                    status = OAD_IMAGE_TOO_BIG;
                }
            }
        }

        /*
         *  In on-chip use cases this will check that the persistent app's
         *  RAM boundary is not violated
         *  in offchip use cases this will check the user app's RAM boundary
         *  is not violated
         */
        if(receivedHeader->ram0StartAddr <= _imgHdr.ram0EndAddr)
        {
            status = OAD_INCOMPATIBLE_IMAGE;
        }
    }

    // Merged image types and images not targed to run on CC26xx are not checked
    return (status);
}

/*********************************************************************
 * @fn      oadCheckImageID
 *
 * @brief   Check image identify header (determines OAD start)
 *
 * @param   idPld - pointer to imageID payload
 *
 * @return  headerValid - SUCCESS or fail code
 */
static uint8_t oadCheckImageID(imgIdentifyPld_t *idPld)
{
    uint8_t status = OAD_SUCCESS;
    uint8_t imgIDExpected[] = OAD_IMG_ID_VAL;

    // Read the active app's image header
    uint8_t  appImageHdrPage = oadFindCurrentImageHdr();
    uint32_t appImageHeaderAddr = FLASH_ADDRESS(appImageHdrPage, 0);
    imgHdr_t *currentImageHeader = (imgHdr_t *)(appImageHeaderAddr);

    if(appImageHdrPage == OAD_IMG_PG_INVALID)
    {
        currentImageHeader = (imgHdr_t *)&_imgHdr;
    }

    // If on-chip and the user app is erased, use persist app
    if(!useExternalFlash && (appImageHdrPage == OAD_IMG_PG_INVALID))
    {
        currentImageHeader = (imgHdr_t *)&_imgHdr;
    }

    // Validate the Image header preamble
    if(memcmp(&imgIDExpected, idPld->imgID, OAD_IMG_ID_LEN))
    {
        status = OAD_INVALID_FILE;
    }

    // Ensure the image is built with a compatible BIM and META
    if(idPld->bimVer  != currentImageHeader->bimVer  ||
        idPld->metaVer != currentImageHeader->metaVer )
    {
        status = OAD_INCOMPATIBLE_FILE;
    }

    // Check that there is space to store the image
    // Host and USR images are not checked for size
    if(idPld->imgType == OAD_IMG_TYPE_APP_ONLY)
    {
        /*
         * Ensure that the propsed image doesn't run into stack start
         * Note this just checks the size of the image, actual start addr will
         * be checked later when the segement comes along
         */
        if(idPld->len > (stackImageHeader->stackStartAddr - 1))
        {
            status = OAD_IMAGE_TOO_BIG;
        }
    }
    else if (idPld->imgType == OAD_IMG_TYPE_STACK_ONLY)
    {
        if(!useExternalFlash)
        {
            /*
             * On-chip OAD, stack only OAD must be followed by app only oad
             * (user application is erased), thus we must only check that
             * the proposed stack can fit within the space between the existing
             * scratch space (usr app region)
             */
            if(idPld->len > (stackImageHeader->stackStartAddr - 1))
            {
              status = OAD_IMAGE_TOO_BIG;
            }
        }
        else
        {
            /*
             * Off-chip OAD, stack only can be done without changing the
             * existing application, must fit between app end and BIM_START
             */
            if(currentImageHeader->imgType != OAD_IMG_TYPE_APP_STACK)
            {
                if((idPld->len - 1) >= (BIM_START - currentImageHeader->imgEndAddr - 1))
                {
                  status = OAD_IMAGE_TOO_BIG;
                }
            }
            else
            {
                if((idPld->len - 1) >= (BIM_START - stackImageHeader->stackStartAddr - 1))
                {
                  status = OAD_IMAGE_TOO_BIG;
                }
            }
        }
    }
    else if (idPld->imgType == OAD_IMG_TYPE_APP_STACK)
    {
        if(!useExternalFlash)
        {
            // On-chip OAD doesn't support App + Stack merged images
            status = OAD_INCOMPATIBLE_IMAGE;
        }
        else
        {
            // Off-chip OAD
            if((idPld->len - 1) > (BIM_START - 1))
            {
                status = OAD_IMAGE_TOO_BIG;
            }
        }
    }
    else if (idPld->imgType == OAD_IMG_TYPE_PERSIST     ||
             idPld->imgType == OAD_IMG_TYPE_BIM         ||
             idPld->imgType == OAD_IMG_TYPE_BLE_FACTORY ||
             idPld->imgType >= OAD_IMG_TYPE_RSVD_BEGIN)
    {
        // Persistent app, BIM, factory are not currently upgradeable
        // Image type must also not be in the reserved range
        status = OAD_INCOMPATIBLE_IMAGE;
    }

    if(status == OAD_SUCCESS)
    {
        // If we are about to accept the image, store the image data
        candidateImageType = idPld->imgType;
        candidateImageLength = idPld->len;

    }
    return (status);
}

/*********************************************************************
 * @fn      oadGetNextBlockReq
 *
 * @brief   Process the Request for next image block.
 *
 * @param   connHandle - connection message was received on
 * @param   blkNum - block number to request from OAD Manager.
 *
 * @return  None
 */
static void oadGetNextBlockReq(uint16_t connHandle, uint32_t blkNum,
                                uint8_t status)
{
    if(blkReqActive)
    {
        blockReqPld_t blkReqPld;

        blkReqPld.cmdID = OAD_EXT_CTRL_BLK_RSP_NOTIF;
        blkReqPld.prevBlkStat = status;
        blkReqPld.requestedBlk = blkNum;

        oadSendNotification(connHandle, oadExtCtrlConfig, OAD_IDX_EXT_CTRL,
                                (uint8_t *)&blkReqPld, sizeof(blockReqPld_t));
    }
}

/*********************************************************************
 * @fn      oadSendNotification
 *
 * @brief   Send GATT notification command from OAD profile
 * @param   connHandle - connection message was received on.
 * @param   status     - SUCCESS or reject reason
 *
 * @return  None.
 */
static uint8_t oadSendNotification(uint16_t connHandle, gattCharCfg_t *charCfg,
                                  uint8_t charIdx, uint8_t *pData, uint8_t len)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, charCfg);

    uint8_t status = OAD_SUCCESS;

    // If notifications enabled
    if (value & GATT_CLIENT_CFG_NOTIFY)
    {
        attHandleValueNoti_t noti;

        noti.pValue = GATT_bm_alloc(connHandle, ATT_HANDLE_VALUE_NOTI,
                                    len, NULL);

        if (noti.pValue != NULL)
        {
            gattAttribute_t *pAttr;

            pAttr= GATTServApp_FindAttr(oadAttrTbl, GATT_NUM_ATTRS(oadAttrTbl),
                                      oadCharVals+charIdx);

            noti.handle = pAttr->handle;
            noti.len = len;

            memcpy(noti.pValue, pData, len);

            if (GATT_Notification(connHandle, &noti, FALSE) != SUCCESS)
            {
                // The stack will free the memory for us if the
                // the notification is successful, otherwise we have
                // to free the memory manually
                GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
                status = OAD_NO_RESOURCES;
            }
            else
            {
                // If we cannot send the noti, notify the app
                status = OAD_SUCCESS;
            }
        }
        else
        {
            // Notify app, out of memory
            status = OAD_NO_RESOURCES;
        }
    }
    else
    {
        // Notifications are required for OAD, if they are not enabled
        // the process cannot continue
        status = OAD_CCCD_NOT_ENABLED;
    }

    return (status);
}

/*********************************************************************
 * @fn      oadFindExtFlImgAddr
 *
 * @brief   Find a location in external flash for the current image's
 *          metadata
 *
 * @param   metaPage - meta page in external flash
 *
 * @return  None.
 */
 static uint8_t oadFindExtFlMetaPage(void)
 {
    // Variable to store the least recently used metadata page
    uint8_t lruMetaPg = 0xFF;

    uint8_t metaPage = 0xFF;

    // Store lowest counter variable
    uint32_t lowestCounter = 0xFFFFFFFF;

    for(uint8_t curPg = EFL_NUM_FACT_IMAGES; curPg < EFL_MAX_META; ++curPg)
    {
        // Buffer to hold imgID data
        uint8_t readID[8];

        uint8_t hdrID[] = OAD_EFL_MAGIC;

        // Read imageID into buffer
        readFlashPg(curPg, 0, readID, OAD_IMG_ID_LEN);

        if(0 != memcmp(readID, hdrID, OAD_IMG_ID_LEN))
        {
            // We have found an empty pg
            metaPage = curPg;

            // Erase the meta page.
            // The image pages will be erased later
            eraseFlash(metaPage);
            return (metaPage);
        }
        else
        {
            uint32_t counter;
            ExtImageInfo_t extFlMetaHdr;
            // Read in the meta header
            readFlashPg(curPg, 0, (uint8_t *)&extFlMetaHdr,
                            sizeof(ExtImageInfo_t));

            counter = extFlMetaHdr.counter;

            // If this counter is the least recently used
            if(counter < lowestCounter)
            {
                // Setup LRU values
                lowestCounter = counter;
                lruMetaPg = curPg;
            }
        }

    }

    // If we didn't find an empty pg, use the LRU
    if(lruMetaPg != 0xFF)
    {
        metaPage = lruMetaPg;
    }

    // Erase the meta page.
    // The image pages will be erased later
    eraseFlash(metaPage);

    return (metaPage);
 }

static uint32_t oadFindExtFlImgAddr(uint8_t imgType)
{
    uint32_t imgAddr = 0;
    if(imgType == OAD_IMG_TYPE_APP_ONLY)
    {
        imgAddr = EFL_IMG_SPACE_START;
    }
    else
    {
        imgAddr = EFL_IMG_SPACE_START + EFL_APP_IMG_SZ;
    }
    return (imgAddr);
}

/*********************************************************************
 * @fn      oadCheckDL
 *
 * @brief   Check validity of the downloaded image.
 *
 * @param   None.
 *
 * @return  TRUE or FALSE for image valid.
 */
static uint8_t oadCheckDL(void)
{
    uint32_t crcFromHdr;
    uint32_t crcCalculated;
    uint8_t crcStatus;


    uint8_t status = OAD_SUCCESS;

    // Read in the CRC
    readFlashPg(imagePage, CRC_OFFSET, (uint8_t *)(&crcFromHdr),
                sizeof(uint32_t));

    // Read in the image info word
    readFlashPg(imagePage, CRC_STAT_OFFSET, &crcStatus,
                sizeof(uint8_t));

    // If for some reason the header shows the CRC is invalid reject the image now
    if (crcStatus == CRC_INVALID)
    {
        return (OAD_CRC_ERR);
    }

    // Calculate CRC of downloaded image.
    crcCalculated = crcCalc(imagePage, 0, useExternalFlash);

    if (crcCalculated == crcFromHdr)
    {
        // Set CRC stat to valid
        crcStatus = CRC_VALID;

        // Only write to the CRC flag if using internal flash
        if(!useExternalFlash)
        {
            // Write CRC status back to flash
            writeFlashPg(imagePage, CRC_STAT_OFFSET, &crcStatus,
                            sizeof(uint8_t));
        }
        status = OAD_SUCCESS;

    }
    else
    {
        status = OAD_CRC_ERR;
    }

    return (status);
}

/*********************************************************************
 * Callback Functions - These run in the BLE-Stack context!
 *********************************************************************/

/*********************************************************************
 * @fn      oadReadAttrCB
 *
 * @brief   Read an attribute.
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr      - pointer to attribute
 * @param   pValue     - pointer to data to be read
 * @param   pLen       - length of data to be read
 * @param   offset     - offset of the first octet to be read
 * @param   maxLen     - maximum length of data to be read
 * @param   method     - type of read message
 *
 * @return  Reads not supported, returns ATT_ERR_INVALID_HANDLE
 */
static bStatus_t oadReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                               uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                               uint16_t maxLen, uint8_t method)
{
    bStatus_t status;

    // Make sure it's not a blob operation (no attributes in the profile are long)
    if (offset > 0)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }
    else
    {
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }

    return (status);
}

/*********************************************************************
 * @fn      oadWriteAttrCB
 *
 * @brief   Validate and Write attribute data
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr      - pointer to attribute
 * @param   pValue     - pointer to data to be written
 * @param   len        - length of data
 * @param   offset     - offset of the first octet to be written
 * @param   method     - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t oadWriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                uint8_t *pValue, uint16_t len, uint16_t offset,
                                uint8_t method)
{
    bStatus_t status = SUCCESS;

    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        if (uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            // Process a CCC write request.
            status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue,
                                                    len, offset,
                                                    GATT_CLIENT_CFG_NOTIFY);
        }
        else
        {
            status = ATT_ERR_ATTR_NOT_FOUND; // Should never get here!
        }
    }
    else
    {
        // 128-bit UUID
        if (!memcmp(pAttr->type.uuid, oadCharUUID[OAD_IDX_IMG_IDENTIFY],
                    ATT_UUID_SIZE))
        {
            /* OAD manager has sent new image information.
             * Read and decide whether to accept or reject an OAD
             * of this new application image.
             */

            // Notify Application
            if (oadTargetWriteCB != NULL)
            {
                // Add the message to the Queue for processing
                uint8_t stat = oadEnqueueMsg(OAD_WRITE_IDENTIFY_REQ, connHandle,
                                                pValue, len);
                if(stat == OAD_NO_RESOURCES)
                {
                    oadResetState();
                }
                // Notify the application that OAD needs to service its Queue
                (*oadTargetWriteCB)(OAD_QUEUE_EVT, NULL);
            }
        }
        else if (!memcmp(pAttr->type.uuid, oadCharUUID[OAD_IDX_IMG_BLOCK],
                         ATT_UUID_SIZE))
        {
            /* OAD is ongoing.
             * the OAD manager has sent a block from the new image.
             */

            // Notify the application.
            if (oadTargetWriteCB != NULL)
            {
                // Add the message to the Queue for processing
                uint8_t stat = oadEnqueueMsg(OAD_WRITE_BLOCK_REQ, connHandle,
                                                pValue, len);
                if(stat == OAD_NO_RESOURCES)
                {
                    oadResetState();
                }
                // Notify the application that OAD needs to service its Queue
                (*oadTargetWriteCB)(OAD_QUEUE_EVT, NULL);
            }
        }
        else if (!memcmp(pAttr->type.uuid, oadCharUUID[OAD_IDX_EXT_CTRL],
                         ATT_UUID_SIZE))
        {
            // Notify the application.
            if (oadTargetWriteCB != NULL)
            {
                // Add the message to the Queue for processing
                uint8_t stat = oadEnqueueMsg(OAD_EXT_CTRL_WRITE_CMD, connHandle,
                                                pValue, len);
                if(stat == OAD_NO_RESOURCES)
                {
                    oadResetState();
                }

                // Notify the application that OAD needs to service its Queue
                (*oadTargetWriteCB)(OAD_QUEUE_EVT, NULL);
            }
        }
        else
        {
            status = ATT_ERR_ATTR_NOT_FOUND; // Should never get here!
        }
    }

    // Setup the active OAD connection handle
    if(status = SUCCESS && activeOadCxnHandle == NULL)
    {
        activeOadCxnHandle = connHandle;
    }

    return (status);
}

/*********************************************************************
 * SWI Functions - These functions run at higher priority than any task
 *********************************************************************/

/*********************************************************************
 * @fn      oadInactivityTimeout
 *
 * @brief   This function is triggered when the OAD clock expires due
 *          to inactivity
 *
 * @param   param - The event to post.
 *
 * @return  TRUE or FALSE for image valid.
 */
static void oadInactivityTimeout(UArg param)
{
    // Notify the application.
    if (oadTargetWriteCB != NULL)
    {
        // Add the message to the Queue for processing
        uint8_t stat = oadEnqueueMsg(OAD_TIMEOUT, NULL, NULL, 0);

        // Reset the state machine
        if(stat == OAD_NO_RESOURCES)
        {
            oadResetState();
        }

        // Notify the application that OAD needs to service its Queue
        (*oadTargetWriteCB)(OAD_QUEUE_EVT, NULL);
    }
}

/*********************************************************************
*********************************************************************/

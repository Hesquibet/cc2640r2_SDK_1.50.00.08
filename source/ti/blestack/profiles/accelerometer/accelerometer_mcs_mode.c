/******************************************************************************

 @file       accelerometer_mcs_mode.c

 @brief Accelerometer Profile

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2009-2017, Texas Instruments Incorporated
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
#include "bcomdef.h"
#include "osal.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"

#include "accelerometer_mcs_mode.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED        4

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Accelerometer Service UUID
CONST uint8 accServUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(ACCEL_SERVICE_UUID), HI_UINT16(ACCEL_SERVICE_UUID)
};

// Accelerometer Enabler UUID
CONST uint8 accDataUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(ACCEL_DATA_UUID), HI_UINT16(ACCEL_DATA_UUID)
};


/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Connection the characteristic configuration was received on
uint16 charCfgConnHandle = 0x00;

/*********************************************************************
 * Profile Attributes - variables
 */

// Accelerometer Service attribute
static CONST gattAttrType_t accelService = { ATT_BT_UUID_SIZE, accServUUID };


// Accel Coordinate Characteristic Properties
static uint8 accelDataCharProps = GATT_PROP_NOTIFY;

// Accel Coordinate Characteristic
static int8 accelData[12] = { 0, 0, 0, 0,
                              0, 0, 0, 0,
                              0, 0, 0, 0, };

// Accel Coordinate Characteristic Config
static uint16 accelConfigData = GATT_CLIENT_CFG_NOTIFY;

/*********************************************************************
 * Profile Attributes - Table
 */
static gattAttribute_t accelAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
  // Accelerometer Service
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                   /* permissions */
    0,                                  /* handle */
    (uint8 *)&accelService                /* pValue */
  },

    // Data Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &accelDataCharProps
    },

      // Data Characteristic Value
      {
        { ATT_BT_UUID_SIZE, accDataUUID },
        0,
        0,
        (uint8*) &accelData
      },

      // Data Characteristic configuration
      {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)&accelConfigData
      },
};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t accel_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                    uint8 *pValue, uint8 len, uint16 offset );

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Accel_InitService
 *
 * @brief   Initializes the Accelerometer service by
 *          registering GATT attributes with the GATT server. Only
 *          call this function once.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t Accel_InitService( uint32 services )
{
  uint8 status = SUCCESS;

  if ( services & ACCEL_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( accelAttrTbl, GATT_NUM_ATTRS( accelAttrTbl ),
                                          NULL, accel_WriteAttrCB, NULL );
  }

  return ( status );
}


/*********************************************************************
 * @fn      Accel_SetParameter
 *
 * @brief   Set an Accelerometer Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to right
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t Accel_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
    case ACCEL_DATA:
      if ( len == 12 )
      {

        osal_memcpy( accelData, value, 12 );

        // Check for notification
        if ( ( (accelConfigData == GATT_CLIENT_CFG_NOTIFY) ||
               (accelConfigData == GATT_CLIENT_CFG_INDICATE)  ) &&
               (charCfgConnHandle != INVALID_CONNHANDLE) )
        {
          gattAttribute_t *attr;

          // Get the right table entry
          attr = GATTServApp_FindAttr( accelAttrTbl,
                                       GATT_NUM_ATTRS( accelAttrTbl ),
                                       (uint8*)&accelData );
          if ( attr != NULL )
          {

            if ( accelConfigData == GATT_CLIENT_CFG_NOTIFY )
            {
              attHandleValueNoti_t notify;

              // Send the notification
              notify.handle = attr->handle;
              notify.len = 12;
              osal_memcpy( notify.value, accelData, 12 );
              ret = GATT_Notification( charCfgConnHandle, &notify, FALSE );
            }
/*
            else //indication instead of notification
            {
              attHandleValueInd_t indicate;

              // Send the notification
              indicate.handle = attr->handle;
              indicate.len = sizeof ( int8 );
              indicate.value[0] = accelCoordinates[idx];
              ret = GATT_HandleValueInd( charCfgConnHandle, &indicate, FALSE );
            }
            */
          }
        }
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ( ret );
}

/*********************************************************************
 * @fn      accel_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pReq - pointer to request
 *
 * @return  Success or Failure
 */
static bStatus_t accel_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                    uint8 *pValue, uint8 len, uint16 offset )
{
  bStatus_t status = SUCCESS;

  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {

    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      case GATT_CLIENT_CHAR_CFG_UUID:
        // Validate the value
        // Make sure it's not a blob operation
        if ( offset == 0 )
        {
          if ( len == 2 )
          {
            uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );

            // Validate characteristic configuration bit field
            if ( !( charCfg == GATT_CLIENT_CFG_NOTIFY   ||
                    charCfg == GATT_CLIENT_CFG_INDICATE ||
                    charCfg == GATT_CFG_NO_OPERATION ) )
            {
              status = ATT_ERR_INVALID_VALUE;
            }
          }
          else
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }

        // Write the value
        if ( status == SUCCESS )
        {
          uint16 *pCurValue = (uint16 *)pAttr->pValue;

          *pCurValue = BUILD_UINT16( pValue[0], pValue[1] );

          charCfgConnHandle = connHandle;
        }

        break;

      default:
          // Should never get here!
          status = ATT_ERR_ATTR_NOT_FOUND;
    }
  }
  else
  {
    // 128-bit UUID
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/*********************************************************************
*********************************************************************/

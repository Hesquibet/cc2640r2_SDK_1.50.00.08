/******************************************************************************

 @file       hci_data.c

 @brief This file handles HCI data for the controller

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
#include "hal_mcu.h"
#include "osal_bufmgr.h"
#include "ll_common.h"
#include "hci_tl.h"
#include "hci_data.h"

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

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */


/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */


/*******************************************************************************
 * EXTERNS
 */

extern uint8  ctrlToHostEnable;
extern uint16 numHostBufs;

/*
** Data API
*/

/*******************************************************************************
 * This function is used to reverse the order of the bytes in an array in place.
 *
 * Public function defined in hci_c_data.h.
 */
void HCI_ReverseBytes( uint8 *buf,
                       uint8 len )
{
  uint8 temp;
  uint8 index = (uint8)(len - 1);
  uint8 i;

  // adjust length as only half the operations are needed
  len >>= 1;

  // reverse the order of the bytes
  for (i=0; i<len; i++)
  {
    temp           = buf[i];
    buf[i]         = buf[index - i];
    buf[index - i] = temp;
  }

  return;
}


/*
** LL Callbacks
*/

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
/*******************************************************************************
 * This Callback is used by the LL to indicate to the HCI that data has been
 * received and placed in the buffer provided by the HCI.
 *
 * Public function defined in hci_c_data.h.
 */
void LL_RxDataCompleteCback( uint16 connHandle,
                             uint8  *pBuf,
                             uint8  len,
                             uint8  fragFlag,
                             int8   rssi )
{
  // unused input parameter; PC-Lint error 715.
  (void)rssi;

  // check if Controller to Host flow control is enabled
  if ( ctrlToHostEnable == TRUE )
  {
#ifdef DEBUG
    // Sanity Check:
    // The number of available Host buffers should be one or more!
    HCI_ASSERT( numHostBufs != 0 );
#endif // DEBUG

    // decrement the number of available Host buffers, and check if exhausted
    if ( --numHostBufs == 0 )
    {
      // enable LL Rx flow control
      MAP_LL_CtrlToHostFlowControl( LL_ENABLE_RX_FLOW_CONTROL );
    }
  }

  // check if this is for the Host
  if ( hciL2capTaskID != 0 )
  {
    hciDataEvent_t *msg;

    // allocated space for OSAL message
    msg = (hciDataEvent_t *)MAP_osal_msg_allocate( sizeof ( hciDataEvent_t ) );

    if ( msg )
    {
      // message header
      msg->hdr.event  = HCI_DATA_EVENT;
      msg->hdr.status = 0xFF;

      // message data
      msg->connHandle = connHandle;
      msg->pbFlag     = FIRST_PKT_CTRL_TO_HOST;
      msg->len        = len;
      msg->pData      = pBuf;

      // send message
      (void)MAP_osal_msg_send( hciL2capTaskID, (uint8 *)msg );
    }
    else // can't queue this packet
    {
      // so free the packet buffer
      MAP_osal_bm_free( pBuf );
    }
  }
  else // not Host, so route to HCI transport layer
  {
    hciPacket_t *msg;

    // allocate an OSAL HCI message
    msg = (hciPacket_t *)MAP_osal_msg_allocate( sizeof(hciPacket_t) );

    if ( msg )
    {
      // message header
      msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
      msg->hdr.status = 0xFF;
      msg->pData      = pBuf-HCI_DATA_MIN_LENGTH;

      // create HCI packet
      // Note: The payload offset starting after the packet header was given to
      //       the LL for receiving data.
      msg->pData[0] = HCI_ACL_DATA_PACKET;
      msg->pData[1] = LO_UINT16(connHandle);
      msg->pData[2] = HI_UINT16(connHandle) | (fragFlag << 4); // packet boundary flags
      msg->pData[3] = len;
      msg->pData[4] = 0x00;                                    // LL length only 1 byte

      // send message
      (void)MAP_osal_msg_send( hciTaskID, (uint8 *)msg );
    }
    else // can't send this packet
    {
      // so free the packet buffer
      MAP_osal_bm_free( pBuf );
    }
  }
}
#endif // ADV_CONN_CFG | INIT_CFG


/*******************************************************************************
 */

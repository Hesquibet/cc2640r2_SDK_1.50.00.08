/******************************************************************************

 @file       icall_hci_tl.h

 @brief This file contains the application HCI TL ICall function prototypes.

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

#ifndef ICALL_HCI_TL_H
#define ICALL_HCI_TL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Callback for application to overwrite data of an HCI serial packet.
 * Note that some commands require overrides to function properly when called
 * from a remote processor.
 *
 * The embedded application may elect to not override these commands if it
 * does expect them to be called (or the application processor precludes these
 * commands from being sent), but if these commands are called and not overridden, unexpected
 * behavior may occur.
 */
typedef void (*HCI_TL_ParameterOverwriteCB_t)(uint8_t *pData);

/*
 * Callback for the application to send HCI Command Status events up to the
 * Remote Host.
 */
typedef void (*HCI_TL_CommandStatusCB_t)(uint8_t *pBuf, uint16_t len);

/*
 * Callback for the application to post and process Callback from LL.
 * This is needed to exit the Hwi/Swi context of the LL callback.
 */
typedef uint8_t (*HCI_TL_CalllbackEvtProcessCB_t)(void *pData, void* callbackFctPtr);

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
extern void HCI_TL_Init(HCI_TL_ParameterOverwriteCB_t overwriteCB,
                        HCI_TL_CommandStatusCB_t csCB,
                        HCI_TL_CalllbackEvtProcessCB_t evtCB,
                        ICall_EntityID taskID);

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
extern uint8_t HCI_TL_compareAppLastOpcodeSent(uint16_t opcode);

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
extern void HCI_TL_SendToStack(uint8_t *msgToParse);

/*********************************************************************
 * @fn      HCI_TL_processStructuredEvent
 *
 * @brief   Interprets a structured Event from the BLE Host and serializes it.
 *
 * @param   pEvt - structured event to serialize.
 *
 * @return  TRUE to deallocate pEvt message, False otherwise.
 */
extern uint8_t HCI_TL_processStructuredEvent(ICall_Hdr *pEvt);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ICALL_HCI_TL_H */

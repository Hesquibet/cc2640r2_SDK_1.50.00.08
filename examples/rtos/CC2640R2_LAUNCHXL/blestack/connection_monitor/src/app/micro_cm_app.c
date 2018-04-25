/**************************************************************************************************
  Filename:       micro_cm_app.c

  Description:    This file contains the Micro Eddystone Beacon Connection Monitorsample
                  sample application definitions and prototypes.

* Copyright (c) 2017, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* *  Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* *  Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* *  Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

#include <string.h>
#include <stdlib.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>

#include "bcomdef.h"
#include "micro_cm_app.h"
#include "micro_ble_cm.h"
#include "npi_task.h"

#include "board.h"

// DriverLib
#include <driverlib/aon_batmon.h>
#include "uble.h"
#include "ugap.h"
#include "urfc.h"

#include "util.h"
#include "gap.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#ifndef UCA_NPI_TASK_STACK_SIZE
#define UCA_NPI_TASK_STACK_SIZE 700
#endif

#define UCA_MSG_BUFF_SIZE       200

/*********************************************************************
 * TYPEDEFS
 */
#if defined (CC2650DK_7ID)     || defined (CC2650_LAUNCHXL) || \
    defined (BOOSTXL_CC2650MA) || defined (CC1350_LAUNCHXL) || \
    defined (CC2640R2_LAUNCHXL)
#define UCA_MRDY_PIN            IOID_UNUSED
#define UCA_SRDY_PIN            IOID_UNUSED
#endif //CC2650DK_7ID

// Stack Task configuration
#define UBT_TASK_PRIORITY                     3

#ifndef UBT_TASK_STACK_SIZE
#define UBT_TASK_STACK_SIZE                   800
#endif

// App Task configuration
#define UCA_TASK_PRIORITY                     1

#ifndef UCA_TASK_STACK_SIZE
#define UCA_TASK_STACK_SIZE                   800
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
/* uNPI Parameters for opening serial port to CM */
static NPI_Params portParamsCM;

// Event globally used to post local events and pend on local events.
Event_Handle syncAppEvent;

// Event globally used to post local events and pend on local events.
static Event_Handle syncStackEvent;

Task_Struct ubtTask;
uint8 ubtTaskStack[UBT_TASK_STACK_SIZE];

Task_Struct ucaTask;
uint8 ucaTaskStack[UCA_TASK_STACK_SIZE];

// Queue object used for app messages
static Queue_Struct stackMsg;
static Queue_Handle stackMsgQueue;

// Queue object used for app messages
Queue_Struct appMsg;
Queue_Handle appMsgQueue;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void MicroCmApp_processRxMsg(_npiFrame_t *pMsg);
static void MicroCmApp_cmdStatusMsg(uint8_t cmId, uint8_t status);

static void MicroCmApp_processAppMsg(ucaEvt_t *pMsg);
static void MicroCmApp_processAPMsgEvt(uint8_t *pMsg);
static void MicroCmApp_taskFxn(UArg a0, UArg a1);

//ubStack High Priority Task
static void ubStack_taskFxn(UArg a0, UArg a1);

static void uBLEStack_eventProxy(void);
static bStatus_t ubStack_enqueueMsg(uint16 event, uint8 data);
/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 *  @fn      MicroCmApp_init
 *
 *  @brief   Called during initialization and contains application
 *           specific initialization (ie. hardware initialization/setup,
 *           table initialization, power up notification, etc), and
 *           profile initialization/setup.
 *
 *  @param   None
 *
 *  @return  None
 */
void MicroCmApp_init(void)
{
  Task_Params ucaTaskParams;

  // Configure App task
  Task_Params_init(&ucaTaskParams);
  ucaTaskParams.stack = ucaTaskStack;
  ucaTaskParams.stackSize = UCA_TASK_STACK_SIZE;
  ucaTaskParams.priority = UCA_TASK_PRIORITY;
  Task_construct(&ucaTask, MicroCmApp_taskFxn, &ucaTaskParams, NULL);
}

/*********************************************************************
 * @fn MicroCmApp_processRxMsg
 *
 * @brief  Callback from uNPI when an Rx uNPI msg is available containing an
 *         an mt frame bound for the Stack.
 *
 * @param   pMsg - NPI message
 *
 * @return  none
 */
void MicroCmApp_processRxMsg(_npiFrame_t *pMsg)
{
  // Check the Command type, probably not a big deal for such a simple application:
  // The cmd0 value is a combination of the upper 3 bits being the message type and
  // the lower 5 bits being the subsystem.
  // UCA_ASYNC_REQ = 0x01 && RPC_SYS_CM_CMD
  // UCA_ASYNC_RSP = 0x02 && RPC_SYS_CM_CMD
  // see http://processors.wiki.ti.com/index.php/Unified_Network_Processor_Interface
  ucaEvt_t *qMsg;
  volatile uint32 keyHwi;

  keyHwi = Hwi_disable();
  if((qMsg = (ucaEvt_t *)malloc(sizeof(ucaEvt_t))))
  {
    qMsg->event = (uint8_t)UCA_NEW_AP_MSG_EVT;
    qMsg->status = 0;
    qMsg->pNpiMsg = pMsg;

    Util_enqueueMsg(appMsgQueue, syncAppEvent, (uint8_t *)qMsg);
  }
  else
  {
    // Free NPI Frame after use
    free(pMsg);
  }
  Hwi_restore(keyHwi);
}

/*********************************************************************
 * @fn      MicroCmApp_cmdStatusMsg
 *
 * @brief   Return immidiate status of CM API commands.
 *
 * @param   cmId   - Command Id requested by Host for which we are sending status.
 * @param   status - Return status of command called.
 *
 * @return  none
 */
void MicroCmApp_cmdStatusMsg(uint8_t cmId, uint8_t status)
{
  _npiFrame_t *npiMsg = NULL;
  uint8_t len = 1;

  if (npiMsg = (_npiFrame_t *)NPITask_mallocFrame(sizeof(_npiFrame_t) + len))
  {
    npiMsg->dataLen = len;
    npiMsg->cmd0    = UCA_ASYNC_REQ;
    npiMsg->cmd1    = cmId; // Same ID as CM Host command id

    npiMsg->pData = (uint8_t *)((uint32_t)npiMsg + sizeof(_npiFrame_t));
    *npiMsg->pData = status;//Send Application a Status Value

    // forward npiFrame to uNPI
    NPITask_sendToHost(npiMsg);
  }
}

/*********************************************************************
 * @fn      MicroCmApp_taskFxn
 *
 * @brief   Main CM application RTOS task loop, handles new CM command requests
 *          and continous monitor calls.
 *
 * @param   a0 - Standard TI RTOS taskFxn arguments.
 * @param   a1 -
 *
 * @return  none
 */
static void MicroCmApp_taskFxn(UArg a0, UArg a1)
{
  // Initialize NPI Interface
  NPITask_Params_init(NPI_SERIAL_TYPE_UART, &portParamsCM);

  portParamsCM.stackSize = UCA_NPI_TASK_STACK_SIZE;
  portParamsCM.mrdyPinID = UCA_MRDY_PIN;
  portParamsCM.srdyPinID = UCA_SRDY_PIN;
  portParamsCM.bufSize   = UCA_MSG_BUFF_SIZE;

  // Kick off NPI task
  NPITask_open(&portParamsCM);

  // Register callback for receiving RPC_SYS_CM_CMD messages inbound from the Host
  // going to the CM app and ubStack. The RPC_SYS_CM_CMD is just an 8 byte command
  // ID that is part of the uNPI standard packet to tell uNPI to route that message
  // to our chosen callback funciton.
  // RPC_SYS_CM_CMD will be added to npi_data.h in the next free slot, should be value 23.
  NPITask_regSSFromHostCB(RPC_SYS_CM_CMD , MicroCmApp_processRxMsg);

  // Loop might need to be its own function to be called whenever a SNP event is posted.
  // Application main loop

  // Create an RTOS event used to wake up this application to process events.
  syncAppEvent = Event_create(NULL, NULL);

  // Create an RTOS queue for message from profile to be sent to app.
  appMsgQueue = Util_constructQueue(&appMsg);

  for(;;)
  {
    volatile uint32 keyHwi;
    uint32_t events = Event_pend(syncAppEvent, Event_Id_NONE, UCA_ALL_EVENTS, BIOS_WAIT_FOREVER);

    // If RTOS queue is not empty, process app message.
    while(!Queue_empty(appMsgQueue))
    {
      keyHwi = Hwi_disable();
      ucaEvt_t *pMsg = (ucaEvt_t *)Util_dequeueMsg(appMsgQueue);
      Hwi_restore(keyHwi);

      if(pMsg)
      {
        // Process message.
        MicroCmApp_processAppMsg(pMsg);

        keyHwi = Hwi_disable();
        free(pMsg);
        Hwi_restore(keyHwi);
      }
    }
    if(events & UCA_FIND_NEXT_PRIOR_EVT)
    {
      (void)ubCM_start(ubCM_findNextPriorityEvt());
    }
  }
}

/*********************************************************************
 * @fn      MicroCmApp_processAppMsg
 *
 * @brief   Process an incoming uNPI Message to CM.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void MicroCmApp_processAppMsg(ucaEvt_t *pMsg)
{
  switch (pMsg->event)
  {
    case UCA_NEW_AP_MSG_EVT:
      MicroCmApp_processAPMsgEvt((uint8_t*)pMsg->pNpiMsg);
      break;
    default:
      // Do nothing.
      break;
  }
}

/*********************************************************************
 * @fn      MicroCmApp_processAPMsgEvt
 *
 * @brief   Process an incoming uNPI Message to CM.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void MicroCmApp_processAPMsgEvt(uint8_t *pMsg)
{
  _npiFrame_t *pNpiMsg = (_npiFrame_t*)pMsg;
  uint8_t status = CM_SUCCESS;

  if (pNpiMsg->cmd0 == UCA_ASYNC_REQ)
  {
    //determine which message was sent and forward payload to command
    switch(pNpiMsg->cmd1)
    {
      case UCA_START:
        //command size sannity check
        if (pNpiMsg->dataLen == 15)
        {
          status = ubCM_startExt(pNpiMsg->pData);
          MicroCmApp_cmdStatusMsg(UCA_START,status);
        }
        else
        {
          //send error message command not as expected
          MicroCmApp_cmdStatusMsg(UCA_START,CM_FAILED_OUT_OF_RANGE);
        }
        break;

      case UCA_STOP:
        //command size sannity check
        if (pNpiMsg->dataLen == 1)
        {
          status = ubCM_stop(*pNpiMsg->pData);
          MicroCmApp_cmdStatusMsg(UCA_STOP,status);
        }
        else
        {
          //send error message command not as expected
          MicroCmApp_cmdStatusMsg(UCA_STOP,CM_FAILED_NOT_FOUND);
        }
        break;

      default:
        //Unknown Command Fail, Cmd id 0 and status 1
        MicroCmApp_cmdStatusMsg(0,1);
        break;
    }
  }
  NPITask_freeFrame(pNpiMsg);
}

/*********************************************************************
 Setup and Run Required High Priority RTOS Task for ubStack functionality
 *********************************************************************/

/*********************************************************************
 * @fn      MicroCmApp_stack_init
 *
 * @brief   Initialize ubtTask.
 *
 * @param   none
 *
 * @return  none
 */
void MicroCmApp_stack_init(void)
{
  Task_Params stackTaskParams;

  // Configure Stack task
  Task_Params_init(&stackTaskParams);
  stackTaskParams.stack = ubtTaskStack;
  stackTaskParams.stackSize = UBT_TASK_STACK_SIZE;
  stackTaskParams.priority = UBT_TASK_PRIORITY;
  Task_construct(&ubtTask, ubStack_taskFxn, &stackTaskParams, NULL);
}

/*********************************************************************
 * @fn      ubStack_taskFxn
 *
 * @brief   Stack task entry point for the micro_ble.
 *
 * @param   none
 *
 * @return  none
 */
static void ubStack_taskFxn(UArg a0, UArg a1)
{
  volatile uint32 keyHwi;

  // Create an RTOS event used to wake up this application to process events.
  syncStackEvent = Event_create(NULL, NULL);

  // Create an RTOS queue for message from profile to be sent to app.
  stackMsgQueue = Util_constructQueue(&stackMsg);

  uble_stackInit(UBLE_ADDRTYPE_PUBLIC, NULL, uBLEStack_eventProxy,
                 RF_TIME_CRITICAL);

#if defined(FEATURE_MONITOR)
#if defined(FEATURE_CM)
  ubCm_init();
#endif /* FEATURE_CM */
#endif /* FEATURE_MONITOR */


  for (;;)
  {
    // Waits for an event to be posted associated with the calling thread.
    // Note that an event associated with a thread is posted when a
    // message is queued to the message receive queue of the thread
    Event_pend(syncStackEvent, Event_Id_NONE, UBT_QUEUE_EVT, BIOS_WAIT_FOREVER);

    // If RTOS queue is not empty, process app message.
    while (!Queue_empty(stackMsgQueue))
    {
      ubtEvt_t *pMsg;

      // malloc() is not thread safe. Must disable HWI.
      keyHwi = Hwi_disable();
      pMsg = (ubtEvt_t *) Util_dequeueMsg(stackMsgQueue);
      Hwi_restore(keyHwi);

      if (pMsg)
      {
        // Only expects UBT_EVT_MICROBLESTACK from ubStack.
        if (pMsg->event == UBT_EVT_MICROBLESTACK)
        {
          uble_processMsg();
        }

        // free() is not thread safe. Must disable HWI.
        keyHwi = Hwi_disable();

        // Free the space from the message.
        free(pMsg);
        Hwi_restore(keyHwi);
      }
    }
  }
}

/*********************************************************************
 * @fn      uBLEStack_eventProxy
 *
 * @brief   Required event_post for ubStack operation.
 *
 */
void uBLEStack_eventProxy(void)
{
  if (ubStack_enqueueMsg(UBT_EVT_MICROBLESTACK, 0) == FALSE)
  {
    // post event anyway when heap is out to avoid malloc error
    Event_post(syncStackEvent, UTIL_QUEUE_EVENT_ID);
  }
}

/*********************************************************************
 * @fn      ubStack_enqueueMsg
 *
 * @brief   Creates a message and puts the message in RTOS queue.
 *
 * @param   event - message event.
 * @param   data - message data.
 *
 * @return  TRUE or FALSE
 */
static bStatus_t ubStack_enqueueMsg(uint16 event, uint8 data)
{
  volatile uint32 keyHwi;
  ubtEvt_t *pMsg;
  uint8_t status = FALSE;

  // malloc() is not thread safe. Must disable HWI.
  keyHwi = Hwi_disable();

  // Create dynamic pointer to message.
  pMsg = (ubtEvt_t*) malloc(sizeof(ubtEvt_t));
  if (pMsg != NULL)
  {
    pMsg->event = event;
    pMsg->data = data;

    // Enqueue the message.
    status = Util_enqueueMsg(stackMsgQueue, syncStackEvent, (uint8*) pMsg);
  }
  Hwi_restore(keyHwi);
  return status;
}

/*********************************************************************
 *********************************************************************/

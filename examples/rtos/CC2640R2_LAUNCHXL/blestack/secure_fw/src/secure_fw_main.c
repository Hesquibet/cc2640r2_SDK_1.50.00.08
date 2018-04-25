/******************************************************************************

 @file       secure_fw_main.c

 @brief This module contains the definitions for the main functionality of a
        Secure FW.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************

 Copyright (c) 2012-2017, Texas Instruments Incorporated
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

/* -----------------------------------------------------------------------------
 *                                          Includes
 * -----------------------------------------------------------------------------
 */
#include <string.h>
#include "hal_flash.h"
#include <driverlib/pwr_ctrl.h>
#include "hal_types.h"
#include "secure_fw.h"
#include "secure_fw_config.h"
#include <inc/hw_device.h>
#include <driverlib/sys_ctrl.h>
#include <driverlib/aon_event.h>
#include <driverlib/osc.h>

/* -----------------------------------------------------------------------------
 *                                          Constants
 * -----------------------------------------------------------------------------
 */


/* -----------------------------------------------------------------------------
 *                                          Externs
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 *                                          Typedefs
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 *                                       Global Variables
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 *                                       Local Variables
 * -----------------------------------------------------------------------------
 */

#if defined(__IAR_SYSTEMS_ICC__)
__no_init uint8 pgBuf[HAL_FLASH_PAGE_SIZE];
#elif defined(__TI_COMPILER_VERSION__)
uint8 pgBuf[HAL_FLASH_PAGE_SIZE];
#endif




/*******************************************************************************
 * @fn          main
 *
 * @brief       C-code main function.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
void main(void)
{
  uint8 secure_fw_magic[SECURE_FW_MAGIC_LEN] = SECURE_FW_SFW_MAGIC_WORD;
  secure_fw_errorcode_t errorCode = secure_fw_error_no_error;
  secure_fw_init();

  /* check if magic word is found */
  int8_t status = secure_fw_compare(secure_fw_header_info.magicWord,
                                   secure_fw_magic,
                                   SECURE_FW_MAGIC_LEN);
  if((status == 0) && (SECURE_FW_VERIFY_ALWAYS == 1))
  {

    if(secure_fw_config_info.secureFWVerify)
    {
#if SECURE_FW_FW_ROLLBACK
      /* check version */
      if(secure_fw_checkVersion() == TRUE)
      {
#endif /* SECURE_FW_FW_ROLLBACK */
        errorCode = secure_fw_checkAddrSanity();
        /* check address sanity  */
        if( errorCode == secure_fw_error_no_error)
        {
          /* validate sign element */

          if(secure_fw_checkCertElement() == TRUE)
          {
            /* ECC verify */
            if(secure_fw_verifyImage_ecc() == SECURE_FW_STATUS_SUCCESS)
            {

              /* Load address of label __iar_program_start from the fixed location
              of the image's reset vector*/
              asm(" MOV R0, #0x0070 ");
              asm(" LDR R1,[R0,#0x4]");

              /* Reset the stack pointer */
              asm(" LDR SP, [R0, #0x0] ");
              /* jump */
              asm(" BX R1");
            }
            else
            {
              //exception
              errorCode = secure_fw_error_verify_fail;
            }
          }
          else
          {
            //exception
            errorCode = secure_fw_error_cert_not_found;
          }
        }
#if SECURE_FW_FW_ROLLBACK
      }
      else
      {
        //exception
        errorCode = secure_fw_error_invalid_fw_version;
      }
#endif /* SECURE_FW_FW_ROLLBACK */
    }
  }
  else
  {
    //exception
    errorCode = secure_fw_error_invalid_magic_word;
  }

  if(errorCode != secure_fw_error_no_error)
  {
    secure_fw_exception(errorCode);
    asm(" WFI ");
  }
}




/**************************************************************************************************
*/

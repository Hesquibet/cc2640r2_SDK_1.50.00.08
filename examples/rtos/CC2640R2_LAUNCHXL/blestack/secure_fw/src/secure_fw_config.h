/******************************************************************************
 @file       secure_fw_config.h

@brief secureFW configuration parameters
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
#ifndef SECURE_FW_CONFIG_H
#define SECURE_FW_CONFIG_H

/******************************************************************************

 Includes

 *****************************************************************************/

#ifdef __cplusplus

extern "C"
{

#endif

/******************************************************************************

 Constants and definitions

 *****************************************************************************/

/*! Secure FW Configuration */
  /*! Currently not supported, runs verification on Secure FW header
  if set to true */
#define SECURE_FW_SELF_CHECK         false
  /*! Verifies app image always if value set to 1 */
#define SECURE_FW_VERIFY_ALWAYS      1
  /*! Currently not supported due to flash space constraints with CCS
    ,if set to 0, rollback to previous FW version is not allowed */
#define SECURE_FW_FW_ROLLBACK        0
  /*! Magic word for start of header */
#define SECURE_FW_SFW_MAGIC_WORD     {0x57,0x46,0x53,0x20,0x4C,0x42,0x53,0x0A}
  /*! type of sign element, ECC if set to 1, AES if set to 2 ( AES not
  supported currently */
#define SECURE_FW_SIGN_TYPE          1
  /*! Locks page 31 if set to true */
#define SECURE_FW_LOCK               true
  /*! Erases flash on failure if set to true, does not erase when set to false*/
#define SECURE_FW_ERASE_FLASH         false

#ifdef __cplusplus

}

#endif

#endif /* SECURE_FW_CONFIG_H */

/******************************************************************************

 @file       secure_fw.c

 @brief This file contains the implementation for secure FW.

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
#include "hal_board.h"
#include <driverlib/vims.h>
#include "secure_fw.h"
#include "SHA2CC26XX.h"

/*********************************************************************
 * MACROS
 */

#define SECURE_FW_POS_FW_VER  SECURE_FW_HEADER_PAGE+8
#define SECURE_FW_POS_FW_LEN  SECURE_FW_POS_FW_VER+4
#define SECURE_FW_POS_FW_START SECURE_FW_POS_FW_LEN+4
#define SECURE_FW_POS_FW_END   SECURE_FW_POS_FW_START+4
#define SECURE_FW_POS_SIGN_TYPE SECURE_FW_POS_FW_END+1
#define SECURE_FW_POS_TIME_STAMP SECURE_FW_POS_SIGN_TYPE+4
#define SECURE_FW_POS_SIGNER_INFO SECURE_FW_POS_TIME_STAMP+4
#define SECURE_FW_POS_SIGN1 SECURE_FW_POS_SIGNER_INFO+8
#define SECURE_FW_POS_SIGN2 SECURE_FW_POS_SIGN1+32
#define SECURE_FW_FW_VER_LEN 4
#define SECURE_FW_FW_LEN  4
#define SECURE_FW_START_ADDR_LEN 4
#define SECURE_FW_END_ADDR_LEN 4
#define SECURE_FW_SIGNER_INFO_LEN 8
#define SECURE_FW_KEY_LEN 32
#define SECURE_FW_SIGN_LEN 32
#define SECURE_FW_HEADER_PAGE 0
#define SECURE_FW_LAST_PAGE 31
#define SECURE_FW_HEADER_LEN 112
#define SECURE_FW_PAGE_SIZE 4096
#define SECURE_FW_ERROR_SIZE 5
#define SECURE_FW_IMG_START_ADDR_LIMIT 0x00000070
#define SECURE_FW_IMG_END_ADDR_LIMIT   0x0001e000
#define SECURE_FW_ADDR_BLOCK   0x1000
#define SECURE_FW_PAGE_SHIFT   12

/*********************************************************************
 * Global Variables
 */
secure_fw_hdr_t secure_fw_header_info;
secure_fw_config_t secure_fw_config_info;

/*********************************************************************
 * LOCAL VARIABLES
 */

/* Cert element stored in flash where public keys in Little endian format*/
#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION(_secureFWCertElement, ".cert_element")
#pragma RETAIN(_secureFWCertElement)
const secure_fw_cert_element_t _secureFWCertElement =
#elif  defined(__IAR_SYSTEMS_ICC__)
#pragma location=".cert_element"
const secure_fw_cert_element_t _secureFWCertElement @ ".cert_element" =
#endif
{
  .type = SECURE_FW_SIGN_TYPE,
  .length = SECURE_FW_CERT_LENGTH,
  .options = SECURE_FW_CERT_OPTIONS,
  .signerInfo = {0x46,0x83,0xe2,0xb9,0x96,0x6a,0xa4,0x4f},
  .certPayload.eccKey.pubKeyX = {0x85,0x65,0xab,0xdb,0x77,0x59,0xef,0x04,0x00,\
                                 0x58,0x87,0x08,0x4c,0x9f,0x64,0x8f,0xa4,0x01,\
                                 0x8b,0xb2,0x9c,0xaa,0x7e,0xed,0x0b,0xaa,0x9a,\
                                 0xf3,0x67,0xbd,0x31,0xd2},
  .certPayload.eccKey.pubKeyY = {0x0f,0xec,0xf1,0x1c,0xb9,0x2b,0x66,0x9a,0x9d,\
                                 0x50,0x2a,0x61,0x34,0xfc,0x03,0x72,0xf0,0x1a,\
                                 0xa9,0xb7,0xbc,0xf7,0x47,0xd2,0x7d,0xe4,0xda,\
                                 0x76,0x2f,0x55,0xfe,0x93}
  };

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void secure_fw_enableCache(uint8_t state);
static uint8_t secure_fw_disableCache(void);
static void eccInit(ECCROMCC26XX_Params *pParams);
static void computeSha2Hash(uint8_t *pHash);
static void copy_byte(uint8_t *pDst, const uint8_t *pSrc, uint32_t len);
static void initEccGlobals(ECCROMCC26XX_CurveParams *pCurve);
static uint32_t Util_buildUint32(uint8_t byte0, uint8_t byte1, uint8_t byte2,
                            uint8_t byte3);
static void reverseOrder(uint8_t *pBufIn,uint8_t *pBufOut);
static uint8_t getPage( uint8_t *pAddr);
static uint8_t getOffset( uint8_t *pAddr);

/*********************************************************************
 * GLOBAL FUNCTION REFERENCES
 ********************************************************************/
extern uint8_t eccRom_verifyHash(uint32_t *, uint32_t *, uint32_t *, uint32_t *,
                                 uint32_t *);

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Initialize the parameters read from header metadata and config

 Public function defined in secure_fw.h
 */
void secure_fw_init()
{
  static uint8_t temp[SECURE_FW_SIGN_LEN] = {0};

  /* Read from metadata and populate structure */
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_HEADER_PAGE,
                secure_fw_header_info.magicWord,
                sizeof(secure_fw_header_info.magicWord));
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_POS_FW_VER,
                secure_fw_header_info.ver,
                sizeof(secure_fw_header_info.ver));
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_POS_FW_LEN,
                secure_fw_header_info.len,
                sizeof(secure_fw_header_info.len));
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_POS_FW_START,
                secure_fw_header_info.imgStartAddr,
                sizeof(secure_fw_header_info.imgStartAddr));
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_POS_FW_END,
                secure_fw_header_info.imgEndAddr,
                SECURE_FW_END_ADDR_LEN);
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_POS_SIGN_TYPE ,
                &secure_fw_header_info.signElement.type,
                sizeof(secure_fw_header_info.signElement.type));
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_POS_TIME_STAMP ,
                secure_fw_header_info.signElement.timeStamp,
                sizeof(secure_fw_header_info.signElement.timeStamp));
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_POS_SIGNER_INFO ,
                secure_fw_header_info.signElement.signerInfo,
                sizeof(secure_fw_header_info.signElement.signerInfo));
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_POS_SIGN1 ,
                 temp,
                 sizeof(temp));
  reverseOrder(temp,
               secure_fw_header_info.signElement.signPayload.eccSignature.sign_r);
  secure_fw_readFlash(SECURE_FW_HEADER_PAGE,SECURE_FW_POS_SIGN2 ,
                temp,
                sizeof(temp));
  reverseOrder(temp,
               secure_fw_header_info.signElement.signPayload.eccSignature.sign_s);

  /* Read secure_fw Config info and populate structure */
  secure_fw_config_info.selfCheck = SECURE_FW_SELF_CHECK;
  secure_fw_config_info.secureFWVerify = SECURE_FW_VERIFY_ALWAYS;
  secure_fw_config_info.certElement.type = SECURE_FW_SIGN_TYPE;
  secure_fw_config_info.certElement.options = SECURE_FW_CERT_OPTIONS;
  secure_fw_config_info.certElement.length =  SECURE_FW_CERT_LENGTH;
}

/*!
 Verify the signature from header using ECC functions from ROM

 Public function defined in secure_fw.h
 */
int8_t secure_fw_verifyImage_ecc()
{
  int8_t status = SECURE_FW_ECC_STATUS_INVALID_SIGNATURE;
  uint8_t *sign1;
  uint8_t *sign2;
  uint8_t *publicKeyX;
  uint8_t *publicKeyY;
  uint8_t  hash[SECURE_FW_SIGN_LEN] = {0};
  uint8_t  temp[SECURE_FW_SIGN_LEN] = {0};
  uint8_t *hashBuf;
  ECCROMCC26XX_Params params;
  eccInit(&params);
  computeSha2Hash(&temp[0]);
  reverseOrder(temp,hash);

  /* check for validity of parameters */

  if (secure_fw_config_info.certElement.certPayload.eccKey.pubKeyX == 0 ||
      secure_fw_config_info.certElement.certPayload.eccKey.pubKeyY == 0 ||
      secure_fw_header_info.signElement.signPayload.eccSignature.sign_r == 0 ||
      secure_fw_header_info.signElement.signPayload.eccSignature.sign_s == 0 ||
      hash == NULL )
  {
    return SECURE_FW_STATUS_ILLEGAL_PARAM;
  }

  /*total memory for operation: workzone and 5 key buffers*/
  uint32_t eccWorkzone[SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES];
  eccRom_workzone = &eccWorkzone[0];

   /* Split allocated memory into buffers */
  publicKeyX = (uint8_t *)eccRom_workzone +
               SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES;
  publicKeyY = publicKeyX +
               SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
  hashBuf =  publicKeyY +
             SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
  sign1  = hashBuf +
           SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
  sign2  = sign1 +
           SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);


  initEccGlobals(&params.curve);

   /* Set length of keys in words in the first word of each buffer*/
  *((uint32_t *)&publicKeyX[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
    (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

  *((uint32_t *)&publicKeyY[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
   (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

  *((uint32_t *)&hashBuf[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
    (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

  *((uint32_t *)&sign1[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
    (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

  *((uint32_t *)&sign2[SECURE_FW_ECC_KEY_LEN_OFFSET]) =
    (uint32_t)(SECURE_FW_ECC_UINT32_BLK_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES));

 /* Copy input key into buffer */
  copy_byte( publicKeyX + SECURE_FW_ECC_KEY_OFFSET,
             _secureFWCertElement.certPayload.eccKey.pubKeyX,
             SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
  copy_byte( publicKeyY + SECURE_FW_ECC_KEY_OFFSET,
             _secureFWCertElement.certPayload.eccKey.pubKeyY,
             SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
   /* copy hash into buffer */
  copy_byte( hashBuf + SECURE_FW_ECC_KEY_OFFSET,hash,
            SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);

  copy_byte( sign1 + SECURE_FW_ECC_KEY_OFFSET,
             secure_fw_header_info.signElement.signPayload.eccSignature.sign_r,
             SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);
  copy_byte( sign2 + SECURE_FW_ECC_KEY_OFFSET,
             secure_fw_header_info.signElement.signPayload.eccSignature.sign_s,
             SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES);

  status = eccRom_verifyHash((uint32_t *)publicKeyX,
                             (uint32_t *)publicKeyY,
                             (uint32_t *)hashBuf,
                             (uint32_t *)sign1,
                             (uint32_t *)sign2);
  /* zero out workzone and 5 buffers as a precautionary measure.*/
  copy_byte((uint8_t *)eccRom_workzone, 0x00,
             SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES);

  /* Map success code.*/
  if (((uint8_t)status) == SECURE_FW_ECC_STATUS_VALID_SIGNATURE)
  {
    status = SECURE_FW_STATUS_SUCCESS;
  }

  return (status);
}

/*!
 Handle exception action if secure fw fails at any of the steps

 Public function defined in secure_fw.h
 */
void secure_fw_exception(secure_fw_errorcode_t errorCode)
{
  uint8_t page;

  if(SECURE_FW_ERASE_FLASH)
  {
    for (page = SECURE_FW_HEADER_PAGE; page < SECURE_FW_LAST_PAGE; page++)
    {
      secure_fw_eraseFlash(page);
    }
    /* Identifier for error */
    secure_fw_writeFlash(SECURE_FW_HEADER_PAGE+1,
                         0,
                         (uint8_t *)&errorCode,
                         sizeof(uint8_t));
  }
}

#if SECURE_FW_FW_ROLLBACK
/*!
 Sanity check for version as defined by configuration

 Public function defined in secure_fw.h
 */
uint8_t secure_fw_checkVersion()
{
  uint8_t ver[SECURE_FW_FW_VER_LEN] = SECURE_FW_VERSION_CHECK;
  if((secure_fw_header_info.ver[0] < ver[0]) &&
     (secure_fw_header_info.ver[0] != ver[0]))
  {
    /* exception */
    return (FALSE);
  }
  else if((secure_fw_header_info.ver[1] < ver[1]) &&
          (secure_fw_header_info.ver[1] != ver[1]))
  {
    /* exception */
    return (FALSE);
  }
  else if((secure_fw_header_info.ver[2] < ver[2]) &&
          (secure_fw_header_info.ver[2] != ver[2]))
  {
    /* exception */
    return (FALSE);
  }
  else if((secure_fw_header_info.ver[3] < ver[3]))
  {
    /* exception */
    return (FALSE);
  }
  else
  {
    return (TRUE);
  }
}
#endif /* SECURE_FW_FW_ROLLBACK*/

/*!
 Check the validity of cert element

 Public function defined in secure_fw.h
 */
uint8_t secure_fw_checkCertElement()
{
  /* read type in sign element and compare with type in cert element */
  int8_t status = secure_fw_compare(secure_fw_header_info.signElement.signerInfo,
                              _secureFWCertElement.signerInfo,8);
  if( status == 0)
  {
    return (TRUE);
  }
  return (FALSE);
}

/*!
 Check the validity of addresses in image header

 Public function defined in secure_fw.h
 */
secure_fw_errorcode_t secure_fw_checkAddrSanity()
{
  uint32_t startAddr = Util_buildUint32(secure_fw_header_info.imgStartAddr[3],
                                        secure_fw_header_info.imgStartAddr[2],
                                        secure_fw_header_info.imgStartAddr[1],
                                        secure_fw_header_info.imgStartAddr[0]);
  uint32_t endAddr = Util_buildUint32(secure_fw_header_info.imgEndAddr[3],
                                      secure_fw_header_info.imgEndAddr[2],
                                      secure_fw_header_info.imgEndAddr[1],
                                      secure_fw_header_info.imgEndAddr[0]);
  uint32_t imgLength = Util_buildUint32(secure_fw_header_info.len[3],
                                        secure_fw_header_info.len[2],
                                        secure_fw_header_info.len[1],
                                        secure_fw_header_info.len[0]);

  if((startAddr < SECURE_FW_IMG_START_ADDR_LIMIT) ||
     (endAddr > SECURE_FW_IMG_END_ADDR_LIMIT) ||
     (endAddr == SECURE_FW_IMG_END_ADDR_LIMIT))
  {
    return (secure_fw_error_invalid_addr);
  }
  else
  {
    if(endAddr-startAddr == imgLength)
    {
      return (secure_fw_error_no_error);
    }
    else
    {
      return (secure_fw_error_invalid_fw_length);
    }
  }
}

/*!
 Read data from flash.

 Public function defined in secure_fw.h
 */
void secure_fw_readFlash(uint8_t page, uint32_t offset, uint8_t *pBuf,
                         uint16_t len)
{
  halIntState_t cs;
  uint8_t *ptr = (uint8_t *)FLASH_ADDRESS(page, offset);

  /* Enter critical section */
  HAL_ENTER_CRITICAL_SECTION(cs);

  /* Read from pointer into buffer.*/
  while (len--)
  {
    *pBuf++ = *ptr++;
  }

  /* Exit critical section.*/
  HAL_EXIT_CRITICAL_SECTION(cs);
}

/*!
 Write data to flash.

 Public function defined in secure_fw.h
 */
void secure_fw_writeFlash(uint8_t page, uint32_t offset, uint8_t *pBuf,
                          uint16_t len)
{
  uint8_t cacheState;

  cacheState = secure_fw_disableCache();

  FlashProgram(pBuf, (uint32_t)FLASH_ADDRESS(page, offset), len);

  secure_fw_enableCache(cacheState);
}

/*!
 Erase data in flash.

 Public function defined in secure_fw.h
 */
void secure_fw_eraseFlash(uint8_t page)
{
  uint8_t cacheState;

  cacheState = secure_fw_disableCache();

  FlashSectorErase((uint32_t)FLASH_ADDRESS(page, 0));

  secure_fw_enableCache(cacheState);
}

/*!
 Utility function to compare the content of two memory locations

 Public function defined in secure_fw.h
 */
int secure_fw_compare(uint8_t *pData1, const uint8_t *pData2, uint8_t len)
{
  int i;
  for(i=0; i<len; i++) if(pData1[i]!=pData2[i]) return (1);
  return (0);
}

/******************************************************************************
 Local Functions
 *****************************************************************************/
/*********************************************************************
 * @fn      secure_fw_disableCache
 *
 * @brief   Resumes system after a write to flash, if necessary.
 *
 * @param   None.
 *
 * @return  VIMS_MODE_ENABLED if cache was in use before this operation,
 *          VIMS_MODE_DISABLED otherwise.
 */
static uint8_t secure_fw_disableCache(void)
{
  uint8_t state = VIMSModeGet(VIMS_BASE);

  /* Check VIMS state */
  if (state != VIMS_MODE_DISABLED)
  {
    /* Invalidate cache */
    VIMSModeSet(VIMS_BASE, VIMS_MODE_DISABLED);

    /* Wait for disabling to be complete */
    while (VIMSModeGet(VIMS_BASE) != VIMS_MODE_DISABLED);
  }

  return (state);
}

/*********************************************************************
 * @fn      secure_fw_enableCache
 *
 * @param   state - VIMS state
 * @brief   Prepares system for a write to flash, if necessary.
 */
static void secure_fw_enableCache(uint8_t state)
{
  if (state != VIMS_MODE_DISABLED)
  {
    /* Enable the Cache. */
    VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);
  }
}

/*********************************************************************
 * @fn     eccInit
 * @brief  Function to initialize the ECCROMCC26XX_Params struct to its defaults.
 *         params should not be modified until after this function is called.
 *         Default parameters use the NIST P-256 curve
 *
 * @param  pParams  Pointer to parameter structure to initialize.
 */
static void eccInit(ECCROMCC26XX_Params *pParams)
{
 /* Initialize Curve to NIST P-256 with window size 3 by default */
  pParams->curve.keyLen      = SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES;
  pParams->curve.workzoneLen = SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES;
  pParams->curve.windowSize  = SECURE_FW_ECC_WINDOW_SIZE;
  pParams->curve.param_p     = &NIST_Curve_P256_p;
  pParams->curve.param_r     = &NIST_Curve_P256_r;
  pParams->curve.param_a     = &NIST_Curve_P256_a;
  pParams->curve.param_b     = &NIST_Curve_P256_b;
  pParams->curve.param_gx    = &NIST_Curve_P256_Gx;
  pParams->curve.param_gy    = &NIST_Curve_P256_Gy;
}

/*********************************************************************
 * @fn         computeSha2Hash
 * @brief      Compute SHA256 hash
 *
 * @param      pHash - pointer to output buffer containing computed hash value
 */
static void computeSha2Hash(uint8_t *pHash)
{
  uint8_t input[SECURE_FW_PAGE_SIZE] = {0};
  uint8_t page = 0;
  uint32_t offset = 0;
  uint32_t startAddr = Util_buildUint32(secure_fw_header_info.imgStartAddr[3],
                                        secure_fw_header_info.imgStartAddr[2],
                                        secure_fw_header_info.imgStartAddr[1],
                                        secure_fw_header_info.imgStartAddr[0]);
  uint32_t endAddr = Util_buildUint32(secure_fw_header_info.imgEndAddr[3],
                                      secure_fw_header_info.imgEndAddr[2],
                                      secure_fw_header_info.imgEndAddr[1],
                                      secure_fw_header_info.imgEndAddr[0]);
  uint32_t imgLength = Util_buildUint32(secure_fw_header_info.len[3],
                                        secure_fw_header_info.len[2],
                                        secure_fw_header_info.len[1],
                                        secure_fw_header_info.len[0]);
  uint32_t imgLengthLeft = imgLength;
  uint8_t mod = 0;
  uint32_t addr = startAddr;
  SHA256_memory_t workzone;
  SHA2CC26XX_initialize(&workzone);
  /* Add timestamp to data used to generate hash */
  copy_byte(input,
            secure_fw_header_info.signElement.timeStamp,
            sizeof(secure_fw_header_info.signElement.timeStamp));
  SHA2CC26XX_execute(&workzone,
                     input,
                     sizeof(secure_fw_header_info.signElement.timeStamp));
   /* Add FW version to data used to generate hash */
  copy_byte(input, secure_fw_header_info.ver, sizeof(secure_fw_header_info.ver));
  SHA2CC26XX_execute(&workzone, input, sizeof(secure_fw_header_info.ver));

 while( imgLengthLeft > 0)
  {
    page = getPage((uint8_t *)&addr);
    offset = getOffset((uint8_t *)&addr);
    if(offset != 0)
    {
      secure_fw_readFlash(page, offset, input, SECURE_FW_PAGE_SIZE - offset);
      SHA2CC26XX_execute(&workzone, input, SECURE_FW_PAGE_SIZE-offset);
      imgLengthLeft = imgLengthLeft -(SECURE_FW_PAGE_SIZE - offset);
      addr = (addr -(uint32_t)FLASH_ADDRESS(page, offset)) +
              SECURE_FW_ADDR_BLOCK;
    }
    else
    {
      mod = (imgLengthLeft/SECURE_FW_PAGE_SIZE);
      if((mod > 1) ||(mod == 1))
      {
        secure_fw_readFlash(page,0, input, SECURE_FW_PAGE_SIZE);
        SHA2CC26XX_execute(&workzone, input, SECURE_FW_PAGE_SIZE);
        imgLengthLeft -= SECURE_FW_PAGE_SIZE;
      }
      else
      {
        secure_fw_readFlash(page, 0, input, imgLengthLeft);
        SHA2CC26XX_execute(&workzone ,input, imgLengthLeft);
        imgLengthLeft = 0;
      }
      addr+=SECURE_FW_ADDR_BLOCK;
    }

  }
   SHA2CC26XX_output(&workzone, pHash);
}

/*********************************************************************
 * @fn         copy_byte
 * @brief      Copy data between memory locatins
 *
 * @param      pDst - pointer to destination buffer
 * @param      pSrc - pointer to source buffer
 * @param      len  - length of data to be copied
 */
static void copy_byte(uint8_t *pDst, const uint8_t *pSrc, uint32_t len)
{
  uint32_t i;
  for(i=0; i<len; i++)
    pDst[i]=pSrc[i];
}

/*********************************************************************
 * @fn         initEccGlobals
 * @brief      Initializa global variables needed for ECC verify operation
 *
 * @param      pCurve - pointer to curve parameters for the curve used for ECC
 *                      operations
 */
static void initEccGlobals(ECCROMCC26XX_CurveParams *pCurve)
{
  /* Store client parameters into ECC ROM parameters */
  eccRom_param_p  = pCurve->param_p;
  eccRom_param_r  = pCurve->param_r;
  eccRom_param_a  = pCurve->param_a;
  eccRom_param_b  = pCurve->param_b;
  eccRom_param_Gx = pCurve->param_gx;
  eccRom_param_Gy = pCurve->param_gy;

  /* Initialize window size */
  eccRom_windowSize = pCurve->windowSize;
}

/*********************************************************************
 * @fn         reverseOrder
 * @brief      Reverse the byte order and copy to output buffer
 *
 * @param      pBufIn - pointer to input buffer
 * @param      pBufOut - pointer to output buffer
 */
static void reverseOrder(uint8_t *pBufIn,uint8_t *pBufOut)
{
  uint8_t i=0;
  for(i=0;i<SECURE_FW_SIGN_LEN;i++)
  {
    pBufOut[i] = pBufIn[SECURE_FW_SIGN_LEN-1-i];
  }
}

/*********************************************************************
 * @fn         Util_buildUint32
 * @brief      Build a uint32_t out of 4 uint8_t variables
 *
 * @param      byte0 - byte - 0
 * @param      byte1 - byte - 1
 * @param      byte2 - byte - 2
 * @param      byte3 - byte - 3
 *
 * @return     combined uint32_t
 */
static uint32_t Util_buildUint32(uint8_t byte0, uint8_t byte1, uint8_t byte2,
                            uint8_t byte3)
{
    return((uint32_t)((uint32_t)((byte0) & 0x00FF) +
                     ((uint32_t)((byte1) & 0x00FF) << 8) +
                     ((uint32_t)((byte2) & 0x00FF) << 16) +
                     ((uint32_t)((byte3) & 0x00FF) << 24)));
}

/*********************************************************************
 * @fn         getPage
 * @brief      Get the flash page corresponding for the given address
 *
 * @param      pAddr - pointer to address in flash
 *
 * @return     flash page number
 */
static uint8_t getPage( uint8_t *pAddr)
{

  uint8_t page = ((uint32_t)((uint32_t)((pAddr[0]) & 0x00FF) +
                             ((uint32_t)((pAddr[1]) & 0x00FF) << 8) +
                             ((uint32_t)((pAddr[2]) & 0x00FF) << 16) +
                             ((uint32_t)((pAddr[3]) & 0x00FF) << 24)))>>
                              SECURE_FW_PAGE_SHIFT;
  return (page);
}

/*********************************************************************
 * @fn         getOffset
 * @brief      Get the offset from start of flash page corresponding
 *             for the given address
 *
 * @param      pAddr - pointer to address in flash
 *
 * @return     flash page number
 */
static uint8_t getOffset( uint8_t *pAddr)
{
   uint8_t offset = ((uint32_t)((uint32_t)((pAddr[0]) & 0x00FF) +
                              ((uint32_t)((pAddr[1]) & 0x00FF) << 8) +
                              ((uint32_t)((pAddr[2]) & 0x00FF) << 16) +
                              ((uint32_t)((pAddr[3]) & 0x00FF) << 24)))%
                               SECURE_FW_ADDR_BLOCK;
   return (offset);
}


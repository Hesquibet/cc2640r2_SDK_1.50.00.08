/******************************************************************************

@file       secure_fw.h

@brief This file contains secure_fw header file.  These are common
API prototypes, constants and macros used by the SecureFW

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
#ifndef SECURE_FW_H
#define SECURE_FW_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "secure_fw_config.h"
/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************/

/*! SecureFW failure error codes */
typedef enum
{
   /*! No Failure */
  secure_fw_error_no_error = 0x00,
  /*! Failure due to invalid magic word */
  secure_fw_error_invalid_magic_word = 0x01,
  /*! Failure due to invalid firmware version */
  secure_fw_error_invalid_fw_version = 0x02,
  /*! Failure due to invalid start or end address*/
  secure_fw_error_invalid_addr       = 0x03,
  /*! Failure when length of image is invalid*/
  secure_fw_error_invalid_fw_length    = 0x04,
   /*! Failure when cert element is not found*/
  secure_fw_error_cert_not_found  = 0x05,
  /*! Failure when verify of image is not successful*/
  secure_fw_error_verify_fail  = 0x06
}secure_fw_errorcode_t;


/*********************************************************************
 * MACROS
 */
/*********************************************************************/

/*! Convert page reference to a memory address in flash */
#define FLASH_ADDRESS(page, offset) (((page) << 12) + (offset))

/*!< Success Return Code      */
#define SECURE_FW_STATUS_SUCCESS                              0
/*! Failure return code */
#define SECURE_FW_STATUS_FAIL                                 1
/*!< Illegal parameter        */
#define SECURE_FW_STATUS_ILLEGAL_PARAM                       -1
/*! Invalid ECC Signature         */
#define SECURE_FW_ECC_STATUS_INVALID_SIGNATURE             0x5A
/*! ECC Signature Successfully Verified  */
#define SECURE_FW_ECC_STATUS_VALID_SIGNATURE               0xA5

/* ECC Window Size.  Determines speed and workzone size of ECC operations.
 Recommended setting is 3. */
#define SECURE_FW_ECC_WINDOW_SIZE                3

/* Key size in uint32_t blocks */
#define SECURE_FW_ECC_UINT32_BLK_LEN(len)        (((len) + 3) / 4)

/* Offset value for number of bytes occupied by length field */
#define SECURE_FW_ECC_KEY_OFFSET                 4

/* Offset of Key Length field */
#define SECURE_FW_ECC_KEY_LEN_OFFSET             0

/* Total buffer size */
#define SECURE_FW_ECC_BUF_TOTAL_LEN(len)         ((len) + SECURE_FW_ECC_KEY_OFFSET)

/*!
 * ECC Workzone length in bytes for NIST P-256 key and shared secret generation.
 * For use with ECC Window Size 3 only.  Used to store intermediary values in
 * ECC calculations.
 */
#define SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES          2000

/*!
 * ECC key length in bytes for NIST P-256 keys.
 */
#define SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES                32

/* ECC Window Size.  Determines speed and workzone size of ECC operations.
 Recommended setting is 3 */
#define SECURE_FW_ECC_WINDOW_SIZE                3

#define SECURE_FW_START_PAGE  0

/* Cert element length for ECC- size of secure_fw_cert_element_t*/
#define SECURE_FW_CERT_LENGTH    sizeof(secure_fw_cert_element_t)
#define SECURE_FW_CERT_OPTIONS   0x0000
#define SECURE_FW_VERSION        {0x00,0x09}

/*SECURE FW macros*/
#define SECURE_FW_ID_OFFSET         0x00000000
#define SECURE_FW_ADDR_OFFSET        4
#define SECURE_FW_VERSION_OFFSET     SECURE_FW_ID_OFFSET+8
#define SECURE_FW_MAGIC_LEN          8
#if SECURE_FW_FW_ROLLBACK
/* Secure Image FW version should be greater than this version */
#define SECURE_FW_VERSION_CHECK      {0x03,0x00,0x00,0x00}
#endif

#define SECURE_FW_ERROR_WORD        {0x45,0x52,0x52,0x4F,0x52};

/*********************************************************************
 * GLOBAL VARIABLES
 */
/* ECC ROM global window size and workzone buffer. */
extern uint8_t eccRom_windowSize;
extern uint32_t *eccRom_workzone;

/* ECC ROM global parameters */
extern uint32_t  *eccRom_param_p;
extern uint32_t  *eccRom_param_r;
extern uint32_t  *eccRom_param_a;
extern uint32_t  *eccRom_param_b;
extern uint32_t  *eccRom_param_Gx;
extern uint32_t  *eccRom_param_Gy;

/* NIST P-256 Curves in ROM
 Note: these are actually strings*/
extern uint32_t NIST_Curve_P256_p;
extern uint32_t NIST_Curve_P256_r;
extern uint32_t NIST_Curve_P256_a;
extern uint32_t NIST_Curve_P256_b;
extern uint32_t NIST_Curve_P256_Gx;
extern uint32_t NIST_Curve_P256_Gy;

/*********************************************************************
 * TYPEDEFS
 */

/*! ECC public key pair */
typedef struct {
  uint8_t pubKeyX[32];
  uint8_t pubKeyY[32];
} secure_fw_eccKey_t;

/*! Cert Payload */
typedef union {
  secure_fw_eccKey_t eccKey;
  uint8_t  aes_key[2];
} secure_fw_cert_payload_t;

/*! Cert Element */
PACKED_TYPEDEF_STRUCT
{
  uint8_t type;
  uint8_t length;
  uint16_t options;
  uint8_t signerInfo[8];
  secure_fw_cert_payload_t certPayload;
} secure_fw_cert_element_t;

/*! ECC signature pair*/
typedef struct {
  uint8_t  sign_r[32];
  uint8_t  sign_s[32];
} secure_fw_ecc_signature_t;

/*! Sign payload in sign element*/
typedef union {
  secure_fw_ecc_signature_t eccSignature;
  uint8_t  aes_cbc_mac[16];
} secure_fw_sign_payload_t;

/*!Sign element */
typedef struct {

  uint8_t type;
  uint8_t timeStamp[4];
  uint8_t signerInfo[8];
  secure_fw_sign_payload_t signPayload;
} secure_fw_sign_element_t;

/*! BFV header populated from image metadata*/
typedef struct {
  uint8_t magicWord[SECURE_FW_MAGIC_LEN];
  /* Image version */
  uint8_t ver[4];
  /* Image length in 4-byte blocks (i.e. HAL_FLASH_WORD_SIZE blocks) */
  uint8_t len[4];
  uint8_t imgStartAddr[4];
  uint8_t imgEndAddr[4];
  secure_fw_sign_element_t signElement;
} secure_fw_hdr_t;

/*!BFV configuration parameters */
typedef struct {
  uint8_t selfCheck;
  uint8_t secureFWVerify;
  uint8_t fwRollback;
  secure_fw_cert_element_t certElement;
} secure_fw_config_t;

/*! ECC NIST Curve Parameters */
typedef struct ECCROMCC26XX_CurveParams
{
    /*!<  Length in bytes of curve parameters and keys */
    uint8_t         keyLen;
    /*!<  Length in bytes of workzone to allocate      */
    uint16_t        workzoneLen;
    /*!<  Window size of operation                     */
    uint8_t         windowSize;
    /*!<  ECC Curve Parameter P                        */
    uint32_t        *param_p;
    /*!<  ECC Curve Parameter R                        */
    uint32_t        *param_r;
    /*!<  ECC Curve Parameter A                        */
    uint32_t        *param_a;
    /*!<  ECC Curve Parameter B                        */
    uint32_t        *param_b;
    /*!<  ECC Curve Parameter Gx                       */
    uint32_t        *param_gx;
    /*!<  ECC Curve Parameter Gy                       */
    uint32_t        *param_gy;
} ECCROMCC26XX_CurveParams;

typedef struct ECCROMCC26XX_Params {
    ECCROMCC26XX_CurveParams curve;   /*!< ECC Curve Parameters   */
    int8_t                   status;  /*!< stored return status   */
} ECCROMCC26XX_Params;
extern secure_fw_hdr_t secure_fw_header_info;
extern secure_fw_config_t secure_fw_config_info;

 /*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      secure_fw_verifyImage_ecc
 *
 * @brief   Using ECDSA verify function in ROM, verify with header signature
 *          using the public keys in the secure_fw configuration
 *
 * @return  TRUE to continue to execute otherwise FALSE to reject the image.
 */
extern int8_t secure_fw_verifyImage_ecc(void);

/*********************************************************************
 * @fn      secure_fw_checkCertElement
 *
 * @brief   Check if there is a matching sign element for the signer info
 *          in metadata cert element
 * @return  TRUE to continue to next step otherwise FALSE to reject the image.
 */
extern uint8_t secure_fw_checkCertElement(void);

#if SECURE_FW_FW_ROLLBACK
/*********************************************************************
 * @fn      secure_fw_checkVersion
 *
 * @brief   Check if the new image version is allowable
 *
 * @return  TRUE to continue to next step otherwise FALSE to reject the image.
 */
extern uint8_t secure_fw_checkVersion(void);
#endif /* SECURE_FW_FW_ROLLBACK */

/*********************************************************************
 * @fn      secure_fw_exception
 *
 * @brief   Handle actions to be done if an exception is thrown at any of
 *          the steps of BFV
 */
extern void secure_fw_exception(secure_fw_errorcode_t errorCode);

/*********************************************************************
 * @fn      secure_fw_init
 *
 * @brief   Populate information from BFV configuration and image metadata
 *          needed for BFV to begin the verfication process
 */
extern void secure_fw_init(void);

/*********************************************************************
 * @fn      secure_fw_compare
 *
 * @brief   Compare contents of two memory location for given length
 * @param   pData1   - pointer to first memory location
 * @param   pData2   - pointer to second memory location
 * @param
 *
 * @return  0 if all content matches else 1
 */
extern int secure_fw_compare(uint8_t *pData1, const uint8_t *pData2, uint8_t len);

/*********************************************************************
 * @fn      secure_fw_checkAddrSanity
 *
 * @brief   Determine if a new image has a valid start and end address, and
 *          length
 *
 * @return  secure_fw_error_invalid_addr if either address is invalid or
 *          secure_fw_error_invalid_fw_length if length is invalid or
 *          secure_fw_error_no_error
 */
secure_fw_errorcode_t secure_fw_checkAddrSanity(void);

/*********************************************************************
 * @fn      secure_fw_readFlash
 *
 * @brief   Read data from flash.
 *
 * @param   page   - page to read from in flash
 * @param   offset - offset into flash page to begin reading
 * @param   pBuf   - pointer to buffer into which data is read.
 * @param   len    - length of data to read in bytes.
 */
extern void secure_fw_readFlash(uint8_t page, uint32_t offset,
                               uint8_t *pBuf, uint16_t len);

/*********************************************************************
 * @fn      secure_fw_writeFlash
 *
 * @brief   Write data to flash.
 *
 * @param   page   - page to write to in flash
 * @param   offset - offset into flash page to begin writing
 * @param   pBuf   - pointer to buffer of data to write
 * @param   len    - length of data to write in bytes
 */
extern void secure_fw_writeFlash(uint8_t page, uint32_t offset,
                                uint8_t *pBuf, uint16_t len);

/*********************************************************************
 * @fn      secure_fw_eraseFlash
 *
 * @brief   Erase selected flash page.
 *
 * @param   page - the page to erase.
 */
extern void secure_fw_eraseFlash(uint8_t page);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SECURE_FW_H */

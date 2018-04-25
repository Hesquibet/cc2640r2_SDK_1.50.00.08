/******************************************************************************

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

/**
 *  @defgroup LinkDB LinkDB
 *  @brief This module implements the Link Database Module
 *  @{
 *  @file       linkdb.h
 *  @brief      LinkDB layer interface
 */

#ifndef LINKDB_H
#define LINKDB_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/** @defgroup LinkDB_Constants LinkDB Constants
 * @{
 */
// Special case connection handles
#define INVALID_CONNHANDLE              0xFFFF  //!< Invalid connection handle, used for no connection handle
#define LOOPBACK_CONNHANDLE             0xFFFE  //!< Loopback connection handle, used to loopback a message

/** @defgroup LinkDB_States LinkDB Connection State Flags
 * @{
 */
// Link state flags
#define LINK_NOT_CONNECTED              0x00    //!< Link isn't connected
#define LINK_CONNECTED                  0x01    //!< Link is connected
#define LINK_AUTHENTICATED              0x02    //!< Link is authenticated
#define LINK_BOUND                      0x04    //!< Link is bonded
#define LINK_ENCRYPTED                  0x10    //!< Link is encrypted
#define LINK_SECURE_CONNECTIONS         0x20    //!< Link uses Secure Connections
#define LINK_IN_UPDATE                  0x40    //!< Link is in update procedure
/** @} End LinkDB_States  */

// Link Database Status callback changeTypes
#define LINKDB_STATUS_UPDATE_NEW        0       //!< New connection created
#define LINKDB_STATUS_UPDATE_REMOVED    1       //!< Connection was removed
#define LINKDB_STATUS_UPDATE_STATEFLAGS 2       //!< Connection state flag changed

// Link Authentication Errors
#define LINKDB_ERR_INSUFFICIENT_AUTHEN      0x05  //!< Link isn't even encrypted
#define LINBDB_ERR_INSUFFICIENT_KEYSIZE     0x0c  //!< Link is encrypted but the key size is too small
#define LINKDB_ERR_INSUFFICIENT_ENCRYPTION  0x0f  //!< Link is encrypted but it's not authenticated
/** @} End LinkDB_Constants  */

/*********************************************************************
 * TYPEDEFS
 */

/** @defgroup LinkDB_Structures LinkDB Data Structures
 * @{
 */
/**
 * @brief SRK and Sign Counter
 */
typedef struct
{
  uint8 srk[KEYLEN];  //!< Signature Resolving Key
  uint32 signCounter; //!< Sign Counter
} linkSec_t;

/**
 * @brief Encryption Params
 */
typedef struct
{
  uint8 ltk[KEYLEN];             //!< Long Term Key
  uint16 div;                    //!< Diversifier
  uint8 rand[B_RANDOM_NUM_SIZE]; //!< random number
  uint8 keySize;                 //!< LTK Key Size
} encParams_t;

/**
 * @brief linkDB item
 */
typedef struct
{
  uint8 taskID;                 //!< Application that controls the link
  uint16 connectionHandle;      //!< Controller connection handle
  uint8 stateFlags;             //!< LINK_CONNECTED, LINK_AUTHENTICATED...
  uint8 addrType;               //!< Address type of connected device
  uint8 addr[B_ADDR_LEN];       //!< Other Device's address
  uint8 addrPriv[B_ADDR_LEN];   //!< Other Device's Private address
  uint8 connRole;               //!< Connection formed as Master or Slave
  uint16 connInterval;          //!< The connection's interval (n * 1.23 ms)
  uint16 MTU;                   //!< The connection's MTU size
  linkSec_t sec;                //!< Connection Security related items
  encParams_t *pEncParams;      //!< pointer to LTK, ediv, rand. if needed.
} linkDBItem_t;

/**
 * @brief linkDB info
 */
typedef struct
{
  uint8 stateFlags;             //!< LINK_CONNECTED, LINK_AUTHENTICATED...
  uint8 addrType;               //!< Address type of connected device
  uint8 addr[B_ADDR_LEN];       //!< Other Device's address
  uint8 addrPriv[B_ADDR_LEN];   //!< Other Device's Private address
  uint8 connRole;               //!< Connection formed as Master or Slave
  uint16 connInterval;          //!< The connection's interval (n * 1.23 ms)
  uint16 MTU;                   //!< The connection's MTU size
} linkDBInfo_t;
/** @} End LinkDB_Structures  */

/** @defgroup LinkDB_CBs LinkDB Callbacks
 * @{
 */
/// @brief function pointer used to register for a status callback
typedef void (*pfnLinkDBCB_t)( uint16 connectionHandle, uint8 changeType );

/// @brief function pointer used to perform specialized link database searches
typedef void (*pfnPerformFuncCB_t)( linkDBItem_t *pLinkItem );
/** @} End LinkDB_CBs */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/// @cond NODOC

extern uint8 linkDBNumConns;

/// @endcond NODOC

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief   Initialize the Link Database
 */
  extern void linkDB_Init( void );

/**
 * @brief       Register with linkDB
 *
 *              Register with this function to receive a callback when
 *              status changes on a connection.  If the stateflag == 0,
 *              then the connection has been disconnected.
 *
 * @param       pFunc - function pointer to callback function
 *
 * @return      @ref SUCCESS if successful
 * @return      @ref bleMemAllocError if no table space available
 *
 */  extern uint8 linkDB_Register( pfnLinkDBCB_t pFunc );

/**
 * @brief       Adds a record to the link database.
 *
 * @param       taskID - Application task ID
 * @param       connectionHandle - new record connection handle
 * @param       stateFlags - @ref LinkDB_States
 * @param       addrType - @ref Addr_type
 * @param       pAddr - new address
 * @param       pAddrPriv - private address (only if addrType is 0x02 or 0x03)
 * @param       connRole - @ref GAP_Profile_Roles
 * @param       connInterval - connection's communications interval (n * 1.23 ms)
 * @param       MTU - connection's MTU size
 *
 * @return      @ref SUCCESS if successful
 * @return      @ref bleIncorrectMode - hasn't been initialized.
 * @return      @ref bleNoResources - table full
 * @return      @ref bleAlreadyInRequestedMode - already exist connectionHandle
 *
 */
  extern uint8 linkDB_Add( uint8 taskID, uint16 connectionHandle,
                           uint8 stateFlags, uint8 addrType, uint8 *pAddr,
                           uint8 *pAddrPriv, uint8 connRole, uint16 connInterval,
                           uint16 MTU );

/**
 * @brief       Remove a record from the link database.
 *
 * @param       connectionHandle - new record connection handle
 *
 * @return      @ref SUCCESS if successful
 * @return      @ref INVALIDPARAMETER - connectionHandle not found.
 *
 */
  extern uint8 linkDB_Remove( uint16 connectionHandle );

/**
 * @brief       Update the stateFlags of a link record.
 *
 * @param       connectionHandle - maximum number of connections.
 * @param       newState - @ref LinkDB_States.  This value is OR'd in
 *                         to this field.
 * @param       add - TRUE to set state into flags, FALSE to remove.
 *
 * @return      @ref SUCCESS if successful
 * @return      @ref bleNoResources - connectionHandle not found.
 *
 */
  extern uint8 linkDB_Update( uint16 connectionHandle, uint8 newState,
                              uint8 add );


/**
 * @brief       Return the number of active connections
 *
 * @return      the number of active connections
 *
 */
  extern uint8 linkDB_NumActive( void );

/**
 * @brief       return the maximum number of connections supported.
 *
 * @return      the number of connections supported
 *
 */
  extern uint8 linkDB_NumConns( void );

/**
 * @brief       update the MTU size of a link or record.
 *
 * @param       connectionHandle - controller link connection handle.
 * @param       newMtu - new MTU size.
 *
 * @return      @ref SUCCESS or failure
 *
 */
  extern uint8 linkDB_UpdateMTU( uint16 connectionHandle, uint16 newMtu );

/**
 * @brief       This function is used to get the MTU size of a link.
 *
 * @param       connectionHandle - controller link connection handle.
 *
 * @return      link MTU size
 */
  extern uint16 linkDB_MTU( uint16 connectionHandle );

/**
 * @brief       Get information about a link
 *
 *              Copies relevant link info into pInfo.  Uses the connection
 *              handle to search the link database.
 *
 * @param       connectionHandle - controller link connection handle.
 * @param       pInfo - address of link info item to copy information
 *
 * @return      @ref SUCCESS if connection found else FAILURE
 */
  extern uint8 linkDB_GetInfo( uint16 connectionHandle, linkDBInfo_t * pInfo );

/**
 * @brief       Find the link.
 *
 *              Uses the connection handle to search
 *              the link database.
 *
 * @param       connectionHandle - controller link connection handle.
 *
 * @return      a pointer to the found link item
 * @return      NULL if not found
 */
  extern linkDBItem_t *linkDB_Find( uint16 connectionHandle );

/**
 * @brief       Find the first link that matches the taskID.
 *
 * @param       taskID - taskID of app
 *
 * @return      a pointer to the found link item
 * @return      NULL if not found
 */
  extern linkDBItem_t *linkDB_FindFirst( uint8 taskID );

/**
 * @brief       Check to see if a physical link is in a specific state.
 *
 * @param       connectionHandle - controller link connection handle.
 * @param       state - @ref LinkDB_States state to look for.
 *
 * @return      TRUE if the link is found and state is set in state flags.
 * @return      FALSE, otherwise.
 */
  extern uint8 linkDB_State( uint16 connectionHandle, uint8 state );

/**
 * @brief       Check to see if the physical link is encrypted and authenticated.
 *
 * @param       connectionHandle - controller link connection handle.
 * @param       keySize - size of encryption keys.
 * @param       mitmRequired - TRUE (yes) or FALSE (no).
 *
 * @return      @ref SUCCESS if the link is authenticated
 * @return      @ref bleNotConnected - connection handle is invalid
 * @return      @ref LINKDB_ERR_INSUFFICIENT_AUTHEN - link is not encrypted
 * @return      @ref LINBDB_ERR_INSUFFICIENT_KEYSIZE - key size encrypted is not large enough
 * @return      @ref LINKDB_ERR_INSUFFICIENT_ENCRYPTION - link is encrypted, but not authenticated
 */
  extern uint8 linkDB_Authen( uint16 connectionHandle, uint8 keySize,
                              uint8 mitmRequired );

/**
 * @brief       Get the role of a physical link.
 *
 * @param       connectionHandle - controller link connection handle.
 *
 * @return      @ref GAP_Profile_Roles
 * @return      0 - unknown
 */
  extern uint8 linkDB_Role( uint16 connectionHandle );

/**
 * @brief       Perform a function of each connection in the link database.
 *
 * @param       cb - connection callback function.
 */
  extern void linkDB_PerformFunc( pfnPerformFuncCB_t cb );

/**
 * @brief       Set a device into Secure Connection Only Mode.
 *
 * @param       state -  TRUE for Secure Connections Only Mode. <br>
 *              FALSE to disable Secure Connections Only Mode.
 */
  extern void linkDB_SecurityModeSCOnly( uint8 state );

/*********************************************************************
 * LINK STATE MACROS
 * Developer note:  Call from Application only.  Do not call from Stack,
 * not supported for ROM patching!
 */

  /**
   * @brief Check to see if a physical link is up (connected).
   *
   * @param connectionHandle - controller link connection handle.
   *
   * @return TRUE if the link is up
   * @return FALSE, otherwise.
   */
  #define linkDB_Up( connectionHandle )  linkDB_State( (connectionHandle), LINK_CONNECTED )

  /**
   * @brief     Check to see if the physical link is encrypted.
   *
   * @param     connectionHandle - controller link connection handle.
   *
   * @return    TRUE if the link is encrypted
   * @return    FALSE, otherwise.
   */
  #define linkDB_Encrypted( connectionHandle )  linkDB_State( (connectionHandle), LINK_ENCRYPTED )

  /**
   * @brief     Check to see if the physical link is authenticated.
   *
   * @param     connectionHandle - controller link connection handle.
   *
   * @return    TRUE if the link is authenticated
   * @return    FALSE, otherwise.
   */
  #define linkDB_Authenticated( connectionHandle )  linkDB_State( (connectionHandle), LINK_AUTHENTICATED )

  /**
   * @brief     Check to see if the physical link is bonded.
   *
   * @param     connectionHandle - controller link connection handle.
   *
   * @return    TRUE if the link is bonded
   * @return    FALSE, otherwise.
   */
  #define linkDB_Bonded( connectionHandle )  linkDB_State( (connectionHandle), LINK_BOUND )

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* LINKDB_H */

/** @} End LinkDB */

/******************************************************************************

@file       multi_role_menu.c

@brief This file contains the multi_role menu configuration for use
with the CC2650 Bluetooth Low Energy Protocol Stack.

Group: CMCU, SCS
Target Device: CC2640R2

******************************************************************************

 Copyright (c) 2013-2017, Texas Instruments Incorporated
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

#include <bcomdef.h>
#include <ti/display/Display.h>
#include "board_key.h"

#include <menu/two_btn_menu.h>
#include "multi_role_menu.h"
#include "multi_role.h"

/*
 * Menu Lists Initializations
 */

// Menu: Main
// 2 actions, 5 submenus, no upper
MENU_OBJ(mrMenuMain, "Main Menu", 6, NULL)
  MENU_ITEM_ACTION("Scan", mr_doScan)
  MENU_ITEM_SUBMENU(&mrMenuConnect)
  MENU_ITEM_SUBMENU(&mrMenuGattRw)
  MENU_ITEM_SUBMENU(&mrMenuConnUpdate)
  MENU_ITEM_SUBMENU(&mrMenuDisconnect)
  MENU_ITEM_ACTION("Advertise", mr_doAdvertise)
MENU_OBJ_END

// Menu: Connect
// Initialize for 15 possible actions. This will be
// reconfigured throughout the application code
MENU_OBJ(mrMenuConnect, "Connect", 15, &mrMenuMain)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
  MENU_ITEM_ACTION("", mr_doConnect)
MENU_OBJ_END

// Menu: GATT_RW
// Initialize for 8 possible actions. This will be
// reconfigured throughout the application code
MENU_OBJ(mrMenuGattRw, "GATT R/W", 8, &mrMenuMain)
  MENU_ITEM_ACTION("", mr_doGattRw)
  MENU_ITEM_ACTION("", mr_doGattRw)
  MENU_ITEM_ACTION("", mr_doGattRw)
  MENU_ITEM_ACTION("", mr_doGattRw)
  MENU_ITEM_ACTION("", mr_doGattRw)
  MENU_ITEM_ACTION("", mr_doGattRw)
  MENU_ITEM_ACTION("", mr_doGattRw)
  MENU_ITEM_ACTION("", mr_doGattRw)
MENU_OBJ_END

// Menu: Connection Update
// Initialize for 8 possible actions. This will be
// reconfigured throughout the application code
MENU_OBJ(mrMenuConnUpdate, "Conn Update", 8, &mrMenuMain)
  MENU_ITEM_ACTION("", mr_doConnUpdate)
  MENU_ITEM_ACTION("", mr_doConnUpdate)
  MENU_ITEM_ACTION("", mr_doConnUpdate)
  MENU_ITEM_ACTION("", mr_doConnUpdate)
  MENU_ITEM_ACTION("", mr_doConnUpdate)
  MENU_ITEM_ACTION("", mr_doConnUpdate)
  MENU_ITEM_ACTION("", mr_doConnUpdate)
  MENU_ITEM_ACTION("", mr_doConnUpdate)
MENU_OBJ_END

// Menu: Disconnect
// Initialize for 8 possible actions. This will be
// reconfigured throughout the application code
MENU_OBJ(mrMenuDisconnect, "Disconnect", 8, &mrMenuMain)
  MENU_ITEM_ACTION("", mr_doDisconnect)
  MENU_ITEM_ACTION("", mr_doDisconnect)
  MENU_ITEM_ACTION("", mr_doDisconnect)
  MENU_ITEM_ACTION("", mr_doDisconnect)
  MENU_ITEM_ACTION("", mr_doDisconnect)
  MENU_ITEM_ACTION("", mr_doDisconnect)
  MENU_ITEM_ACTION("", mr_doDisconnect)
  MENU_ITEM_ACTION("", mr_doDisconnect)
MENU_OBJ_END

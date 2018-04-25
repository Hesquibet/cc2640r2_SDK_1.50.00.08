"""
/******************************************************************************
 @file    npi_cm_host.py

 @brief This example script can be used to control a TI BLE 3.1.0 HostTest device
  as a BLE central device as well as a connection monitor device with UART
  commands. The script acts as a PC based host controlling who the host_test
  app joins and which connection the connection monitor will attempt to
  monitor.

  NOTE: To run this script the user should install python 2.7 as well as an
  up to date pyserial plugin for python 2.7.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2017-2017, Texas Instruments Incorporated
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
"""

import argparse
import serial
import binascii
import threading
import time
import sys

__version__ = "1.0.0"

#Maintain connection monitor return data globally for shared use
cmInfoOutPut = []

#set of peripheral devices to join to. BLE address should be fixed for a given launchpad
#You will need to update with your own peripheral device addresses since it is unique
#for every device.
peerAddrs = []

HCI_HEADER_LEN = 3
HCI_COMMAND_COMPLETE_EVENT = 0x0E

# Setup connected host_test devices with arbitrary keys prior to connection establishment
PIXIT_IRK_MASTER    = [0x33,0x42,0xCF,0x14,0xBC,0x55,0x17,0x31,0x75,0x4F,0xBB,0xA4,0xC7,0xF2,0x8C,0x13]
PIXIT_CSRK_MASTER   = [45,0x0A,0xF4,0xB0,0x03,0x07,0xB0,0x40,0x87,0xF4,0x18,0x23,0x75,0x4A,0xFB,0xA4]
GAP_PROFILE_CENTRAL = 0x08


# Parse and print return values hci uart interface on host_test application
def parse_cm_conn_info(data):
    #advance index to data start position
    rtn_val = []

    rtn_val.append(int(data[11],16))
    rtn_val.append(int(data[10],16))
    rtn_val.append(int(data[9],16))
    rtn_val.append(int(data[8],16))
    print "Access Address: 0x" + str(data[11]+data[10]+data[9]+data[8])

    rtn_val.append(int(data[13],16))
    rtn_val.append(int(data[12],16))
    print "Connection Interval: " + str(int("0x"+str(data[13]+data[12]),16))

    rtn_val.append(int(data[14],16))
    print "Hop Value: 0x" + str(data[14])

    rtn_val.append(int(data[16],16))
    rtn_val.append(int(data[15],16))
    print "Master Clock Accuracy: 0x" + str(data[16]+data[15])

    rtn_val.append(int(data[17],16))
    print "next Channel: 0x" + str(data[17])

    rtn_val.append(int(data[22],16))
    rtn_val.append(int(data[21],16))
    rtn_val.append(int(data[20],16))
    rtn_val.append(int(data[19],16))
    rtn_val.append(int(data[18],16))
    print "Channel Map: 0x" + str(data[22]+data[21]+data[20]+data[19]+data[18])

    return (rtn_val)

def print_opcode_info(opCode, status, packet):
    returnValue = []
    print "OpCode: "+str(opCode)
    print "Status" +str(status)

    if (opCode == "0x041d"):
        print "Reset Event"
    elif (opCode == "0x067f"):
        print "HCI_EXT_GAP_CMD_STATUS_EVENT"
    elif (opCode == "0x0600"):
        print "HCI_EXT_GAP_DEVICE_INIT_DONE_EVENT"
    elif (opCode == "0x0605"):
        print "GAP_LinkEstablished"
    elif (opCode == "0x0607"):
        print "GAP_LinkParamUpdate"
    elif (opCode == "0x0424"):
        print "HCI_EXT_GET_ACTIVE_CONNECTION_INFO_EVENT"
        returnValue = parse_cm_conn_info(packet)

    print packet
    return returnValue

# Read one return packet from HCI...
def read_from_port(serPort):
    global cmInfoOutPut
    while True:
        if (serPort.inWaiting()):
            opCode = 0
            status = 'NULL'
            incoming_packet = []
            rtn_value = []
            print "-----------------------------------------------"
            while (serPort.inWaiting()):
                data_str = str(serPort.read(serPort.inWaiting()).encode('hex'))
                data_str = [data_str[i:i + 2] for i in range(0, len(data_str), 2)]
                incoming_packet += data_str
                for i in range(len(data_str)):
                    if data_str[i] == '04':
                        opCode = "0x"+data_str[i+4] + data_str[i+3]
                        if data_str[i+5] == '00':
                            status = 'SUCCESS'
                        else:
                            status = str(data_str[i+5])
                        break
                time.sleep(0.01)

            rtn_value = print_opcode_info(opCode,status,incoming_packet)
            if (rtn_value != []):
                cmInfoOutPut = rtn_value
            print "-----------------------------------------------"


# Parse and print return values from uNPI on connection monitor
def read_from_port_unpi(serPort):
    while True:
        if (serPort.inWaiting()):
            opCode = 0
            status = 'NULL'
            incoming_packet = []
            #print "-----------------------------------------------"
            while (serPort.inWaiting()):
                data_str = str(serPort.read(serPort.inWaiting()).encode('hex'))
                #Break up all incoming serial port data into nibbles
                data_str = [data_str[i:i + 2] for i in range(0, len(data_str), 2)]
                incoming_packet += data_str
                time.sleep(0.01)

            #First make sure the return packet is cmId 0x04 and starts with valid 0xFE tag
            if(incoming_packet[0] == 'fe' and \
            incoming_packet[4] == '04' and \
            len(incoming_packet)>=11):

                # OUTPUT[1]: Below is the Human Readable output of the CM, comment out
                # and enable alternate output for CSV formate
                timeStamp1 = int(incoming_packet[10]+incoming_packet[9]+
                                 incoming_packet[8]+incoming_packet[7],16)

                print "Master "+str(int(incoming_packet[5],16))+\
                      " RSSI -"+str(255-int(incoming_packet[6],16)) +\
                      " time: "+str(timeStamp1/4000000.0) +" seconds"
                if (len(incoming_packet)>12):
                    timeStamp2 = int(incoming_packet[22]+incoming_packet[21]+
                    incoming_packet[20]+incoming_packet[19],16)

                    print "Slave     "+str(int(incoming_packet[17],16))+\
                                " RSSI -"+str(255-int(incoming_packet[18],16))+\
                                " time: "+str(timeStamp2/4000000.0) +" seconds"

                # OUTPUT[2]: Below is the CSV output of the CM:
                # Format: deviceType, id, rssi, timeStamp seconds
                # Example:
                # master, 1, -26, 39.8925
                # slave, 1, -35, 40.00000
                '''
                timeStamp1 = int(incoming_packet[10]+incoming_packet[9]+
                                 incoming_packet[8]+incoming_packet[7],16)

                print "master, "+str(int(incoming_packet[5],16))+", "+
                                     str(255-int(incoming_packet[6],16))+", "+
                                     str(timeStamp1/4000000.0)+", "
                if (len(incoming_packet)>12):
                    timeStamp2 = int(incoming_packet[22]+incoming_packet[21]+
                                     incoming_packet[20]+incoming_packet[19],16)

                    print "slave, "+str(int(incoming_packet[17],16))+", "+
                                        str(255-int(incoming_packet[18],16))+", "+
                                        str(timeStamp2/4000000.0)+", "
                '''
            else:
                print "Raw CM output: "+str(incoming_packet)
            #print "-----------------------------------------------"

def hci_reset(com):
    msg = bytearray([0x01, 0x1d, 0xfc, 0x01, 0x00])
    print "Writing to " + serial_port + ": " + binascii.hexlify(msg)
    com.write(msg)

def send_gap_deviceInit(com, profileRole, maxScanRsps, IRK, CSRK, signCounter):
    opCode = [0x00,0xfe]
    sCnt1 = signCounter & 0xff
    sCnt2 = (signCounter >> 8 )& 0xff
    sCnt3 = (signCounter >> 16 ) & 0xff
    sCnt4 = (signCounter >> 24 ) & 0xff
    length = 38

    msg = bytearray([0x01]+opCode+ [length, profileRole,maxScanRsps]+ IRK+ CSRK+[sCnt1,sCnt2,sCnt3,sCnt4])
    print "Outgoing gap_device_init "+binascii.hexlify(msg)
    com.write(msg)

def send_gap_establishLinkReq(com, peripheralAddress):
    opCode = [0x09,0xfe]
    length = 9
    highDutyCycle=0
    whiteList=0
    addrTypePeer=0

    msg = bytearray([0x01]+opCode+ [length, highDutyCycle,whiteList,addrTypePeer]+ peripheralAddress)
    print "Outgoing gap_establishLinkReq "+binascii.hexlify(msg)
    com.write(msg)

def hci_getCmConnInfo(com, connId):
    opCode = [0x24,0xfc]
    length = 1

    msg = bytearray([0x01]+opCode+ [length, connId])
    print "Outgoing hci_getCmConnInfo "+binascii.hexlify(msg)
    com.write(msg)

def serializeUNpiMsg(payload):
    length = len(payload)-2
    len0 = length % 256
    len1 = length // 256
    fcs = length
    for x in range(0,len(payload)):
            fcs ^= payload[x]
    return bytearray([0xFE,len0,len1] + payload + [fcs])

def cmStart(com, payload):
    uNpiAysncReq = [0x57]
    cmdId        = [0x01]
    length = 1

    msg = serializeUNpiMsg((uNpiAysncReq+cmdId+payload))
    print "Outgoing cmStart() cmd "+binascii.hexlify(msg)
    com.write(msg)

def setPeerAddress(bleAddressString):
    if(bleAddressString[:2]=='0x'):
        #trim off '0x' prefix
        bleAddressString = bleAddressString[2:]
        array = [int(bleAddressString[i:i + 2],16) for i in range(0, len(bleAddressString), 2)]
    else:
        array = [int(bleAddressString[i:i + 2],16) for i in range(0, len(bleAddressString), 2)]

    #return reverse of array
    return array[::-1]

def main():
    global cmInfoOutPut, peerAddrs

    #Init and connect 1st Host device, after this sequence of 2 calls you should see
    # an return packet that says: GAP_LinkEstablished with a Status above printed as 0 or success
    send_gap_deviceInit(host1, GAP_PROFILE_CENTRAL, 20, PIXIT_IRK_MASTER, PIXIT_CSRK_MASTER, 0)
    time.sleep(1)
    send_gap_establishLinkReq(host1,peerAddrs)
    time.sleep(1)

    #Query connection infromation from BLE Central Host to kickstart the connection monitor
    hci_getCmConnInfo(host1,0)
    time.sleep(.1)
    cmStart(cm, cmInfoOutPut)


def parse_args():
    script_description = "Example Host Control for BLE Central Device and Connection Monitor"
    parser = argparse.ArgumentParser(prog='npi_cmd_host',
                                                                     description=script_description)
    parser.add_argument("-v", "--version", action="version",
                                            version="%(prog)s " + __version__)
    parser.add_argument("cm_port", default='', nargs='?', help="ComPort of Connection Monitor")
    parser.add_argument("host_port", default='', nargs='?', help="ComPort of Host_Test Device")
    parser.add_argument("peerAddress", default='', nargs='?', help="BLE Address of peripheral")
    args = parser.parse_args()

    return args

def version_control():
    print sys.version
    if sys.version[:3] != "2.7":
            raise Exception("Must be using Python 2.7!")

if __name__ == '__main__':
    global peerAddrs
    version_control()
    rtn_args = parse_args()

    if(len(sys.argv) < 4):
        print '\nUsing Default Hardcoded Values: '
        print 'or enter 3 default(cm_port host_port peerAddress) arguments.'
        print 'See -h help.\n'
        #set your own hard coded com ports
        host_port = 'COM128'
        cm_port     = 'COM158'
        strPeerAddress = '0xB0912265CBE2'
        peerAddrs = setPeerAddress(strPeerAddress)
        print "Host comport is: "+host_port
        print "Connection Monitor comport is: "+cm_port
        print "Peer Address is: "+strPeerAddress
    else:
        host_port = rtn_args.host_port
        cm_port     = rtn_args.cm_port
        print "\nHost comport is: "+host_port
        print "Connection Monitor comport is: "+cm_port
        peerAddrs = setPeerAddress(rtn_args.peerAddress)
        print "Peer Address is: "+rtn_args.peerAddress

    #Kick off a thread for each com to monitor Rx data over UART
    host1 = serial.Serial(port=host_port,baudrate=115200)
    thread1 = threading.Thread(target=read_from_port, args=(host1,))
    thread1.start()

    cm = serial.Serial(port=cm_port,baudrate=115200)
    thread2 = threading.Thread(target=read_from_port_unpi, args=(cm,))
    thread2.start()

    main()

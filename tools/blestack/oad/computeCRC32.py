"""
/******************************************************************************
 @file  computeCRC32.py

 @brief This tool is used as a helper file for OAD Image Tool

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
import zlib


def computeCRC32(argv=None):
    '''
    Computes CRC32 over entire file and updates the image header
    '''
    argLen = len (argv)

    if argLen != 3:
        print "'computeCRC32'::Usage: 1:Filename, 2: dataStartOffset, " \
              "3:crcOffset"
        return 1

    fileName = argv[0]
    dataStartOffset = int(argv[1])
    crcOffset = int(argv[2])

    crcbytes = crc32_withOffset(fileName, dataStartOffset)
    updateCRCbytes(fileName, crcbytes, crcOffset)

    return 0


def crc32(fileName):
    '''
    Computes CRC32 over entire file
    '''
    filePtr = open(fileName,"rb")
    content = filePtr.readlines()
    filePtr.close()
    crc = None
    for eachLine in content:
      if not crc:
          crc = zlib.crc32(eachLine)
      else:
          crc = zlib.crc32(eachLine, crc)

    # Return 8 digits crc
    return format((crc & 0xFFFFFFFF), '08x')


def crc32_withOffset(fileName, dataOffset):
    '''
    Computes CRC32 over entire file
    '''
    filePtr = open(fileName,"rb")
    filePtr.seek(dataOffset, 0)  # seek to image data offset file

    content = filePtr.readlines()
    filePtr.close()
    crc = None
    for eachLine in content:
      if not crc:
          crc = zlib.crc32(eachLine)
      else:
          crc = zlib.crc32(eachLine, crc)

    # Return 8 digits crc
    return format((crc & 0xFFFFFFFF), '08x')


def HexToByte(hexStr):
    bytes = []

    hexStr = ''.join( hexStr.split(" ") )

    for i in range(0, len(hexStr), 2):
            bytes.append( chr( int (hexStr[i:i+2], 16 ) ) )

    return ''.join(bytes)


def int2byte(val, width=32):
    return ''.join([chr((val >> 8*n) & 255) for n in reversed(range(width/8))])


def int2byte1(val, width=32):
    bytes = [0] * 4

    for n in reversed(range(width/8)):
        bytes[n] = ((val >> 8*n) & 255)

    return bytes


def updateCRCbytes(fileName, crcbytes, crcOffset):
    x = int(crcbytes, 16)

    crcByteStr = (int2byte1(x))

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')

    # Rewind back to the start
    filePtr.seek(0, 0)
    filePtr.seek(crcOffset, 0)  # Seek to file start offset

    filePtr.write(chr(crcByteStr[0]))
    filePtr.write(chr(int(crcByteStr[1])))
    filePtr.write(chr(int(crcByteStr[2])))
    filePtr.write(chr(int(crcByteStr[3])))

    filePtr.close()

    return

"""
/******************************************************************************
 @file  imgBinUtil.py

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
import struct

fixedHdrLen  = 44
boundrySegLen = 24
payloadSegLenOffset = fixedHdrLen + boundrySegLen + 4

def fileTypeTag(argv=None):
    argLen = len (argv)

    if argLen != 3:
        print "'fileTypeTag'::Usage: 1:Filename, 2:FileType, 3:offset"
        return 1

    fileName = argv[0]
    fileType = int(argv[1])
    fileOffset = int(argv[2])

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')
    filePtr.seek(fileOffset, 0)  # Seek to file offset file
    filePtr.write(struct.pack('h', fileType))

    filePtr.close()


def computeAddr(addrbytes):
    adrTemp = ' '.join([str(ord(a)) for a in addrbytes]).split(' ')

    # Fourth byte will be zero for cc26xx
    addr = int(adrTemp[0]) + 256*int(adrTemp[1]) + 65536*int(adrTemp[2])

    return addr


def HexToByte( hexStr ):
    bytes = []

    hexStr = ''.join(hexStr.split(" "))

    for i in range(0, len(hexStr), 2):
        bytes.append(chr(int(hexStr[i:i+2], 16)))

    return ''.join(bytes)


def int2byte(val, width=32):
    return ''.join([chr((val >> 8*n) & 255) for n in reversed(range(width/8))])


def int2byte1(val, width=32):
    bytes = [0] * 4
    for n in reversed(range(width/8)):
        bytes[n] = ((val >> 8*n) & 255)

    return bytes


def updateImageLen(argv=None):
    argLen = len (argv)

    if argLen != 4:
        print "'updateImageLen'::Usage: 1:Filename, 2:imgStartOffset, " \
              "3:imgEndOffset, 4:imgLenOffset"
        return 1

    fileName = argv[0]
    imgStartOffset = int(argv[1])
    imgEndOffset = int(argv[2])
    imgLenOffset = int(argv[3])

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')

    filePtr.seek(imgStartOffset, 0)  # Seek to file offset file
    addrTmp = (filePtr.read(4))
    imgStAddr = computeAddr(addrTmp)

    # Rewind back to start
    filePtr.seek(0, 0)
    filePtr.seek(imgEndOffset, 0)
    addrTmp = (filePtr.read(4))
    imgEndAddr = computeAddr(addrTmp)

    # Compute image length
    imgLen = imgEndAddr - imgStAddr + 1

    imgLenByteStr = (int2byte1(imgLen))

    # Rewind back to the start
    filePtr.seek(0, 0)
    filePtr.seek(imgLenOffset, 0)  # Seek to file start offset
    filePtr.write(chr((imgLenByteStr[0])))
    filePtr.write(chr((imgLenByteStr[1])))
    filePtr.write(chr((imgLenByteStr[2])))
    filePtr.write(chr((imgLenByteStr[3])))

    filePtr.close()


def updateMergedImageLen(argv=None):
    argLen = len (argv)

    if argLen != 3:
        print "'updateMergedImageLen'::Usage: 1:filename, 2:imgLen, " \
              "3:imgLenOffset"
        return 1

    fileName = argv[0]
    imgLen = int(argv[1])
    imgLenOffset = int(argv[2])

    imgLenByteStr = (int2byte1(imgLen))

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')

    # Rewind back to the start
    filePtr.seek(0, 0)
    filePtr.seek(imgLenOffset, 0)  # Seek to file start offset
    filePtr.write(chr((imgLenByteStr[0])))
    filePtr.write(chr((imgLenByteStr[1])))
    filePtr.write(chr((imgLenByteStr[2])))
    filePtr.write(chr((imgLenByteStr[3])))

    filePtr.close()


def getImgRange(argv=None):
    argLen = len (argv)

    if argLen != 4:
        print "'getImgRange'::Usage: 1:Filename, 2:imgStartOffset, " \
              "3:imgEndOffset, 4:imgLenOffset"
        return 1

    fileName = argv[0]

    imgStartOffset = int(argv[1])
    imgEndOffset = int(argv[2])
    imgLenOffset = int(argv[3])

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')

    filePtr.seek(imgStartOffset, 0)  # Seek to file offset filez
    addrTmp = (filePtr.read(4))
    imgStAddr = computeAddr(addrTmp)

    # Rewind back to the start
    filePtr.seek(0, 0)
    filePtr.seek(imgEndOffset, 0)
    addrTmp = (filePtr.read(4))
    imgEndAddr = computeAddr(addrTmp)

    # Compute image length
    imgLen = imgEndAddr - imgStAddr
    imgLenByteStr = (int2byte1(imgLen))

    # Rewind back to the start
    filePtr.seek(0, 0)
    filePtr.seek(imgLenOffset, 0)  # Seek to file start offset
    filePtr.write(chr((imgLenByteStr[0])))
    filePtr.write(chr((imgLenByteStr[1])))
    filePtr.write(chr((imgLenByteStr[2])))
    filePtr.write(chr((imgLenByteStr[3])))

    filePtr.close()

    return imgStAddr, imgEndAddr


def updateImageSegLen(argv=None):
    '''
    Function updates the image payload length in congiouse image segment of
    image payload.
    '''
    argLen = len (argv)

    if argLen != 3:
        print "'updateImageSegLen'::Usage: 1:Filename, 2:imgStartOffset, " \
              "3:imgEndOffset"
        return 1

    fileName = argv[0]

    imgStartOffset = int(argv[1])
    imgEndOffset = int(argv[2])
    segType = int(argv[2])

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')

    filePtr.seek(imgStartOffset, 0)  # Seek to file offset file
    addrTmp = (filePtr.read(4))
    imgStAddr = computeAddr(addrTmp)

    # Rewind back to the start
    filePtr.seek(0, 0)
    filePtr.seek(imgEndOffset, 0)
    addrTmp = (filePtr.read(4))
    imgEndAddr = computeAddr(addrTmp)

    # Compute total image length
    imgLen = imgEndAddr - imgStAddr + 1

    # Calculate length of contiguous image segment
    # todo: Need to update based on the segments present
    # segment length = total image length - (length of fixed header +
    #                                        length of boundary segment)
    #                = total image length - (44 + 20)
    segLen = imgLen - (fixedHdrLen + boundrySegLen)
    imgLenByteStr = (int2byte1(segLen))

    # Segment payload length offset
    imgLenOffset  = payloadSegLenOffset

    # Rewind back to the start
    filePtr.seek(0, 0)
    filePtr.seek(imgLenOffset, 0)  # Seek to img segment length offset
    filePtr.write(chr((imgLenByteStr[0])))
    filePtr.write(chr((imgLenByteStr[1])))
    filePtr.write(chr((imgLenByteStr[2])))
    filePtr.write(chr((imgLenByteStr[3])))

    filePtr.close()


def updateMergedImageSegLen(argv=None):
    '''
    Function updates the image payload length in congiouse image segment of
    image payload.
    '''
    argLen = len (argv)

    if argLen != 3:
        print "'updateMergedImageSegLen'::Usage: 1:Filename, 2:imgLen, " \
              "3:imgEndOffset"
        return 1

    fileName = argv[0]
    imgLen = int(argv[1])
    imgEndOffset = int(argv[2])
    segType = int(argv[2])

    # Calculate length of contiguous image segment
    # todo: Need to update based on the segments present
    # segment length = total image length - (length of fixed header +
    #                                        length of boundary segment)
    #                = total image length - (44 + 20)
    segLen = imgLen - (fixedHdrLen + boundrySegLen)
    print "Merged image segLen = %d" % segLen

    imgLenByteStr = (int2byte1(segLen))

    # Segment payload length offset
    imgLenOffset  = payloadSegLenOffset

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')
    # Rewind back to the start  filePtr.seek(0, 0)
    filePtr.seek(imgLenOffset, 0)  # Seek to img segment length offset
    filePtr.write(chr((imgLenByteStr[0])))
    filePtr.write(chr((imgLenByteStr[1])))
    filePtr.write(chr((imgLenByteStr[2])))
    filePtr.write(chr((imgLenByteStr[3])))

    filePtr.close()

def updateMergedImageEndAddr(argv=None):
    '''
    Function updates the image payload length in congiouse image segment of
    image payload.
    '''
    argLen = len (argv)

    if argLen != 3:
        print "'updateMergedImageEndAddr':: Usage: 1:Filename, 2:imgEndAddr, " \
              "3:imgEndOffset"
        return 1

    fileName = argv[0]
    imgEndAddr = int(argv[1])
    imgEndOffset = int(argv[2])
    segType = int(argv[2])

    imgEndByteStr = (int2byte1(imgEndAddr))

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')

    # Rewind back to the start  filePtr.seek(0, 0)
    filePtr.seek(imgEndOffset, 0)  # Seek to img segment length offset
    filePtr.write(chr((imgEndByteStr[0])))
    filePtr.write(chr((imgEndByteStr[1])))
    filePtr.write(chr((imgEndByteStr[2])))
    filePtr.write(chr((imgEndByteStr[3])))

    filePtr.close()


def updateCopyNeedFlag(argv=None):
    argLen = len (argv)

    if argLen != 3:
        print "'updateCopyNeedFlag':: Usage: 1:Filename, 2:CopyStatusOffset, " \
              "3:status"
        return 1

    fileName = argv[0]
    CopyStatusOffset = int(argv[1])
    status = int(argv[2])

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')
    filePtr.seek(CopyStatusOffset, 0)  # Seek to file offset file
    statusByte = (int2byte1(status))

    # Rewind back to the start
    filePtr.seek(0, 0)
    filePtr.seek(CopyStatusOffset, 0)  # Seek to file start offset
    filePtr.write(chr((statusByte[0])))

    filePtr.close()


def updateCRCStatFlag(argv=None):
    argLen = len (argv)

    if argLen != 3:
        print "'updateCRCStatFlag':: Usage: 1:Filename, 2:CRCStatusOffset, " \
              "3:status"
        return 1

    fileName = argv[0]
    CRCStatusOffset = int(argv[1])
    status = int(argv[2])

    # Open file in binary mode
    filePtr = open(fileName, 'r+b')
    filePtr.seek(CRCStatusOffset, 0)  # Seek to file offset file

    # Rewind back to the start
    filePtr.seek(0, 0)
    filePtr.seek(CRCStatusOffset, 0)  # Seek to file start offset
    filePtr.write(chr((statusByte[0])))

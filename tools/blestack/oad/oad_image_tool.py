"""
/******************************************************************************
 @file  oad_image_tool.py

 @brief This tool is used to generate OAD/production images for OAD
        enabled projects using the TI-BLE SDK.

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

from struct import unpack
import argparse
import computeCRC32
import imgBinUtil
import intelhex
import os
import textwrap
import sys
from collections import namedtuple


__version__ = "2.0.0"

imgStartOffset  = 76
imgEndOffset    = 36
imgLenOffset    = 24
imgInitialHeaderLen = 44
imgSeg0Len = 24
imgSeg1Len = 12
imgSeg3Len = 85
# This is the decimal representation of 'OAD IMG ' in little endian
imgIDVal = 2325912709029445967

initialHeaderUnpack = '<QIBBHBBBBIIIIIHH'
segment0Unpack = '<BHBIIIII'
segment1Unpack = '<BHBII'
segment3Unpack = '<BHBIBIQQQQQQQQQ'

def getAddress(fn, text):
    with open(fn) as f:
        for line in f:
            if len(line.strip()) != 0 :
                # Do something with the empty line
                if len(line.split()) == 2:
                    test = line.split()[1]
                    if test.split('=', 1 )[0] == text:
                        return test.split('=', 1 )[1]

    raise ValueError("Unable to find string " + text + " in " +
                     os.path.basename(fn))


def getBoundary(bdFileName):
    boundary_address = getAddress(bdFileName , "ICALL_STACK0_START")

    return int(boundary_address, 16)


def getTgtBoundary(tgtBdFileName):
    boundary_address = getAddress(tgtBdFileName , "OAD_TGT_FLASH_START")

    return int(boundary_address, 16)


def getAddress_CCS(fn, text):
    with open(fn) as f:
        for line in f:
            if line.find('--define=ICALL_STACK0_START=') != -1:
                return line[28:]


def getBoundary_CCS(bdFileName):
    boundary_address = getAddress_CCS(bdFileName, "--define=ICALL_STACK0_START")

    return int(boundary_address, 16)


def getAppRange(boundary_address):
    appImgStartAddr = "0000"

    app_boundary = hex(boundary_address - 1)[2:]
    app_range = appImgStartAddr + ":" + app_boundary

    return int(appImgStartAddr, 16), int(app_boundary, 16)


def getStackRange(boundary_address):
    maxImgEndAddr = "1EFFF"

    stack_boundary = hex(boundary_address)[2:]
    stack_range = stack_boundary + ":" + maxImgEndAddr

    return int(stack_boundary, 16), int(maxImgEndAddr, 16)


def getMergeBinLen(flash_range):
    flash_end = flash_range.split(':', 2)[1]
    flash_end = flash_end.split('0x', 2)[1]
    imgLen = int(flash_end, 16) + 1

    return imgLen


def getMergeBinEndAddr(flash_range):
    flash_end = flash_range.split(':', 2)[1]
    flash_end = flash_end.split('0x', 2)[1]
    imgEndAddr = int(flash_end, 16)

    return imgEndAddr


def getStackImgRange(projdir, stackHexFileName, boundary_address, outputPath):
    stack_boundary, flash_limit = getStackRange(boundary_address)

    # Generate binary file
    if stackHexFileName.endswith('.hex'):
        outfileStackPath = os.path.abspath(stackHexFileName[:-4] + ".bin")
    else:
        raise Exception("Stack hex not valid")

    intelhex.hex2bin(stackHexFileName, outfileStackPath, start=stack_boundary,
                     end=flash_limit)

    # Get the image range
    imgRange = imgBinUtil.getImgRange([outfileStackPath, imgStartOffset,
                                      imgEndOffset, imgLenOffset])

    return imgRange


def createTgtBinfile(projType, projdir, binaryFileType, appHexFileName,
                     tgtBdFileName, outputPath):
    # Find boundary
    if projType == 'ccs':
        boundary_address = getBoundary_CCS(projdir)
    else:
        boundary_address = getTgtBoundary(tgtBdFileName)


    stack_boundary, flash_limit = getStackRange(boundary_address)

    flash_path = os.path.join(projdir, appHexFileName)
    outfileTargetPath = os.path.abspath(outputPath + ".bin")

    intelhex.hex2bin(flash_path, outfileTargetPath, start=stack_boundary,
                     end=flash_limit)

    imgBinUtil.updateImageLen([outfileTargetPath, imgStartOffset, imgEndOffset,
                               imgLenOffset])
    computeCRC32.computeCRC32([outfileTargetPath, '12', '8'])


def createAppStackBinfile(projType, projdir, binaryFileType, appHexFileName,
                          bdFileName, outputPath, stackHexFileName=None):
    # Find boundary
    if projType == 'ccs':
        boundary_address = getBoundary_CCS(bdFileName)
    else:
        boundary_address = getBoundary(bdFileName)

    if (binaryFileType == 2):  # Stack only
        stack_boundary, flash_limit = getStackRange(boundary_address)

        stack_path = os.path.join(projdir, appHexFileName)
        outfileStackPath = os.path.abspath(outputPath + ".bin")

        # Generate binary file
        intelhex.hex2bin(stack_path, outfileStackPath, start=stack_boundary,
                         end=flash_limit)

        # Get the image range
        args = [outfileStackPath, imgStartOffset, imgEndOffset, imgLenOffset]
        imgStAddr, imgEndAddr = imgBinUtil.getImgRange(args)

        # Rerun hex2bin to get correct length of binary image
        intelhex.hex2bin(stack_path, outfileStackPath, start=imgStAddr,
                         end=imgEndAddr)

        imgBinUtil.updateImageLen([outfileStackPath, imgStartOffset,
                                   imgEndOffset, imgLenOffset])
        imgBinUtil.updateImageSegLen([outfileStackPath, imgStartOffset,
                                      imgEndOffset])
        computeCRC32.computeCRC32([outfileStackPath, "12", "8"])

    elif (binaryFileType == 1):
        # Get app image range
        appImgStartAddr, app_boundary = getAppRange(boundary_address)

        # To get the actual stack range, need to build stack binary and then
        # read the image header to find the actual address
        stack_boundary, flash_limit = getStackImgRange(projdir,
                                                       stackHexFileName,
                                                       boundary_address,
                                                       outputPath)

        app_path = os.path.join(projdir, appHexFileName)
        stack_path = os.path.join(projdir, stackHexFileName)
        outfileAppPath = os.path.abspath(outputPath + ".bin")

        # Generate application binary
        intelhex.hex2bin(app_path, outfileAppPath, start=appImgStartAddr,
                         end=app_boundary)

        # Get the image range
        imgRngParams = [outfileAppPath, imgStartOffset, imgEndOffset,
                        imgLenOffset]
        imgStAddr, imgEndAddr = imgBinUtil.getImgRange(imgRngParams)

        # Rerun the hex2bin to get correct length binary image
        intelhex.hex2bin(app_path, outfileAppPath, start=imgStAddr,
                         end=imgEndAddr)

        # Update file type field
        imgBinUtil.fileTypeTag([outfileAppPath,"1"])

        # Update imageLength field
        imgBinUtil.updateImageLen([outfileAppPath, imgStartOffset,
                                   imgEndOffset, imgLenOffset])

        # Update image segment's length field
        imgBinUtil.updateImageSegLen([outfileAppPath, imgStartOffset,
                                      imgEndOffset])

        # Update crc32 field
        computeCRC32.computeCRC32([outfileAppPath, "12", "8"])

        # Generate merged binary
        flashRange = str(hex(appImgStartAddr)) + ':' + str(hex(flash_limit))


        mergedBinLen = getMergeBinLen(flashRange)


        # Create IntelHex objects for each hex
        appHex = intelhex.IntelHex(app_path)
        stackHex = intelhex.IntelHex(stack_path)

        # Define boundaries for each hex
        appHex = appHex[appImgStartAddr:app_boundary+1]
        stackHex = stackHex[stack_boundary:flash_limit+1]

        # Merge hex's
        inputHexFiles = [appHex, stackHex]
        mergedHex = intelhex.IntelHex()
        for hexFile in inputHexFiles:
            try:
                mergedHex.merge(hexFile, overlap="replace")
            except:
                print("Fatal Error: FAILED merge due to overlap when merging")
                sys.exit(1)

        # Define boundaries for merged hex
        mergedHex = mergedHex[appImgStartAddr:flash_limit+1]

        # Write hex to file
        mergeHexFilePath = os.path.abspath(outputPath + "_merged.hex")
        mergedHex.write_hex_file(mergeHexFilePath)

        # Convert hex to bin
        outFileMergedPath = os.path.abspath(outputPath + "_merged.bin")

        intelhex.hex2bin(mergeHexFilePath, outFileMergedPath,
                         start=appImgStartAddr, end=flash_limit)
        imgBinUtil.fileTypeTag([outFileMergedPath, "3", "18"])

        # Update imageLength field
        imgBinUtil.updateMergedImageLen([outFileMergedPath, mergedBinLen,
                                         imgLenOffset])
        args = [outFileMergedPath, mergedBinLen, imgEndOffset]
        imgBinUtil.updateMergedImageSegLen(args)

        mergedImgEndAddr = getMergeBinEndAddr(flashRange)

        imgBinUtil.updateMergedImageEndAddr([outFileMergedPath,
                                             mergedImgEndAddr, imgEndOffset])

        # Add crc32 bytes
        computeCRC32.computeCRC32([outFileMergedPath, "12", "8"])


def main(args):
    secondHexFileName = ""
    argLen = len(sys.argv)
    binaryFileType = args.BinaryType

    if binaryFileType == 1 or binaryFileType == 3:
        if argLen == 9:
            secondHexFileName = args.HexPath2
        else:
            raise Exception("Second hex file path and file name required!")

    elif binaryFileType == 0 or binaryFileType == 2:
        if argLen != 8:
            raise Exception("Boundary file path and file name required!")
    else:
        raise Exception("Invalid binary file type!")

    # Generate binary file
    if binaryFileType == 0:
        createTgtBinfile(args.ProjType, args.ProjDir, binaryFileType,
                         args.HexPath1, args.bdFilePath, args.output)
    else:
        createAppStackBinfile(args.ProjType, args.ProjDir, binaryFileType,
                              args.HexPath1, args.bdFilePath, args.output,
                              secondHexFileName)

    #Open the created binary file for header info extraction
    filePtr = open(args.output+'.bin', 'r+b')

    #Read the information into a tuple, that is then made into a struct
    initialHeader = filePtr.read(imgInitialHeaderLen)
    headerTuple = namedtuple('OadImgHdr','imgID crc32 bimVer metaVer techType imgCpStat crcStat imgType imgNo imgVld len prgEntry softVer imgEndAddr hdrLen rfu2')
    metaVector = headerTuple._make(unpack(initialHeaderUnpack,initialHeader))

    print_initHeader(metaVector)

    #While the end of the header isn't found, read in addiitional segments.
    headerEnd = 0
    while(not headerEnd):
        nextSegment = filePtr.read(1)
        #Check to see which types of segment are present
        if(nextSegment == b'\x00'):
            filePtr.seek(-1,1) #Goes back in order to read segType into info struct
            nextSegment = filePtr.read(imgSeg0Len)
            headerTuple = namedtuple('Seg0Hdr','segType wirelessTech rfuSeg payloadLen stackEntryAddr iCallStackAddr ramStartAddr ramEndAddr')
            metaVector = headerTuple._make(unpack(segment0Unpack,nextSegment))
            print_segInfo(metaVector)
        elif(nextSegment == b'\x01'):
            filePtr.seek(-1,1)
            nextSegment = filePtr.read(imgSeg1Len)
            headerTuple = namedtuple('Seg1Hdr','segType wirelessTech rfuSeg payloadLen startAddr')
            metaVector = headerTuple._make(unpack(segment1Unpack,nextSegment))
            headerEnd = 1
            print_segInfo(metaVector)
        elif(nextSegment == b'\x03'):
            filePtr.seek(-1,1)
            nextSegment = filePtr.read(imgSeg3Len)
            headerTuple = namedtuple('Seg3Hdr','segType wirelessTech rfuSeg payloadLen secVer timeStamp signInfo signature1 signature2 signature3 signature4 signature5 signature6 signature7 signature8')
            metaVector = headerTuple._make(unpack(segment3Unpack,nextSegment))
            print_segInfo(metaVector)
        else:
            print 'invalid segment found'
            break

def parse_args():
    script_description = "OAD Hex to Binary Converter version"
    parser = argparse.ArgumentParser(prog='oad_image_tool',
                                     description=script_description)
    parser.add_argument("-v", "--version", action="version",
                        version="%(prog)s " + __version__)
    parser.add_argument("ProjType", help="Target project Type: (iar/ccs)")
    parser.add_argument("ProjDir", help="Target project directory")
    parser.add_argument("BinaryType", type=int, help="BIM = 0, "
                                                     "Application = 1, "
                                                     "Stack = 2, "
                                                     "Application + Stack = 3")
    parser.add_argument("HexPath1", help="First .hex file path (app/stack)")
    parser.add_argument("bdFilePath", help="Boundary file path")
    parser.add_argument("HexPath2", nargs='?',
                        help="Second .hex file name (app/stack)")
    parser.add_argument("-o", "--output", required=True,
                        help="Path and filename of output file(s) without "
                             "extension")
    args = parser.parse_args()

    return args

## print_initHeader
## metaVecotr - a struct  containing the data read in from an OAD header
## This function prints out basic information contained in an OAD image header
def print_initHeader(metaVector):
    hdr = metaVector._asdict()
    if(hdr['imgID']==imgIDVal):
        hdr['imgID'] = 'Valid'
    else:
        hdr['imgID'] = 'Invalid'
    print(textwrap.dedent("""
        OAD IMG HDR
    Field       |    Value
      imgID     |    {imgID}
       CRC      |    0x{crc32:X}
     bimVer     |    {bimVer}
    metaVer     |    {metaVer}
    techType    |    0x{techType:X}
    imgCpStat   |    0x{imgCpStat:X}
    crcStat     |    0x{crcStat:X}
    imgType     |    0x{imgType:X}
    imgNo       |    0x{imgNo:X}
    imgVld      |    0x{imgVld:X}
    len         |    0x{len:X}
    prgEntry    |    0x{prgEntry:X}
    softVer     |    0x{softVer:X}
    imgEndAddr  |    0x{imgEndAddr:X}
    hdrLen      |    0x{hdrLen:X}
        """.format(**hdr)))

## print_segInfo
## metaVecotr - a struct  containing the data read in from an OAD header
## This function prints out information based on detected sgements contained in an OAD image header
def print_segInfo(metaVector):
    hdr = metaVector._asdict()
    #Checks the the wireless tech type
    techValue = hdr['wirelessTech']
    hdr['wirelessTech'] = ''
    if(techValue==0):
        hdr['wirelessTech'] = 'No Wireless Technology'
    if(techValue&1!=0):
        hdr['wirelessTech'] += ' [BLE]'
    if(techValue&2!=0):
        hdr['wirelessTech'] += ' [TIMAC Sub1G]'
    if(techValue&4!=0):
        hdr['wirelessTech'] += ' [TIMAC 2.4G]'
    if(techValue&8!=0):
        hdr['wirelessTech'] += ' [Zigbee]'
    if(techValue&16!=0):
        hdr['wirelessTech'] += ' [RF4CE]'
    if(techValue&32!=0):
        hdr['wirelessTech'] += ' [Thread]'
    if(techValue&64!=0):
        hdr['wirelessTech'] += ' [EasyLink]'
    #Prints out the corresponding segment type
    if(hdr['segType']==0):
        print(textwrap.dedent("""
                Segment HDR
        Field        |          Value
       segType       |    Boundary Segment
    Wireless Tech    |      {wirelessTech}
       segLen        |      0x{payloadLen:X}
   STACK ENTRY ADDR  |    0x{stackEntryAddr:X}
   ICALL STACK ADDR  |    0x{iCallStackAddr:X}
    RAM START ADDR   |     0x{ramStartAddr:X}
    RAM END ADDR     |      0x{ramEndAddr:X}
        """.format(**hdr)))
    elif(hdr['segType']==1):
        print(textwrap.dedent("""
                Segment HDR
        Field        |          Value
      segType        |      Contiguous Image Segment
   Wireless Tech     |      {wirelessTech}
       segLen        |      0x{payloadLen:X}
    Start Address    |       0x{startAddr:X}
        """.format(**hdr)))
    elif(hdr['segType']==3):
        print(textwrap.dedent("""
                Segment HDR
        Field        |          Value
      segType        |      Security Segment
   Wireless Tech     |      {wirelessTech}
       segLen        |      0x{payloadLen:X}
   Security Version  |       0x{secVer:X}
     Timestamp       |      0x{timeStamp:X}
     Signer Info     |      0x{signInfo:X}
     Signature       |      0x{signature1:X}{signature2:X}{signature3:X}{signature4:X}{signature5:X}{signature6:X}{signature7:X}{signature8:X}
        """.format(**hdr)))
    else:
        print 'invalid print command'
def version_control():
    if sys.version[:5] != "2.7.3":
        raise Exception("Must be using Python 2.7.3!")


if __name__ == '__main__':
    version_control()
    oad_args = parse_args()
    main(oad_args)

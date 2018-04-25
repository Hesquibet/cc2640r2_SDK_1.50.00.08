"""
/******************************************************************************
 @file  secure_fw_tool.py

 @brief This tool is used to add header metadata to images for firmware security
        verification.

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
"""

# tested with python 2.7.10
__version__ = "1.0.1"

import os.path
import sys
import hashlib
from ecdsa import NIST256p
from ecdsa import SigningKey
import argparse
import string

true = 1
false = 0
META_MAGIC = "\x57\x46\x53\x20\x4C\x42\x53\x0A"
default_out_bin = 'ble_simple_peripheral_secure_fw_cc2640r2lp_app.bin'
input_bin = false
FW_VER ="\x03\x00\x00\x01"
TIMESTAMP = "\x00\x00\x00\x00"
START_ADDR = "\x00\x00\x00\x70"
def_magic = false
def_start = false


def CheckExt(choices):
    class Act(argparse.Action):
        def __call__(self,parser,namespace,fname,option_string=None):
            ext = os.path.splitext(fname)[1][1:]
            if ext not in choices:
                option_string = '({})'.format(option_string) if option_string else ''
                parser.error("missing file or file extension not valid")
            else:
                setattr(namespace,self.dest,fname)
    return Act

# Read image bin file for signing message
def ReadMessageForSign(file) :
    byte = TIMESTAMP+ FW_VER
	# calculating length from end address as it can differ 
    BUF_SIZE = int(args.endAddr,16) -int(START_ADDR.encode('hex'),16)
    with open(file,'rb') as f:
        byte += f.read(BUF_SIZE)
    return byte

#compute the ECC signature 
def ComputeSignature(file):
    sk = SigningKey.from_pem(open(args.privateKey).read())
    vk = sk.get_verifying_key()
    open("public.pem","w").write(vk.to_pem())
    sig = sk.sign(data=ReadMessageForSign(file),hashfunc=hashlib.sha256)
    signerHash = hashlib.sha256()
    for c in vk.to_string():
        signerHash.update(c.encode('hex'))  
    L = list(signerHash.hexdigest())
    signerInfo =""
    for x in range(48,64):
        signerInfo+=L[x]
    return [signerInfo,sig]

#add the metadata to image binary file
def AddHeadertoImage(def_magic,def_start) :
    #preparing the header metadata
    FW_LEN = int(args.endAddr,16) -int(START_ADDR.encode('hex'),16)	
    SIGN_TYPE ="\x01"    
    input_file = open(args.Inbin, "rb")
    if not input_file :                
        print "Error opening file"
    output_f = open(args.securebin, "wb")
    if not output_f :                
        print "Error opening file"
    #writing metadata to new binary file
    if def_magic is true:
        output_f.write(META_MAGIC)
    else :
        output_f.write(args.MagicWord.decode('hex'))
    output_f.write(FW_VER)
    lenStr = ("0%07X"%FW_LEN)
    output_f.write(lenStr.decode('hex'))
    if def_start is true:
        output_f.write(START_ADDR)
    else:
        output_f.write((args.startAddr).decode('hex'))
    output_f.write((args.endAddr).decode('hex'))
    output_f.write(SIGN_TYPE)
    output_f.write(TIMESTAMP)
    signature = ComputeSignature(args.Inbin)    
    signerInfo = signature[0]
    output_f.write(signerInfo.decode('hex'))    
    output_f.write(signature[1])
    # fill unused bytes with 0xFF
    metadata = bytearray(b'\xFF'*11)
    output_f.write(metadata)
   #appending image binary data to header
    for chunk in iter(lambda: input_file.read(1024), b""):
       output_f.write(chunk)
    input_file.close()
    output_f.close()
    
# Parsing command line arguments   
parser = argparse.ArgumentParser(description='Script for generating image metadata')
parser.add_argument('Inbin', help="Path to input application image in .bin format",action=CheckExt({'bin'}))
parser.add_argument('privateKey', help="Private key file in .pem format", action=CheckExt({'pem'}))
parser.add_argument('endAddr',help="End address of firmware ")
parser.add_argument('-s','--startAddr', help="Start address of firmware", default = START_ADDR)
parser.add_argument('-m', '--MagicWord', help="Magic word to identify start of image metadata,number of bytes required is 8", default = META_MAGIC)
parser.add_argument('-b', '--securebin', help="Path to secure bin file.", default = default_out_bin, action=CheckExt({'bin'}))

#parse the user's command line arguments
if len(sys.argv[1:]) == 0:
    parser.print_help()
    parser.exit()
    
args = parser.parse_args()    

#if input file is hex convert to bin, else use it directly
if not args.Inbin :          
    print(">> Missing input image file \n")
    parser.print_usage()
    parser.exit()   

ReadMessageForSign(args.Inbin)

#print default value
if not os.path.exists(args.privateKey) :    
    print(">> Key file not present \n")
    parser.print_usage()
    parser.exit() 

#print default value
if not args.startAddr:
    def_start = false
    print(">> Missing start Address of image\n")
    parser.print_usage()
    parser.exit()    
else:
    if( args.startAddr == START_ADDR):
        print " Default start Address is:",START_ADDR.encode('hex')
        def_start = true
    elif len(args.startAddr.decode('hex')) is not 4:
        print "Invalid start address,number of bytes required is 4"
        print len(args.startAddr.decode('hex'))
        parser.print_usage()
        parser.exit()
    else :
        print "New start address is:",args.startAddr
        def_start = false
    
#print default value
if not args.endAddr:
    print(">> Missing end Address of image\n")
    parser.print_usage()
    parser.exit()
    
#print default value
if not args.MagicWord :
    def_magic = 0
    print(">> Missing magic word \n")
    parser.print_usage()
    parser.exit()
else :    
    if( args.MagicWord == META_MAGIC):
        print " Default Magic word is:",META_MAGIC.encode('hex')
        def_magic = 1
    elif len(args.MagicWord.decode('hex')) is not 8:
        print "Invalid length of new magic word,number of bytes required is 8"
        parser.print_usage()
        parser.exit()
    else :
        print "New Magic word is:",args.MagicWord
        def_magic = 0
    
AddHeadertoImage(def_magic,def_start)

print("******************************************************************************************")
print("Success")
print("******************************************************************************************")
 
 # Clean up pipes, silently ignore broken pipe
try:
    sys.stdout.close()
except IOError:
    pass

try:
    sys.stderr.close()
except IOError:
    pass

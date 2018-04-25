'''
/******************************************************************************
 @file  output_converter.py

 @brief This script converts a bin file into a hex file.

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
'''
import argparse
import intelhex
import ntpath
import os
import re
import sys


__version__ = '1.0.0'


def verifyFilePath(file, extension=None):
    abs_file = ntpath.abspath(file)

    # Check if input file exists
    if ntpath.isfile(abs_file):
        # Check if input file ends with extension if passed in
        if extension:
            if not abs_file.endswith(extension):
                raise Exception('Error: File must be a ' + extension +
                                ' file! (' + abs_file + ')')
            else:
                return abs_file
        else:
            return abs_file
    else:
        raise Exception('Error: File is not valid! (' + abs_file + ')')


def assembleOutputPath(out_param, input_path):
    if out_param:
        # Make sure that output directory exists if output param exists
        abs_out_path = ntpath.abspath(out_param)
        output_dir, output_filename = ntpath.split(abs_out_path)
        if not ntpath.exists(output_dir):
            os.mkdir(output_dir)

        return abs_out_path
    else:
        # Default output is same directory and name as input .bin file
        output_dir, input_filename = ntpath.split(input_path)
        output_filename = input_filename[:-4] + '.hex'

        return ntpath.join(output_dir, output_filename)


def processBoundaryFile(file_param):
    boundary = '0x0'

    if file_param:
        abs_bd_path = verifyFilePath(file_param, '.bdef')
        with open(abs_bd_path, 'r') as f:
            bd_file = f.read()

        re_str = r'^-D (ICALL_STACK0_START|OAD_TGT_FLASH_START)=(0x[0-9a-z]+)'
        regex_output = re.findall(re_str, bd_file, re.MULTILINE)

        if len(regex_output) == 1:
            boundary = regex_output[0][1]
        else:
            raise Exception('Error in boundary file! Stack boundary not found!')

    return boundary


def convertBinToHex(inputBin, outputHex, offset=0):
    try:
        intelhex.bin2hex(inputBin, outputHex, offset)
    except:
        print('Error: Conversion failed!')


def main(args):
    # Check if input file is valid
    abs_bin_path = verifyFilePath(args.inputBinPath, '.bin')

    # Set output path and output name
    output_path = assembleOutputPath(args.output, abs_bin_path)

    # Extract ICALL_STACK0_ADDR boundary
    boundary = processBoundaryFile(args.boundary)
    if args.address:
        boundary = args.address

    # Convert .bin to .hex
    convertBinToHex(abs_bin_path, output_path, int(boundary, 16))


def parse_args():
    script_description = 'This script converts a bin file into a hex file'
    parser = argparse.ArgumentParser(prog='output_converter',
                                     description=script_description)
    parser.add_argument('-v', '--version', action='version',
                        version='%(prog)s ' + __version__)
    parser.add_argument('inputBinPath', help='Path to input bin file')
    parser.add_argument('-b', '--boundary', help='Path to boundary file '
                                                 'generated by Frontier')
    parser.add_argument('-a', '--address', help='Start address of image in hex')
    parser.add_argument('-o', '--output', help='Path to output hex file. The '
                                               'default is the same directory '
                                               'and name of the input bin file')
    args = parser.parse_args()

    if args.boundary and args.address:
        raise Exception('Only one of the boundary and address arguments can be '
                        'provided')

    return args


def version_control():
    if sys.version[:5] != '2.7.3':
        raise Exception('Must be using Python 2.7.3!')


if __name__ == '__main__':
    version_control()
    output_converter_args = parse_args()
    main(output_converter_args)

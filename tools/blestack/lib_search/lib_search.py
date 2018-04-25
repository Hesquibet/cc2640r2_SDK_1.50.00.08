"""
/******************************************************************************
 @file  lib_search.py

 @brief This file contains lib search source. The main purpose of this tool
        is to determine which BLE stack libraries should be linked into a
        given stack project.

        All stack configurations are defined in a single file in the stack
        project. That file is provided as input to this script during a stack
        project pre-build step. The configurations are determined and the
        correct pre-built stack libraries are added to a linker command file
        as output. The stack project then links to this output file.

        This script was tested using Python v3.5 64 bit for Windows. An
        equivalent Windows executable is provided which was created using
        pyinstaller. For all other systems, this script is provided as a
        starting point and is not guaranteed to work.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2015-2017, Texas Instruments Incorporated
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
import os
import shutil
import sys
import re
from lxml import etree


__version__ = '2.0.0'


class InputFeaturesClass:
    """
    Class to contain input features and regex from XML
    """
    def __init__(self, layer):
        self.layer = layer
        self.features_list = []
        self.regex_list = []

    def add_feature(self, feature):
        if feature not in self.features_list:
            self.features_list.append(feature)

    def add_regex_pair(self, search, replace):
        self.regex_list.append((search, replace))


class InputRegexClass:
    """
    Class to contain input regex from XML
    """
    def __init__(self):
        self.regex_list = []

    def add_regex_pair(self, search, replace):
        self.regex_list.append((search, replace))


def input_arg_expansion(arg):
    """
    Verify input argument exists and if so expand to absolute path
    """
    if not os.path.exists(arg):
        arg = os.path.join(os.getcwd(), arg)
    if not os.path.exists(arg):
        raise Exception('Error: Cannot find input file \'' + arg + '\'\n')

    arg = os.path.abspath(arg)
    return arg


def write_cmd_file(cmd_file, cmd_str):
    """
    Write to linker command file contents
    """
    with open(cmd_file, 'w+') as fw:
        fw.write(cmd_str)
        return True


def generate_cmd(cmd_file, libs):
    """
    Construct linker command file contents
    """

    return_val = False
    cmd_header = '/*\n * DO NOT MODIFY. This file is automatically generate' + \
                 'd during the pre-build\n *                step by the lib' + \
                 '_search utility\n */\n\n'

    new_cmd = cmd_header
    for lib in libs:
        new_cmd += '\"' + lib + '\"\n'

    cmd_file = os.path.abspath(cmd_file)

    # Read current cmd file if it exists
    if os.path.exists(cmd_file):
        with open(cmd_file, 'r') as fr:
            old_cmd = fr.read()

            # Write to cmd file
            if old_cmd != new_cmd:
                return_val = write_cmd_file(cmd_file, new_cmd)
    else:
        # Write to cmd file
        return_val = write_cmd_file(cmd_file, new_cmd)

    return return_val


def strip_comments(file_str):
    """
    Strip opt file of comments
    """
    opening_regex_match = re.search(r'/\*', file_str)
    nested_end_index = 0
    closing_end_index = 0

    if opening_regex_match:
        inc_end_index = opening_regex_match.end() + 1
        possible_nested_comment_regex_match = \
            re.search(r'/\*', file_str[inc_end_index:])

        closing_regex_match = re.search(r'\*/', file_str)

        if possible_nested_comment_regex_match:
            nested_end_index = possible_nested_comment_regex_match.end()

        if closing_regex_match:
            closing_end_index = closing_regex_match.end()

        if possible_nested_comment_regex_match and closing_regex_match and \
                ((inc_end_index + nested_end_index) < closing_end_index):
            file_str = strip_comments(file_str[inc_end_index:])

        if closing_regex_match:
            if opening_regex_match.start() > 0:
                return_str = file_str[:opening_regex_match.start()] + \
                             file_str[closing_end_index:]
                return strip_comments(return_str)
            else:
                return_str = file_str[closing_end_index:]
                return strip_comments(return_str)
        else:
            raise Exception('Error: check comment syntax')
    else:
        return_str = re.sub(r'\r', r'', file_str)
        return_str = re.sub(r'//.*', r'', return_str)
        return_str = re.sub(r'\n +', r'\n', return_str)
        return_str = re.sub(r' +\n+', r'\n', return_str)
        return_str = re.sub(r'^\n', r'', return_str)
        return_str = re.sub(r'\n$', r'', return_str)

        return return_str


def parse_opt(opt, input_features_class, input_regex_class):
    """
    Read and parse opt file
    """
    out_list = []

    with open(opt, 'r') as f:
        opt_contents = f.read()

    opt_contents = strip_comments(opt_contents)

    for item in input_features_class.regex_list:
        opt_contents = re.sub(item[0], item[1], opt_contents)

    for pair in input_regex_class.regex_list:
        opt_contents = re.sub(pair[0], pair[1], opt_contents)

    opt_list = opt_contents.split('\n')

    for define in opt_list[:]:
        if len(define) > 0 and not define.startswith('-D'):
            out_list.append(define)

    return out_list


def parse_lib_name(lib_path, layer, feature_list):
    """
    Read and parse features in library name
    """
    # Find device
    re_match = re.search(r'cc[0-6x]{4}_', lib_path)
    if re_match:
        device = re_match.group()[:-1]
        lib_path = lib_path[len(device)+1:]
    else:
        print('Could not find device. Exiting now.')
        sys.exit()

    define_list = lib_path.split('_')

    if define_list[1] == 'tl':
        define_list = define_list[2:]
    else:
        define_list = define_list[1:]

    output_list = []
    idx = 0

    while idx < len(define_list):
        if define_list[idx] == 'ext':
            output_list.append(define_list[idx] + '_' + define_list[idx + 1])
            idx += 2
        else:
            output_list.append(define_list[idx])
            idx += 1

    return device, output_list


def parse_features_xml(xml_path):
    """
    Read and parse input XML
    """
    input_features_class_list = []
    input_regex_class = InputRegexClass()
    patch_list = []

    with open(xml_path, 'r') as f:
        lxml_parser = etree.XMLParser(remove_blank_text=True)
        tree = etree.parse(f, lxml_parser)
        root = tree.getroot()

        if root.tag == 'lib_search':
            for child in root:
                if child.tag == 'layer':
                    name = child.findtext('name')
                    input_features_class = InputFeaturesClass(name)

                    feature_list = child.find('features')
                    for feature in feature_list:
                        input_features_class.add_feature(feature.text)

                    regex = child.find('regex')
                    subs = regex.findall('sub')
                    for sub in subs:
                        search = sub.findtext('search')
                        replace = sub.findtext('replace')
                        input_features_class.add_regex_pair(re.compile(search),
                                                            replace)

                    input_features_class_list.append(input_features_class)

                elif child.tag == 'regex':
                    subs = child.findall('sub')
                    for sub in subs:
                        search = sub.findtext('search')
                        replace = sub.findtext('replace')
                        input_regex_class.add_regex_pair(re.compile(search),
                                                         replace)

    return input_features_class_list, input_regex_class


def lib_search(opt, lib_dir, device, xml_path):
    """
    Top level function. Using xml definition file, scan for all
    necessary libraries for the stack project which called this
    script as a pre-build action
    """

    # Create state variable for library match
    lib_match = False

    # Create empty list of final libraries needed for linking
    final_libs_list = []
    final_opt_list = []

    input_features_class_list, input_regex_class = parse_features_xml(xml_path)

    for input_features_class in input_features_class_list:
        valid_features_list = input_features_class.features_list
        layer = input_features_class.layer
        opt_defines_list = parse_opt(opt, input_features_class,
                                     input_regex_class)

        for root, dirs, files in os.walk(os.path.abspath(os.path.join(lib_dir,
                                                                      layer))):
            for file in files:
                if file.endswith('.a'):
                    lib_device, lib_defines_list = parse_lib_name(file[:-2],
                                                                  layer,
                                                            valid_features_list)

                    for feature_candidate in opt_defines_list:
                        if feature_candidate in valid_features_list:
                            final_opt_list.append(feature_candidate)

                    final_opt_list.sort()
                    lib_defines_list.sort()

                    if final_opt_list == lib_defines_list and \
                            (file.startswith(device) or
                            file.startswith('cc26xx')):
                        full_lib_path = os.path.abspath(os.path.join(root,file))
                        print('<<< Using library: ' + full_lib_path + ' >>>')
                        final_libs_list.append(full_lib_path)
                        lib_match = True

                    # Reset
                    final_opt_list = []

    if not lib_match:
        raise Exception('Error: No libraries matched!')

    return final_libs_list


def generate_symbols_file(opt, src_dir, dst_dir):
    feature_list = ['pxxx', 'xcxx', 'xxbx', 'xxxo', 'pcxx', 'pxxo', 'xcbx',
                    'xxbo', 'coc', 'del']

    regex_list = [('-DHOST_CONFIG=', ''),
                  ('(?<!\+)PERIPHERAL_CFG(?!\+)', 'pxxx'),
                  ('(?<!\+)CENTRAL_CFG(?!\+)', 'xcxx'),
                  ('(?<!\+)BROADCASTER_CFG(?!\+)', 'xxbx'),
                  ('(?<!\+)OBSERVER_CFG(?!\+)', 'xxxo'),
                  ('PERIPHERAL_CFG\+OBSERVER_CFG', 'pxxo'),
                  ('CENTRAL_CFG\+BROADCASTER_CFG', 'xcbx'),
                  ('PERIPHERAL_CFG\+CENTRAL_CFG', 'pcxx'),
                  ('-DV41_FEATURES=', ''),
                  ('L2CAP_COC_CFG', 'coc'),
                  ('-DATT_DELAYED_REQ', 'del')]

    symbol_filename = 'default.symbols'

    input_features_class = InputFeaturesClass('')
    input_regex_class = InputRegexClass()

    for feature in feature_list:
        input_features_class.add_feature(feature)

    for regex_pair in regex_list:
        input_regex_class.add_regex_pair(regex_pair[0], regex_pair[1])

    opt_list = parse_opt(opt, input_features_class, input_regex_class)

    for root, dirs, files in os.walk(src_dir):
        for file in files:
            if file.endswith('.symbols'):
                symbol_list = file[:-8].split('_')

                opt_list.sort()
                symbol_list.sort()

                if opt_list == symbol_list:
                    symbol_filename = file

    shutil.copy2(os.path.join(src_dir, symbol_filename),
                 os.path.join(dst_dir, 'ble_r2.symbols'))

    print('\n<<< Using ' + symbol_filename + ' >>>')


def main(args):
    # Check input file existence and expand to absolute path
    opt = input_arg_expansion(args.opt)
    lib_dir = input_arg_expansion(args.lib_dir)

    # Expand input path
    if not os.path.isabs(args.cmd):
        args.cmd = os.path.join(os.getcwd(), args.cmd)

    # Make directory if it doesn't exist
    if not os.path.exists(os.path.dirname(args.cmd)):
        os.makedirs(os.path.dirname(args.cmd))

    # Search for all necessary libraries
    final_libs_list = lib_search(opt, lib_dir, args.device,
                                             args.xml)

    # If valid libraries were found generate linker cmd file
    if final_libs_list:
        if generate_cmd(args.cmd, final_libs_list):
            print('<<< Modifying ' + os.path.basename(args.cmd) +
                  '. Stack configurations have changed. >>>')
        else:
            print('<<< Not modifying ' + os.path.basename(args.cmd) +
                  '. Stack configurations have not changed. >>>')

    # Generate symbols file
    generate_symbols_file(args.opt, args.symbol_src,
                          os.path.abspath(os.path.join(args.cmd, '..')))


def parse_args():
    script_description = 'Used in conjunction with BLE Stack releases as a ' \
                         'pre-build step. Parses provided .opt file and ' \
                         'creates a linker file with necessary libs to be ' \
                         'linked into project.'
    parser = argparse.ArgumentParser(prog='lib_search',
                                     description=script_description)
    parser.add_argument('-v', '--version', action='version',
                        version='%(prog)s ' + __version__)
    parser.add_argument('opt', help='Local project configuration file for '
                                    'stack project')
    parser.add_argument('lib_dir', help='Directory containing library files')
    parser.add_argument('cmd', help='Linker command file that will be written '
                                    'to')
    parser.add_argument('device', help='device (e.g. cc2640, cc1350)')
    parser.add_argument('xml', help='path to features xml')
    parser.add_argument('symbol_src', help='path directory containing symbols')
    args = parser.parse_args()

    return args


def version_control():
    if sys.version[:5] != '2.7.3':
        print('Must be using Python 2.7.3!')
        sys.exit()


if __name__ == '__main__':
    version_control()
    lib_search_args = parse_args()
    main(lib_search_args)

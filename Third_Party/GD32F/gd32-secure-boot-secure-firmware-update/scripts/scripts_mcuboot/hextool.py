#! /usr/bin/env python3
import os
import re
import subprocess
import sys
import bincopy

#获取当前脚本所在的路径
current_file_path = os.path.dirname(os.path.abspath(__file__))
# 追加路径
sys.path.append(current_file_path)

import argparse
import macro_parser 

flash_base_s_re = re.compile(r"^.*\s*RE_FLASH_BASE\s*(.*)")
flash_base_ns_re = re.compile(r"^.*\s*RE_FLASH_BASE\s*(.*)")
offset_re = re.compile(r"^.*\s*RE_([0-9A-Z_]+)_OFFSET\s*(.*)")

def convert_to_hex(args):
    flash_base_s = macro_parser.evaluate_macro(args.config, flash_base_s_re, 0, 1)
    flash_base_ns = macro_parser.evaluate_macro(args.config, flash_base_ns_re, 0, 1)
    offsets = macro_parser.evaluate_macro(args.config, offset_re, 1, 2)
    if (args.type == 'IMG_0_APP' or args.type == 'IMG_1_APP') and (args.nsec == 1):
        act_offset = offsets[args.type] + flash_base_ns
    else:
        act_offset = offsets[args.type] + flash_base_s
    act_offset = offsets[args.type] + flash_base_s
    # cmd = args.exe + ' ' + args.infile + ' -Binary -offset ' + str(hex(act_offset)) + ' -o ' + args.outfile + ' -Intel'
    # print(cmd)
    # subprocess.run(cmd, stdin=None, input=None, stdout=None, stderr=None, shell=True, timeout=None, check=False)

    f = bincopy.BinFile(None, False, 8," utf-8")
    f.add_binary_file(args.infile, act_offset, False)

    with open(args.outfile, "wb") as fs:
        fs.write(f.as_ihex().encode('utf-8'))
        fs.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config', metavar='filename', required=True,
                          help='Location of the file that contains macros')
    parser.add_argument('-t', '--type', metavar='type', required=True,
                          help='SYS_SET / MBL / SYS_STATUS / IMG_0_PROT / IMG_0_AROT / IMG_0_APP / IMG_1_PROT / IMG_1_AROT / IMG_1_APP')
    parser.add_argument('-e', '--exe', metavar='filename', required=True,
                          help='Location of the file that contains macros')    
    parser.add_argument('-ns', '--nsec', type=int, default=0, help='Not security')    
    parser.add_argument("infile")
    parser.add_argument("outfile")
    
    args = parser.parse_args()
    try:
        file_path = os.path.abspath(args.config)
        args.config = file_path
    except Exception as e:
        print("err try: ", "may not need config file")

    convert_to_hex(args)

if __name__ == '__main__':
    main()
from multiprocessing.dummy import current_process
import os
import argparse
import sys
from datetime import datetime

old_lib_lines = 0

#获取当前脚本所在的路径
current_file_path = os.path.dirname(os.path.abspath(__file__))

# rom_map_path      = current_file_path + '/../../Project/GD32F527-EVAL/MDK-ARM/IBL/Gdm32/Listings/ibl.map'
# new_lib_path      = current_file_path + '/../../Export/symbol/ibl_symbol_mbedtls'

mbedtls_fun = {
    "boot_write_image_ok",
    "boot_write_magic",
    "flash_area_open",
}

# function define
# 读取指定行并返回字符串
def read_specific_line(file_path, line_number):
    with open(file_path, 'r', encoding='utf-8') as file:
        for index, line in enumerate(file):
            if index == line_number - 1:
                return line.strip()

# 读取第二个空格后的内容
def find_char_after_second_space(s):
    words = s.split()
    if len(words) >= 3:
        return ' '.join(words[2:])
    else:
        return "字符串中的空格数量不足2个"
# s = "这是一个 用于 测试 的 字符串"
# result = find_char_after_third_space(s)
# print(result)

# 查找文件中指定字符串所在的行数
def find_string_in_file(file_path, target_string):
    with open(file_path, 'r', encoding='utf-8') as file:
        line_number = 0
        for line in file:
            line_number += 1
            if target_string in line:
                return line_number
    return -1

# 获取指定文件的总行数
def get_file_line_num(file_path) :
    old_lib = open(file_path, 'r')
    line_num = 0
    for line in old_lib:
        line_num += 1
    return line_num

# /* function realize */

def do_gen_mbl_lib(args):

    rom_map_path      = args.map
    new_lib_path      = args.old_file
    new_file = open(new_lib_path, 'w')

    str_write = '#<SYMDEFS># ARM Linker, 6090000: Last Updated: '
    current_time = datetime.now()
    time_string = current_time.strftime("%Y-%m-%d %H:%M:%S")
    str_write += time_string + '\n'
    new_file.writelines(str_write)

    for old_lib_str in mbedtls_fun :
        try:
            str_in_rom_map_line = find_string_in_file(rom_map_path, '.text.' + old_lib_str + ' ')
            splipt_str = read_specific_line(rom_map_path, str_in_rom_map_line).split()
            addr = int(splipt_str[0], 16) + 1
            addr_str = hex(addr)
            # print(addr_str)
            new_file.writelines(addr_str + ' T ' + old_lib_str + '\n')
        except Exception as e:
            print("err try: ", old_lib_str)

    new_file.close()

subcmds = {
        'symbol_gen': do_gen_mbl_lib,
        }

def args():
    parser = argparse.ArgumentParser()
    subs = parser.add_subparsers(help='subcommand help', dest='subcmd')

    symbol_gen = subs.add_parser('symbol_gen', help='Generate mbedtls library')
    symbol_gen.add_argument('-m', '--map', metavar='filename', required=True, help='Location of the map')
    symbol_gen.add_argument('-o', '--old_file', metavar='filename', required=True, help='Location of the old library file')

    args = parser.parse_args()

    file_path = os.path.abspath( args.map)
    args.map = file_path
    file_path = os.path.abspath( args.old_file)
    args.old_file = file_path

    if args.subcmd is None:
        print('Must specify a subcommand', file=sys.stderr)
        sys.exit(1)

    subcmds[args.subcmd](args)

if __name__ == '__main__':
    args()

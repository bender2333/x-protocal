from multiprocessing.dummy import current_process
import os
import argparse
import sys
import shutil
import re
import bincopy
from fnmatch import fnmatch

#获取当前脚本所在的路径
current_file_path = os.path.dirname(os.path.abspath(__file__))
ROOT = current_file_path + "/.."
# 追加路径
sys.path.append(ROOT)
print(sys.path)

from scripts.symbol_tool.symbol_generate_name import do_gen_mbedtls_lib
from scripts.symbol_tool.symbol_generate_name_mbl import do_gen_mbl_lib
from scripts.scripts_mcuboot.sysset import gen_sysset
from scripts.scripts_mcuboot.gentool import Assembly
from scripts.scripts_mcuboot.hextool import convert_to_hex
from scripts.scripts_mcuboot.imgtool.main import sign

ROTPK       = ROOT + "/scripts/certs/ec_secp256r1_prikey.pem "
#ROTPK       = ROOT + "/scripts/certs/ec_rsa2048_prikey.pem "
ENCK        = ROOT + "/scripts/certs/root-rsa-2048.pem "
CONFIG_FILE = ROOT + "/config/config_gdm32.h"
IMGTOOL     = ROOT + "/scripts/scripts_mcuboot/imgtool.py"
SREC_CAT    = ROOT + "/scripts/scripts_mcuboot/srec_cat.exe"

# 类定义
class lib_gen:
    def __init__(self, map_file, old_file):
        self.map = map_file
        self.old_file = old_file

class sysset_gen:
    def __init__(self, config, outfile):
        self.config = config
        self.outfile = outfile

class hex_gen:
    def __init__(self, config, exe, in_file, out_file, type, nsec=1):
        self.config = config
        self.exe = exe
        self.infile = in_file
        self.outfile = out_file
        self.type = type
        self.nsec = nsec

# /* function realize */
def delete_file(file_path):
    if os.path.exists(file_path):
        os.remove(file_path)

# 查找文件中指定字符串所在的行数
def find_string_in_file(file_path, target_string):
    with open(file_path, 'r', encoding='utf-8') as file:
        line_number = 0
        for line in file:
            line_number += 1
            if target_string in line:
                return line_number
    return -1

# 读取字符串s第n个空格后的字符串内容
def find_char_after_n_space(s, n):
    words = s.split()
    if len(words) >= n+1:
        return ' '.join(words[n:])
    else:
        return "error"

# 读取指定行并返回字符串
def read_specific_line(file_path, line_number):
    with open(file_path, 'r', encoding='utf-8') as file:
        for index, line in enumerate(file):
            if index == line_number - 1:
                return line.strip()

def get_version_num(config_file, target_string):
    ver_str = []
    version_line = find_string_in_file(config_file, target_string)
    version_string = read_specific_line(config_file, version_line)

    version_only = find_char_after_n_space(version_string, 2)
    if version_only!="error":
        #  version_num = int(version_only[2:], 16)
        if bool(version_only[3:4].lstrip('0')):
            ver_str = version_only[3:4].lstrip('0')+'.'
        else :
            ver_str = "0"+'.'
        if bool(version_only[5:7].lstrip('0')):
            ver_str += version_only[5:7].lstrip('0')+'.'
        else :
            ver_str += "0"+'.'
        if bool(version_only[7:10].lstrip('0')):
            ver_str += version_only[7:10].lstrip('0')
        else :
            ver_str += "0"
    return ver_str

def get_app_offset(config_file, target_string):
    app_offset_num = 0
    app_offset_line = find_string_in_file(config_file, target_string)
    app_offset_string = read_specific_line(config_file, app_offset_line)

    app_offset_only = find_char_after_n_space(app_offset_string, 2)
    if app_offset_only!="error":
        app_offset_num = int(app_offset_only[2:], 16)
    return app_offset_num

def gen_bin_file_copy(args):
    # gen bin file
    # if args.ide_type == "KEIL" :
    #     axf_file_path = os.path.abspath(args.output_path + args.output_name + '.axf')
    #     fromelf = args.ide_path + "/ARM/ARMCC/bin/fromelf.exe"
    #     bin_out = args.output_path + "../" + args.output_name + ".bin "
    #     os.system(fromelf + " --bin --8x1 --bincombined --output=" + bin_out + axf_file_path)

    #     # copy bin file
    #     bin_copy_path = ROOT + '/scripts/images/'+ args.output_name + '.bin'
    #     delete_file(bin_copy_path)
    #     shutil.copy(bin_out, bin_copy_path)

    axf_file_path = os.path.abspath(args.output_path + args.output_name + '.hex')
    bin_out = args.output_path + "../" + args.output_name + ".bin "
    f = bincopy.BinFile(axf_file_path)
    with open(bin_out, "wb") as fs:
        fs.write(f.as_binary())
        fs.close()
    # copy bin file
    bin_copy_path = ROOT + '/scripts/images/'+ args.output_name + '.bin'
    # bin_copy_path = args.output_path + "../" + args.output_name + '_sign.bin'
    delete_file(bin_copy_path)
    shutil.copy(bin_out, bin_copy_path)

def plt_config_file_copy(args):
    if args.platform_type == "" :
        print("IBL need define PLATFORM_XXX")
    # 去掉指定的子字符串
    plt_string = args.platform_type.replace("PLATFORM_", "")
    # 将结果转换为小写
    lplt_string = plt_string.lower()

    # copy bin file
    bin_copy_path = ROOT + '/config/'+ "config_gdm32_" + lplt_string + '.h'
    # delete_file(CONFIG_FILE)
    shutil.copy(bin_copy_path, CONFIG_FILE)

# IBL after build
def ibl_after_build(args):
    plt_config_file_copy(args)

    # copy image file to scripts/images
    image_file_path = os.path.abspath(args.output_path + args.output_name + '.hex')
    image_copy_path = ROOT + '/scripts/images/'+ args.output_name + '.hex'
    delete_file(image_copy_path)
    shutil.copy(image_file_path, image_copy_path)

    # gen bin file
    gen_bin_file_copy(args)

    # gen ibl_symbol_mbedtls lib
    in_map_path = args.output_path + "/../Listings/" + args.output_name + ".map"
    out_lib_path = ROOT + "/Export/symbol/ibl_symbol_mbedtls"
    ibl_gen_args = lib_gen(in_map_path, out_lib_path)
    do_gen_mbedtls_lib(ibl_gen_args)

def mbl_after_build(args):
    # print(args.name)
    images_prj_path = ROOT + '/scripts/images/'
    sysset_file = images_prj_path + 'sysset.bin'
    delete_file(sysset_file)
    # Generate system setting hex
    sysset_gen_args = sysset_gen(CONFIG_FILE, sysset_file)
    gen_sysset(sysset_gen_args)

    # gen bin file
    gen_bin_file_copy(args)

    # Add image header, ptlvs and concatenate the cert
    ver_str = get_version_num(CONFIG_FILE, "RE_MBL_VERSION")
    cmd_str = "python " + IMGTOOL + " sign --align 4 -v "
    cmd_str += ver_str + " -k " + ROTPK
    cmd_str += "-H 0x200 --pad-header -S 0xFFE00 --public-key-format hash --max-align 8 "
    cmd_str += ROOT + '/scripts/images/'+ args.output_name + '.bin '
    in_file = ROOT + '/scripts/images/'+ args.output_name + '.bin '
    mbl_sign_file = ROOT + '/scripts/images/'+ 'mbl-sign.bin'
    delete_file(mbl_sign_file)
    cmd_str += mbl_sign_file
    os.system(cmd_str)
    # sign(ROTPK, "hash", 4, ver_str, False, 0x200, True, 0xFFE00,False, False, None, False, "little", 128, None, in_file, mbl_sign_file,    None, None, None, None, False, None, None, "",    None, 8, False, None, None, None, None)
 

    # gen mbl-sys.bin
    mbl_sys_file = ROOT + '/scripts/images/'+ 'mbl-sys.bin'
    output = Assembly(CONFIG_FILE, mbl_sys_file)
    output.add_image(sysset_file, "SYS_SET")
    output.add_image(mbl_sign_file, "MBL")
    delete_file(sysset_file)

    # gen hex
    mbi_sys_hex_file = ROOT + '/scripts/images/mbl-sys.hex'
    delete_file(mbi_sys_hex_file)
    hexgen_args = hex_gen(CONFIG_FILE, SREC_CAT, mbl_sys_file, mbi_sys_hex_file, "SYS_SET")
    convert_to_hex(hexgen_args)

    shutil.copy(mbi_sys_hex_file, args.output_path + args.output_name + '.hex')

    # gen mbl_symbol lib
    in_map_path = args.output_path + "/../Listings/" + args.output_name + ".map"
    out_lib_path = ROOT + "/Export/symbol/mbl_symbol"
    mbl_gen_args = lib_gen(in_map_path, out_lib_path)
    do_gen_mbl_lib(mbl_gen_args)

def app_after_build(args):
    # print(args.name)
    # gen bin file
    gen_bin_file_copy(args)

    # Add image header, ptlvs and concatenate the cert
    ver_str = get_version_num(CONFIG_FILE, "RE_APP_VERSION")
    cmd_str = "python " + IMGTOOL + " sign --align 4 -v "
    cmd_str += ver_str + " -k " + ROTPK
    app0_offset = get_app_offset(CONFIG_FILE, "RE_IMG_0_APP_OFFSET")
    app1_offset = get_app_offset(CONFIG_FILE, "RE_IMG_1_APP_OFFSET")
    cmd_str += "-H 0x200 --pad-header -S "
    size = hex(app1_offset - app0_offset)
    cmd_str += size
    print(args.enc_enable)
    if args.enc_enable == "True" :
        cmd_str += " -E " + ENCK
    cmd_str += " --public-key-format hash --max-align 8 --pad-sig "
    cmd_str += args.output_path + "../" + args.output_name + '.bin '
    app_sign_file = args.output_path + "../" + args.output_name + 'app-sign.bin'
    delete_file(app_sign_file)
    cmd_str += app_sign_file
    # print(cmd_str)
    os.system(cmd_str)

    # gen hex "
    app_hex_file = ROOT + '/scripts/images/app-sign.hex'
    delete_file(app_hex_file)
 

    hexgen_args = hex_gen(CONFIG_FILE, SREC_CAT, app_sign_file, app_hex_file, "IMG_0_APP")
    convert_to_hex(hexgen_args)

    shutil.copy(app_hex_file, args.output_path + args.output_name + '.hex')

    # gen hex1
    app1_hex_file = ROOT + '/scripts/images/app-sign1.hex'
    delete_file(app1_hex_file)

    hexgen_args = hex_gen(CONFIG_FILE, SREC_CAT, app_sign_file, app1_hex_file, "IMG_1_APP")
    convert_to_hex(hexgen_args)

    shutil.copy(app1_hex_file, args.output_path + args.output_name + '1.hex')

subcmds_name = {
        'IBL': ibl_after_build,
        'MBL': mbl_after_build,
        'APP': app_after_build,
        }

# /* function realize */
def do_gen_after_build(args):
    if args.name is None:
        print('Must specify a project name', file=sys.stderr)
        sys.exit(1)

    subcmds_name[args.name](args)

subcmds = {
        'project_deal': do_gen_after_build,
        }

def args():
    parser = argparse.ArgumentParser()
    subs = parser.add_subparsers(help='subcommand help', dest='subcmd')

    symbol_gen = subs.add_parser('project_deal', help='Deal project after it is build')
    symbol_gen.add_argument('-pn', '--name', metavar='project name', required=True, help='Name of the project: IBL, MBL, or APP')
    symbol_gen.add_argument('-op', '--output_path', metavar='output path', required=True, help='Path of the output file')
    symbol_gen.add_argument('-on', '--output_name', metavar='output name', required=True, help='Name of the output file')
    symbol_gen.add_argument('-ide', '--ide_type', metavar='ide type', required=True, help='Type of IDE')
    symbol_gen.add_argument('-ip', '--ide_path', metavar='ide path', required=True, help='Path of IDE')
    symbol_gen.add_argument('-plt', '--platform_type', metavar='platform type', required=False, help='platform type')

    symbol_gen.add_argument('-enc', '--enc_enable', metavar='APP encryption enabled', required=False, default="False", help='APP encryption')

    args = parser.parse_args()

    if args.subcmd is None:
        print('Must specify a subcommand', file=sys.stderr)
        sys.exit(1)

    subcmds[args.subcmd](args)

if __name__ == '__main__':
    args()

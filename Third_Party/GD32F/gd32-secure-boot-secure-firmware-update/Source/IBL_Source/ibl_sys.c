/*!
    \file    ibl_sys.c
    \brief   IBL system for GD32 SDK

    \version 2024-06-30, V1.0.0, demo for GD32
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "ibl_includes.h"
#include "mbedtls/aes.h"
#include "mbedtls/platform.h"

#define VERSION_COUNTER_IN_SETTING    0

int32_t err_process;

uint32_t flash_offset_sys_status;
#define FLASH_OFFSET_SYS_STATUS_PING (RE_SYS_STATUS_OFFSET)
#define FLASH_OFFSET_SYS_STATUS_PONG (RE_SYS_STATUS_OFFSET + SYS_STATUS_AREA_SZ)

const uint8_t valid_image_status[] = {
    (IMG_FLAG_OLDER | IMG_FLAG_VERIFY_NONE | IMG_FLAG_IA_NONE),  // b'00000, Reset
    (IMG_FLAG_NEWER | IMG_FLAG_VERIFY_NONE | IMG_FLAG_IA_NONE),  // b'00001, New image downloaded
    (IMG_FLAG_NEWER | IMG_FLAG_VERIFY_OK | IMG_FLAG_IA_NONE),    // b'00011, Newer image and MBL verify it OK
    (IMG_FLAG_OLDER | IMG_FLAG_VERIFY_OK | IMG_FLAG_IA_NONE),    // b'00010, Older image and MBL verify it OK
    (IMG_FLAG_NEWER | IMG_FLAG_VERIFY_FAIL | IMG_FLAG_IA_NONE),  // b'00101, MBL verify it failed
    (IMG_FLAG_NEWER | IMG_FLAG_VERIFY_FAIL | IMG_FLAG_IA_OK),    // b'01101, Newer, IA OK, but MBL verify it failed after next reboot
    (IMG_FLAG_NEWER | IMG_FLAG_VERIFY_OK | IMG_FLAG_IA_OK),      // b'01011, Newer image and MBL verify OK and Initial Attestation OK
    (IMG_FLAG_OLDER | IMG_FLAG_VERIFY_OK | IMG_FLAG_IA_OK),      // b'01010, Older image and MBL verify OK and Initial Attestation OK
    (IMG_FLAG_NEWER | IMG_FLAG_VERIFY_OK | IMG_FLAG_IA_FAIL),    // b'10011, Initial Attestation failed
};

#if VERSION_COUNTER_IN_SETTING
static uint8_t bitmap_to_uint8(uint16_t bitmap)
{
    uint8_t val = 0;

    while (((bitmap & 1) == 0) && (val < 16)) {
        val++;
        bitmap >>= 1;
    }
    return val;
}

static uint16_t uint8_to_bitmap(uint8_t val)
{
    uint16_t bitmap = 0xFFFF;

    return (bitmap << val);
}

static void cntr_to_version(uint16_t *mbl_nv_cntr, uint8_t *major, uint8_t *minor)
{
    uint8_t offset;
    *major = bitmap_to_uint8(mbl_nv_cntr[0]);
    offset = *major + 1;
    *minor = bitmap_to_uint8(mbl_nv_cntr[offset]);
}
#endif

int sys_setting_get_internal(void *settings)
{
    int ret;
#if 0
    uint8_t *ptr;
    ptr = cmse_check_pointed_object(settings, CMSE_SECURE);
    if (ptr == NULL)
        return -1;
#endif
    if (settings == NULL)
        return -1;
    ret = flash_read(FLASH_OFFSET_SYS_SETTING, settings, sizeof(struct sys_setting_t));
    if (ret != 0)
        return -2;

    return 0;
}

void sys_setting_show(void)
{
    struct sys_setting_t setting;
    uint8_t mbl_major, mbl_minor;
    uint8_t img_major, img_minor;
    uint16_t mbl_rev;
    uint16_t img_rev;

    sys_setting_get_internal(&setting);
    //cntr_to_version(setting.mbl_nv_cntr, &mbl_major, &mbl_minor);
    //cntr_to_version(setting.img_nv_cntr, &img_major, &img_minor);
    mbl_major = (setting.mbl_initial_version >> 24) & 0xff;
    mbl_minor = (setting.mbl_initial_version >> 16) & 0xff;
    mbl_rev = setting.mbl_initial_version & 0xffff;
    img_major = (setting.img_initial_version >> 24) & 0xff;
    img_minor = (setting.img_initial_version >> 16) & 0xff;
    img_rev = setting.img_initial_version & 0xffff;

    ibl_printf("================================\r\n");
    ibl_printf("Magic:\t\t0x%08x\r\n", setting.sys_magic);
    ibl_printf("Sys Set Offset:\t0x%08x\r\n", setting.sysset_offset);
    ibl_printf("Sys Status Offset:\t0x%08x\r\n", setting.sysstatus_offset);
    ibl_printf("Image0 Offset:\t0x%08x\r\n", setting.img0_offset);
    ibl_printf("Image1 Offset:\t0x%08x\r\n", setting.img1_offset);
    ibl_printf("SST Offset:\t0x%08x\r\n", setting.sst_offset);
    ibl_printf("Audit Offset:\t0x%08x\r\n", setting.audit_offset);
    ibl_printf("MBL initial version:\t%d.%d.%d\r\n", mbl_major, mbl_minor, mbl_rev);
    ibl_printf("Image initial version:\t%d.%d.%d\r\n", img_major, img_minor, img_rev);
    ibl_printf("Version Locked:\t0x%08x\r\n", setting.ver_locked);
    ibl_printf("Flash Total Size:\t0x%08x\r\n", setting.flash_totsz);
}

#if VERSION_COUNTER_IN_SETTING
int sys_setting_get_version(uint32_t type, uint8_t *major, uint8_t *minor)
{
    struct sys_setting_t setting;

    sys_setting_get_internal(&setting);

    if (type == IMG_TYPE_MBL)
        cntr_to_version(setting.mbl_nv_cntr, major, minor);
    else
        cntr_to_version(setting.img_nv_cntr, major, minor);

    return 0;
}

int sys_setting_set_version(uint32_t type, uint8_t major, uint8_t minor)
{
    uint32_t offset_major, offset_minor;
    uint16_t l_major_map, l_minor_map;  /* Bitmap format */
    uint8_t l_major, l_minor;
    struct sys_setting_t setting;
    uint16_t ver, l_ver;
    int ret;

    if ((major > MAX_VER_MAJOR) || (minor > MAX_VER_MINOR)) {
        return -1;
    }

    if (type == IMG_TYPE_MBL)
        offset_major = FLASH_OFFSET_SYS_SETTING + (uint32_t)&(setting.mbl_nv_cntr) - (uint32_t)&setting;
    else
        offset_major = FLASH_OFFSET_SYS_SETTING + (uint32_t)&(setting.img_nv_cntr) - (uint32_t)&setting;

    ret = flash_read_indirect(offset_major, &l_major_map, 2);
    if (ret != 0) goto exit;

    l_major = bitmap_to_uint8(l_major_map);
    offset_minor = offset_major + 2 + l_major * 2;
    ret = flash_read_indirect(offset_minor, &l_minor_map, 2);
    if (ret != 0) goto exit;
    l_minor = bitmap_to_uint8(l_minor_map);

    ver = (major << 8) | minor;
    l_ver = (l_major << 8) | l_minor;

    if (ver > l_ver) {
        if (major > l_major) {
            l_major_map = uint8_to_bitmap(major);
            l_minor_map = uint8_to_bitmap(minor);
            ret = flash_write(offset_major, &l_major_map, 2);
            if (ret != 0) goto exit;

            offset_minor = offset_major + 2 + major * 2;
            ret = flash_write(offset_minor, &l_minor_map, 2);
            if (ret != 0) goto exit;
        } else {
            l_minor_map = uint8_to_bitmap(minor);
            ret = flash_write(offset_minor, &l_minor_map, 2);
            if (ret != 0) goto exit;
        }
    } else {
        /* Do nothing */
        return 1;
    }

    return 0;

exit:
    ibl_trace(IBL_WARN, "Setting: R/W flash error.\r\n");
    return -3;
}
#endif

int is_valid_image_status(uint8_t status)
{
    uint32_t sz = sizeof(valid_image_status);
    int i;
    int ret = 0;
    for (i = 0; i < sz; i++) {
        if (status == valid_image_status[i]) {
            ret = 1;
            break;
        }
    }
    return ret;
}

static void print_tlv(uint8_t t, uint8_t l, uint8_t *v)
{
    int i;
    ibl_printf("%d\t%d\t", t, l);
    for (i = 0; i < l; i++) {
        if (i == l -1)
            ibl_printf("%02x", v[i]);
        else
            ibl_printf("%02x-", v[i]);
    }
    ibl_printf("\r\n");
}

#if (SYS_STATUS_ENCRPTED == 0)
static int is_ping_active(uint32_t *act_cnt)
{
    uint32_t cnt_ping, cnt_pong;
    int is_ping;
    int ret;

    ret = flash_read_indirect(FLASH_OFFSET_SYS_STATUS_PING, &cnt_ping, 4);
    if (ret != 0) goto exit;
    ret = flash_read_indirect(FLASH_OFFSET_SYS_STATUS_PONG, &cnt_pong, 4);
    if (ret != 0) goto exit;

    if (cnt_ping == 0xFFFFFFFF)  /* First use, valid count is from 0 to 0xFFFFFFFE */
        cnt_ping = 0;
    if (cnt_pong == 0xFFFFFFFF)  /* First use, valid count is from 0 to 0xFFFFFFFE */
        cnt_pong = 0;

    if ((cnt_ping == (cnt_pong + 1)) || (cnt_ping == (cnt_pong + 2))) {
        is_ping = 1;
    } else {
        is_ping = 0;
    }

    if (act_cnt) {
        if (is_ping)
            *act_cnt = cnt_ping;
        else
            *act_cnt = cnt_pong;
    }
    return is_ping;

exit:
    return -1;
}

static int is_type_existed(uint8_t type, uint32_t start, uint32_t tot_len)
{
    uint32_t stop;
    int ret;
    uint8_t t, l;
    int find = -1;

    /* Check if the type has already existed. */
    stop = start + tot_len;
    while (start < stop) {
        ret = flash_read_indirect(start, &t, sizeof(t));
        if (ret != 0) goto exit;
        if (t == SYS_UNKNOWN_TYPE)  // Empty
            break;
        start += sizeof(t);
        ret = flash_read_indirect(start, &l, sizeof(l));
        if (ret != 0) goto exit;
        start += sizeof(l);
        if (type == t) {
            find = start - sizeof(t) - sizeof(l);
            break;
        } else {
            start += l;
        }
    }
    return find;

exit:
    return -2;
}

static int flash_copy(uint32_t src, uint32_t dst, uint32_t len)
{
#define READ_BLK_SZ    32
    uint8_t tmp_buf[READ_BLK_SZ];
    uint32_t end = src + len;
    int ret;

    while (src < end) {
        if (end - src > READ_BLK_SZ) {
            ret = flash_read_indirect(src, tmp_buf, READ_BLK_SZ);
            if (ret != 0) goto exit;
            ret = flash_write(dst, tmp_buf, READ_BLK_SZ);
            if (ret != 0) goto exit;
            dst += READ_BLK_SZ;
            src += READ_BLK_SZ;
        } else {
            ret = flash_read_indirect(src, tmp_buf, (end - src));
            if (ret != 0) goto exit;
            ret = flash_write(dst, tmp_buf, (end - src));
            if (ret != 0) goto exit;
            dst += (end - src);
            src = end;
        }
    }
    return 0;
exit:
    return -2;
}

static int flash_add_tlv(uint32_t addr, uint8_t type, uint8_t len, uint8_t* pval)
{
    uint8_t buf[MAX_TLV_VALUE_SIZE + 2];

    buf[0] = type;
    buf[1] = len;
    memcpy(buf + 2, pval, len);
    return flash_write(addr, buf, len + 2);

#if 0
    ret = flash_write(addr, &type, sizeof(type));
    if (ret != 0) goto exit;
    addr += sizeof(type);
    ret = flash_write(addr, &len, sizeof(len));
    if (ret != 0) goto exit;
    addr += sizeof(len);
    ret = flash_write(addr, pval, len);
    if (ret != 0) goto exit;

    return 0;
exit:
    return -2;
#endif
}

int sys_status_set_internal(uint8_t type, uint8_t len, uint8_t* pval)
{
    uint32_t act_cnt, tot_len = 0, act_cnt_new, tot_len_new = 0;
    uint32_t start, p, stop, newstart, q, newstop, cpylen;
    uint8_t t, l, v;
    int is_ping, find;
    int ret;

    if ((len > MAX_TLV_VALUE_SIZE) || (len == 0)) {
        return -1;
    }

#if 0//defined(FLASH_INTERNAL)
    if (len & 0x01) {
        len = len + 1;
    }
#endif

    /* Check which is the active system status, ping or pong */
    is_ping = is_ping_active(&act_cnt);
    if (is_ping < 0) {
        goto exit;
    } else if (is_ping > 0) {
        start = FLASH_OFFSET_SYS_STATUS_PING;
        newstart = FLASH_OFFSET_SYS_STATUS_PONG;
    } else {
        start = FLASH_OFFSET_SYS_STATUS_PONG;
        newstart = FLASH_OFFSET_SYS_STATUS_PING;
    }

    /* Calculate next active count */
    if (act_cnt == 0xFFFFFFFE)
        act_cnt_new = 0;
    else
        act_cnt_new = act_cnt + 1;

    /* Get total length */
    p = start + sizeof(act_cnt);
    ret = flash_read_indirect(p, &tot_len, sizeof(tot_len));
    if (ret != 0) goto exit;
    p += sizeof(tot_len);

    /* Empty */
    if (tot_len == 0xFFFFFFFF) {
        tot_len = 0;
    }

    if (tot_len > SYS_STATUS_AREA_SZ - SYS_STATUS_HEADER_SZ) {
        tot_len = SYS_STATUS_AREA_SZ - SYS_STATUS_HEADER_SZ;
    }

    /* Check if the type has already existed. */
    find = is_type_existed(type, p, tot_len);
    if (find == -1) {
        /* New insert */
        // ibl_trace(IBL_INFO, "sys_status_set_internal: insert new item. \r\n");
        if ((len + SYS_STATUS_TLV_HEADER_SZ) <= (SYS_STATUS_AREA_SZ - SYS_STATUS_HEADER_SZ - tot_len)) {
            /* Erase all */
            ret = flash_erase(newstart, SYS_STATUS_AREA_SZ);
            if (ret != 0) goto exit;

            /* Move old to new */
            q = newstart + SYS_STATUS_HEADER_SZ;
            ret = flash_copy((start + SYS_STATUS_HEADER_SZ), q, tot_len);
            if (ret != 0) goto exit;
            q += tot_len;

            /* Write new item */
            ret = flash_add_tlv(q, type, len, pval);
            if (ret != 0) goto exit;
            q += sizeof(type) + sizeof(len) + len;

            /* Write total length */
            tot_len_new = q - (newstart + SYS_STATUS_HEADER_SZ);
            ret = flash_write((newstart + sizeof(act_cnt_new)), &tot_len_new, sizeof(tot_len_new));
            if (ret != 0) goto exit;

            /* Write active count */
            ret = flash_write(newstart, &act_cnt_new, sizeof(act_cnt_new));
            if (ret != 0) goto exit;
        } else {
            ibl_trace(IBL_WARN, "sys_status_set_internal: memory is not enough for (%d, %d). \r\n", type, len);
        }
    } else if (find >= 0) {
        /* Already existed */
        // ibl_trace(IBL_INFO, "sys_status_set_internal: update existed item. \r\n");
        p = find + sizeof(t);
        ret = flash_read_indirect(p, &l, sizeof(l));
        if (ret != 0) goto exit;

        if (len <= (SYS_STATUS_AREA_SZ - (tot_len - l + SYS_STATUS_HEADER_SZ))) {
            /* Erase all */
            ret = flash_erase(newstart, SYS_STATUS_AREA_SZ);
            if (ret != 0) goto exit;

            /* Copy the first half */
            q = newstart + SYS_STATUS_HEADER_SZ;
            cpylen = find - (start + SYS_STATUS_HEADER_SZ);
            ret = flash_copy((start + SYS_STATUS_HEADER_SZ), q, cpylen);
            if (ret != 0) goto exit;
            q += cpylen;

            /* Write new item */
            ret = flash_add_tlv(q, type, len, pval);
            if (ret != 0) goto exit;
            q += sizeof(type) + sizeof(len) + len;

            /* Copy the last half */
            p = find + SYS_STATUS_TLV_HEADER_SZ + l;
            cpylen = tot_len - (p - start - SYS_STATUS_HEADER_SZ);
            ret = flash_copy(p, q, cpylen);
            if (ret != 0) goto exit;
            q += cpylen;

            /* Write total length */
            tot_len_new = q - (newstart + SYS_STATUS_HEADER_SZ);
            ret = flash_write((newstart + sizeof(act_cnt_new)), &tot_len_new, sizeof(tot_len_new));
            if (ret != 0) goto exit;

            /* Write active count */
            ret = flash_write(newstart, &act_cnt_new, sizeof(act_cnt_new));
            if (ret != 0) goto exit;
        } else {
            ibl_trace(IBL_WARN, "sys_status_set_internal: memory is not enough for (%d, %d). \r\n", type, len);
        }
    }

    return 0;

exit:
    ibl_trace(IBL_WARN, "sys_status_set_internal: Read/Write flash error. \r\n");
    return -2;

}

int sys_status_get_internal(uint8_t type, uint8_t len, uint8_t* pval)
{
    int is_ping, find;
    uint32_t start, tot_len = 0;
    int ret;
    uint8_t l;

    /* Check which is the active system status, ping or pong */
    is_ping = is_ping_active(NULL);
    if (is_ping < 0) {
        goto exit;
    } else if (is_ping > 0) {
        start = FLASH_OFFSET_SYS_STATUS_PING;
    } else {
        start = FLASH_OFFSET_SYS_STATUS_PONG;
    }

    /* Get total length */
    start += sizeof(uint32_t);
    ret = flash_read_indirect(start, &tot_len, sizeof(tot_len));
    if (ret != 0) goto exit;
    start += sizeof(tot_len);

    /* Empty */
    if (tot_len == 0xFFFFFFFF) {
        tot_len = 0;
    }

    /* Check if found */
    find = is_type_existed(type, start, tot_len);
    if (find >= 0) {
        /* Found */
        start = find + sizeof(type);
        ret = flash_read_indirect(start, &l, sizeof(l));
        if (ret != 0) goto exit;
        start += sizeof(l);
        if (l > len)
            l = len;
        if (l > MAX_TLV_VALUE_SIZE)
            l = MAX_TLV_VALUE_SIZE;
        ret = flash_read_indirect(start, pval, l);
        if (ret != 0) goto exit;
        return SYS_STATUS_FOUND_OK;
    } else {
        /* Not found */
        return SYS_STATUS_NOT_FOUND;
    }

exit:
    ibl_trace(IBL_WARN, "Status: Read/Write flash error. \r\n");
    return SYS_STATUS_FOUND_ERR;
}


void sys_status_show(void)
{
    int is_ping;
    uint32_t start, stop, tot_len = 0;
    uint8_t t, l, v[MAX_TLV_VALUE_SIZE];
    char *title;
    int ret;

    /* Check which is the active system status, ping or pong */
    is_ping = is_ping_active(NULL);
    if (is_ping < 0) {
        goto exit;
    } else if (is_ping > 0) {
        start = FLASH_OFFSET_SYS_STATUS_PING;
        title = "System Status: Ping\r\n";
    } else {
        start = FLASH_OFFSET_SYS_STATUS_PONG;
        title = "System Status: Pong\r\n";
    }

    /* Get total length */
    start += sizeof(uint32_t);
    ret = flash_read_indirect(start, &tot_len, sizeof(tot_len));
    if (ret != 0) goto exit;
    start += sizeof(tot_len);

    ibl_printf("%s=============================\r\n", title);

    /* Empty */
    if (tot_len == 0xFFFFFFFF) {
        return;
    }

    ibl_printf("T\tL\tV\r\n");

    /* List types. */
    stop = start + tot_len;
    while (start < stop) {
        ret = flash_read_indirect(start, &t, sizeof(t));
        if (ret != 0) goto exit;
        if (t == SYS_UNKNOWN_TYPE)  // Empty
            break;
        start += sizeof(t);
        ret = flash_read_indirect(start, &l, sizeof(l));
        if (ret != 0) goto exit;
        start += sizeof(l);
        if (l > MAX_TLV_VALUE_SIZE)
            l = MAX_TLV_VALUE_SIZE;
        ret = flash_read_indirect(start, &v, l);
        if (ret != 0) goto exit;
        start += l;

        print_tlv(t, l, v);
    }

exit:
    return;
}

void sys_status_dump(int is_ping)
{
    uint32_t start, tot_len = 0;
    uint8_t *tmp_buf = NULL;
    char *title;
    int ret;

    if (is_ping < 0) {
        return;
    } else if (is_ping > 0) {
        start = FLASH_OFFSET_SYS_STATUS_PING;
        title = "Status PING:\r\n";
    } else {
        start = FLASH_OFFSET_SYS_STATUS_PONG;
        title = "Status PONG:\r\n";
    }

    /* Get total length */
    ret = flash_read_indirect((start + 4), &tot_len, sizeof(tot_len));
    if (ret != 0) goto exit;
    if (tot_len == 0xFFFFFFFF) {
        ibl_printf("----------------------\r\n%s\r\n\tEmpty\r\n\r\n", title);
        return;
    }
    tot_len += 8;  // valid count, total length

    tmp_buf = ibl_malloc(tot_len);
    if (NULL == tmp_buf)
        return;

    ret = flash_read_indirect(start, tmp_buf, tot_len);
    if (ret != 0) goto exit;

    ibl_print_data(IBL_ALWAYS, title, tmp_buf, tot_len);

exit:
    if (tmp_buf)
        ibl_free(tmp_buf);
    return;
}

#else

static int sys_status_crypt(IN uint8_t mode,
                        IN uint8_t *key,
                        IN uint8_t *input,
                        IN size_t length,
                        OUT uint8_t *output)
{
    mbedtls_aes_context ctx;
    uint8_t iv[AES_KEY_SZ];
    int keybits = AES_KEY_SZ * BITS_PER_BYTE;
    int ret = 0;

    if ((length % AES_BLOCK_SZ != 0) || length == 0)
        return -1;

    mbedtls_aes_init(&ctx);

    if (mode == MBEDTLS_AES_ENCRYPT) {
        mbedtls_aes_setkey_enc(&ctx, key, keybits);
    } else {
        mbedtls_aes_setkey_dec(&ctx, key, keybits);
    }

    ret = mbedtls_aes_crypt_ecb(&ctx, mode, input, output);
    if (ret != 0) goto exit;
    input += AES_BLOCK_SZ;
    length -= AES_BLOCK_SZ;

    if (length > 0) {
        memset(iv , 0x5A, AES_KEY_SZ);
        output += AES_BLOCK_SZ;
        ret = mbedtls_aes_crypt_cbc(&ctx, mode, length, iv, input, output);
    }

exit:
    mbedtls_aes_free(&ctx);
    return ret;
}

static int get_active_header(    IN uint8_t *key,
                            OUT struct sys_status_header_t *sys_hdr,
                            OUT uint32_t *flash_offset)
{
    struct sys_status_header_t *hdr;
    uint8_t blk_ping[AES_BLOCK_SZ], blk_pong[AES_BLOCK_SZ];
    uint32_t cnt_ping, cnt_pong;
    int is_ping, ret, status;
#if (IBL_VERSION >= V_1_1)
    int ping_valid = 1, pong_valid = 1;
#endif

    /* Read PING header */
    ret = flash_read_indirect(FLASH_OFFSET_SYS_STATUS_PING, blk_ping, AES_BLOCK_SZ);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
    ret = sys_status_crypt(MBEDTLS_AES_DECRYPT, key, blk_ping, AES_BLOCK_SZ, blk_ping);
    if (ret != 0) {
        status = SYS_STATUS_ERR_CRYPT;
        goto exit;
    }
    hdr = (struct sys_status_header_t *)blk_ping;
    cnt_ping = hdr->act_cntr;
#if (IBL_VERSION >= V_1_1)
    if (hdr->magic != SYS_STATUS_MAGIC_CODE) {
        ping_valid = 0;
    }
#endif

    /* Read PONG header */
    ret = flash_read_indirect(FLASH_OFFSET_SYS_STATUS_PONG, blk_pong, AES_BLOCK_SZ);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
    ret = sys_status_crypt(MBEDTLS_AES_DECRYPT, key, blk_pong, AES_BLOCK_SZ, blk_pong);
    if (ret != 0) {
        status = SYS_STATUS_ERR_CRYPT;
        goto exit;
    }
    hdr = (struct sys_status_header_t *)blk_pong;
    cnt_pong = hdr->act_cntr;
#if (IBL_VERSION >= V_1_1)
    if (hdr->magic != SYS_STATUS_MAGIC_CODE) {
        pong_valid = 0;
    }
#endif

#if (IBL_VERSION >= V_1_1)
    if ((ping_valid == 0) && ((pong_valid == 0))) {
        status = SYS_STATUS_ERR_MAGIC;
        goto exit;
    } else if ((ping_valid == 0) && ((pong_valid == 1))) {
        is_ping = 0;
    } else if ((ping_valid == 1) && ((pong_valid == 0))) {
        is_ping = 1;
    } else {
        /* Which is active, PING or PONG ? */
        if (cnt_ping == 0xFFFFFFFF)  /* First use, valid count is from 0 to 0xFFFFFFFE */
            cnt_ping = 0;
        if (cnt_pong == 0xFFFFFFFF)  /* First use, valid count is from 0 to 0xFFFFFFFE */
            cnt_pong = 0;

        if ((cnt_ping == (cnt_pong + 1)) || (cnt_ping == (cnt_pong + 2))) {
            is_ping = 1;
        } else {
            is_ping = 0;
        }
    }
#else
    /* Which is active, PING or PONG ? */
    if (cnt_ping == 0xFFFFFFFF)  /* First use, valid count is from 0 to 0xFFFFFFFE */
        cnt_ping = 0;
    if (cnt_pong == 0xFFFFFFFF)  /* First use, valid count is from 0 to 0xFFFFFFFE */
        cnt_pong = 0;

    if ((cnt_ping == (cnt_pong + 1)) || (cnt_ping == (cnt_pong + 2))) {
        is_ping = 1;
    } else {
        is_ping = 0;
    }
#endif

    if (is_ping) {
        memcpy((void*)sys_hdr, blk_ping, SYS_STATUS_HEADER_SZ);
        *flash_offset = FLASH_OFFSET_SYS_STATUS_PING;
    } else {
        memcpy((void*)sys_hdr, blk_pong, SYS_STATUS_HEADER_SZ);
        *flash_offset = FLASH_OFFSET_SYS_STATUS_PONG;
    }

    status = SYS_STATUS_OK;
exit:
    return status;
}

static int find_tlv_with_type(uint8_t *buf, uint32_t len, uint8_t type)
{
    uint8_t *p, t, l;
    uint32_t offset = 0;
    int find = -1;

    /* Check if the type has already existed. */
    p = buf;
    while (offset < len) {
        t = *p;  l = *(p + 1);
        if (t == type) {
            find = offset;
            break;
        }
        p += SYS_STATUS_TLV_HEADER_SZ + l;
        offset += SYS_STATUS_TLV_HEADER_SZ + l;
    }
    return find;
}

int sys_status_init(void)
{
    struct sys_status_header_t sys_hdr;
    uint8_t key[AES_KEY_SZ];
    int ret, status = SYS_STATUS_OK;

    /* Derive AES key from HUK */
    ret = derive_sys_status_crypt_key(key, AES_KEY_SZ);
    if (ret != KD_SUCCESS) {
        status = SYS_STATUS_ERR_KEY;
        goto exit;
    }

    /* Fill with an empty header of sys status */
    sys_hdr.tot_len = AES_BLOCK_SZ;
    sys_hdr.act_cntr = 0;
    sys_hdr.magic = SYS_STATUS_MAGIC_CODE;
    sys_hdr.checksum = (sys_hdr.tot_len ^ sys_hdr.magic);
    ret = sys_status_crypt(MBEDTLS_AES_ENCRYPT, key, (uint8_t*)&sys_hdr, AES_BLOCK_SZ, (uint8_t*)&sys_hdr);
    if (ret != 0) {
        status = SYS_STATUS_ERR_CRYPT;
        goto exit;
    }

    /* Erase PING and PONG */
    ret = flash_erase(FLASH_OFFSET_SYS_STATUS_PING, SYS_STATUS_AREA_SZ * 2);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
    ret = flash_write(FLASH_OFFSET_SYS_STATUS_PING, (void*)&sys_hdr, AES_BLOCK_SZ);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
    ret = flash_write(FLASH_OFFSET_SYS_STATUS_PONG, (void*)&sys_hdr, AES_BLOCK_SZ);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
    ibl_memset(key, 0, sizeof(key));
exit:
    return status;
}

int sys_status_check_integrity(void)
{
    struct sys_status_header_t sys_hdr;
    uint8_t key[AES_KEY_SZ];
    uint32_t flash_offset, alen, sz;
    uint8_t *buf = NULL, *p, t, l;
    int ret, status;

    /* Derive AES key from HUK */
    ret = derive_sys_status_crypt_key(key, AES_KEY_SZ);
    if (ret != KD_SUCCESS) {
        status = SYS_STATUS_ERR_KEY;
        goto exit;
    }

    /* Found the active system status */
    status = get_active_header(key, &sys_hdr, &flash_offset);
    if (status != SYS_STATUS_OK) {
        goto exit;
    }

    /* Check header */
    if ((sys_hdr.tot_len > SYS_STATUS_AREA_SZ) || (sys_hdr.tot_len < SYS_STATUS_HEADER_SZ)) {
        status = SYS_STATUS_ERR_TOTAL_LEN;
        goto exit;
    }
    if (sys_hdr.magic != SYS_STATUS_MAGIC_CODE) {
        status = SYS_STATUS_ERR_MAGIC;
        goto exit;
    }

    /* Get active system status */
    if (sys_hdr.tot_len % AES_BLOCK_SZ) {
        alen = ((sys_hdr.tot_len >> 4) << 4) + AES_BLOCK_SZ;
    } else {
        alen = sys_hdr.tot_len;
    }
    buf = mbedtls_calloc(alen, 1);
    if (NULL == buf) {
        status = SYS_STATUS_ERR_MEM;
        goto exit;
    }
    ret = flash_read_indirect(flash_offset, buf, alen);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
    ret = sys_status_crypt(MBEDTLS_AES_DECRYPT, key, buf, alen, buf);
    if (ret != 0) {
        status = SYS_STATUS_ERR_CRYPT;
        goto exit;
    }

    /* Check checksum */
    if (cal_checksum(buf, sys_hdr.tot_len) != 0) {
        status = SYS_STATUS_ERR_CHECKSUM;
        goto exit;
    }

    /* Check if the TLVs are correct */
    p = buf + SYS_STATUS_HEADER_SZ;
    sz = sys_hdr.tot_len - SYS_STATUS_HEADER_SZ;
    while (sz > 0) {
        t = *p; l = *(p + 1);
        if ((t == SYS_UNKNOWN_TYPE) || (l > MAX_TLV_VALUE_SIZE)) {
            status = SYS_STATUS_ERR_TLV;
            goto exit;
        }
        p += SYS_STATUS_TLV_HEADER_SZ + l;
        sz -= SYS_STATUS_TLV_HEADER_SZ + l;
    }
    if (sz != 0) {
        status = SYS_STATUS_ERR_TLV;
        goto exit;
    }

    status = SYS_STATUS_OK;

exit:
    ibl_memset(key, 0, sizeof(key));
    if (buf)
        mbedtls_free(buf);
    return status;
}

int sys_status_set_internal(uint8_t type, uint8_t len, uint8_t* pval)
{
    struct sys_status_header_t sys_hdr, *hdr;
    uint8_t key[AES_KEY_SZ];
    uint32_t start, newstart, tlen, nlen, olen;
    uint8_t *buf = NULL, *p;
    int loc, ret, status;

    if ((len > MAX_TLV_VALUE_SIZE) || (len == 0)) {
        status = SYS_STATUS_ERR_INPUT;
        goto exit1;
    }

    /* Derive AES key from HUK */
    ret = derive_sys_status_crypt_key (key, AES_KEY_SZ);
    if (ret != KD_SUCCESS) {
        status = SYS_STATUS_ERR_KEY;
        goto exit1;
    }

    /* Found the active system status */
    status = get_active_header(key, &sys_hdr, &start);
    if (status != SYS_STATUS_OK) {
        goto exit;
    }

    /* Check header */
    if (sys_hdr.tot_len > SYS_STATUS_AREA_SZ) {
        status = SYS_STATUS_ERR_TOTAL_LEN;
        goto exit;
    }
    if (sys_hdr.magic != SYS_STATUS_MAGIC_CODE) {
        status = SYS_STATUS_ERR_MAGIC;
        goto exit;
    }

    /* Get active system status */
    if (sys_hdr.tot_len % AES_BLOCK_SZ) {
        tlen = ((sys_hdr.tot_len >> 4) << 4) + AES_BLOCK_SZ;
    } else {
        tlen = sys_hdr.tot_len;
    }
    nlen = sys_hdr.tot_len + SYS_STATUS_TLV_HEADER_SZ + len;
    if (nlen % AES_BLOCK_SZ) {
        nlen = ((nlen >> 4) << 4) + AES_BLOCK_SZ;
    }
    buf = mbedtls_calloc(nlen, 1);  // consider new tlv entry
    if (NULL == buf) {
        status = SYS_STATUS_ERR_MEM;
        goto exit;
    }
    ret = flash_read_indirect(start, buf, tlen);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
    ret = sys_status_crypt(MBEDTLS_AES_DECRYPT, key, buf, tlen, buf);
    if (ret != 0) {
        status = SYS_STATUS_ERR_CRYPT;
        goto exit;
    }

    /* Add or update TLV */
    loc = find_tlv_with_type((buf + SYS_STATUS_HEADER_SZ), (sys_hdr.tot_len - SYS_STATUS_HEADER_SZ), type);    /* Check if found */
    if (loc >= 0) {  // already existed, need to update
        olen = *(buf + AES_BLOCK_SZ + loc + 1);
        if (olen == len) {
            p = buf + AES_BLOCK_SZ + loc + SYS_STATUS_TLV_HEADER_SZ;
            memcpy(p, pval, olen);
        } else {
            status = SYS_STATUS_ERR_TLV;
            goto exit;
        }
    } else {  // new tlv
        if (sys_hdr.tot_len + SYS_STATUS_TLV_HEADER_SZ + len > SYS_STATUS_AREA_SZ) {
            status = SYS_STATUS_ERR_TOTAL_LEN;
            goto exit;
        }
        p = buf + sys_hdr.tot_len;
        *p++ = type;
        *p++ = len;
        memcpy(p, pval, len);
        p += len;
        /* padding with 0xFF for 16-bytes alignment */
        memset(p, 0xFF, (nlen - ((uint32_t)p - (uint32_t)buf)));
        /* update total length */
        sys_hdr.tot_len += SYS_STATUS_TLV_HEADER_SZ + len;
        tlen = nlen;
    }

    /* Update header */
    hdr = (struct sys_status_header_t *)buf;
    hdr->tot_len = sys_hdr.tot_len;
    if (sys_hdr.act_cntr == 0xFFFFFFFE)
        hdr->act_cntr = 0;
    else
        hdr->act_cntr = sys_hdr.act_cntr + 1;
    hdr->checksum = 0;
    hdr->checksum = cal_checksum(buf, hdr->tot_len);

    /* Encrypt it and write to another entry */
    ret = sys_status_crypt(MBEDTLS_AES_ENCRYPT, key, buf, tlen, buf);
    if (ret != 0) {
        status = SYS_STATUS_ERR_CRYPT;
        goto exit;
    }
    if (start == FLASH_OFFSET_SYS_STATUS_PING)
        newstart = FLASH_OFFSET_SYS_STATUS_PONG;
    else
        newstart = FLASH_OFFSET_SYS_STATUS_PING;
//    ret = flash_erase(newstart, SYS_STATUS_AREA_SZ);
    ret = flash_erase(newstart, SYS_STATUS_AREA_SZ);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
#if (IBL_VERSION == V_1_0)
    ret = flash_write(newstart, buf, tlen);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
#else
    ret = flash_write((newstart + AES_BLOCK_SZ), (buf + AES_BLOCK_SZ), (tlen - AES_BLOCK_SZ));
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
    ret = flash_write(newstart, buf, AES_BLOCK_SZ);
    if (ret != 0) {
        status = SYS_STATUS_ERR_FLASH;
        goto exit;
    }
#endif

    status = SYS_STATUS_OK;

exit:
    if (buf)
        mbedtls_free(buf);
exit1:
    ibl_memset(key, 0, sizeof(key));
    if (status != SYS_STATUS_OK)
        ibl_trace(IBL_WARN, "Status set failed(0x%x,%d).\r\n", status, ret);
    return status;
}

int sys_status_get_internal(uint8_t type, uint8_t len, uint8_t* pval)
{
    struct sys_status_header_t sys_hdr;
    uint8_t key[AES_KEY_SZ];
    uint32_t start, tlen;
    uint8_t *buf = NULL, *p, l;
    int loc, ret, status;

    /* Derive AES key from HUK */
    ret = derive_sys_status_crypt_key(key, AES_KEY_SZ);
    if (ret != 0) goto exit1;

    /* Found the active system status */
    ret = get_active_header(key, &sys_hdr, &start);
    if (ret != SYS_STATUS_OK) goto exit;

    /* Check header */
    if (sys_hdr.tot_len > SYS_STATUS_AREA_SZ) {
        ret = SYS_STATUS_ERR_TOTAL_LEN;
        goto exit;
    }
    if (sys_hdr.magic != SYS_STATUS_MAGIC_CODE) {
        ret = SYS_STATUS_ERR_MAGIC;
        goto exit;
    }

    /* Get active system status */
    if (sys_hdr.tot_len % AES_BLOCK_SZ) {
        tlen = ((sys_hdr.tot_len >> 4) << 4) + AES_BLOCK_SZ;
    } else {
        tlen = sys_hdr.tot_len;
    }
    buf = mbedtls_calloc(tlen, 1);
    if (NULL == buf) goto exit;

    ret = flash_read_indirect(start, buf, tlen);
    if (ret != 0) goto exit;

    ret = sys_status_crypt(MBEDTLS_AES_DECRYPT, key, buf, tlen, buf);
    if (ret != 0) goto exit;

    /* Find item with the type */
    loc = find_tlv_with_type((buf + SYS_STATUS_HEADER_SZ), (sys_hdr.tot_len - SYS_STATUS_HEADER_SZ), type);    /* Check if found */
    if (loc >= 0) {
        /* Found */
        p = buf + SYS_STATUS_HEADER_SZ + loc;
        if (*(p + 1) > len)
            memcpy(pval, (p + SYS_STATUS_TLV_HEADER_SZ), len);
        else
            memcpy(pval, (p + SYS_STATUS_TLV_HEADER_SZ), *(p + 1));
        status = SYS_STATUS_FOUND_OK;
    } else {
        /* Not found */
        status = SYS_STATUS_NOT_FOUND;
        memset(pval, 0xFF, len);
    }

    ibl_memset(key, 0, sizeof(key));
    mbedtls_free(buf);
    return status;

exit:
    if (buf)
        mbedtls_free(buf);
    memset(pval, 0xFF, len);
exit1:
    ibl_memset(key, 0, sizeof(key));
    status = SYS_STATUS_FOUND_ERR;
    ibl_trace(IBL_WARN, "Status get failed(0x%x,%d).\r\n", status, ret);
    return status;
}

int sys_set_fw_version_internal(uint32_t type, uint32_t version)
{
    uint8_t status_type;
    uint32_t local_ver = 0;
    int ret;

    if (type == IMG_TYPE_MBL)
        status_type = SYS_MBL_VER_COUNTER;
    else
        status_type = SYS_IMG_VER_COUNTER;

    ret = sys_status_get_internal(status_type, LEN_SYS_VER_COUNTER, (uint8_t *)&local_ver);
    if ((ret != SYS_STATUS_NOT_FOUND) && (ret != SYS_STATUS_FOUND_OK)) {
        return ret;
    }
    if (ret == SYS_STATUS_NOT_FOUND)
        local_ver = 0;

    ibl_trace(IBL_INFO, "local version: %d.%d.%d\r\n",
                    ((local_ver >> 24) & 0xff), ((local_ver >> 16) & 0xff), (local_ver & 0xffff));
    /* New version MUST BE higher than local */
    if (version <= local_ver) {
        return 0;
    }

    return sys_status_set_internal(status_type, LEN_SYS_VER_COUNTER, (uint8_t *)&version);
}

void sys_status_show(void)
{
    struct sys_status_header_t sys_hdr;
    uint32_t start, tlen, offset = 0, sz;
    uint8_t key[AES_KEY_SZ], *buf = NULL, *p;
    uint8_t t, l, *v;
    char *title;
    int ret;

    /* Derive AES key from HUK */
    ret = derive_sys_status_crypt_key(key, AES_KEY_SZ);
    if (ret != 0) goto exit1;

    /* Found the active system status */
    ret = get_active_header(key, &sys_hdr, &start);
    if (ret != SYS_STATUS_OK) goto exit;

    /* Check header */
    if ((sys_hdr.tot_len > SYS_STATUS_AREA_SZ) || (sys_hdr.magic != SYS_STATUS_MAGIC_CODE)) {
        ret = -0xFF;
        goto exit;
    }

    if (start == FLASH_OFFSET_SYS_STATUS_PING)
        title = "\r\nSystem Status: Ping\r\n";
    else
        title = "\r\nSystem Status: Pong\r\n";

    ibl_printf("%s=============================\r\n", title);
    ibl_printf("Total Length: 0x%x (%d)\t\r\n", sys_hdr.tot_len, sys_hdr.tot_len);
    ibl_printf("Active Counter: 0%x\t\r\n", sys_hdr.act_cntr);
    ibl_printf("Checksum: 0x%x\t\t\r\n", sys_hdr.checksum);

    /* Get active system status */
    if (sys_hdr.tot_len % AES_BLOCK_SZ) {
        tlen = ((sys_hdr.tot_len >> 4) << 4) + AES_BLOCK_SZ;
    } else {
        tlen = sys_hdr.tot_len;
    }
    buf = mbedtls_calloc(tlen, 1);
    if (NULL == buf) goto exit;

    ret = flash_read_indirect(start, buf, tlen);
    if (ret != 0) goto exit;

    ret = sys_status_crypt(MBEDTLS_AES_DECRYPT, key, buf, tlen, buf);
    if (ret != 0) goto exit;

    ibl_printf("T\tL\tV\r\n");

    /* List types. */
    p = buf + SYS_STATUS_HEADER_SZ;
    sz = sys_hdr.tot_len - SYS_STATUS_HEADER_SZ;
    while (offset < sz) {
        t = *p;  l = *(p + 1);  v = p + 2;
        print_tlv(t, l, v);
        p += SYS_STATUS_TLV_HEADER_SZ + l;
        offset += SYS_STATUS_TLV_HEADER_SZ + l;
    }

exit:
    if (buf)
        mbedtls_free(buf);
exit1:
    ibl_memset(key, 0, sizeof(key));
    if (ret != 0)
        ibl_trace(IBL_WARN, "Status show error(0x%x).\r\n", ret);
    return;
}

void sys_status_dump(int is_ping)
{
    struct sys_status_header_t *sys_hdr;
    uint32_t start, tlen;
    uint8_t key[AES_KEY_SZ], blk[AES_BLOCK_SZ], *buf = NULL;
    char *title;
    int ret;

    if (is_ping ) {
        start = FLASH_OFFSET_SYS_STATUS_PING;
        title = "Status PING:\r\n";
    } else {
        start = FLASH_OFFSET_SYS_STATUS_PONG;
        title = "Status PONG:\r\n";
    }

    /* Derive AES key from HUK */
    ret = derive_sys_status_crypt_key(key, AES_KEY_SZ);
    if (ret != 0) goto exit1;

    /* Read header */
    ret = flash_read_indirect(start, blk, AES_BLOCK_SZ);
    if (ret != 0) goto exit;

    ret = sys_status_crypt(MBEDTLS_AES_DECRYPT, key, blk, AES_BLOCK_SZ, blk);
    if (ret != 0) goto exit;

    sys_hdr = (struct sys_status_header_t *)blk;

    /* Check header */
    if ((sys_hdr->tot_len > SYS_STATUS_AREA_SZ) || (sys_hdr->magic != SYS_STATUS_MAGIC_CODE)) {
        ret = -0xFF;
        goto exit;
    }

    /* Allocate memory for decrypt TLVs */
    if (sys_hdr->tot_len % AES_BLOCK_SZ) {
        tlen = ((sys_hdr->tot_len >> 4) << 4) + AES_BLOCK_SZ;
    } else {
        tlen = sys_hdr->tot_len;
    }
    buf = mbedtls_calloc(tlen, 1);
    if (NULL == buf) goto exit;

    ret = flash_read_indirect(start, buf, tlen);
    if (ret != 0) goto exit;

    ret = sys_status_crypt(MBEDTLS_AES_DECRYPT, key, buf, tlen, buf);
    if (ret != 0) goto exit;

    ibl_print_data(IBL_ALWAYS, title, buf, sys_hdr->tot_len);

exit:
    if (buf)
        mbedtls_free(buf);
exit1:
    ibl_memset(key, 0, sizeof(key));
    if (ret != 0)
        ibl_trace(IBL_WARN, "Status dump error(0x%x).\r\n", ret);
    return;
}

#endif

#if 0
int sys_jtag_challenge(IN uint8_t *plaintext, IN uint8_t *ciphertext)
{
#define SYS_JTAG_MAX_RETRIES    10
    int result, ret, first_try = 0;
    uint16_t cnt;
    uint8_t jtag_status;

    ret = sys_status_get_internal(SYS_JTAG_UNLOCK_COUNTER, LEN_SYS_UNLOCK_COUNTER, (uint8_t *)(&cnt));
    if (ret == SYS_STATUS_NOT_FOUND) {
        /* Not found JTAG unlock counter in the system status. First try. */
        first_try = 1;
        cnt = 1;
    } else if (ret <= SYS_STATUS_FOUND_ERR) {
        return ret;
    }

    /* Check if counter is full. */
    if (cnt >= SYS_JTAG_MAX_RETRIES) {
        /* Locked, no retry */
        return -1;
    }

    result = efuse_check_jtag_enable(plaintext, ciphertext);
    if (result == 1) {
        /* Jtag unlock, save status to system status */
        // TODO, hw operation to unlock jtag

        /* Reset jtag unlock counter */
        cnt = 0;
        ret = sys_status_set_internal(SYS_JTAG_UNLOCK_COUNTER, LEN_SYS_UNLOCK_COUNTER, (uint8_t *)&cnt);
        if (ret != 0) goto exit;
    } else {
        /* Keep jtag lock, increase jtag unlock counter */
        if (!first_try) {
            cnt++;
        }
        ret = sys_status_set_internal(SYS_JTAG_UNLOCK_COUNTER, LEN_SYS_UNLOCK_COUNTER, (uint8_t *)&cnt);
        if (ret != 0) goto exit;
    }

    return 0;

exit:
    return -2;
}
#endif

#ifdef ROM_SELF_TEST
void sys_status_self_test(void)
{
    uint32_t value = 0, expect_val;
    int ret;

    if (flash_offset_sys_status == 0) {
        if (get_sys_setting_info(&flash_offset_sys_status, NULL)) {
            flash_offset_sys_status = 0x8000;
        }
    }

    ibl_printf("\r\n========= Test Start =============\r\n");
    ibl_printf("\r\n========= Test 0 =============\r\n");
#if (SYS_STATUS_ENCRPTED == 1)
    sys_status_init();
#else
    flash_erase(flash_offset_sys_status, 0x2000);
#endif
    sys_status_show();
    sys_status_dump(1);
    sys_status_dump(0);

    ibl_printf("\r\n========= Test 1 =============\r\n");
    ibl_printf("Set Error Process: %d. 0 for ISP, 1 for while(1)\r\n", 0);
    ret = sys_set_err_process(0);
    if (ret != 0) {
        ibl_printf("Set error (%d)\r\n", ret);
        return;
    }
    ret = sys_status_get_internal(SYS_ERROR_PROCESS, LEN_SYS_ERROR_PROCESS, (uint8_t *)&value);
    if (ret != 0) {
        ibl_printf("Get error (%d)\r\n", ret);
        return;
    }
    ibl_printf("Get Error Process: %d. \r\n", value);

    ibl_printf("\r\n========= Test 2 =============\r\n");
    ibl_printf("Set Trace Level: %d\r\n", IBL_DBG);
    sys_set_trace_level(IBL_DBG);
    sys_status_get_internal(SYS_TRACE_LEVEL, LEN_SYS_TRACE_LEVEL, (uint8_t *)&value);
    ibl_printf("Get Trace Level: %d\r\n", value);

    ibl_printf("\r\n========= Test 3 =============\r\n");
    ret = sys_set_img_flag(IMAGE_0, (IMG_FLAG_NEWER_MASK | IMG_FLAG_VERIFY_MASK), (IMG_FLAG_NEWER | IMG_FLAG_VERIFY_OK));
    if (ret != 0) {
        ibl_printf("Set error (%d)\r\n", ret);
        return;
    }
    expect_val = (IMG_FLAG_NEWER | IMG_FLAG_VERIFY_OK);
    ret = sys_status_get_internal(SYS_IMAGE0_STATUS, LEN_SYS_IMAGE_STATUS, (uint8_t *)&value);
    if (expect_val != value) {
        ibl_printf("Set (%d), but get (%d), ret %d\r\n", expect_val, value, ret);
        return;
    }
    ibl_printf("Image 0 status: 0x%x\r\n", value);
    sys_set_img_flag(IMAGE_0, IMG_FLAG_VERIFY_MASK, IMG_FLAG_VERIFY_FAIL);
    sys_status_get_internal(SYS_IMAGE0_STATUS, LEN_SYS_IMAGE_STATUS, (uint8_t *)&value);
    ibl_printf("If set verify failed, Image 0 status: 0x%x\r\n", value);

    ibl_printf("\r\n========= Test 4 =============\r\n");
    sys_set_img_flag(IMAGE_0, (IMG_FLAG_NEWER_MASK | IMG_FLAG_VERIFY_MASK | IMG_FLAG_IA_MASK), (IMG_FLAG_NEWER | IMG_FLAG_VERIFY_OK | IMG_FLAG_IA_OK));
    sys_status_get_internal(SYS_IMAGE0_STATUS, LEN_SYS_IMAGE_STATUS, (uint8_t *)&value);
    ibl_printf("Image 0 status: 0x%x\r\n", value);
    sys_set_img_flag(IMAGE_0, IMG_FLAG_VERIFY_MASK, IMG_FLAG_VERIFY_FAIL);
    sys_status_get_internal(SYS_IMAGE0_STATUS, LEN_SYS_IMAGE_STATUS, (uint8_t *)&value);
    ibl_printf("If set verify failed, Image 0 status: 0x%x\r\n", value);
    sys_reset_img_flag(IMAGE_0);
    sys_status_get_internal(SYS_IMAGE0_STATUS, LEN_SYS_IMAGE_STATUS, (uint8_t *)&value);
    ibl_printf("After reset flag, Image 0 status: 0x%x\r\n", value);

    ibl_printf("\r\n========= Test 5 =============\r\n");
    ibl_printf("Set Running Image Index: %d\r\n", IMAGE_1);
    sys_set_running_img(IMAGE_1);
    sys_status_get_internal(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, (uint8_t *)&value);
    ibl_printf("Get Running Image Index: %d\r\n", value);

    ibl_printf("\r\n========= Test 6 =============\r\n");
    ibl_printf("Set MBL Version: 0x%x\r\n", 0x02020005);
    ibl_printf("Set IMG Version: 0x%x\r\n", 0x01000006);
    sys_set_fw_version(IMG_TYPE_MBL, 0x02020005);
    sys_set_fw_version(IMG_TYPE_IMG, 0x01000006);
    sys_status_get_internal(SYS_MBL_VER_COUNTER, LEN_SYS_VER_COUNTER, (uint8_t *)&value);
    ibl_printf("Get MBL Version: 0x%x\r\n", value);
    sys_status_get_internal(SYS_IMG_VER_COUNTER, LEN_SYS_VER_COUNTER, (uint8_t *)&value);
    ibl_printf("Get IMG Version: 0x%x\r\n", value);

    ibl_printf("\r\n========= Test 7 =============\r\n");
    ibl_printf("Set MBL PK Version: %d\r\n", 2);
    ibl_printf("Set AROT PK Version: %d\r\n", 4);
    sys_set_pk_version(PK_TYPE_MBL, 2);
    sys_set_pk_version(PK_TYPE_AROT, 4);
    sys_status_get_internal(SYS_MBLPK_VER_COUNTER, LEN_SYS_PKVER_COUNTER, (uint8_t *)&value);
    ibl_printf("Get MBL PK Version: %d\r\n", (uint8_t)value);
    sys_status_get_internal(SYS_AROTPK_VER_COUNTER, LEN_SYS_PKVER_COUNTER, (uint8_t *)&value);
    ibl_printf("Get AROT PK Version: %d\r\n", (uint8_t)value);

    ibl_printf("\r\n========= Test 8 =============\r\n");
    sys_status_show();
    sys_status_dump(1);
    sys_status_dump(0);
    ibl_printf("\r\n========= Test End =============\r\n");
}
#endif


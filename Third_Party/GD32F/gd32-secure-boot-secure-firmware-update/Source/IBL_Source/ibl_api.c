/*!
    \file    ibl_api.c
    \brief   IBL api for GD32 SDK

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
#include "ibl_api.h"

#include "Platform/IBL/ymodem.h"

/* Secure APIs */
const uint32_t ibl_api[MAX_API_NUM] = {
    /* COMMON */
    (uint32_t)log_uart_init,
    (uint32_t)ibl_printf,
    (uint32_t)uart_putc,
    (uint32_t)ibl_trace_ex,
    (uint32_t)ibl_rand,
    (uint32_t)ibl_set_mutex_func,
    (uint32_t)ibl_memset,
    (uint32_t)ibl_memcmp,
    (uint32_t)ibl_strlen,
    (uint32_t)ibl_strncmp,

    (uint32_t)uart_rx_to,
    (uint32_t)ibl_strtoul,

    /* VERIFY */
    (uint32_t)cal_checksum,
    (uint32_t)img_verify_sign,
    (uint32_t)img_verify_hash,
    (uint32_t)img_verify_hdr,
    (uint32_t)img_verify_pkhash,
    (uint32_t)img_validate,
    0,                       /* no this cert_validate now */

    /* SYS */
    (uint32_t)sys_setting_get,
    (uint32_t)sys_status_set,
    (uint32_t)sys_status_get,
    (uint32_t)sys_set_trace_level,
    (uint32_t)sys_set_err_process,
    (uint32_t)sys_set_img_flag,
    (uint32_t)sys_reset_img_flag,
    (uint32_t)sys_set_running_img,
    (uint32_t)sys_set_fw_version,
    (uint32_t)sys_set_pk_version,
    (uint32_t)sys_set_trng_seed,
    
    /* boot */
    (uint32_t)jump_to_img,
    (uint32_t)ymodem_download_check,

    /* FLASH */
    (uint32_t)is_valid_flash_offset,
    (uint32_t)is_valid_flash_addr,
    (uint32_t)flash_total_size,
    (uint32_t)flash_erase_size,
    (uint32_t)flash_init,
    (uint32_t)flash_read,
    (uint32_t)flash_write,
    (uint32_t)flash_write_fast,
    (uint32_t)flash_erase,

    /* FMC APIs */
    (uint32_t)fmc_unlock,
    (uint32_t)fmc_lock,
    (uint32_t)fmc_flag_clear,
#if defined(PLATFORM_GD32F5XX)
    (uint32_t)fmc_page_erase,
    (uint32_t)fmc_mass_erase,
#endif

#if defined(PLATFORM_GD32A513) || defined(PLATFORM_GD32G5X3)
    (uint32_t)fmc_doubleword_program,
#else
    (uint32_t)fmc_word_program,
#endif

#ifndef SECURITY_PROTECT_DISABLE
    (uint32_t)ibl_fwdg_reload,
#endif /* SECURITY_PROTECT_DISABLE */


//#if (IBL_VERSION >= V_1_1)
//    (uint32_t)do_symm_key_derive,
//#endif
};

static int enter_ibl_protection(uint32_t *vtor_value, uint32_t *hardfault_target, uint32_t *icache_enabled)
{
    /* unimplemented yet */
    return 0;
}

static void exit_ibl_protection(uint32_t vtor_value, uint32_t hardfault_target, uint32_t icache_enabled)
{
    /* unimplemented yet */
}

int sys_setting_get(void *settings)
{
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    ret = sys_setting_get_internal(settings);
exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_status_set(uint8_t type, uint8_t len, uint8_t *pval)
{
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    if (type >= SYS_MAX_ROM_TYPE) {
        ret = -1;
        goto exit;
    }

    ret = sys_status_set_internal(type, len, pval);

exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_status_get(uint8_t type, uint8_t len, uint8_t* pval)
{
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    ret = sys_status_get_internal(type, len, pval);

exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_set_err_process(uint8_t method)
{
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    if (method > 1) {
        ret = -1;
        goto exit;
    }
    ret = sys_status_set_internal(SYS_ERROR_PROCESS, LEN_SYS_ERROR_PROCESS, &method);

exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_set_trace_level(uint8_t trace_level)
{
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    if (trace_level > IBL_DBG) {
        ret = -1;
        goto exit;
    }
    ret = sys_status_set_internal(SYS_TRACE_LEVEL, LEN_SYS_TRACE_LEVEL, &trace_level);

exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_set_img_flag(uint8_t idx, uint8_t mask, uint8_t flag)
{
    uint8_t type, img_status = 0;
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    if (idx == IMAGE_0)
        type = SYS_IMAGE0_STATUS;
    else if (idx == IMAGE_1)
        type = SYS_IMAGE1_STATUS;
    else {
        ret = -3;
        goto exit;
    }

    ret = sys_status_get_internal(type, LEN_SYS_IMAGE_STATUS, &img_status);
    if ((ret != SYS_STATUS_NOT_FOUND) && (ret != SYS_STATUS_FOUND_OK)) {
        goto exit;
    }
    if (ret == SYS_STATUS_NOT_FOUND)
        img_status = 0;

    /* the flag bits have been set in img_status */
    if ((img_status & mask) == flag) {
        ret = 0;
        goto exit;
    }

    img_status = ((img_status & ~mask) | flag);

    if (!is_valid_image_status(img_status)) {
        ret = -4;
        goto exit;
    }

    ret = sys_status_set_internal(type, LEN_SYS_IMAGE_STATUS, &img_status);

exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_reset_img_flag(uint8_t idx)
{
    uint8_t type, img_status = 0;
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    if (idx == IMAGE_0)
        type = SYS_IMAGE0_STATUS;
    else if (idx == IMAGE_1)
        type = SYS_IMAGE1_STATUS;
    else {
        ret = -1;
        goto exit;
    }

    ret = sys_status_set_internal(type, LEN_SYS_IMAGE_STATUS, &img_status);

exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_set_running_img(uint8_t idx)
{
    uint8_t img_idx = 0xff;
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    ret = sys_status_get_internal(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &img_idx);
    if ((ret != SYS_STATUS_NOT_FOUND) && (ret != SYS_STATUS_FOUND_OK)) {
        goto exit;
    }

    /* the index to set is same as current running index */
    if (img_idx == idx) {
        ret = 0;
        goto exit;
    }

    if ((idx == IMAGE_0) || (idx == IMAGE_1))
        ret = sys_status_set_internal(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &idx);
    else {
        ret = -3;
    }
exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_set_fw_version(uint32_t type, uint32_t version)
{
    uint8_t status_type;
    uint32_t local_ver = 0;
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    ret = sys_set_fw_version_internal(type, version);

exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_set_pk_version(uint32_t type, uint8_t key_ver)
{
    uint8_t status_type;
    uint8_t local_ver = 0;
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    if (type == PK_TYPE_MBL)
        status_type = SYS_MBLPK_VER_COUNTER;
    else if (type == PK_TYPE_AROT)
        status_type = SYS_AROTPK_VER_COUNTER;
    else {
        ret = -3;
        goto exit;
    }

    ret = sys_status_get_internal(status_type, LEN_SYS_PKVER_COUNTER, (uint8_t *)&local_ver);
    if ((ret != SYS_STATUS_NOT_FOUND) && (ret != SYS_STATUS_FOUND_OK)) {
        goto exit;
    }
    if (ret == SYS_STATUS_NOT_FOUND)
        local_ver = 0;

    /* New public key version MUST BE higher than local */
    if (key_ver <= local_ver) {
        ret = 0;
        goto exit;
    }

    ret = sys_status_set_internal(status_type, LEN_SYS_PKVER_COUNTER, &key_ver);

exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int sys_set_trng_seed(uint8_t val)
{
    uint8_t is_trng_seed = val ? 1 : 0;
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    /* unimplemented yet */
#if defined(PLATFORM_GDM32)
    if (!IS_READ_PROTECT()) {
        btrng_seed = val;
        ret = sys_status_set_internal(SYS_TRNG_SEED, LEN_SYS_TRNG_SEED, &is_trng_seed);
    } else {
        ret = 0;
    }
#elif defined (PLATFORM_MSP_AN521)
    btrng_seed = 0;
    ret = 0;
#endif
exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

#if defined(PLATFORM_GDM32) || defined(PLATFORM_GD32F5XX)
const char cant_write[] = "Locked, can't write.\r\n";
const char cant_read[] = "No reading.\r\n";
uint8_t efuse_get_ctl(void)
{
    uint32_t ctl = 0;
//    uint32_t vtor_value, hardfault_target, icache_enabled;

//    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
//        return 0;
//    }
#if defined(PLATFORM_GD32F5XX)
    efuse_read(EFUSE_CTL_EFADDR, 1, &ctl);
#endif /* PLATFORM_GD32F5XX */
//exit:
//    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ctl;
}

int efuse_set_ctl(uint32_t ctl)
{
    uint32_t usctl;
//    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

//    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
//        return -0xFF;
//    }
#if defined(PLATFORM_GD32F5XX)
    usctl = EFUSE_CTL;
    if (usctl & EFUSE_CTL_LK) {
        ibl_trace(IBL_WARN, cant_write);
        ret = -1;
    } else {
        ret = efuse_control_write(ctl);
        if (ret != EFUSE_READY)
            ret = -2;
    }
#endif /* PLATFORM_GD32F5XX */
//exit:
//    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

uint8_t efuse_get_tzctl(void)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_set_tzctl(uint8_t tzctl)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

uint8_t efuse_get_fp(void)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_set_fp(uint8_t fp)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

uint8_t efuse_get_usctl(void)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_set_usctl(uint8_t usctl)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_get_mcui(uint8_t *mcui)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_set_mcui(uint8_t *mcui)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_get_aeskey(uint8_t *aeskey)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_set_aeskey(uint8_t *aeskey)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_get_rotpk(uint8_t *rotpk)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}
int efuse_set_rotpk(uint8_t *rotpk)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_get_dbg_pwd(uint8_t *pwd)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_set_dbg_pwd(uint8_t *pwd)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_get_rss(uint8_t *rss)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_set_rss(uint8_t *rss)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_get_uid(uint8_t *uid)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_get_rf(uint8_t *rf, uint32_t offset, uint32_t sz)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_set_rf(uint8_t *rf, uint32_t offset, uint32_t sz)
{
#if defined(PLATFORM_GD32F5XX)
    /* not support */
#endif /* PLATFORM_GD32F5XX */
    return 0;
}

int efuse_get_usdata(uint8_t *usdata, uint32_t offset, uint32_t sz)
{
//    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;
    uint32_t usdata32 = 0;

//    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
//        return -0xFF;
//    }
#if defined(PLATFORM_GD32F5XX)
    ibl_memset(usdata, 0, sz);
    ret = efuse_read(USER_DATA_EFADDR, 1, &usdata32);
    if (ret != EFUSE_READY)
        ret = -1;
    else
        *usdata = (uint8_t)usdata32;
#endif /* PLATFORM_GD32F5XX */
//exit:
//    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}

int efuse_set_usdata(uint8_t *usdata, uint32_t offset, uint32_t sz)
{
    uint8_t usctl;
//    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

//    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
//        return -0xFF;
//    }
#if defined(PLATFORM_GD32F5XX)
    usctl = EFUSE_CTL;
    if (usctl & EFUSE_CTL_UDLK) {
        ibl_trace(IBL_WARN, cant_write);
        ret = -1;
    } else {
        ret = efuse_user_data_write(*usdata);
        if (ret != EFUSE_READY)
            ret = -2;
    }
#endif /* PLATFORM_GD32F5XX */
//exit:
//    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}
#endif  /* PLATFORM_GDM32 */

#if (IBL_VERSION >= V_1_1)
int do_symm_key_derive(uint8_t *label, size_t label_sz,
                    uint8_t *key, size_t key_len)
{
    uint32_t vtor_value, hardfault_target, icache_enabled;
    int ret;

    if (enter_ibl_protection(&vtor_value, &hardfault_target, &icache_enabled)) {
        return -0xFF;
    }

    ret = do_symm_key_derive_inner(label, label_sz, key, key_len);

exit:
    exit_ibl_protection(vtor_value, hardfault_target, icache_enabled);
    return ret;
}
#endif

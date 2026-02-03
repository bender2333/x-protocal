/*!
    \file    ibl_api.h
    \brief   IBL api header file for GD32 SDK

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

#ifndef __IBL_API_H__
#define __IBL_API_H__

#define MAX_API_NUM    256

#include "Platform/IBL/ymodem.h"

struct ibl_api_t {
    void (*log_uart_init)(void);
    int (*printf)(const char *format, ...);
    void (*uart_putc)(uint8_t c);
    int (*trace_ex)(uint32_t level, const char *fmt, ...);
    int (*rand)(unsigned char *output, unsigned int len);
    void (*set_mutex_func)(int tolock, void *func);
    void (*ibl_memset)(void *s, int c, unsigned int count);
    int (*ibl_memcmp)(const void * buf1, const void * buf2, unsigned int count);
    size_t (*ibl_strlen)(const char *str);
    int (*ibl_strncmp)(const char *s1, const char *s2, size_t len);
    
    uint8_t (*uart_rx_to)(ymodem_packet_str *ymodem_packet, uint16_t len, uint32_t timeout);
    unsigned long (*ibl_strtoul)(const char *cp, char **endp, unsigned int base);

    uint32_t (*cal_checksum)(IN uint8_t *ptr, IN uint32_t sz);
    int (*img_verify_sign)(IN uint8_t *hash,
                        IN uint32_t hlen,
                        IN uint8_t *pk,
                        IN uint8_t *sig);
    int (*img_verify_hash)(IN uint32_t faddr,  /* Flash Adress */
                        IN uint32_t len,
                        IN uint8_t *hash,
                        IN uint32_t hlen,
                        IN uint8_t *seed,
                        IN int seed_len);
    int (*img_verify_hdr)(IN void *hdr,
                    IN uint8_t img_type);

    int (*img_verify_pkhash)(IN uint8_t *pk,
                IN uint32_t klen,
                IN uint8_t *pkhash,
                IN uint32_t hlen);

    int (*img_validate)(IN uint32_t img_faddr,
                   IN uint8_t img_type,
                   IN uint8_t *pk,
                   OUT void *img_info);

    int (*cert_validate)(IN uint32_t crt_faddr,
                    IN size_t crt_sz,
                    IN uint8_t *verify_pk,
                    OUT uint8_t *img_pk);

    int (*sys_setting_get)(void *settings);
    int (*sys_status_set)(uint8_t type, uint8_t len, uint8_t *pval);
    int (*sys_status_get)(uint8_t type, uint8_t len, uint8_t *pval);
    int (*sys_set_trace_level)(uint8_t trace_level);
    int (*sys_set_err_process)(uint8_t method);
    int (*sys_set_img_flag)(uint8_t idx, uint8_t mask, uint8_t flag);
    int (*sys_reset_img_flag)(uint8_t idx);
    int (*sys_set_running_img)(uint8_t idx);
    int (*sys_set_fw_version)(uint32_t type, uint32_t version);
    int (*sys_set_pk_version)(uint32_t type, uint8_t key_ver);
    int (*sys_set_trng_seed)(uint8_t val);
    
    /* BOOT */
    void (*jump_to_img)(uint32_t msp, uint32_t reset);
    void (*ymodem_download_check)(ymodem_packet_str *packet_str, delay_ms_fun delay_ms);

    int32_t (*is_valid_flash_offset)(uint32_t offset);
    int32_t (*is_valid_flash_addr)(uint32_t addr);
    uint32_t (*flash_total_size)(void);
    uint32_t (*flash_erase_size)(void);
    int (*flash_init)(void);
    int (*flash_read)(uint32_t addr, void *data, int len);
    int (*flash_write)(uint32_t addr, const void *data, int len);
    int (*flash_write_fast)(uint32_t addr, const void *data, int len);
    int (*flash_erase)(uint32_t addr, int len);

    void (*fmc_unlock)(void);
    void (*fmc_lock)(void);
    void (*fmc_flag_clear)(uint32_t fmc_flag);
#if defined(PLATFORM_GD32F5XX)
    int (*fmc_page_erase)(uint32_t page);
    int (*fmc_mass_erase)(void);
#endif  /* PLATFORM_GDM32 */

#if defined(PLATFORM_GD32A513)
    int (*fmc_word_program)(uint32_t address, uint64_t data);
#else
    int (*fmc_word_program)(uint32_t address, uint32_t data);
#endif


#ifndef SECURITY_PROTECT_DISABLE
    void (*ibl_fwdg_reload)(void);
#endif /* SECURITY_PROTECT_DISABLE */

    int (*do_iak_getpub)(int type, int rsa_keysz,
                            int ecc_gid, int key_format,
                            uint8_t *output, uint32_t *len);
    int (*do_iak_sign)(int type, int rsa_keysz, int ecc_gid,
                   const uint8_t *hash, size_t hash_len,
                   uint8_t *sig, size_t *sig_len);
/* #if (IBL_VERSION >= V_1_1) */
    int (*do_symm_key_derive)(uint8_t *label, size_t label_sz,
                        uint8_t *key, size_t key_len);
/* #endif (IBL_VERSION >= V_1_1) */
};

void ibl_set_mutex_func(int tolock, void *func);
int sys_setting_get(void *settings);
int sys_set_err_process(uint8_t method);
int sys_set_trace_level(uint8_t trace_level);
int sys_set_img_flag(uint8_t idx, uint8_t mask, uint8_t flag);
int sys_reset_img_flag(uint8_t idx);
int sys_set_running_img(uint8_t idx);
int sys_set_fw_version(uint32_t type, uint32_t version);
int sys_set_pk_version(uint32_t type, uint8_t version);
int sys_set_trng_seed(uint8_t val);
int sys_status_set(uint8_t type, uint8_t len, uint8_t *pval);
int sys_status_get(uint8_t type, uint8_t len, uint8_t* pval);

uint8_t efuse_get_ctl(void);
int efuse_set_ctl(uint32_t ctl);
uint8_t efuse_get_tzctl(void);
int efuse_set_tzctl(uint8_t tzctl);
uint8_t efuse_get_fp(void);
int efuse_set_fp(uint8_t fp);
uint8_t efuse_get_usctl(void);
int efuse_set_usctl(uint8_t usctl);
int efuse_get_mcui(uint8_t *mcui);
int efuse_set_mcui(uint8_t *mcui);
int efuse_get_aeskey(uint8_t *aeskey);
int efuse_set_aeskey(uint8_t *aeskey);
int efuse_get_rotpk(uint8_t *rotpk);
int efuse_set_rotpk(uint8_t *rotpk);
int efuse_get_dbg_pwd(uint8_t *pwd);
int efuse_set_dbg_pwd(uint8_t *pwd);
int efuse_get_rss(uint8_t *rss);
int efuse_set_rss(uint8_t *rss);
int efuse_get_uid(uint8_t *uid);
int efuse_get_huk(uint8_t *huk);
int efuse_get_rf(uint8_t *rf, uint32_t offset, uint32_t sz);
int efuse_set_rf(uint8_t *rf, uint32_t offset, uint32_t sz);
int efuse_get_usdata(uint8_t *usdata, uint32_t offset, uint32_t sz);
int efuse_set_usdata(uint8_t *usdata, uint32_t offset, uint32_t sz);

int do_iak_getpub(int type, int rsa_keysz,
                            int ecc_gid, int key_format,
                            uint8_t *output, uint32_t *len);
int do_iak_sign(int type, int rsa_keysz, int ecc_gid,
                   const uint8_t *hash, size_t hash_len,
                   uint8_t *sig, size_t *sig_len);

int do_symm_key_derive(uint8_t *label, size_t label_sz,
                    uint8_t *key, size_t key_len);
#endif  /* __IBL_API_H__ */


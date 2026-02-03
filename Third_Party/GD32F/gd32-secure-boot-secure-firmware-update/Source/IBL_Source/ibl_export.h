/*!
    \file    ibl_export.h
    \brief   IBL export header file for GD32 SDK

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

#ifndef __IBL_EXPORT_H__
#define __IBL_EXPORT_H__

#define V_1_0                0x1
#define V_1_1                0x101

#ifndef IN
#define IN
#define OUT
#endif

#include "stdint.h"
#include "stddef.h"
#include "ibl_trace.h"
#include "Platform/IBL/ibl_efuse.h"
#include "Platform/IBL/ibl_region.h"
#include "ibl_image.h"
#include "ibl_sys.h"
#include "ibl_state.h"
#include "ibl_api.h"

extern struct ibl_api_t *p_ibl_api;

#define ibl_log_uart_init                       p_ibl_api->log_uart_init
#define ibl_printf                              p_ibl_api->printf
#define ibl_uart_putc                           p_ibl_api->uart_putc
#define ibl_trace_ex                            p_ibl_api->trace_ex
#define ibl_rand                                p_ibl_api->rand
#define ibl_set_mutex_func                      p_ibl_api->set_mutex_func
#define ibl_memset                              p_ibl_api->ibl_memset
#define ibl_memcmp                              p_ibl_api->ibl_memcmp
#define ibl_strlen                              p_ibl_api->ibl_strlen
#define ibl_strncmp                             p_ibl_api->ibl_strncmp

#define ibl_uart_rx_to                          p_ibl_api->uart_rx_to
#define ibl_strtoul                             p_ibl_api->ibl_strtoul

#define ibl_cal_checksum                        p_ibl_api->cal_checksum
#define ibl_img_verify_sign                     p_ibl_api->img_verify_sign
#define ibl_img_verify_hash                     p_ibl_api->img_verify_hash
#define ibl_img_verify_hdr                      p_ibl_api->img_verify_hdr
#define ibl_img_verify_pkhash                   p_ibl_api->img_verify_pkhash
#define ibl_img_validate                        p_ibl_api->img_validate
#define ibl_cert_validate                       p_ibl_api->cert_validate

#define ibl_sys_setting_get                     p_ibl_api->sys_setting_get
#define ibl_sys_status_set                      p_ibl_api->sys_status_set
#define ibl_sys_status_get                      p_ibl_api->sys_status_get
#define ibl_sys_set_trace_level                 p_ibl_api->sys_set_trace_level
#define ibl_sys_set_err_process                 p_ibl_api->sys_set_err_process
#define ibl_sys_set_img_flag                    p_ibl_api->sys_set_img_flag
#define ibl_sys_reset_img_flag                  p_ibl_api->sys_reset_img_flag
#define ibl_sys_set_running_img                 p_ibl_api->sys_set_running_img
#define ibl_sys_set_fw_ver                      p_ibl_api->sys_set_fw_version
#define ibl_sys_set_pk_ver                      p_ibl_api->sys_set_pk_version
#define ibl_sys_set_trng_seed                   p_ibl_api->sys_set_trng_seed

/* boot */
#define ibl_jump_to_img                         p_ibl_api->jump_to_img
#define ibl_ymodem_download_check               p_ibl_api->ymodem_download_check

#define ibl_is_valid_flash_offset               p_ibl_api->is_valid_flash_offset
#define ibl_is_valid_flash_addr                 p_ibl_api->is_valid_flash_addr
#define ibl_flash_total_size                    p_ibl_api->flash_total_size
#define ibl_flash_erase_size                    p_ibl_api->flash_erase_size
#define ibl_flash_init                          p_ibl_api->flash_init
#define ibl_flash_read                          p_ibl_api->flash_read
#define ibl_flash_write                         p_ibl_api->flash_write
#define ibl_flash_write_fast                    p_ibl_api->flash_write_fast
#define ibl_flash_erase                         p_ibl_api->flash_erase

#define ibl_fmc_unlock                          p_ibl_api->fmc_unlock
#define ibl_fmc_lock                            p_ibl_api->fmc_lock
#define ibl_fmc_flag_clear                      p_ibl_api->fmc_flag_clear
#if defined(PLATFORM_GD32F5XX)
#define ibl_fmc_page_erase                      p_ibl_api->fmc_page_erase
#define ibl_fmc_mass_erase                      p_ibl_api->fmc_mass_erase
#endif /* PLATFORM_GD32F5XX */

#if defined(PLATFORM_GD32A513)
#define ibl_fmc_word_program                    p_ibl_api->fmc_doubleword_program
#else
#define ibl_fmc_word_program                    p_ibl_api->fmc_word_program
#endif

#ifndef SECURITY_PROTECT_DISABLE
#define ibl_fwdg_reload                         p_ibl_api->ibl_fwdg_reload
#endif /* SECURITY_PROTECT_DISABLE */

/*#if (IBL_VERSION >= V_1_1)*/
#define ibl_do_symm_key_derive                  p_ibl_api->do_symm_key_derive
/*#endif (IBL_VERSION >= V_1_1) */
#endif  /* __IBL_EXPORT_H__ */

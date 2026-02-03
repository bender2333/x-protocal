/*!
    \file    ibl_boot.h
    \brief   IBL boot header file for GD32 SDK

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

#ifndef __IBL_BOOT_H__
#define __IBL_BOOT_H__

#include "ibl_includes.h"

#define __NAKED        __attribute__((naked))

void clear_msp_stack_asm(void);
void jump_to_img_asm(uint32_t reset_handler_addr);
void jump_to_img(IN uint32_t msp, IN uint32_t reset);
int check_sys_config(IN struct sys_setting_t *setting);
int check_hw_info(OUT struct ibl_state_t *state);
int check_efuse_setting(OUT struct ibl_state_t *state, OUT uint8_t *rotpk);
int validate_mbl_cert(IN uint32_t mbl_offset, IN uint8_t *rotpk, OUT uint8_t *mbl_pk);
int validate_mbl(IN uint32_t mbl_offset, IN uint8_t *mbl_pk, OUT struct sw_info_t *mbl_info);

void ibl_err_process(struct ibl_state_t *state);
void ibl_verify_none(struct ibl_state_t *state, uint32_t mbl_offset, uint32_t img_hdr_size, uint32_t arm_vector[2]);
void ibl_verify_image(struct ibl_state_t *state, uint32_t mbl_offset);
char* reset_reason_get(uint8_t reason);

void ymodem_packet_init(ymodem_packet_str *packet_str);
void ibl_ymdoem_update(struct ibl_state_t *state);

#endif  /* __IBL_BOOT_H__ */

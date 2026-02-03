/*!
    \file    ibl_target_cfg.h
    \brief   IBL target header file for GD32 SDK

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

#ifndef __IBL_TARGET_CFG_H__
#define __IBL_TARGET_CFG_H__

#include "Platform/gd32xx.h"

#define HASH_SHA256                 hau_hash_sha_256
#define HASH_SHA224                 hau_hash_sha_224
#define HASH_SHA1                   hau_hash_sha_1
#define HASH_MD5                    hau_hash_md5

extern void SystemClkInit(void);
uint32_t systick_config_noint(void);
void ppc_reset(void);
int system_reset_cfg(void);
enum reset_flag_t sys_reset_flag_get(void);
int tzspc_cfg(void);

#ifdef ROM_SELF_TEST
struct tziac_test_t {
    uint32_t reg;                      /* test register address */
    uint32_t reg_bits;                 /* test register related bits */
    uint32_t stat;                     /* status register */
    uint32_t stat_bits;                /* status register related bits */
    uint32_t periph;                   /* peripheral position */
};
void target_cfg_self_test(void);
#endif /* ROM_SELF_TEST */

#endif  /* __IBL_TARGET_CFG_H__ */

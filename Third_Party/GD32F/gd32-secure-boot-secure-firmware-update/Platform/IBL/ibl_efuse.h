/*!
    \file    ibl_efuse.h
    \brief   IBL EFUSE header file for GD32 SDK

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

#ifndef __IBL_EFUSE_H__
#define __IBL_EFUSE_H__

//#include "rom_includes.h"

#if defined(PLATFORM_GD32F5XX) || defined(PLATFORM_GD32H7XX) || defined(PLATFORM_GD32A513) || defined(PLATFORM_GD32G553) || defined(PLATFORM_GD32G5X3) || defined(PLATFORM_GD32E50X)
#define EFUSE_ROTPK_SZ              IMG_PK_LEN        /* 256 bits */
#define EFUSE_HUK_SZ                16        /* 128 bits */
#define EFUSE_OTP_SZ                16        /* 128 bits */

#define GET_IBL_OPT()               (IMG_VERIFY_OPT-1)

#if INNER_ROTPK_EFUSE == 0
int efuse_get_rotpk_inner(uint8_t *rotpk);
#endif /* INNER_ROTPK_EFUSE */

#if INNER_HUK_EFUSE == 0
int efuse_get_huk_inner(uint8_t *huk);
#endif /* INNER_HUK_EFUSE */

#endif

int is_key_valid(uint8_t *key, size_t key_sz);

#ifdef ROM_SELF_TEST
void efuse_self_test(void);
#endif
#endif  /* __IBL_EFUSE_H__ */

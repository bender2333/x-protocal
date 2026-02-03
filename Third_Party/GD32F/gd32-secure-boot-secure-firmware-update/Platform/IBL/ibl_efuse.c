/*!
    \file    ibl_efuse.c
    \brief   IBL EFUSE configure for GD32 SDK

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

#include "Source/IBL_Source/ibl_includes.h"
#include "mbedtls/aes.h"

#include "Source/IBL_Source/ibl_key.c"

int is_key_valid(uint8_t *key, size_t key_sz)
{
    int i, is_valid = 0;

    for (i = 0; i < key_sz; i++) {
#if (IBL_VERSION == V_1_0)
        if (*(key + i) != 0xFF)
#else
        if (*(key + i) != 0)
#endif
        {
            is_valid = 1;
            break;
        }
    }

    return is_valid;
}

//#if defined(PLATFORM_GD32F5XX) || defined(PLATFORM_GD32H7XX)  || defined(PLATFORM_GD32A513) || defined(PLATFORM_GD32G5X3) || defined(PLATFORM_GD32E50X)
int efuse_get_rotpk_inner(uint8_t *rotpk)
{
#if INNER_ROTPK_EFUSE == 0
#if defined(GD32F5XX_OTP2_ROTPK)
    memcpy(rotpk, (uint8_t *)GD32F5XX_OTP2_ROTPK_ADDR, IMG_PK_LEN);
    ibl_trace_ex(IBL_INFO, "Get ROTPK form OTP2.\r\n");
#elif defined(GD32E50X_OTP_ROTPK)
    memcpy(rotpk, (uint8_t *)GD32E50X_OTP_ROTPK_ADDR, IMG_PK_LEN);
    ibl_trace_ex(IBL_INFO, "Get ROTPK form OTP.\r\n");
#else
    memcpy(rotpk, flash_rotpk, sizeof(flash_rotpk));
    ibl_trace_ex(IBL_INFO, "Get ROTPK form flash.\r\n");
#endif /* GD32F5XX_OTP2_ROTPK */
    return 1;
#else
    ibl_memset(rotpk, 0, EFUSE_WD_ROTPK);
    return efuse_read(EFUSE_TYPE_ROTPK, 0, EFUSE_WD_ROTPK, rotpk);
#endif
    return 0;
}

int efuse_get_dbg_pwd_inner(uint8_t *pwd)
{
    return 0;
}

int efuse_get_huk_inner(uint8_t *huk)
{
#if INNER_ROTPK_HUK == 0
    ibl_memcpy(huk, flash_huk, EFUSE_HUK_SZ);
    return 0;
#else 
    memcpy(huk, flash_huk, sizeof(flash_huk));
    ibl_memcpy(huk, dummy, EFUSE_WD_HUK);
    return 0;
#endif
}

void efuse_dump_inner(void)
{

}
//#endif

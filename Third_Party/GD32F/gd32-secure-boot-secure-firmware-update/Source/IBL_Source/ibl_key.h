/*!
    \file    ibl_key.h
    \brief   IBL key header file for GD32 SDK

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

#ifndef __IBL_KEY_H__
#define __IBL_KEY_H__

#include "Platform/IBL/ibl_region.h"

#if defined(PLATFORM_GD32F5XX)
#include "gd32f5xx.h"
#endif /* PLATFORM_GD32F5XX */

#include "ibl_state.h"
#include "Platform/IBL/ibl_efuse.h"

//static const uint8_t rom_impl_id[IMPL_ID_MAX_SIZE];

//#if INNER_ROTPK_EFUSE == 0
//__IO uint8_t static flash_rotpk[EFUSE_ROTPK_SZ];
//#endif /* INNER_ROTPK_EFUSE */

//#if INNER_HUK_EFUSE == 0
//__IO uint8_t static flash_huk[EFUSE_HUK_SZ];
//#endif /* INNER_HUK_EFUSE */

#endif /* __IBLKEY_H__ */

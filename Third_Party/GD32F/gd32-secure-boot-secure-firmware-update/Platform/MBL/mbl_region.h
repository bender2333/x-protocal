/*!
    \file    mbl_region.h
    \brief   MBL region definition for GD32 SDK

    \version 2024-06-30, V1.0.0, firmware for GD32
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

#ifndef __MBL_REGION_H__
#define __MBL_REGION_H__

#if defined(PLATFORM_GD32F5XX)
#ifndef GD32F5XX_OPT1_USED
#include "config/config_gdm32_gd32f5xx.h"
#else
#include "../../../config/config_gdm32_gd32f5xx_otp_bankswp.h"
#endif
#elif defined(PLATFORM_GD32H7XX)
#include "../../../config/config_gdm32_gd32h7xx.h"
#elif defined(PLATFORM_GD32A513)
#include "../../../config/config_gdm32_gd32a513.h"
#elif defined(PLATFORM_GD32G5X3) && defined GD32G5X3_BANKSWAP
#include "../../../config/config_gdm32_gd32g5x3_bankswap.h"
#elif defined(PLATFORM_GD32G5X3) && !defined GD32G5X3_BANKSWAP
#include "../../../config/config_gdm32_gd32g5x3.h"
#elif defined(PLATFORM_GD32E50X)
#include "../../../config/config_gdm32_gd32e50x.h"

#else
#error Unknown platform.
#endif

#ifdef PLATFORM_GD32F5XX
#ifndef GD32F5XX_BANKS_SWP_USED
// #define GD32F5XX_BANKS_SWP_USED
#endif
#endif /* PLATFORM_GD32F5XX */

/* MBL: code and ro data */
#define MBL_BASE_ADDRESS (RE_FLASH_BASE + RE_MBL_OFFSET + RE_VTOR_ALIGNMENT)
#define MBL_CODE_START MBL_BASE_ADDRESS
#define MBL_CODE_SIZE (28 * 1024 - RE_VTOR_ALIGNMENT + 64) /* 28 KB */

/* SRAM: shared SRAM, store initial boot state */
#define MBL_SHARED_DATA_START (RE_SRAM_BASE + RE_SHARED_DATA_START) /* the same as IBL_SHARED_DATA_START */
#define MBL_SHARED_DATA_SIZE (RE_MBL_DATA_START - RE_SHARED_DATA_START)

/* SRAM: STACK, HEAP and other Global varaiables */
#define MBL_DATA_START (RE_SRAM_BASE + RE_MBL_DATA_START) /* skip IBL variables and shared data */
#define MBL_DATA_SIZE 0x8000
#define MBL_BUF_SIZE 0x3000
#define MBL_MSP_STACK_SIZE 0x4000

#endif /* __MBL_REGION_H__ */

/*!
    \file    ibl_region.h
    \brief   IBL region header file for GD32 SDK

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

#ifndef GD32F5XX_OPT1_USED
#include "config/config_gdm32_gd32f5xx.h"

/* FLASH: code and ro data */
#define FLASH_BASE_IBL (RE_FLASH_BASE)
#define IBL_CODE_START FLASH_BASE_IBL
#define IBL_CODE_SIZE (0x3000) /* MAX 12K */

#define FLASH_BASE_LIB (IBL_CODE_START + IBL_CODE_SIZE)
#define FLASH_LIB_START FLASH_BASE_LIB

#define FLASH_LIB_SIZE (0x50000) /* MAX 320K */

/* Flash: system settings and system status */
#define FLASH_OFFSET_SYS_SETTING (RE_FLASH_BASE - FLASH_BASE_IBL + RE_SYS_SET_OFFSET)

#else
#include "../../../config/config_gdm32_gd32f5xx_otp_bankswp.h"
/* FLASH: code and ro data */
#define FLASH_BASE_IBL (0x1FF00000)
#define IBL_CODE_START FLASH_BASE_IBL
#define IBL_CODE_SIZE (0x4000) /* MAX 16K */

#define FLASH_BASE_LIB (IBL_CODE_START + IBL_CODE_SIZE)
#define FLASH_LIB_START FLASH_BASE_LIB
#define FLASH_LIB_SIZE (0x1C000) /* MAX 112K */

/* Flash: system settings and system status */
#define FLASH_OFFSET_SYS_SETTING (RE_FLASH_BASE - 0x08000000 + RE_SYS_SET_OFFSET)

#endif /* GD32F5XX_OPT1_USED */

/* SRAM: STACK, HEAP and other Global varaiables */
#define SRAM_BASE_ADDRESS (RE_SRAM_BASE)

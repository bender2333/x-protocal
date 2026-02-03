/*!
    \file    app_region.h
    \brief   app region definition for GD32 SDK

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

#ifndef _APP_REGION_H__
#define _APP_REGION_H__

#include "../../config/config_gdm32.h"

#define SRAM_DATA_BASE_ADDR                     (RE_SRAM_BASE + RE_APP_DATA_START)
#define APP_CODE_START                          (RE_FLASH_BASE + RE_IMG_0_APP_OFFSET + RE_VTOR_ALIGNMENT)

#if defined(GD32F5XX_BANKS_SWP_USED) || defined(GD32G5X3_BANKSWAP)
#define APP_CODE_SIZE                           (RE_IMG_SIZE-RE_VTOR_ALIGNMENT)
#else
#define APP_CODE_SIZE                           (RE_IMG_1_APP_OFFSET - RE_IMG_0_APP_OFFSET-RE_VTOR_ALIGNMENT)
#endif


#define APP_DATA_START                          (SRAM_DATA_BASE_ADDR)
#if defined(PLATFORM_GD32A513)
#define APP_DATA_SIZE                           (0x000C000 - RE_APP_DATA_START)
#elif defined(PLATFORM_GD32E50X)
#define APP_DATA_SIZE                           (0x0020000 - RE_APP_DATA_START)
#else
#define APP_DATA_SIZE                           (0x00070000 - RE_APP_DATA_START)
#endif /* PLATFORM_GD32A513 */
#define APP_BUF_SIZE                            0x3000
#define APP_MSP_STACK_SIZE                      0x4000

#endif  /* _APP_REGION_H__ */

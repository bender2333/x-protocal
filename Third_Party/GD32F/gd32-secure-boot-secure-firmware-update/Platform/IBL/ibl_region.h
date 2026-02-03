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

#ifndef __IBL_REGION_H__
#define __IBL_REGION_H__

#if defined(PLATFORM_GD32F5XX)
#include "Platform/GD32F5xx/ibl_region.h"
#elif defined(PLATFORM_GD32H7XX)
#include "Platform/GD32H7xx/ibl_region.h"
#elif defined(PLATFORM_GD32A513)
#include "Platform/GD32A513/ibl_region.h"
#elif defined(PLATFORM_GD32G5X3)
#include "Platform/GD32G5x3/ibl_region.h"
#elif defined(PLATFORM_GD32E50X)
#include "Platform/GD32E50x/ibl_region.h"
#else
#error Unknown platform.
#endif

#define FLASH_API_ARRAY_BASE                    (FLASH_LIB_START)
#define FLASH_API_ARRAY_RSVD                    0x800

#define IBL_DATA_START                          SRAM_BASE_ADDRESS
#define IBL_DATA_SIZE                           0x400
#define IBL_HEAP_SIZE                           0x7000
#define IBL_MSP_STACK_SIZE                      0x4000

/* SRAM: shared SRAM, store initial boot state */
#define IBL_SHARED_DATA_START                   (IBL_DATA_START + IBL_DATA_SIZE)
#define IBL_SHARED_DATA_SIZE                    0x800

/* Macros to pick linker symbols */
#define REGION(a, b, c)                         a##b##c
#define REGION_NAME(a, b, c)                    REGION(a, b, c)
#define REGION_DECLARE(a, b, c)                 extern uint32_t REGION_NAME(a, b, c)

#endif  /* __IBL_REGION_H__ */

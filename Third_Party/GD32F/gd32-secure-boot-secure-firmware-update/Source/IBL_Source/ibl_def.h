/*!
    \file    ibl_def.h
    \brief   IBL define header file for GD32 SDK

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

#ifndef __IBL_DEF_H__
#define __IBL_DEF_H__

#define V_1_0 0x1
#define V_1_1 0x101
#define V_2_1 0x201
#define IBL_VERSION V_1_1

#include "stdarg.h"
#include "stdint.h"

typedef unsigned int size_t;

#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */
#define _TRUE 1
#define _FALSE 0

#define __LOCK 1
#define __UNLOCK 0

#define IN
#define OUT

enum
{
    MUTEX_TYPE_PKCAU = 0,
    MUTEX_TYPE_CRYP,
    MUTEX_TYPE_HASH,
    MUTEX_TYPE_MAX
};

/* support platform define */
// #ifndef PLATFORM_GD32F5XX
// #define PLATFORM_GD32F5XX
// #endif /* PLATFORM_GD32F5XX */

/* define whether use inner key, if not define will get key form EFUSE */
#define INNER_ROTPK_EFUSE 0
#define INNER_HUK_EFUSE 0

/* MCU platform define */
#if defined PLATFORM_GD32F5XX

/*  GD32F5XX_OPT1_USED if defined IBL code is saved in OTP1,
    OTP1 is only 128KB, so some function will not be implemented
*/
// #define GD32F5XX_OPT1_USED
#if defined GD32F5XX_BANKS_SWP_USED
#ifndef GD32F5XX_OPT1_USED
#warning "GD32F5XX_BANKS_SWP_USED defined but GD32F5XX_OPT1_USED not defined."
#endif /* GD32F5XX_OPT1_USED */
#endif /* GD32F5XX_BANKS_SWP_USED */

/*  GD32F5XX_OTP2_ROTPK if defined ROTPK is saved in OTP2
    (start: 0x1FF20000, sizes: 64bits )
    if defined INNER_ROTPK_EFUSE need to be set 0
*/
// #define GD32F5XX_OTP2_ROTPK

#if defined GD32F5XX_OTP2_ROTPK
#define GD32F5XX_OTP2_ROTPK_ADDR (0x1FF20000U)
#endif /* GD32F5XX_OTP2_ROTPK */

#elif defined PLATFORM_GD32E50X

// #define GD32E50X_OTP_ROTPK

#if defined GD32E50X_OTP_ROTPK
#define GD32E50X_OTP_ROTPK_ADDR (0x1FFF7000U)
#endif /* GD32E50X_OTP_ROTPK */

#endif /* PLATFORM_GD32F5XX */

// #define GD32G5X3_OTP_ROTPK
#if defined GD32G5X3_OTP_ROTPK
#define GD32G5X3_OTP_ROTPK_ADDR (0x1FFF7000)
#endif /* GD32G5X3_OTP_ROTPK */

/* support platform consle */
#define LOG_UART UART6

// #define IMG_VERIFY_OPT             IBL_VERIFY_NONE
#define IMG_VERIFY_OPT IBL_VERIFY_IMG_ONLY

/*
 * Image Signature Algorithm.
 */
#define IMG_SIG_ED25519 0x1
#define IMG_SIG_ECDSA256 0x2 /* default is SECP256R1_SHA256 */
#define IMG_SIG_RSA2048 0x3  /* default is PKCS1_PSS_RSA2048_SHA256 */
#define SIGN_ALGO_SET IMG_SIG_ECDSA256

/* security configure */
// #define SECURITY_PROTECT_DISABLE     /* disable security protect */
#if !defined(SECURITY_PROTECT_DISABLE)
// #define SPC_PROTECT_ENABLE              /* enable SPC protect */

#ifdef SPC_PROTECT_ENABLE
#define SPC_PROTECT_LEVEL_NONE /* configure SPC LEVEL to none, for debug only, not recommend */
// #define SPC_PROTECT_LEVEL_LOW            /* configure SPC LEVEL to low, for debug only */
// #define SPC_PROTECT_LEVEL_HIGH           /* configure SPC LEVEL to high */
#endif /* SPC_PROTECT_ENABLE */

#endif /* SECURITY_PROTECT_DISABLE */

#if defined(PLATFORM_GD32H7XX)
#define MEM_CMP_NOT_INV_ICACHE /* not define when use O2 optimize */
#endif                         /* PLATFORM_GD32H7XX */

/* once enable next image also need reload fwdg counter */
// #define FWDG_PROTECT_ENABLE
#ifdef FWDG_PROTECT_ENABLE
#define FWDG_WAIT_SECOND 7 /* FWDG wait second, less tahn 8 */
#endif                     /* FWDG_PROTECT_ENABLE */

/* enable ibl test suit */
// #define IBL_TEST_SUIT

///* checke macro  */
// #ifndef SPC_PROTECT_ENABLE
// #warning "SPC_PROTECT_ENABLE not defined!"
// #endif /* SPC_PROTECT_ENABLE */

// #ifndef FWDG_PROTECT_ENABLE
// #warning "FWDG_PROTECT_ENABLE not defined!"
// #endif /* FWDG_PROTECT_ENABLE */

#endif /* __IBL_DEF_H__ */

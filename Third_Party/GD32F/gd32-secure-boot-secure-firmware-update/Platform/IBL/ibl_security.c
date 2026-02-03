/*!
    \file    ibl_security.c
    \brief   IBL security configure for GD32 SDK

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

#ifndef SECURITY_PROTECT_DISABLE

uint8_t ibl_spc_get(void)
{
#if defined (PLATFORM_GD32F5XX)
    efuse_state_enum efuse_state = EFUSE_READY;
#endif /* PLATFORM_GD32F5XX */
    uint32_t  efuse_ctl_buf = 0;
    uint8_t   ob_spc_state = 0;
    uint8_t   spc_state = 0;
#if defined(PLATFORM_GD32F5XX)
    /* read efuse statue, get efuse SPC level */
    efuse_state = efuse_read(EFUSE_CTL_EFADDR, 1, &efuse_ctl_buf);
    if(EFUSE_READY != efuse_state) {
        ibl_trace(IBL_ERR, "read efuse contrl error(%d).\r\n", efuse_state);
    }

    /* read ob statue, get ob SPC level */
    ob_spc_state = (uint8_t)(FMC_OBCTL0 >> 8U);
#endif /* PLATFORM_GD32F5XX */
    if(FMC_HSPC == ob_spc_state) {
        spc_state = FMC_HSPC;
    } else if(1 == (efuse_ctl_buf&0x01)) {
        spc_state = FMC_LSPC;
    }  else if(FMC_LSPC == ob_spc_state) {
        spc_state = FMC_LSPC;
    } else {
         spc_state = FMC_NSPC;
    }
    
    return  spc_state;
}

void ibl_spc_configure(uint8_t *spc_state)
{
#ifdef SPC_PROTECT_LEVEL_LOW
    if(FMC_LSPC == *spc_state) {
        return ;
    }
#else
    if(FMC_NSPC == *spc_state) {
        return ;
    }
#endif /* SPC_PROTECT_LEVEL_LOW */

    /* disable security protection */
    fmc_unlock();
    ob_unlock();
#ifdef SPC_PROTECT_LEVEL_HIGH
    ob_security_protection_config(FMC_HSPC);
#elif defined SPC_PROTECT_LEVEL_LOW
    ob_security_protection_config(FMC_LSPC);
#else
    ob_security_protection_config(FMC_NSPC);
#endif /* SPC_PROTECT_LEVEL_HIGH */
#if defined(PLATFORM_GD32A513)
    ob_reset();
#else

#if defined (PLATFORM_GD32G5X3) || defined (PLATFORM_GD32E50X)
    /* do nothing */
#else
    ob_start();
#endif
#endif
    ob_lock();
    fmc_lock();

    /* reload option bytes and generate a system reset */
    NVIC_SystemReset();
}

void ibl_spc_set(void)
{
    uint8_t   spc_state = 0;
    spc_state = ibl_spc_get();
    if(FMC_HSPC == spc_state) {
        /* now do nothing */
        ibl_trace(IBL_ALWAYS, "spc is FMC_HSPC.\r\n");
    } else {
#ifdef SPC_PROTECT_ENABLE
        ibl_spc_configure(&spc_state);
#endif /* SPC_PROTECT_ENABLE */
    }
}

void ibl_fwdg_configure(void)
{
#ifdef FWDG_PROTECT_ENABLE
    /* confiure FWDGT counter clock: 32KHz(IRC32K) / 64 = 0.5 KHz */
    fwdgt_config(FWDG_WAIT_SECOND * 500, FWDGT_PSC_DIV64);

    /* After FWDG_WAIT_SECOND seconds to generate a reset */
    fwdgt_enable();
#endif /* FWDG_PROTECT_ENABLE */
}

void ibl_fwdg_reload(void)
{
#ifdef FWDG_PROTECT_ENABLE
    /* reload FWDGT counter */
    fwdgt_counter_reload();
#endif /* FWDG_PROTECT_ENABLE */
}

void ibl_security_configure(void)
{
#ifdef SPC_PROTECT_ENABLE
    ibl_spc_set();
    /* double set */
    ibl_spc_set();
#endif /* SPC_PROTECT_ENABLE */

#ifdef FWDG_PROTECT_ENABLE
    ibl_fwdg_configure();
#endif /* FWDG_PROTECT_ENABLE */

#ifdef GD32F5XX_OTP2_ROTPK
    /* otp2 read lock enable, if enable otp2 cannot read before next reset */
    fmc_unlock();
    otp2_rlock_enable();
    fmc_lock();
#endif /* GD32F5XX_OTP2_ROTPK */
}

#endif /* SECURITY_PROTECT_DISABLE */


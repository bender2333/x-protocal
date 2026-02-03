/*!
    \file    ibl_target_cfg.c
    \brief   IBL target configure for GD32 SDK

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

/* Enable system reset request for CPU 0 */
#define ENABLE_CPU0_SYSTEM_RESET_REQUEST (1U << 4U)

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

//uint32_t systick_config_noint(void)
//{
//  SysTick->LOAD  = SysTick_LOAD_RELOAD_Msk;
//  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
//  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
//                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick Timer */
//  return (0UL);                                                     /* Function successful */
//}

int system_reset_cfg(void)
{
    return 0;
}

enum reset_flag_t sys_reset_flag_get(void)
{
    uint32_t retv = RESET_BY_UNKNOWN;

#if defined(PLATFORM_GD32F5XX) || defined(PLATFORM_GD32H7XX) || defined(PLATFORM_GD32A513) || defined(PLATFORM_GD32G5X3) || defined(PLATFORM_GD32E50X)
    uint32_t flag = RCU_RSTSCK;

#if (IBL_VERSION >= V_1_1)
#if defined(PLATFORM_GD32E50X)
#else
    if (flag & RCU_RSTSCK_BORRSTF) {
        retv = RESET_BY_OBLDR;
    } else
#endif /* PLATFORM_GD32E50X */
#endif
    if (flag & RCU_RSTSCK_EPRSTF) {
        retv = RESET_BY_PIN;
    } else if (flag & RCU_RSTSCK_PORRSTF) {
        retv = RESET_BY_PWR_ON;
    } else if (flag & RCU_RSTSCK_SWRSTF) {
        retv = RESET_BY_SW;
    } else if (flag & RCU_RSTSCK_FWDGTRSTF) {
        retv = RESET_BY_FWDG;
    } else if (flag & RCU_RSTSCK_WWDGTRSTF) {
        retv = RESET_BY_WWDG;
    } else if (flag & RCU_RSTSCK_LPRSTF) {
        retv = RESET_BY_LOW_PWR;
    } else {
        retv = RESET_BY_UNKNOWN;
    }
#endif  /* PLATFORM_GD32F5XX || PLATFORM_GD32H7XX */
    rcu_all_reset_flag_clear();
    return retv;
}

void target_cfg_self_test(void)
{
}


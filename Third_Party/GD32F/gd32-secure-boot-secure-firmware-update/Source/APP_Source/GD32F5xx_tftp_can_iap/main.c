/*!
    \file    main.c
    \brief   running led

    \version 2023-10-16, V0.0.0, firmware for applicatiom
*/

/*
    Copyright (c) 2023, GigaDevice Semiconductor Inc.

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

#if defined(PLATFORM_GD32F5XX) 
#include "gd32f5xx.h"
#include "gd32f527_eval.h"
#elif defined(PLATFORM_GD32H7XX)
#include "gd32h7xx.h"
#include "gd32h759i_eval.h"
#elif defined(PLATFORM_GD32A513)
#include "gd32a513.h"
#include "gd32a513v_eval.h"
#elif defined(PLATFORM_GD32G5X3)
#include "gd32g5x3.h"
#include "gd32g553q_eval.h"
#elif defined(PLATFORM_GD32E50X)
#include "gd32e50x.h"
#include "gd32e507z_eval.h"
#endif

#include "../systick.h"
#include "Platform/APP/app_region.h"

#include "bootutil/image.h"
#include "flash_hal/flash_layout.h"
#include "include/mcuboot_config/mcuboot_config.h"
#include "include/storage/flash_map.h"
#include "../src/bootutil_priv.h"
#include "Source/IBL_Source/ibl_export.h"
#include "Source/APP_Source/shell_drv_uart/drv_uart.h"
#include "Utilities/Third_Party/letter-shell-shell2.x/shell.h"
#include "Source/APP_Source/Common/task.h"
#include "Source/APP_Source/Common/sys_init.h"
#include "Source/IBL_Source/ibl_export_mbedtls.h"

#include "netconf.h"
#include "main.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"

extern __IO uint32_t g_localtime;

struct ibl_api_t *p_ibl_api = (struct ibl_api_t *)FLASH_API_ARRAY_BASE;
static uint8_t alloc_buf[APP_BUF_SIZE];

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

int main(void)
{
    const struct flash_area *fap;
    struct image_header *hdr = (struct image_header *)(RE_FLASH_BASE + RE_IMG_0_APP_OFFSET);
    SCB->VTOR = APP_CODE_START;

    ibl_trace(IBL_ALWAYS, "APP image version is %d.%d.%d\r\n", hdr->ih_ver.iv_major, hdr->ih_ver.iv_minor, hdr->ih_ver.iv_revision);

    systick_config();
#ifndef SECURITY_PROTECT_DISABLE
    ibl_fwdg_reload();
#endif /* SECURITY_PROTECT_DISABLE */
    gd_eval_led_init(LED1);
    gd_eval_led_init(LED2);

    mbedtls_memory_buffer_alloc_init(alloc_buf, sizeof(alloc_buf));

    /* write image ok flag, then this image will always run */
    flash_area_open(FLASH_AREA_0_ID, &fap);
    boot_write_image_ok(fap);

#if defined PLATFORM_GD32F5XX && defined GD32F5XX_BANKS_SWP_USED
    /* should do this, after image ok  */
    /* bank is swap, chang cache is need */
    if(0 != (SYSCFG_CFG0&SYSCFG_CFG0_FMC_SWP) && 0 != (FMC_OBCTL0&FMC_OBCTL0_NWA)) {
        /* unlock the flash program/erase controller */
        fmc_unlock();
        ob_unlock();
        fmc_nwa_enable();
        ob_nwa_select(OB_NWA_BANK1);
        ob_start();
        ob_lock();
        /* lock the main FMC after the erase operation */
        fmc_lock();
        ibl_trace(IBL_ALWAYS, "Select no waiting time area BANK1, system will reboot.\r\n");
        /* generate a system reset */
        NVIC_SystemReset();
    } else if ((0 == (SYSCFG_CFG0&SYSCFG_CFG0_FMC_SWP) && 0 == (FMC_OBCTL0&FMC_OBCTL0_NWA))) {
        /* unlock the flash program/erase controller */
        fmc_unlock();
        ob_unlock();
        fmc_nwa_enable();
        ob_nwa_select(OB_NWA_BANK0);
        ob_start();
        ob_lock();
        /* lock the main FMC after the erase operation */
        fmc_lock();
        ibl_trace(IBL_ALWAYS, "Select no waiting time area BANK0, system will reboot.\r\n");
        /* generate a system reset */
        NVIC_SystemReset();
    } else {
        /* disable no waiting time area load when system reset */
        if(0 != (FMC_CTL& FMC_CTL_NWLDE)) {
            /* unlock the flash program/erase controller */
            fmc_unlock();
            fmc_nwa_disable();
            ob_nwa_select(OB_NWA_BANK0);
            /* lock the main FMC after the erase operation */
            fmc_lock();
            ibl_trace(IBL_ALWAYS, "disable no waiting time area load when system reset.\r\n");
        }
    }
#endif /*  */

    shell_com_init();
    sys_init_fun();

    while(1) {
#ifndef SECURITY_PROTECT_DISABLE
        ibl_fwdg_reload();
#endif /* SECURITY_PROTECT_DISABLE */
        shell_task();

#ifndef USE_ENET_INTERRUPT
        /* check if any packet received */
        if(enet_rxframe_size_get()) {
            /* process received ethernet packet */
            lwip_frame_recv();
        }
#endif /* USE_ENET_INTERRUPT */

        /* handle periodic timers for LwIP */
#ifdef TIMEOUT_CHECK_USE_LWIP
        sys_check_timeouts();

#ifdef USE_DHCP
        lwip_dhcp_address_get();
#endif /* USE_DHCP */

#else
        lwip_timeouts_check(g_localtime);
#endif /* TIMEOUT_CHECK_USE_LWIP */
    }
}

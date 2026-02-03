/*!
    \file    ymodem_update.c
    \brief   ymodem update

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

#include "Platform/gd32xx.h"
#include "Source/APP_Source/systick.h"
#include "Utilities/Third_Party/letter-shell-shell2.x/shell_ext.h"
#include "Source/IBL_Source/ibl_export.h"
#include "Source/IBL_Source/ibl_def.h"
#include "Utilities/Third_Party/mcuboot-main/boot/gigadevice/flash_hal/flash_layout.h"
#include "Utilities/Third_Party/mcuboot-main/boot/bootutil/src/bootutil_priv.h"

#include "task.h"
#include "main.h"
#include "netconf.h"

#ifdef USE_IAP_TFTP

extern int16_t last_packet_falg;
extern __IO uint32_t g_localtime;

void iap_tftp_init(void);

void tftp_update(void)
{
    const struct flash_area *fap;

    ibl_trace(IBL_ALWAYS, "APP tftp update start\r\n");
#ifdef USE_IAP_TFTP
    /* Initialize the TFTP server */
    iap_tftp_init();
#endif
    while(last_packet_falg == 0){
        lwip_timeouts_check(g_localtime);
    }
    flash_area_open(FLASH_AREA_1_ID, &fap);
    boot_write_magic(fap);
    ibl_trace(IBL_ALWAYS, "tftp update success, system reboot.\r\n");
    delay_1ms(100);
    /* generate a system reset */
    NVIC_SystemReset();
}

void tftp_update_set(void)
{
    /* now still in interrupt service function, so we just set flag */
    shell_task_flag |= TASK_FLAG_TFTP_UPDATE;
}

SHELL_EXPORT_CMD(tftp_update, tftp_update_set, tftp update image);
#endif

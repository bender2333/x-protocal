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

ymodem_packet_str ymodem_packet;

static void ymodem_packet_init(ymodem_packet_str *packet_str)
{
    packet_str->down_off =  RE_FLASH_BASE-FLASH_BASE+RE_IMG_1_APP_OFFSET;
    packet_str->down_size = FLASH_PARTITION_SIZE;
    packet_str->wait_second = 9;
    packet_str->ymodem_rx = (ymodem_rx_fun)ibl_uart_rx_to;
    packet_str->ymodem_printf = (ymodem_printf_fun)ibl_printf;
    packet_str->ymodem_flash_erase = (ymodem_flash_erase_fun)ibl_flash_erase;
    packet_str->ymodem_flash_write = (ymodem_flash_write_fun)ibl_flash_write;
    packet_str->ymodem_strtoul = (ymodem_strtoul_fun)ibl_strtoul;
    packet_str->ymodem_send = (ymodem_send_fun)ibl_uart_putc;
#ifndef SECURITY_PROTECT_DISABLE
    packet_str->ymodem_fwdg_reload = (ymodem_fwdg_reload_fun)ibl_fwdg_reload;
#endif /* SECURITY_PROTECT_DISABLE */
}

void ymodem_update(void)
{
    const struct flash_area *fap;

    ibl_trace(IBL_ALWAYS, "APP ");
    ymodem_packet_init(&ymodem_packet);

    /* wait IDLEF set and clear it */
    while(RESET == usart_flag_get(LOG_UART, USART_FLAG_IDLE)) {
    }
    usart_flag_clear(LOG_UART, USART_FLAG_IDLE);
    usart_interrupt_enable(LOG_UART, USART_INT_RBNE);
    usart_interrupt_enable(LOG_UART, USART_INT_IDLE);

    ibl_ymodem_download_check(&ymodem_packet, (delay_ms_fun)delay_1ms);
    if (0 != (ymodem_packet.ymodem_flag&YMODEM_FLAG_STP)) {
        if (0 != (ymodem_packet.ymodem_flag&YMODEM_FLAG_EMAX)) {
            ibl_trace(IBL_ERR, "Ymodem update error.\r\n");
            while (1) {
            }
        } else { 
            ibl_printf("\r\n");
            flash_area_open(FLASH_AREA_1_ID, &fap);
            boot_write_magic(fap);
            ibl_trace(IBL_ALWAYS, "Ymodem update success, system reboot.\r\n");
            delay_1ms(100);
            /* generate a system reset */
            NVIC_SystemReset();
        }
    }
}

void ymodem_update_set(void)
{
    /* now still in interrupt service function, so we just set flag */
    shell_task_flag |= TASK_FLAG_UPDATE;
}

SHELL_EXPORT_CMD(ymodem_update, ymodem_update_set, ymodem update image);

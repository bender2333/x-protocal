/*!
    \file    mbl.c
    \brief   Main boot loader for GD32 SDK

    \version 2026-06-30, V1.0.0, demo for GD32
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

#include "mbl_includes.h"
#include "Source/IBL_Source/ibl_export_mbedtls.h"
#include "mbl_systick.h"
#include "bootutil/bootutil.h"
#include "bootutil/bootutil_log.h"
#include "../src/bootutil_priv.h"

#include "Platform/IBL/ymodem.h"
#define __NAKED        __attribute__((naked))
#if defined (__ICCARM__)
extern uint32_t MBL_CMSE_VENEER$$Base;
extern uint32_t MBL_CMSE_VENEER$$Limit;
extern uint32_t MBL_BUF$$Base;
extern uint32_t HEAP$$Limit;
#else
REGION_DECLARE(Image$$, MBL_CMSE_VENEER, $$Base);
REGION_DECLARE(Image$$, MBL_CMSE_VENEER, $$Limit);
REGION_DECLARE(Image$$, MBL_BUF, $$ZI$$Base);
#endif

struct arm_vector_table {
    uint32_t msp;
    uint32_t reset;
};

#if defined GD32G5X3_BANKSWAP
uint8_t run_img;
uint8_t run_imgt;
#endif

static void do_boot(struct boot_rsp *rsp);

void mbl_get_rotpk(uint8_t *rotpk, uint32_t key_len);

struct ibl_api_t *p_ibl_api = (struct ibl_api_t *)FLASH_API_ARRAY_BASE;
static uint8_t alloc_buf[MBL_BUF_SIZE];

typedef void (*mbedtls_sha256_init_fun)( mbedtls_sha256_context *ctx );

ymodem_packet_str ymodem_packet = {0};

void ymodem_packet_init(ymodem_packet_str *packet_str)
{
#if defined GD32G5X3_BANKSWAP
    ibl_sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &run_img);
    if(1 == run_img){
        packet_str->down_off =  RE_FLASH_BASE-FLASH_BASE + RE_IMG_0_APP_OFFSET;
    }else {
        packet_str->down_off =  RE_FLASH_BASE-FLASH_BASE + RE_IMG_1_APP_OFFSET;
    }
#else 
    packet_str->down_off =  RE_FLASH_BASE-FLASH_BASE + RE_IMG_1_APP_OFFSET;
#endif
    packet_str->down_size = FLASH_PARTITION_SIZE;
    packet_str->wait_second = 3;
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

#if defined GD32G5X3_BANKSWAP

__NAKED
__attribute__((section(".ramfunc")))
void jump_to_img_asm(uint32_t reset_handler_addr)
{
    __ASM volatile(
        ".syntax unified                 \n"
        "mov     r7, r0                  \n"
        "movs    r0, #0                  \n" /* Clear registers: R0-R12, */
        "mov     r1, r0                  \n" /* except R7 */
        "mov     r2, r0                  \n"
        "mov     r3, r0                  \n"
        "mov     r4, r0                  \n"
        "mov     r5, r0                  \n"
        "mov     r6, r0                  \n"
        "mov     r8, r0                  \n"
        "mov     r9, r0                  \n"
        "mov     r10, r0                 \n"
        "mov     r11, r0                 \n"
        "mov     r12, r0                 \n"
        "mov     lr,  r0                 \n"
        "bx      r7                      \n" /* Jump to Reset_handler */
    );
}

__attribute__((section(".ramfunc")))
void jump_to_img(IN uint32_t msp, IN uint32_t reset)
{
    static uint32_t img_reset;

    img_reset = reset;

#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)
    /* Restore the Main Stack Pointer Limit register's reset value
     * before passing execution to runtime firmware to make the
     * bootloader transparent to it.
     */
    __set_MSPLIM(0);
#endif
    __set_MSP(msp);
    __DSB();
    __ISB();
    jump_to_img_asm(img_reset);
}
#endif


/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

int main(void)
{
    struct ibl_state_t ibl_state;
    uint32_t boot_idx = 0, image_addr = 0;
    struct image_header *hdr = (struct image_header *)(RE_FLASH_BASE + RE_MBL_OFFSET);
    uint32_t arm_vector[2];
    uint32_t bonly_nspe = 0;
    int ret, i;
    uint8_t val;
#if defined (GD32F5XX_BANKS_SWP_USED) || defined (GD32G5X3_BANKSWAP)
    uint8_t run_img = 0;
#endif /* GD32F5XX_BANKS_SWP_USED */
    struct boot_rsp rsp;
    int rc;
    const struct flash_area *fap;
#if defined(PLATFORM_GD32H7XX)
    /* enable i-cache */
    SCB_EnableICache();

    /* enable d-cache */
    SCB_EnableDCache();
#endif

#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)
#if defined (__ICCARM__)
    uint32_t msp_stack_bottom = (uint32_t)&MBL_BUF$$Base + 0x1000;
#else
    uint32_t msp_stack_bottom = (uint32_t)&REGION_NAME(Image$$, MBL_BUF, $$ZI$$Base) + 0x1000;
#endif
    __set_MSPLIM(msp_stack_bottom);
#endif

    SCB->VTOR = MBL_BASE_ADDRESS;

    /* Reinitialize uart since system clock source may change */
    ibl_log_uart_init();

    ibl_trace(IBL_ALWAYS, "MBL version is %d.%d.%d\r\n", hdr->ih_ver.iv_major, hdr->ih_ver.iv_minor, hdr->ih_ver.iv_revision);

    systick_config();

    /* Read Initial boot state from shared SRAM */
    memcpy(&ibl_state, (void *)IBL_SHARED_DATA_START, sizeof(struct ibl_state_t));

    mbl_get_rotpk(ibl_state.mbl_pk, IMG_PK_LEN);

    mbedtls_memory_buffer_alloc_init(alloc_buf, sizeof(alloc_buf));

#if defined (GD32F5XX_BANKS_SWP_USED) || defined (GD32G5X3_BANKSWAP)
    /* bank switch set */
    ibl_sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &run_img);
    if (0xff == run_img) {
        run_img = 0;
        BOOT_LOG_INF("run img init");
        ibl_sys_status_set(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &run_img);
    }
    ibl_sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &run_img);

#if defined GD32G5X3_BANKSWAP
    /* for now run_imgt is just used for gd32G5x3 */
    run_imgt = run_img;
#endif

#if defined (GD32F5XX_BANKS_SWP_USED)
    rcu_periph_clock_enable(RCU_SYSCFG);
    if (0 != run_img) {
        BOOT_LOG_INF("FMC swap is bank1");
        syscfg_fmc_swap_config(SYSCFG_FMC_SWP_BANK1);
    } else {
        BOOT_LOG_INF("FMC swap is bank0");
        syscfg_fmc_swap_config(SYSCFG_FMC_SWP_BANK0);
    }
#endif
#endif /* GD32F5XX_BANKS_SWP_USED */

    /* Read Error Process */
    ret = ibl_sys_status_get(SYS_ERROR_PROCESS, LEN_SYS_ERROR_PROCESS, &val);
    if (ret == SYS_STATUS_NOT_FOUND || val <= ERR_PROCESS_ENTER_ISP) {
        /* ymodem update */
        ibl_trace(IBL_ALWAYS, "MBL ");
        ymodem_packet_init(&ymodem_packet);

#if defined(PLATFORM_GD32H7XX) || defined(PLATFORM_GD32A513)
        /* wait IDLEF set and clear it */
        while(RESET == usart_flag_get(LOG_UART, USART_FLAG_IDLE)) {
        }
        usart_flag_clear(LOG_UART, USART_FLAG_IDLE);
#endif /* PLATFORM_GD32H7XX */
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
#if defined GD32G5X3_BANKSWAP
                ibl_sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &run_img);
                if(run_img == 1){
                    flash_area_open(FLASH_AREA_0_ID, &fap);
                    ibl_flash_erase(fap->fa_off + fap->fa_size - BOOT_MAGIC_SZ, BOOT_MAGIC_SZ);
                    boot_write_magic(fap);
                }else{
                    flash_area_open(FLASH_AREA_1_ID, &fap);
                    ibl_flash_erase(fap->fa_off + fap->fa_size - BOOT_MAGIC_SZ, BOOT_MAGIC_SZ);
                    boot_write_magic(fap);
                }
#else
                flash_area_open(FLASH_AREA_1_ID, &fap);
                ibl_flash_erase(fap->fa_off + fap->fa_size - BOOT_MAGIC_SZ, BOOT_MAGIC_SZ);
                boot_write_magic(fap);
#endif
                ibl_trace(IBL_ALWAYS, "Ymodem update success, system reboot.\r\n");
                delay_1ms(100);
                /* generate a system reset */
                NVIC_SystemReset();
            }
        }
        usart_interrupt_disable(LOG_UART, USART_INT_RBNE);
        usart_interrupt_disable(LOG_UART, USART_INT_IDLE);
    }

    rc = boot_go(&rsp);
    if (rc != 0) {
        ibl_trace(IBL_ERR, "Unable to find bootable image, please reboot!\r\n");
        while (1) {
        }
    } else {
        ibl_trace(IBL_ALWAYS, "Jump to APP\r\n");
        do_boot(&rsp);
    }
}

#if defined GD32G5X3_BANKSWAP
__attribute__((section(".ramfunc")))
#endif
static void do_boot(struct boot_rsp *rsp)
{
    static struct arm_vector_table *vt;

    uintptr_t flash_base;
    int rc;
#if !defined SECURITY_PROTECT_DISABLE
    ibl_fwdg_reload();
#endif /* SECURITY_PROTECT_DISABLE */
    /* Jump to flash image */
    rc = flash_device_base(rsp->br_flash_dev_id, &flash_base);
    assert(rc == 0);

    vt = (struct arm_vector_table *)(flash_base +
                                     rsp->br_image_off +
                                     rsp->br_hdr->ih_hdr_size);
    
#if defined GD32G5X3_BANKSWAP 
    ibl_sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &run_img);
    if (1 == run_img) {
        rcu_periph_clock_enable(RCU_SYSCFG);
        syscfg_flash_bank_remap_set(SYSCFG_FLASH_BANK0_MAPPED);
        } else {
            syscfg_flash_bank_remap_set(SYSCFG_FLASH_BANK1_MAPPED);
        }

    jump_to_img(vt->msp, vt->reset);
#else 
    ibl_jump_to_img(vt->msp, vt->reset);
#endif
}


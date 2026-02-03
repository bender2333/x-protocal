/*!
    \file    ibl_boot.c
    \brief   IBL boot for GD32 SDK

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

#include "ibl_includes.h"

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include "../mbl/mbl_region.h"

ymodem_packet_str *ymodem_packet;

__NAKED
void clear_msp_stack_asm(void)
{
    __ASM volatile(
        ".syntax unified                             \n"
        "mov     r0, #0                              \n"
        "ldr     r1, =Image$$ARM_LIB_STACKHEAP$$ZI$$Base  \n"
        "ldr     r2, =Image$$ARM_LIB_STACKHEAP$$ZI$$Limit \n"
        "subs    r2, r2, r1                          \n"
        "Loop:                                       \n"
        "subs    r2, #4                              \n"
        "itt     ge                                  \n"
        "strge   r0, [r1, r2]                        \n"
        "bge     Loop                                \n"
        "bx      lr                                  \n"
         : : : "r0" , "r1" , "r2" , "memory"
    );
}

__NAKED
void jump_to_img_asm(uint32_t reset_handler_addr)
{
    __ASM volatile(
        ".syntax unified                 \n"
        "mov     r7, r0                  \n"
        "bl      clear_msp_stack_asm     \n" /* Clear msp stack before jump to mbl, heap and zi/rw data will be cleared after MBL boot */
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

int check_sys_config(IN struct sys_setting_t *setting)
{
    int ret;
    uint8_t val;
    uint32_t offset;

    ret = flash_read(FLASH_OFFSET_SYS_SETTING, (void *)setting, sizeof(struct sys_setting_t));
    if (ret != 0) {
        return -1;
    }

    /* Check magic */
    if (setting->sys_magic != SYS_SETTING_MAGIC) {
        return -2;
    }

    /* get flash offset for sys_status */
    flash_offset_sys_status = setting->sysstatus_offset;

    if (setting->flash_totsz == 0xFFFFFFFF)
        flash_tot_sz = 0;
    else
        flash_tot_sz = setting->flash_totsz;

    flash_nodec_config(0, (flash_offset_sys_status >> 12), (flash_offset_sys_status >> 12) + 1);

#if (SYS_STATUS_ENCRPTED == 1)
    /* init sys status */
    if (setting->ver_locked == 0xFFFFFFFF) {
        ret = sys_status_init();
        if (ret != 0) {
            return -3;
        }
        ret = sys_status_set_internal(SYS_MBL_VER_COUNTER, LEN_SYS_VER_COUNTER, (uint8_t *)&setting->mbl_initial_version);
        if (ret != 0) {
            return -4;
        }
        ret = sys_status_set_internal(SYS_IMG_VER_COUNTER, LEN_SYS_VER_COUNTER, (uint8_t *)&setting->img_initial_version);
        if (ret != 0) {
            return -5;
        }
        offset = FLASH_OFFSET_SYS_SETTING + (uint32_t)(&((struct sys_setting_t *) 0)->ver_locked);
        setting->ver_locked = 0xCDCDCDCD;
        ret = flash_write(offset, (void*)&(setting->ver_locked), sizeof(setting->ver_locked));
        if (ret != 0) {
            return -6;
        }
        ibl_trace(IBL_ALWAYS, "Sys status init OK.\r\n");
    } else {
        ret = sys_status_check_integrity();
        if (ret != 0) {
            return -7;
        }
    }
#endif /* (SYS_STATUS_ENCRPTED == 1) */

    /* Read Trace Level */
    ret = sys_status_get_internal(SYS_TRACE_LEVEL, LEN_SYS_TRACE_LEVEL, &val);
    if (ret == SYS_STATUS_NOT_FOUND) {
        trace_level = IBL_DBG;
    } else if (ret >= SYS_STATUS_FOUND_OK){
        trace_level = val;
    } else {
        return -8;
    }
    ibl_trace(IBL_INFO, "Sys status checked OK.\r\n");

    /* Read Error Process */
    ret = sys_status_get_internal(SYS_ERROR_PROCESS, LEN_SYS_ERROR_PROCESS, &val);
    if (ret == SYS_STATUS_NOT_FOUND) {
        err_process = ERR_PROCESS_ENTER_ISP;
    } else if (ret >= SYS_STATUS_FOUND_OK){
        if (val > ERR_PROCESS_ENTER_ISP)
            err_process = ERR_PROCESS_ENDLESS_LOOP;
        else
            err_process = ERR_PROCESS_ENTER_ISP;
    } else {
        return -9;
    }

    /* Read trng seed config */
    ret = sys_status_get_internal(SYS_TRNG_SEED, LEN_SYS_TRNG_SEED, &val);
    if (ret == SYS_STATUS_NOT_FOUND) {
        btrng_seed = 1;
    } else if (ret >= SYS_STATUS_FOUND_OK){
        btrng_seed = val;
    } else {
        return -10;
    }
    return 0;
}

int check_hw_info(OUT struct ibl_state_t *state)
{
#if (IBL_VERSION >= V_1_1)
    uint8_t RFCTL = 0;
#endif

    /* 1. System Core Clock init */
    SystemCoreClockUpdate();

    /* 2. Systick configure */
    systick_config();

    /* 3. enable HAU and CAU */
#if defined(PLATFORM_GD32F5XX)
    rcu_periph_clock_enable(RCU_HAU);
    rcu_periph_clock_enable(RCU_CAU);
#elif defined(PLATFORM_GD32H7XX)
    rcu_periph_clock_enable(RCU_HAU);
    rcu_periph_clock_enable(RCU_CAU);
#elif defined(PLATFORM_GD32G5X3)
    rcu_periph_clock_enable(RCU_CAU);
#endif /* PLATFORM_GD32F5XX */

    /* 4. Check reset reason: Power reset, Watchdog reset, SW reset, .etc */
    state->reset_flag = sys_reset_flag_get();

    /* 5. Read IBL version */
    state->ibl_ver = IBL_VERSION;

    return 0;
}

int check_efuse_setting(OUT struct ibl_state_t *state, OUT uint8_t *rotpk)
{
    uint32_t boot_option;
    /* Copy some EFUSE parameters to IBL state. */
    efuse_get_rotpk_inner(rotpk);
    flash_get_obstat(&state->obstat);

#ifndef IMG_VERIFY_OPT
    boot_option = GET_IBL_OPT();

    if (boot_option == 3)
        state->ibl_opt = IBL_VERIFY_CERT_IMG;
    else if (boot_option == 1)
        state->ibl_opt = IBL_VERIFY_IMG_ONLY;
    else
        state->ibl_opt = IBL_VERIFY_NONE;
#else
     state->ibl_opt = IMG_VERIFY_OPT;
#endif /* IMG_VERIFY_OPT */

    if (!is_key_valid(rotpk, EFUSE_ROTPK_SZ))
        state->ibl_opt = IBL_VERIFY_NONE;

    /* implement id unimplemented yet */
//    ibl_memcpy(state->impl_id, f_impl_id, sizeof(rom_impl_id));
    return 0;
}

int validate_mbl_cert(IN uint32_t mbl_offset, IN uint8_t *rotpk, OUT uint8_t *mbl_pk)
{
    /* not implemented yet */
    return 0;
}

int validate_mbl(IN uint32_t mbl_offset, IN uint8_t *mbl_pk, OUT struct sw_info_t *mbl_info)
{
    int ret;

    ret = img_validate(mbl_offset, IMG_TYPE_MBL, mbl_pk, (void *)mbl_info);

    return ret;
}

static void delay_us(uint32_t n)
{
    /* random delay 0us, return direct */
    if(n==0) {
        return ;
    }
    SysTick->CTRL = 0;
    SysTick->LOAD  = (n+1) *((uint32_t)(SystemCoreClock - 1UL));          /* set reload register */
    SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick */
    while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) {
    }
    SysTick->CTRL = 0;
    /* reconfig systick */
    systick_config();
}

/*!
    \brief    random delay 0-100US
    \param[in]  none
    \param[out] none
    \retval     none
*/
void random_delay(void)
{
    int ret = 0;
    char *pers = "ibl_delay";
    uint8_t data_buf[4];
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const uint8_t *)pers, strlen(pers));
    if(0 != ret) {
        ibl_trace(IBL_ERR, " failed\n ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        goto exit;
    }

    if ( ( ret = mbedtls_ctr_drbg_random(&ctr_drbg, data_buf, sizeof(data_buf) ) ) != 0) {
        ibl_trace(IBL_ERR, " failed\n ! mbedtls_ctr_drbg_random returned %d\n", ret );
        goto exit;
    }
    delay_us(1.0 * (*(uint32_t *)data_buf) / 0xffffffff * 100);

exit:
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);
}

void ibl_err_process(struct ibl_state_t *state)
{
    store_ibl_state(state);
    ibl_trace(IBL_ERR, "Loop here. Please re-boot.\r\n");
    while (1){
    }
}

void ibl_verify_none(struct ibl_state_t *state, uint32_t mbl_offset, uint32_t img_hdr_size, uint32_t arm_vector[2])
{
    int ret = 0;
    /* Store Initial Boot State and  */
    state->boot_status = BOOT_OK;
    store_ibl_state(state);

    /* Jump to MBL entry point */
    ret = flash_read(mbl_offset + img_hdr_size, arm_vector, 8);
    if ((ret != 0) || !is_valid_flash_addr(arm_vector[1])) {
        ibl_trace(IBL_ERR, "MBL Entry is invalid(%08x).\r\n", arm_vector[1]);
        state->boot_status = BOOT_FAIL_BAD_ENTRY;
        ibl_err_process(state);
    }

    /* fwdg reload */
    ibl_fwdg_reload();

#if defined(PLATFORM_GD32H7XX)
    SCB_InvalidateICache();
//    SCB_InvalidateDCache();
#endif /* PLATFORM_GD32H7XX */

    ibl_trace(IBL_ALWAYS, "Jump to MBL.\r\n");
    /* disable systick */
    SysTick->CTRL  = 0;
    jump_to_img(arm_vector[0], arm_vector[1]);
    while (1){
    }
}

void ibl_verify_image(struct ibl_state_t *state, uint32_t mbl_offset)
{
    int ret = 0;
    uint32_t ver;
    
    /* Validate MBL Image */
    ret = validate_mbl(mbl_offset, state->mbl_pk, &(state->mbl_info));
    if (ret != IMG_OK) {
        ibl_trace(IBL_ERR, "Validate MBL failed(%d).\r\n", ret);
        state->boot_status = BOOT_FAIL_BAD_MBL;
        ibl_err_process(state);
    } else {
        ibl_trace(IBL_ALWAYS, "Validate MBL Image OK.\r\n");
    }
    /* fwdg reload */
    ibl_fwdg_reload();

    state->boot_status = BOOT_VERIFY_MBL_OK;

    /* Update NV counter */
    ver = state->mbl_info.version;
    ret = sys_set_fw_version_internal(IMG_TYPE_MBL, ver);
    if (ret < 0) {
        ibl_trace(IBL_ERR, "Update MBL NV counter failed(%d).\r\n", ret);
        state->boot_status = BOOT_FAIL_SET_NV_CNTR;
        ibl_err_process(state);
    } else if (ret == 0){
        ibl_trace(IBL_ALWAYS, "Update MBL version to %d.%d.%d\r\n",
                            ((ver >> 24) & 0xFF), ((ver >> 16) & 0xFF), (ver & 0xFFFF));
    }

}

char* reset_reason_get(uint8_t reason)
{
    switch(reason){
    /* clear TMR error output status 0 */
    case 0:
        return "unknown";
        break;
    /* clear TMR error output status 1 */
    case 1:
        return  "ob load";
        break;
    /* clear TMR error output status 2 */
    case 2:
        return  "pin";
        break;
    case 3:
        return  "power on";
        break;
    case 4:
        return  "sw";
        break;
    case 5:
        return  "free wdg";
        break;
    case 6:
        return  "window wdg";
        break;
    case 7:
        return  "low power";
        break;

    default:
        return "unknown";
        break;
    }
}

void ymodem_packet_init(ymodem_packet_str *packet_str)
{
    packet_str->down_off =  RE_FLASH_BASE - FLASH_BASE + RE_MBL_OFFSET;
    packet_str->down_size = MBL_CODE_SIZE + RE_VTOR_ALIGNMENT;
    packet_str->wait_second = 3;
    packet_str->ymodem_rx = (ymodem_rx_fun)uart_rx_to;
    packet_str->ymodem_printf = (ymodem_printf_fun)ibl_printf;
    packet_str->ymodem_flash_erase = (ymodem_flash_erase_fun)flash_erase;
    packet_str->ymodem_flash_write = (ymodem_flash_write_fun)flash_write;
    packet_str->ymodem_strtoul = (ymodem_strtoul_fun)ibl_strtoul;
    packet_str->ymodem_send = (ymodem_send_fun)uart_putc;
#ifndef SECURITY_PROTECT_DISABLE
    packet_str->ymodem_fwdg_reload = (ymodem_fwdg_reload_fun)ibl_fwdg_reload;
#endif /* SECURITY_PROTECT_DISABLE */
}

void ibl_ymdoem_update(struct ibl_state_t *state)
{
        ymodem_packet = (ymodem_packet_str *)ibl_calloc(1, sizeof(ymodem_packet_str));
        ibl_trace(IBL_ALWAYS, "IBL ");
        ymodem_packet_init(ymodem_packet);

#if defined(PLATFORM_GD32H7XX) || defined(PLATFORM_GD32A513)
        /* wait IDLEF set and clear it */
        while(RESET == usart_flag_get(LOG_UART, USART_FLAG_IDLE)) {
        }
        usart_flag_clear(LOG_UART, USART_FLAG_IDLE);
#endif /* PLATFORM_GD32H7XX */
        usart_interrupt_enable(LOG_UART, USART_INT_RBNE);
        usart_interrupt_enable(LOG_UART, USART_INT_IDLE);

        ymodem_download_check(ymodem_packet, (delay_ms_fun)delay_ms);
        if (0 != (ymodem_packet->ymodem_flag&YMODEM_FLAG_STP)) {
            if (0 != (ymodem_packet->ymodem_flag&YMODEM_FLAG_EMAX)) {
                ibl_trace(IBL_ERR, "Ymodem update error.\r\n");
                ibl_free(ymodem_packet);
                ibl_err_process(state);
            } else {
                ibl_printf("\r\n");
                ibl_trace(IBL_ALWAYS, "Ymodem update success, system reboot.\r\n");
                ibl_free(ymodem_packet);
                delay_ms(100);
                /* generate a system reset */
                NVIC_SystemReset();
            }
        }
        if (ymodem_packet != NULL) {
            ibl_free(ymodem_packet);
        }
        usart_interrupt_disable(LOG_UART, USART_INT_RBNE);
        usart_interrupt_disable(LOG_UART, USART_INT_IDLE);
}

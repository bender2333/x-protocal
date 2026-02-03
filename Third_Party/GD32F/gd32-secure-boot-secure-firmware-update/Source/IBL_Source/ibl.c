/*!
    \file    ibl.c
    \brief   IBL for GD32 SDK

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
#include "mbedtls/platform.h"
#include "mbedtls/ecp.h"
#include "mbedtls/ssl_ciphersuites.h"
#include "mbedtls/entropy_poll.h"

#include "Platform/IBL/ymodem.h"
#include "Platform/IBL/ibl_systick.h"


#if defined IBL_TEST_SUIT
#include "ibl_test_suit.h"
#endif /* IBL_TEST_SUIT */

uint32_t img_hdr_size = 0;

extern void random_delay(void);

REGION_DECLARE(Image$$, ARM_LIB_STACKHEAP, $$ZI$$Base);
REGION_DECLARE(Image$$, IBL_DATA, $$ZI$$Base);
REGION_DECLARE(Image$$, IBL_HEAP, $$ZI$$Limit);

void ibl_zi_data_init(void)
{
    uint32_t data_start = (uint32_t)&REGION_NAME(Image$$, IBL_DATA, $$ZI$$Base);
    uint32_t data_end = (uint32_t)&REGION_NAME(Image$$, IBL_HEAP, $$ZI$$Limit);

    /* SRAM memory clear */
    memset((void*)data_start, 0, (data_end - data_start));
}

REGION_DECLARE(Image$$, IBL_DATA, $$RW$$Base);
REGION_DECLARE(Image$$, IBL_DATA, $$RW$$Limit);
REGION_DECLARE(Load$$, IBL_DATA, $$RW$$Base); 

void ibl_rw_data_init(void)
{
    uint32_t i;
    uint8_t* dst = (uint8_t*)&REGION_NAME(Image$$, IBL_DATA, $$RW$$Base);
    uint8_t* src = (uint8_t*)&REGION_NAME(Load$$, IBL_DATA, $$RW$$Base);
    uint32_t len = (uint32_t)&REGION_NAME(Image$$, IBL_DATA, $$RW$$Limit) - (uint32_t)&REGION_NAME(Image$$, IBL_DATA, $$RW$$Base);

    /* SRAM memory init */
    memcpy(dst, src, len);
} 


#if (SYS_STATUS_ENCRPTED == 0)
static int set_initial_version(struct sys_setting_t *setting)
{
    int32_t ret;
    uint32_t mbl_version;
    uint32_t img_version;

    ret = sys_status_get_internal(SYS_MBL_VER_COUNTER, LEN_SYS_VER_COUNTER, (uint8_t *)&mbl_version);
    if (ret == SYS_STATUS_FOUND_ERR) {
        return -1;
    } else if (ret == SYS_STATUS_NOT_FOUND) {
        sys_status_set_internal(SYS_MBL_VER_COUNTER, LEN_SYS_VER_COUNTER, (uint8_t *)&setting->mbl_initial_version);
    } else {
        /* version counter had been inited, do nothing */
    }

    ret = sys_status_get_internal(SYS_IMG_VER_COUNTER, LEN_SYS_VER_COUNTER, (uint8_t *)&img_version);
    if (ret == SYS_STATUS_FOUND_ERR) {
        return -1;
    } else if (ret == SYS_STATUS_NOT_FOUND) {
        sys_status_set_internal(SYS_IMG_VER_COUNTER, LEN_SYS_VER_COUNTER, (uint8_t *)&setting->img_initial_version);
    } else {
        /* version counter had been inited, do nothing */
    }

    return 0;
}
#endif

void mbedtls_init(void)
{
    /* Platform Related */
    mbedtls_platform_set_calloc_free(ibl_calloc, ibl_free);
    mbedtls_platform_set_snprintf(ibl_snprintf);
    mbedtls_platform_set_printf(ibl_printf);
    mbedtls_platform_set_time(NULL);                 /* Not used by IBL. */
    mbedtls_platform_set_hardware_poll(ibl_hardware_poll);

    /* HW PKA Related */
    ibl_set_mutex_func(__LOCK, NULL);
    ibl_set_mutex_func(__UNLOCK, NULL);
    mbedtls_hwpka_flag_set(0
                        | MBEDTLS_HW_EXP_MOD
                        | MBEDTLS_HW_RSA_PRIVATE
                        | MBEDTLS_HW_ECDSA_SIGN
                        | MBEDTLS_HW_ECDSA_VERIFY
                        | MBEDTLS_HW_ECP_MUL
                        | MBEDTLS_HW_ECP_CHECK
                        | MBEDTLS_HW_MPI_MUL);

    /* Others */
#if defined (MBEDTLS_ECP_C)
    mbedtls_ecp_curve_val_init();
#endif /* MBEDTLS_ECP_C */

#if defined(MBEDTLS_SSL_TLS_C)
    mbedtls_ciphersuite_preference_init(NULL);
#endif /* MBEDTLS_SSL_TLS_C */
#if defined(PLATFORM_GD32F5XX) ||  defined(PLATFORM_GD32H7XX) || defined(PLATFORM_GD32G5X3)
    /* enable CAU clock */
    rcu_periph_clock_enable(RCU_CAU);
    /* deinitialize CAU */
    cau_deinit();
#endif
#if defined(PLATFORM_GD32F5XX)
    rcu_periph_clock_enable(RCU_PKCAU);
    /* wait for PKCAU busy flag to reset */
    while(RESET != pkcau_flag_get(PKCAU_FLAG_BUSY));
#endif /* PLATFORM_GD32F5XX */
}

int gdmain(void)
{
    int ret;
    uint8_t val;
    struct sys_setting_t setting;
    struct ibl_state_t state;
    uint8_t rotpk[EFUSE_ROTPK_SZ];
    uint32_t mbl_offset;
    uint32_t arm_vector[2];
    uint32_t msp_stack_bottom = (uint32_t)&REGION_NAME(Image$$, ARM_LIB_STACKHEAP, $$ZI$$Base);
#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)
    __set_MSPLIM(msp_stack_bottom);
#endif

    SCB->VTOR = IBL_CODE_START;

    cache_enable();
    ibl_zi_data_init();
    ibl_rw_data_init();
    memset(&state, 0, sizeof(state));
    state.boot_status = BOOT_START;

    /* Initialize the platform related function pointers for MbedTLS */
    /* MUST have done before r/w system status and image verification */
    mbedtls_init();

    /* Read Trace Level */
    ret = sys_status_get_internal(SYS_TRACE_LEVEL, LEN_SYS_TRACE_LEVEL, &val);
    if (ret == SYS_STATUS_NOT_FOUND || ret == SYS_STATUS_FOUND_ERR) {
        trace_level = IBL_DBG;
    } else if (ret >= SYS_STATUS_FOUND_OK){
        trace_level = val;
    } 

    /* Check HW info */
    check_hw_info(&state);

    /* Config UART for trace print */
    log_uart_init();
    ibl_trace(IBL_ALWAYS, "GIGA DEVICE\r\n");

    /* Initialize flash for reading system info */
    ret = flash_init();
    if (ret != 0) {
        ibl_trace(IBL_ERR, "Init flash error(%d).\r\n", ret);
        ibl_err_process(&state);
    }

    /* Check EFUSE settings */
    check_efuse_setting(&state, rotpk);

    /* security is deployed here */
    ibl_security_configure();

#if (IBL_VERSION >= V_1_1)
    ibl_trace(IBL_ALWAYS, "IBL version %d.%d\r\n", (state.ibl_ver >> 8), (state.ibl_ver & 0xFF));
    ibl_trace(IBL_ALWAYS, "Reset by %s.\r\n", reset_reason_get(state.reset_flag));
#endif

    state.boot_status = BOOT_HW_INIT_OK;

    /* it is a good time for test suit now */
#ifdef IBL_TEST_SUIT
    ibl_test_suit_init();
#endif /* IBL_TEST_SUIT */

    if (IBL_VERIFY_NONE != state.ibl_opt) {
        /* Check System Config in the Flash */
        ret = check_sys_config(&setting);
        if (ret != 0) {
            ibl_trace(IBL_ERR, "Check sys config error(%d).\r\n", ret);
            state.boot_status = BOOT_FAIL_BAD_SYS_CONF;
            ibl_err_process(&state);
        }
    }

    /* fwdg reload */
    ibl_fwdg_reload();

    /* ymodem update */
    if (ERR_PROCESS_ENTER_ISP == err_process) {
        ibl_ymdoem_update(&state);
    }

    /* fwdg reload */
    ibl_fwdg_reload();

#if (SYS_STATUS_ENCRPTED == 0)
    /* Set initial MBL and IMG versions to sys_status */
    ret = set_initial_version(&setting);
    if (ret != 0) {
        ibl_trace(IBL_ERR, "Set initial version error(%d).\r\n", ret);
        state.boot_status = BOOT_FAIL_SET_INITIAL_VER;
        goto ErrProcess;
    }
#endif
    state.boot_status = BOOT_SYS_CONFIG_OK;

    mbl_offset = setting.mbl_offset;

    /* random delay 0-100us, before verify */
    random_delay();

    /* Check boot option */
    if (state.ibl_opt == IBL_VERIFY_IMG_ONLY) {
        ibl_memcpy(state.mbl_pk, rotpk, IMG_PK_LEN);
        ibl_verify_image(&state, mbl_offset);
        ibl_verify_none(&state, mbl_offset, img_hdr_size, arm_vector);
    } else if (state.ibl_opt == IBL_VERIFY_NONE) {
        ibl_memcpy(state.mbl_pk, rotpk, IMG_PK_LEN);
        /* here need change */
        mbl_offset = RE_MBL_OFFSET + RE_VTOR_ALIGNMENT;
        ibl_verify_none(&state, mbl_offset, img_hdr_size, arm_vector);
    } else {
        ibl_trace(IBL_ERR, "Bad boot option(%d).\r\n", state.ibl_opt);
        state.boot_status = BOOT_FAIL_BAD_OPT;
        ibl_err_process(&state);
    }

    return ret;
}

/*!
    \file    ibl_trng.c
    \brief   IBL trng configure for for GD32 SDK

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
#include "mbedtls/md5.h"

unsigned char btrng_seed;
static unsigned int randPool;       /* Pool of randomness. */
static unsigned int randCount;      /* Pseudo-random incrementer */

#if defined (PLATFORM_GDM32) || defined(PLATFORM_GD32F5XX) 
int trng_ready_check(void)
{
    uint32_t timeout = 0;
    uint32_t trng_flag = STATUS_RESET;
    int reval = 1;

    /* check wherther the random data is valid */
    do {
        timeout++;
        trng_flag = trng_flag_get(TRNG_FLAG_DRDY);
    } while((STATUS_RESET == trng_flag) &&(0xFFFF > timeout));

    if (STATUS_RESET == trng_flag) {
        /* ready check timeout */
        trng_flag = trng_flag_get(TRNG_FLAG_CECS);
        ibl_trace(IBL_ERR, "Clock error(%d).\r\n", trng_flag);
        trng_flag = trng_flag_get(TRNG_FLAG_SECS);
        ibl_trace(IBL_ERR, "Seed error(%d).\r\n", trng_flag);
        reval = 0;
    }

    /* return check status */
    return reval;
}

/*!
    \brief      configure TRNG module
    \param[in]  none
    \param[out] none
    \retval     ErrStatus: SUCCESS or ERROR
*/
int trng_configuration(void)
{
    int reval = 0;

    /* TRNG module clock enable */
    rcu_periph_clock_enable(RCU_TRNG);

    /* TRNG registers reset */
    trng_deinit();
    trng_enable();
    /* check TRNG work status */
    if (!trng_ready_check())
        reval = -1;

    return reval;
}
#endif /* PLATFORM_GDM32 || PLATFORM_GD32F5XX */

int random_get(unsigned char *output, unsigned int len)
{
    mbedtls_md5_context md5;
    unsigned char tmp[16];
    unsigned int n, total = len;
    unsigned char *p = output;
    unsigned int rand;
    int ret;

    if (randCount == 0)
        randCount = 0x39017842;

#if defined (PLATFORM_GDM32) || defined(PLATFORM_GD32F5XX)
    if (btrng_seed) {
        ret = trng_configuration();
        if (ret < 0) {
            ibl_trace(IBL_ERR, "TRNG config error.\r\n");
            return ret;
        }
        rand = trng_get_true_random_data();
        ibl_memcpy(tmp, &rand, sizeof(rand));
    } else
#else
    btrng_seed = 0;
#endif
    {
        if (randPool == 0) {
            randPool = 0xDEADB00B;
        }
        ibl_memcpy(tmp, &randPool, sizeof(randPool));
    }

    while (len > 0) {
        n = len;
        if (n > 16)
            n = 16;

        *(uint32_t *)(tmp + 12) += randCount;
#if defined(MBEDTLS_MD5_C)
        mbedtls_md5_ret((unsigned char *)tmp, 16, tmp);
#endif

        randCount++;
        ibl_memcpy(p, tmp, n);
        p += n;
        len -= n;
    }
    if (!btrng_seed)
        randPool = *(unsigned int*)tmp;
    return 0;
}

int ibl_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen)
{
    int ret;
    ((void) data);

    ret = random_get(output, len);
    if (ret < 0) {
        return -1;
    }
    if (olen)
        *olen = len;

    return( 0 );
}

#ifdef ROM_SELF_TEST
void trng_self_test(void)
{
#if defined PLATFORM_GDM32
    uint32_t random_data = 0, random_lastdata = 0;
    uint8_t retry = 0;
    int count = 0;

    ibl_printf("\r\n========= TRNG Test Start =============\r\n");
    /* configure TRNG module */
    while((trng_configuration() < 0) && retry < 3){
        ibl_printf("TRNG init fail.\r\n");
        ibl_printf("TRNG init retry...\r\n");
        retry++;

        if (retry >= 3) {
            ibl_printf("TRNG init failed and return.\r\n");
            return;
        }
    }

    ibl_printf("TRNG init ok.\r\n");
    /* get the first random data */
    random_lastdata = trng_get_true_random_data();

    ibl_printf("Get random data: ");
    while (count++ < 1000){
        if ((count % 8) == 1) {
            ibl_printf("\r\n\t");
        }
        /* check wherther the random data is valid and get it */
        if (trng_ready_check()){
            random_data = trng_get_true_random_data();
            if(random_data != random_lastdata){
                random_lastdata = random_data;
                ibl_printf("0x%08x ", random_data);
            }else{
                /* the random data is invalid */
                ibl_printf("\r\nError: Get the random data is same \r\n");
            }
        }
    }
    ibl_printf("\r\n========= TRNG Test End =============\r\n");
#endif /* PLATFORM_GDM32 */
}
#endif /* ROM_SELF_TEST */


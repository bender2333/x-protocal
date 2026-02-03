/*!
    \file    app_test_suit.c
    \brief   APP test suit GD32 SDK

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

#include "Utilities/Third_Party/mbedtls-2.17.0/include/mbedtls/aes.h"
#include "Utilities/Third_Party/mbedtls-2.17.0/include/mbedtls/sha256.h"
#include "Utilities/Third_Party/mbedtls-2.17.0/include/mbedtls/md.h"
#include "Utilities/Third_Party/mbedtls-2.17.0/include/mbedtls/entropy.h"
#include "Utilities/Third_Party/mbedtls-2.17.0/include/mbedtls/ctr_drbg.h"
#include "Utilities/Third_Party/mbedtls-2.17.0/include/mbedtls/ecdsa.h"
#include "Utilities/Third_Party/mbedtls-2.17.0/include/mbedtls/platform.h"

#include "Source/IBL_Source/ibl_export.h"
#include "app_test_suit.h"

#include "Utilities/Third_Party/letter-shell-shell2.x/shell_ext.h"
#include "Source/APP_Source/Common/task.h"


#ifdef APP_TEST_SUIT

void app_test_suit_init(void)
{
    app_test_suit *test_suit_start;
    unsigned int test_suit_num = 0, i = 0;

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    extern const unsigned int app_test_suit_sec$$Base;
    extern const unsigned int app_test_suit_sec$$Limit;

    test_suit_start = (app_test_suit *)(&app_test_suit_sec$$Base);
    test_suit_num = ((unsigned int)(&app_test_suit_sec$$Limit)
                            - (unsigned int)(&app_test_suit_sec$$Base))
                            / sizeof(app_test_suit);

#elif defined(__ICCARM__)
    #pragma section = "app_test_suit_sec"
    test_suit_start = ((app_test_suit *)(__section_begin("app_test_suit_sec")));
    test_suit_num  = ((unsigned int)(__section_end("app_test_suit_sec")) - (unsigned int)(__section_begin("app_test_suit_sec")));
    test_suit_num /= sizeof(app_test_suit);    

#elif defined(__GNUC__)
    extern const unsigned int _test_suit_start;
    extern const unsigned int _test_suit_end;
    
    test_suit_start = (app_test_suit *)(&_test_suit_start);
    test_suit_num  = ((unsigned int)(&_test_suit_end)
                            - (unsigned int)(&_test_suit_start))
                            / sizeof(app_test_suit);

#else
    #error not supported compiler, please use command table mode
#endif

    for (i=0; i<test_suit_num; i++){
        (*(test_suit_start+i))();
    }
}

#if SIGN_ALGO_SET == IMG_SIG_ECDSA256

static const uint8_t hash[]   = {
                            0x8a, 0x83, 0x66, 0x5f, 0x37, 0x98, 0x72, 0x7f,
                            0x14, 0xf9, 0x2a, 0xd0, 0xe6, 0xc9, 0x9f, 0xda,
                            0xb0, 0x8e, 0xe7, 0x31, 0xd6, 0xcd, 0x64, 0x4c,
                            0x13, 0x12, 0x23, 0xfd, 0x2f, 0x4f, 0xed, 0x2a,
};

static const uint8_t pk[]     = {
                            0xB1, 0x23, 0x0F, 0x3A, 0xB4, 0x59, 0x45, 0xDA,
                            0x63, 0x67, 0xD0, 0xF8, 0x66, 0x0F, 0x62, 0x38,
                            0xAF, 0x69, 0x65, 0x98, 0x39, 0xA6, 0xCA, 0x63,
                            0x7D, 0x65, 0x58, 0x4B, 0xBD, 0x15, 0x32, 0x66,
                            0xEA, 0x75, 0xC6, 0xE6, 0xA8, 0x0A, 0x9B, 0x96,
                            0xF9, 0x84, 0x82, 0x95, 0x80, 0xB7, 0x0E, 0x40,
                            0x0D, 0x51, 0xD1, 0x9D, 0xF2, 0x67, 0x90, 0xC6,
                            0x35, 0x61, 0xD1, 0xA2, 0xBA, 0x35, 0x40, 0x8E,
};

static const uint8_t sig[]   = {
                            0xDD, 0xFE, 0x28, 0x4B, 0xD0, 0x37, 0x0C, 0xCD,
                            0x05, 0x1B, 0xC7, 0xEF, 0x80, 0xDA, 0x66, 0x47,
                            0xB5, 0x88, 0xAC, 0x12, 0x03, 0x6A, 0x29, 0x4F,
                            0x9A, 0x20, 0xE2, 0xCB, 0x79, 0x65, 0xE8, 0x60,
                            0x76, 0x11, 0xF5, 0xFF, 0xA7, 0x2F, 0x13, 0x77,
                            0x75, 0x46, 0x28, 0xE8, 0x2A, 0x45, 0x51, 0xB5,
                            0x18, 0x0B, 0x2F, 0x73, 0x24, 0x9A, 0x6B, 0xAD,
                            0xD8, 0xED, 0x10, 0x4D, 0xFF, 0x27, 0xFF, 0xD8
};

static const uint8_t sig_err[]   = {
                            0xFE, 0xDD, 0x28, 0x4B, 0xD0, 0x37, 0x0C, 0xCD,
                            0x05, 0x1B, 0xC7, 0xEF, 0x80, 0xDA, 0x66, 0x47,
                            0xB5, 0x88, 0xAC, 0x12, 0x03, 0x6A, 0x29, 0x4F,
                            0x9A, 0x20, 0xE2, 0xCB, 0x79, 0x65, 0xE8, 0x60,
                            0x76, 0x11, 0xF5, 0xFF, 0xA7, 0x2F, 0x13, 0x77,
                            0x75, 0x46, 0x28, 0xE8, 0x2A, 0x45, 0x51, 0xB5,
                            0x18, 0x0B, 0x2F, 0x73, 0x24, 0x9A, 0x6B, 0xAD,
                            0xD8, 0xED, 0x10, 0x4D, 0xFF, 0x27, 0xFF, 0xD8
};

void secp256r1_verify_test(void)
{
    int res = 0, i = 0;
    ibl_printf("\r\n/*** test suit %s start ***/\r\n", __func__);

    res = ibl_img_verify_sign((uint8_t *)hash, 32, (uint8_t *)pk, (uint8_t *)sig);
    if(res == 0) {
        ibl_printf("verfiy test ok \r\n");
    } else {
        ibl_printf("verfiy test faile: %d \r\n", res);
    }

    ibl_printf("/*** test suit %s end ***/\r\n", __func__);
}
APP_TEST_SUIT_INIT(secp256r1_verify_test);

void secp256r1_verify_errdata_test(void)
{
    int res = 0, i = 0;
    ibl_printf("\r\n/*** test suit %s start ***/\r\n", __func__);

    res = ibl_img_verify_sign((uint8_t *)hash, 32, (uint8_t *)pk, (uint8_t *)sig_err);
    if(res != 0) {
        ibl_printf("verfiy test ok \r\n");
    } else {
        ibl_printf("verfiy test faile: %d \r\n", res);
    }

    ibl_printf("/*** test suit %s end ***/\r\n", __func__);
}
APP_TEST_SUIT_INIT(secp256r1_verify_errdata_test);
#endif /* SIGN_ALGO_SET == IMG_SIG_ECDSA256 */

static const char sha_result[] = 
{
                            0x75,0x09,0xe5,0xbd,0xa0,0xc7,0x62,0xd2,
                            0xba,0xc7,0xf9,0x0d,0x75,0x8b,0x5b,0x22,
                            0x63,0xfa,0x01,0xcc,0xbc,0x54,0x2a,0xb5,
                            0xe3,0xdf,0x16,0x3b,0xe0,0x8e,0x6c,0xa9
};
#if !defined GD32G5X3_BANKSWAP
void sha256_test(void)
{
    int32_t ret;
    uint8_t message_digest[256] = {0};
    uint8_t message_digest_length = 0;

    ibl_printf("\r\n/*** test suit %s start ***/\r\n", __func__);

    /* init of a local sha256 context */
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);

    ret = mbedtls_sha256_starts_ret(&ctx, 0);   /* 0 for sha256 */
    
    if (0 == ret) {
        /* casting is fine because size_t is unsigned int in ARM C */
        ret = mbedtls_sha256_update_ret(&ctx, (const uint8_t *)"hello world!", ibl_strlen("hello world!")); 

        if (0 == ret) {
            ret = mbedtls_sha256_finish_ret(&ctx, message_digest);

              if (0 == ret) {
                  message_digest_length = 32; /* sha256 */
                  ret = ibl_memcmp(message_digest, sha_result, message_digest_length);
                  if(ret == 0) {
                        ibl_printf("sha256 test ok \r\n");
                    } else {
                        ibl_printf("sha256 test faile\r\n");
                    }
              } else {
                    message_digest_length = 0;
              }
        }
    }

    mbedtls_sha256_free(&ctx);
    ibl_printf("/*** test suit %s end ***/\r\n", __func__);
}
APP_TEST_SUIT_INIT(sha256_test);
#endif
void app_test(void)
{
    shell_task_flag |= TASK_FLAG_APPTEST;
}
SHELL_EXPORT_CMD(app_test, app_test, run app testsuit);

#endif /* APP_TEST_SUIT */

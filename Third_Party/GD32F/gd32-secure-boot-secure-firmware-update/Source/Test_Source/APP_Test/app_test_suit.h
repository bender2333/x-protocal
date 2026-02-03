/*!
    \file    app_test_suit.h
    \brief   APP test suit header file for GD32 SDK

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

#ifndef __APP_TEST_SUIT__
#define __APP_TEST_SUIT__

typedef void (*app_test_suit)(void);

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    #define SECTION(x)                  __attribute__((section(x)))
#elif defined(__ICCARM__)
    #define SECTION(x)                  @x
#elif defined(__GNUC__)
    #define SECTION(x)                  __attribute__((section(x)))
#else
    #define SECTION(x)
#endif
            
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    /* keil __attribute__((used)) make do not optimization */
    #define     APP_TEST_SUIT_INIT(func)                                                             \
                const app_test_suit                                                                  \
                app_test_suit_##func __attribute__((used)) SECTION("app_test_suit_sec") =                    \
                func;
#elif defined(__ICCARM__)
    /* IAR __root make do not optimization */
    #define     APP_TEST_SUIT_INIT(func)                                           \
                __root const app_test_suit                                         \
                app_test_suit_##func SECTION("app_test_suit_sec") =                        \
                func;
#elif defined(__GNUC__)
    #define     APP_TEST_SUIT_INIT(func)                                       \
                const app_test_suit                                            \
                app_test_suit_##func  __attribute__((used)) SECTION("app_test_suit_sec") =                    \
                func;
#else
    #define     APP_TEST_SUIT_INIT(func)
#endif

void app_test_suit_init(void);

#endif /* __IBL_TEST_SUIT__ */

/*!
    \file    sys_init.c
    \brief   system function initialization

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

#include "sys_init.h"

void sys_init_fun(void)
{
    sys_init *sys_init_start;
    unsigned int sys_init_num = 0, i = 0;

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    extern const unsigned int sys_init_sec$$Base;
    extern const unsigned int sys_init_sec$$Limit;

    sys_init_start = (sys_init *)(&sys_init_sec$$Base);
    sys_init_num = ((unsigned int)(&sys_init_sec$$Limit)
                            - (unsigned int)(&sys_init_sec$$Base))
                            / sizeof(sys_init);

#elif defined(__ICCARM__)
    #pragma section = "sys_init_sec"
    sys_init_start = ((sys_init *)(__section_begin("sys_init_sec")));
    sys_init_num  = ((unsigned int)(__section_end("sys_init_sec")) - (unsigned int)(__section_begin("sys_init_sec")));
    sys_init_num /= sizeof(sys_init);    

#elif defined(__GNUC__)
    extern const unsigned int _sys_init_start;
    extern const unsigned int _sys_init_end;
    
    sys_init_start = (sys_init *)(&_sys_init_start);
    sys_init_num  = ((unsigned int)(&_sys_init_end)
                            - (unsigned int)(&_sys_init_start))
                            / sizeof(sys_init);

#else
    #error not supported compiler, please use command table mode
#endif

    for (i=0; i<sys_init_num; i++){
        (*(sys_init_start+i))();
    }
}


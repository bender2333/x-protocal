/*!
    \file    ibl_trace.h
    \brief   IBL trace header file for GD32 SDK

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

#ifndef __IBL_TRACE_H__
#define __IBL_TRACE_H__

/* Define Trace Level */
#define IBL_OFF            0
#define IBL_ALWAYS         1
#define IBL_ERR            2
#define IBL_WARN           3
#define IBL_INFO           4
#define IBL_DBG            5

extern int32_t trace_level;

int ibl_printf(const char *format, ...);
int ibl_trace_ex(uint32_t level, const char *fmt, ...);
int ibl_snprintf(char *out, uint32_t out_sz, const char *format, ...);
int ibl_print_data(int32_t level, char *title, uint8_t *data, uint32_t count);

#ifdef ROM_PROJECT
#define IBL_ASSERT(expr)                            \
    if ((expr) == 0) {                              \
        ibl_printf("ASSERT: "#expr);                \
        for( ;; );                                  \
    }
#else
#define IBL_ASSERT(expr)                            \
    if ((expr) == 0) {                              \
        for( ;; );                                  \
    }
#endif

#if 1
#define ibl_trace ibl_trace_ex
#else
#define ibl_trace(level, fmt, arg...) do {\
    if (level <= trace_level) {\
        if (level == IBL_ERR) {\
            ibl_printf("ERR: "fmt, ##arg);\
        } else if (level == IBL_WARN) {\
            ibl_printf("WARN: "fmt, ##arg);\
        } else if (level == IBL_INFO) {\
            ibl_printf("INFO: "fmt, ##arg);\
        } else if (level == IBL_DBG) {\
            ibl_printf("DBG: "fmt, ##arg);\
        } else {\
            ibl_printf(fmt, ##arg);\
        }\
    }\
} while(0);
#endif
#endif  /* __IBL_TRACE_H__ */

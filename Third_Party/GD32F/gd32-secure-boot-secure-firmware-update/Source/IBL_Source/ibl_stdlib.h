/*!
    \file    ibl_stdlib.h
    \brief   IBL standard library header file for GD32 SDK

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

#ifndef __IBL_STDLIB_H__
#define __IBL_STDLIB_H__

//#define REPLACE_AEABI_MEMCLR
//#define REPLACE_AEABI_MEMCPY

#define TOLOWER(x)        ((x) | 0x20)
#define isxdigit(c)        (('0' <= (c) && (c) <= '9') \
                        || ('a' <= (c) && (c) <= 'f') \
                        || ('A' <= (c) && (c) <= 'F'))

#define isdigit(c)        ('0' <= (c) && (c) <= '9')
#define isspace(x)        (x == ' ' || x == '\t' \
                        || x == '\n' || x =='\f' \
                        || x =='\b' || x=='\r')

#define in_range(c, lo, up)  ((uint8_t)c >= lo && (uint8_t)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)

void* ibl_memcpy(void *des, const void *src, unsigned int n);
void * ibl_memmove(void *out, const void *in, size_t nbytes);
void* ibl_memset(void *s, int c, unsigned int count);
int ibl_memcmp(const void * buf1, const void * buf2, unsigned int count);
int ibl_rand(unsigned char *output, unsigned int len);
size_t ibl_strlen(const char *str);
int ibl_strncmp(const char *s1, const char *s2, size_t len);
int ibl_strcmp(const char *s1, const char *s2);
char * ibl_strncpy(char* dst, const char* src, size_t len);
char * ibl_strchr(const char *s, int c);
char *ibl_strstr(const char *s1, const char *s2);
int ibl_atoi(const char *nptr);
unsigned long ibl_strtoul(const char *cp, char **endp, unsigned int base);
long ibl_strtol(const char *cp, char **endp, unsigned int base);
int ibl_str2hex(char *input, int input_len, unsigned char *output, int output_len);
void ibl_set_mutex_func(int tolock, void *func);
void ibl_get_mutex(int type);
void ibl_put_mutex(int type);

#undef memcpy
#define memcpy        ibl_memcpy
#undef memset
#define memset        ibl_memset
#undef memcmp
#define memcmp      ibl_memcmp
#undef memmove
#define memmove        ibl_memmove
#undef strlen
#define strlen        ibl_strlen
#undef strncmp
#define strncmp        ibl_strncmp
#undef strcmp
#define strcmp        ibl_strcmp
#undef strchr
#define strchr        ibl_strchr
#undef strstr
#define strstr        ibl_strstr
#undef strncpy
#define strncpy        ibl_strncpy
#undef atoi
#define atoi        ibl_atoi
#undef strtol
#define strtol        ibl_strtol
#undef strtoul
#define strtoul        ibl_strtoul

#endif /* __IBL_STDLIB_H */


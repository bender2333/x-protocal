/*!
    \file    ibl_stdlib.c
    \brief   IBL standard library for GD32 SDK

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

void (*ibl_lock_func)(int type);
void (*ibl_unlock_func)(int type);

#ifndef asm
#define asm                     __asm
#endif

void* ibl_memcpy(void *des, const void *src, unsigned int n)
{
    unsigned char *xdes = (unsigned char *)des;
    unsigned char *xsrc = (unsigned char *)src;

#if defined(PLATFORM_GD32H7XX)
#ifndef MEM_CMP_NOT_INV_ICACHE
    SCB_InvalidateICache();
#endif /* MEM_CMP_NOT_INV_ICACHE */
#endif /* PLATFORM_GD32H7XX */

    while (n--) {
        *xdes++ = *xsrc++;
    }
    return des;
}

void * ibl_memmove(void *out, const void *in, size_t nbytes)
{
    char* dest = out;
    const char* src = in;

    if (dest < src) {
        while (nbytes > 0) {
            nbytes--;
            *dest++ = *src++;
        }
    } else {
        dest += nbytes;
        src += nbytes;
        while (nbytes > 0) {
            nbytes--;
            *--dest = *--src;
        }
    }
    return out;
}

void* ibl_memset(void *s, int c, unsigned int count)
{
    unsigned char *xs = s;

    while (count--) {
        *xs++ = c;
    }
    return s;
}

int ibl_memcmp(const void * buf1, const void * buf2, unsigned int count)
{
    if(!count)
        return 0;

    while (--count && *((char *)buf1) == *((char *)buf2)) {
        buf1 = (char *)buf1 + 1;
        buf2 = (char *)buf2 + 1;
    }

    return (*((char *)buf1) - *((char *)buf2));
}

#if 0
int rom_srand(int seed)
{
    rom_trng = seed;
    rom_trng_cnt = 1;
    return 0;
}

int ibl_rand(void)
{
    if (rom_trng == 0)
        rom_trng = 0xDEADB00B;

    rom_trng = ((rom_trng & 0x007F00FF) << 7) ^
        ((rom_trng & 0x0F80FF00) >> 8) ^
        (rom_trng_cnt << 13) ^ (rom_trng_cnt >> 9);

    rom_trng_cnt++;

    return (int)rom_trng;
}
#else
#ifdef IBL_PROJECT
int ibl_rand(unsigned char *output, unsigned int len)
{
    return random_get(output, len);
}
#endif
#endif

size_t ibl_strlen(const char *str)
{
    const char *start = str;
    while (*str)
        str++;
    return str - start;
}

int ibl_strncmp(const char *s1, const char *s2, size_t len)
{
    while (*s1 != '\0' && *s1 == *s2 && len > 0) {
        s1++;
        s2++;
        len--;
    }
    if (len == 0)
        return 0;
    return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}

int ibl_strcmp(const char *s1, const char *s2)
{
    int ret;

    IBL_ASSERT((NULL != s1) && (NULL != s2));
    while(!(ret = *(unsigned char*)s1 - *(unsigned char*)s2) && *s1) {
        s1++;
        s2++;
    }

    if (ret < 0) {
        return -1;
    } else if (ret > 0) {
        return 1;
    }
    return 0;
}

char * ibl_strncpy(char* dst, const char* src, size_t len)
{
    char* out = dst;

    while (*src && len > 0) {
        *out++ = *src++;
        len--;
    }

    while (len > 0) {
        *out++ = '\0';
        len--;
    }
    return dst;
}

char * ibl_strchr(const char *s, int c)
{
#if 0
    char* result = 0;
    while (1) {
        if (*s == c)
            result = (char*) s;
        if (*s == 0)
            return result;
        s++;
    }
    return 0;
#else
    while (*s && *s != (char)c)
        s++;

    if (*s == (char)c)
        return (char *)s;

    return 0;

#endif
}

char *ibl_strstr(const char *s1, const char *s2)
{
    /* find first occurrence of s2[] in s1[] */
    if (*s2 == '\0')
        return (char *)s1;

    for (; (s1 = ibl_strchr(s1, *s2)) != 0; ++s1) {
        /* match rest of prefix */
        const char *sc1, *sc2;
        for (sc1 = s1, sc2 = s2; ; ) {
            if (*++sc2 == '\0')
                return (char *)s1;
            else if (*++sc1 != *sc2)
                break;
        }
    }
    return 0;
}

int ibl_atoi(const char *nptr)
{
    int c;              /* current char */
    int total;         /* current total */
    int sign;           /* if '-', then negative, otherwise positive */

    /* skip whitespace */
    while ( isspace((int)(unsigned char)*nptr) )
        ++nptr;

    c = (int)(unsigned char)*nptr++;
    sign = c;           /* save sign indication */
    if (c == '-' || c == '+')
        c = (int)(unsigned char)*nptr++;    /* skip sign */

    total = 0;

    while (isdigit(c)) {
        total = 10 * total + (c - '0');     /* accumulate digit */
        c = (int)(unsigned char)*nptr++;    /* get next char */
    }

    if (sign == '-')
        return -total;
    else
        return total;   /* return result, negated if necessary */
}

unsigned long ibl_strtoul(const char *cp, char **endp, unsigned int base)
{
    unsigned long result = 0,value;

    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((TOLOWER(*cp) == 'x') && isxdigit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    } else if (base == 16) {
        if (cp[0] == '0' && TOLOWER(cp[1]) == 'x')
            cp += 2;
    }
    while (isxdigit(*cp) &&
           (value = isdigit(*cp) ? *cp-'0' : TOLOWER(*cp)-'a'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}

long ibl_strtol(const char *cp, char **endp, unsigned int base)
{
    if(*cp=='-')
        return -strtoul(cp+1,endp,base);
    return strtoul(cp,endp,base);
}

int ibl_str2hex(char *input, int input_len, unsigned char *output, int output_len)
{
    int index = 0;
    char iter_char = 0;

    if (input == NULL || input_len <= 0 || input_len % 2 != 0 ||
        output == NULL || output_len < input_len / 2) {
        return -1;
    }

    memset(output, 0, output_len);

    for (index = 0; index < input_len; index += 2) {
        if (input[index] >= '0' && input[index] <= '9') {
            iter_char = input[index] - '0';
        } else if (input[index] >= 'A' && input[index] <= 'F') {
            iter_char = input[index] - 'A' + 0x0A;
        } else if (input[index] >= 'a' && input[index] <= 'f') {
            iter_char = input[index] - 'a' + 0x0A;
        } else {
            return -2;
        }
        output[index / 2] |= (iter_char << 4) & 0xF0;

        if (input[index + 1] >= '0' && input[index + 1] <= '9') {
            iter_char = input[index + 1] - '0';
        } else if (input[index + 1] >= 'A' && input[index + 1] <= 'F') {
            iter_char = input[index + 1] - 'A' + 0x0A;
        } else if (input[index + 1] >= 'a' && input[index + 1] <= 'f') {
            iter_char = input[index + 1] - 'a' + 0x0A;
        } else {
            return -3;
        }
        output[index / 2] |= (iter_char) & 0x0F;
    }

    return 0;
}

void ibl_set_mutex_func(int tolock, void *func)
{
    if (tolock)
        ibl_lock_func = func;
    else
        ibl_unlock_func = func;
}


void ibl_get_mutex(int type)
{
    if (ibl_lock_func != NULL)
        ibl_lock_func(type);
}

void ibl_put_mutex(int type)
{
    if (ibl_unlock_func != NULL)
        ibl_unlock_func(type);
}



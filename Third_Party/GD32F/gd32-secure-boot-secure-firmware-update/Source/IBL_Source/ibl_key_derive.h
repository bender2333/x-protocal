/*!
    \file    ibl_key_derive.h
    \brief   IBL key derive header file for GD32 SDK

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

#ifndef __IBL_KEY_DERIVE_H__
#define __IBL_KEY_DERIVE_H__

#include "Platform/IBL/ibl_platform_mbedtls_config.h"

/* TLS-1.2 PRF supports only SHA-256 and SHA-384.  We choose SHA-256. */
#define SHA256_HASH_SIZE        32
#define SHA256_HASH_BLOCK_SIZE     64

#define KD_SUCCESS                     (0)

#define DERIVE_KEY_SALT ( (uint8_t *) "rom_key_derive" )
#define DERIVE_KEY_SALT_LENGTH ( strlen( (const char*) DERIVE_KEY_SALT ) )

#define OUTPUT_FORMAT_PEM              0
#define OUTPUT_FORMAT_DER              1

#define RAND_CTR_DRBG                    1
#define RAND_SIMPLE                        2

#ifndef CONFIG_HW_SECURITY_ENGINE
#define MPI_PRINT(string, X) do{\
    int i;\
    ibl_printf(string "%d %d %p\r\n", (X)->s, (X)->n, (X)->p);\
    for(i=0; i<(X)->n;)\
    {\
        ibl_printf("0x%08x ", *((X)->p + i));\
        if(++i % 8 == 0)\
            ibl_printf("\r\n");\
    }\
    ibl_printf("\r\n");\
}while(0);
#endif

/* TLS-1.2 PRF supports only SHA-256 and SHA-384.  We choose SHA-256. */
typedef struct tls12_prf_generator_s
{
    /* The TLS 1.2 PRF uses the key for each HMAC iteration,
    * hence we must store it for the lifetime of the generator.
    * This is different from HKDF, where the key is only used
    * in the extraction phase, but not during expansion. */
    uint8_t *key;
    size_t key_len;

    /* `A(i) + seed` in the notation of RFC 5246, Sect. 5 */
    uint8_t *Ai_with_seed;
    size_t Ai_with_seed_len;

    /* `HMAC_hash( prk, A(i) + seed )` in the notation of RFC 5246, Sect. 5. */
    uint8_t output_block[SHA256_HASH_SIZE];

    /* Indicates how many bytes in the current HMAC block have
    * already been read by the user. */
    uint8_t offset_in_block;

    /* The 1-based number of the block. */
    uint8_t block_number;
} tls12_prf_generator_t;

int derive_sys_status_crypt_key(OUT uint8_t *key, IN size_t key_sz);
int do_iak_getpub_inner(IN int type, IN int rsa_keysz,
                            IN int ecc_gid, IN int key_format,
                            OUT uint8_t *output, OUT uint32_t *len);
int do_iak_sign_inner(IN int type, IN int rsa_keysz, IN int ecc_gid,
                   IN const uint8_t *hash, IN size_t hash_len,
                   OUT uint8_t *sig, OUT size_t *sig_len);

int do_symm_key_derive_inner(        IN uint8_t *label, IN size_t label_sz,
                                OUT uint8_t *key, IN size_t key_len);
#ifdef ROM_SELF_TEST
void key_derive_self_test(void);
void rand_self_test(IN uint8_t type, IN uint32_t flash_start);
void iak_sign_self_test(IN uint32_t type, IN uint32_t rsa_keysz, IN uint32_t ecc_gid);
#endif
#endif  /* __IBL_KEY_DERIVE_H__ */

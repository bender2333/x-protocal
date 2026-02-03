/*!
    \file    ibl_hmac.c
    \brief   IBL hmac for GD32 SDK

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
#include "mbedtls/platform_util.h"
#include "ibl_hmac.h"

int hmac_abort_internal(hmac_internal_data *hmac)
{
    mbedtls_platform_zeroize(hmac->opad, sizeof(hmac->opad));
    mbedtls_sha256_free(&hmac->hash_ctx);

    return 0;
}

void hmac_init_internal(hmac_internal_data *hmac)
{
    /* Instances of psa_hash_operation_s can be initialized by zeroization. */
    memset(hmac, 0, sizeof(*hmac));
}

int hmac_setup_internal(hmac_internal_data *hmac,
                                             const uint8_t *key,
                                             size_t key_length)
{
    unsigned char ipad[SHA256_HASH_BLOCK_SIZE];
    size_t i;
    size_t hash_size = SHA256_HASH_SIZE;
    size_t block_size = SHA256_HASH_BLOCK_SIZE;
    int ret;

    /* Sanity checks on block_size, to guarantee that there won't be a buffer
    * overflow below. This should never trigger if the hash algorithm
    * is implemented correctly. */
    /* The size checks against the ipad and opad buffers cannot be written
    * `block_size > sizeof( ipad ) || block_size > sizeof( hmac->opad )`
    * because that triggers -Wlogical-op on GCC 7.3. */
    if (block_size > sizeof(ipad))
        return(MBEDTLS_ERR_SHA256_BAD_INPUT_DATA);
    if (block_size > sizeof( hmac->opad))
        return(MBEDTLS_ERR_SHA256_BAD_INPUT_DATA);
    if (block_size < hash_size)
        return(MBEDTLS_ERR_SHA256_BAD_INPUT_DATA);

    if (key_length > block_size) {
        mbedtls_sha256_init(&hmac->hash_ctx);
        mbedtls_sha256_starts_ret(&hmac->hash_ctx, 0);

        ret = mbedtls_sha256_update_ret(&hmac->hash_ctx, key, key_length);
        if (ret != 0)
            goto cleanup;

        memset(ipad, '!', sizeof(ipad));
        ret = mbedtls_sha256_finish_ret(&hmac->hash_ctx, ipad);
        if (ret != 0)
            goto cleanup;
    }
    /* A 0-length key is not commonly used in HMAC when used as a MAC,
    * but it is permitted. It is common when HMAC is used in HKDF, for
    * example. Don't call `memcpy` in the 0-length because `key` could be
    * an invalid pointer which would make the behavior undefined. */
    else if (key_length != 0) {
        memcpy(ipad, key, key_length);
    }

    /* ipad contains the key followed by garbage. Xor and fill with 0x36
    * to create the ipad value. */
    for (i = 0; i < key_length; i++)
        ipad[i] ^= 0x36;
    memset(ipad + key_length, 0x36, block_size - key_length);

    /* Copy the key material from ipad to opad, flipping the requisite bits,
    * and filling the rest of opad with the requisite constant. */
    for (i = 0; i < key_length; i++)
        hmac->opad[i] = (ipad[i] ^ 0x36 ^ 0x5C);
    memset(hmac->opad + key_length, 0x5C, block_size - key_length);

    mbedtls_sha256_init(&hmac->hash_ctx);
    mbedtls_sha256_starts_ret(&hmac->hash_ctx, 0);

    ret = mbedtls_sha256_update_ret(&hmac->hash_ctx, ipad, block_size);

cleanup:
    mbedtls_platform_zeroize(ipad, key_length);

    return ret;
}

int hmac_update_internal(hmac_internal_data *hmac,
                                    const uint8_t *input,
                                    size_t input_length )
{
    return mbedtls_sha256_update_ret(&hmac->hash_ctx, input, input_length);
}

int hmac_finish_internal(hmac_internal_data *hmac,
                                              uint8_t *mac,
                                              size_t mac_size)
{
    unsigned char tmp[SHA256_HASH_SIZE];
    size_t hash_size = SHA256_HASH_SIZE;
    size_t block_size = SHA256_HASH_BLOCK_SIZE;
    int ret;

    ret = mbedtls_sha256_finish_ret(&hmac->hash_ctx, tmp);
    if (ret != 0)
        goto exit;
    /* From here on, tmp needs to be wiped. */

    mbedtls_sha256_init(&hmac->hash_ctx);
    ret = mbedtls_sha256_starts_ret(&hmac->hash_ctx, 0);
    if (ret != 0)
        goto exit;

    ret = mbedtls_sha256_update_ret(&hmac->hash_ctx, hmac->opad, block_size);
    if (ret != 0)
        goto exit;

    ret = mbedtls_sha256_update_ret(&hmac->hash_ctx, tmp, hash_size);
    if (ret != 0)
        goto exit;

    ret = mbedtls_sha256_finish_ret(&hmac->hash_ctx, tmp);
    if (ret != 0)
        goto exit;

    memcpy(mac, tmp, mac_size);

exit:
    mbedtls_sha256_free(&hmac->hash_ctx);
    mbedtls_platform_zeroize(tmp, hash_size);
    return ret;
}


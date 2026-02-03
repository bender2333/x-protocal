/*!
    \file    ibl_key_derive.c
    \brief   IBL key derive for GD32 SDK

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
#include "mbedtls/sha256.h"
#include "mbedtls/aes.h"
#include "mbedtls/pk.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "ibl_hmac.h"

/* Set up a TLS-1.2-prf-based generator (see RFC 5246, Section 5).
 *
 * Note that if this function fails, you must call psa_generator_abort()
 * to potentially free embedded data structures and wipe confidential data.
 */
static int key_derivation_tls12_prf_setup(
    tls12_prf_generator_t *tls12_prf,
    const unsigned char *key,
    size_t key_len,
    const uint8_t *salt,
    size_t salt_length,
    const uint8_t *label,
    size_t label_length )
{
    uint8_t hash_len = SHA256_HASH_SIZE;
    size_t Ai_with_seed_len = hash_len + salt_length + label_length;
    int overflow;
    int ret = 0;

    tls12_prf->key = mbedtls_calloc(1, key_len);
    if (tls12_prf->key == NULL) {
        ret = -1;
        goto exit;
    }
    tls12_prf->key_len = key_len;
    memcpy(tls12_prf->key, key, key_len);

    overflow = (salt_length + label_length < salt_length) ||
                (salt_length + label_length + hash_len < hash_len);
    if (overflow) {
        ret = -2;
        goto exit;
    }

    tls12_prf->Ai_with_seed = mbedtls_calloc(1, Ai_with_seed_len);
    if (tls12_prf->Ai_with_seed == NULL) {
        ret = -3;
        goto exit;
    }
    tls12_prf->Ai_with_seed_len = Ai_with_seed_len;

    /* Write `label + seed' at the end of the `A(i) + seed` buffer,
    * leaving the initial `hash_length` bytes unspecified for now. */
    if (label_length != 0) {
        memcpy(tls12_prf->Ai_with_seed + hash_len,
        label, label_length);
    }

    if (salt_length != 0) {
        memcpy(tls12_prf->Ai_with_seed + hash_len + label_length,
                salt, salt_length);
    }

    /* The first block gets generated when
    * psa_generator_read() is called. */
    tls12_prf->block_number    = 0;
    tls12_prf->offset_in_block = hash_len;

    return KD_SUCCESS;

exit:
    return ret;
}

static int key_derivation_tls12_prf_generate_next_block(
    tls12_prf_generator_t *tls12_prf)
{
    uint8_t hash_len = SHA256_HASH_SIZE;
    hmac_internal_data hmac;
    int ret, status = KD_SUCCESS;

    uint8_t *Ai;
    size_t Ai_len;

    /* We can't be wanting more output after block 0xff, otherwise
    * the capacity check in psa_generator_read() would have
    * prevented this call. It could happen only if the generator
    * object was corrupted or if this function is called directly
    * inside the library. */
    if (tls12_prf->block_number == 0xff)
        return -1;

    /* We need a new block */
    ++tls12_prf->block_number;
    tls12_prf->offset_in_block = 0;

    /* Recall the definition of the TLS-1.2-PRF from RFC 5246:
    *
    * PRF(secret, label, seed) = P_<hash>(secret, label + seed)
    *
    * P_hash(secret, seed) = HMAC_hash(secret, A(1) + seed) +
    *                        HMAC_hash(secret, A(2) + seed) +
    *                        HMAC_hash(secret, A(3) + seed) + ...
    *
    * A(0) = seed
    * A(i) = HMAC_hash( secret, A(i-1) )
    *
    * The `psa_tls12_prf_generator` structures saves the block
    * `HMAC_hash(secret, A(i) + seed)` from which the output
    * is currently extracted as `output_block`, while
    * `A(i) + seed` is stored in `Ai_with_seed`.
    *
    * Generating a new block means recalculating `Ai_with_seed`
    * from the A(i)-part of it, and afterwards recalculating
    * `output_block`.
    *
    * A(0) is computed at setup time.
    *
    */

    hmac_init_internal(&hmac);

    /* We must distinguish the calculation of A(1) from those
    * of A(2) and higher, because A(0)=seed has a different
    * length than the other A(i). */
    if (tls12_prf->block_number == 1) {
        Ai     = tls12_prf->Ai_with_seed + hash_len;
        Ai_len = tls12_prf->Ai_with_seed_len - hash_len;
    } else {
        Ai     = tls12_prf->Ai_with_seed;
        Ai_len = hash_len;
    }

    /* Compute A(i+1) = HMAC_hash(secret, A(i)) */
    ret = hmac_setup_internal(&hmac,
                                tls12_prf->key,
                                tls12_prf->key_len);
    if (ret != 0) {
        status = -2;
        goto cleanup;
    }

    ret = hmac_update_internal(&hmac, Ai, Ai_len);
    if (ret != 0) {
        status = -3;
        goto cleanup;
    }

    ret = hmac_finish_internal(&hmac,
                                tls12_prf->Ai_with_seed,
                                hash_len);
    if (ret != 0) {
        status = -4;
        goto cleanup;
    }

    /* Compute the next block `HMAC_hash(secret, A(i+1) + seed)`. */
    ret = hmac_setup_internal(&hmac,
                                tls12_prf->key,
                                tls12_prf->key_len);
    if (ret != 0) {
        status = -5;
        goto cleanup;
    }

    ret = hmac_update_internal(&hmac,
                                tls12_prf->Ai_with_seed,
                                tls12_prf->Ai_with_seed_len);
    if (ret != 0) {
        status = -6;
        goto cleanup;
    }

    ret = hmac_finish_internal(&hmac,
                                    tls12_prf->output_block,
                                    hash_len);
    if (ret != 0) {
        status = -7;
        goto cleanup;
    }

cleanup:
    hmac_abort_internal(&hmac);
    if (status != KD_SUCCESS)
        ibl_trace(IBL_ERR, "TLS12 prf generate error(%d,%d).\r\n", status, ret);

    return status;
}

static int key_derivation_tls12_prf_read(
                                        tls12_prf_generator_t *tls12_prf,
                                        uint8_t *output,
                                        size_t output_length )
{
    uint8_t hash_length = SHA256_HASH_SIZE;
    int status;
    uint8_t n;

    while (output_length != 0) {
        /* Copy what remains of the current block */
        n = hash_length - tls12_prf->offset_in_block;

        /* Check if we have fully processed the current block. */
        if (n == 0) {
            status = key_derivation_tls12_prf_generate_next_block(tls12_prf);
            if (status != KD_SUCCESS)
                return status;
            continue;
        }

        if (n > output_length)
            n = (uint8_t)output_length;

        memcpy(output, tls12_prf->output_block + tls12_prf->offset_in_block, n);
        output += n;
        output_length -= n;
        tls12_prf->offset_in_block += n;
    }

    return KD_SUCCESS;
}

static void key_derivation_tls12_prf_finish(tls12_prf_generator_t *tls12_prf)
{
    if (tls12_prf->key != NULL) {
        mbedtls_platform_zeroize(tls12_prf->key, tls12_prf->key_len);
        mbedtls_free(tls12_prf->key);
    }

    if (tls12_prf->Ai_with_seed != NULL) {
        mbedtls_platform_zeroize(tls12_prf->Ai_with_seed, tls12_prf->Ai_with_seed_len);
        mbedtls_free(tls12_prf->Ai_with_seed);
    }
    memset(tls12_prf, 0, sizeof(*tls12_prf));
}

static int key_derivation(IN uint8_t *secret, IN size_t secret_sz,
                                IN uint8_t *label, IN size_t label_sz,
                                OUT uint8_t *output, IN size_t output_length)
{
    tls12_prf_generator_t tls12_prf;
    int ret, status;

    memset(&tls12_prf, 0, sizeof(tls12_prf));

    ret = key_derivation_tls12_prf_setup(&tls12_prf,
                                    secret, secret_sz,
                                    DERIVE_KEY_SALT, DERIVE_KEY_SALT_LENGTH,
                                    label, label_sz);
    if (ret != KD_SUCCESS) {
        status = -1;
        goto exit;
    }

    ret = key_derivation_tls12_prf_read(&tls12_prf,
                                    output, output_length);
    if (ret != KD_SUCCESS) {
        status = -2;
        goto exit;
    }

    key_derivation_tls12_prf_finish(&tls12_prf);
    return KD_SUCCESS;

exit:
    key_derivation_tls12_prf_finish(&tls12_prf);
    ibl_trace(IBL_ERR, "Key derive error(%d,%d)\r\n", status, ret);
    return status;
}

int derive_sys_status_crypt_key(OUT uint8_t *key, IN size_t key_sz)
{
#define LABLE_SYS_STATUS    ((uint8_t *)"lable_sys_status")
#define LABLE_SYS_STATUS_LENGTH (strlen((const char*) LABLE_SYS_STATUS))
    uint8_t huk_hash[SHA256_HASH_SIZE];
    uint8_t output[SHA256_HASH_SIZE];
    int ret, i;

    if (NULL == key || (key_sz > SHA256_HASH_SIZE))
        return -1;

#if defined(PLATFORM_GD32F5XX) || defined(PLATFORM_GD32H7XX) || defined(PLATFORM_GD32G5X3)
    efuse_get_huk_inner(huk_hash);
    ret = key_derivation(huk_hash, EFUSE_HUK_SZ,
                            LABLE_SYS_STATUS, LABLE_SYS_STATUS_LENGTH,
                            output, SHA256_HASH_SIZE);
#else /* PLATFORM_GD32F5XX */
    efuse_get_huk_inner(huk_hash);
    ret = key_derivation(huk_hash, EFUSE_HUK_SZ,
                            LABLE_SYS_STATUS, LABLE_SYS_STATUS_LENGTH,
                            output, SHA256_HASH_SIZE);
#endif
    if (ret != KD_SUCCESS) {
        return -2;
    }

    memcpy(key, output, key_sz);

    ibl_memset(huk_hash, 0, SHA256_HASH_SIZE);
    ibl_memset(output, 0, SHA256_HASH_SIZE);
    return 0;
}

#if (IBL_VERSION >= V_1_1)
int do_symm_key_derive_inner(        IN uint8_t *label, IN size_t label_sz,
                                OUT uint8_t *key, IN size_t key_len)
{
////    uint8_t huk_hash[SHA256_HASH_SIZE];
////    uint8_t output[SHA256_HASH_SIZE];
////    int ret, i;

////    if (NULL == key || (key_len > SHA256_HASH_SIZE))
////        return -1;

////#if defined(ROM_SELF_TEST)
////    efuse_get_huk_inner(huk_hash);
////    HASH_SHA256(huk_hash, EFUSE_HUK_SZ, huk_hash);
////#else
////    HASH_SHA256((uint8_t *)(EFUSE_REG_HUK), EFUSE_HUK_SZ, huk_hash);
////#endif
////    //ibl_print_data(IBL_ALWAYS, "HUK hash:", huk_hash, SHA256_HASH_SIZE);

////    ret = key_derivation(huk_hash, SHA256_HASH_SIZE,
////                            label, label_sz,
////                            output, SHA256_HASH_SIZE);
////    if (ret != KD_SUCCESS) {
////        return -2;
////    }

////    memcpy(key, output, key_len);

////    ibl_memset(huk_hash, 0, SHA256_HASH_SIZE);
////    ibl_memset(output, 0, SHA256_HASH_SIZE);
    return 0;
}
#endif

#ifdef ROM_SELF_TEST
static int key_derive_crypt_test(IN uint8_t mode,
                        IN uint8_t *input,
                        IN size_t length,
                        OUT uint8_t *output)
{
    mbedtls_aes_context ctx;
    uint8_t key[AES_KEY_SZ];
#ifdef USE_AES_CBC
    uint8_t iv[AES_KEY_SZ];
#endif
    int keybits = AES_KEY_SZ * BITS_PER_BYTE;
    uint8_t *buf = output;
    int ret = 0;

    if (length % AES_BLOCK_SZ != 0)
        return -1;

    ret = derive_sys_status_crypt_key(key, AES_KEY_SZ);
    if (ret != KD_SUCCESS)
        return -2;

    mbedtls_aes_init(&ctx);

#ifdef USE_AES_CBC
    memset(iv , 0x5A, AES_KEY_SZ);
#endif
    if (mode == MBEDTLS_AES_ENCRYPT) {
        mbedtls_aes_setkey_enc(&ctx, key, keybits);
    } else {
        mbedtls_aes_setkey_dec(&ctx, key, keybits);
    }

#ifdef USE_AES_CBC
    ret = mbedtls_aes_crypt_cbc(&ctx, mode, length, iv, input, output);
#else
    while (length > 0) {
        ret = mbedtls_aes_crypt_ecb(&ctx, mode, input, buf);
        if (ret != 0)
            break;
        input += AES_BLOCK_SZ;
        buf += AES_BLOCK_SZ;
        length -= AES_BLOCK_SZ;
    }
#endif
exit:
    mbedtls_aes_free(&ctx);
    return ret;
}

#endif  // ROM_SELF_TEST


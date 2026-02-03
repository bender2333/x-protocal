/**
 *  DES block cipher implemented by Freethink
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_DES_C)

#include "mbedtls/des.h"

#ifndef MBEDTLS_ROM_TEST
#include <string.h>
#endif


#if defined(MBEDTLS_DES_ALT)
#include "Platform/gd32xx.h"

/* Implementation that should never be optimized out by the compiler */
static void mbedtls_zeroize( void *v, size_t n ) {
    volatile unsigned char *p = (unsigned char*)v; while( n-- ) *p++ = 0;
}

void mbedtls_des_init( mbedtls_des_context *ctx )
{
    memset( ctx, 0, sizeof( mbedtls_des_context ) );
}

void mbedtls_des_free( mbedtls_des_context *ctx )
{
    if( ctx == NULL )
        return;

    mbedtls_zeroize( ctx, sizeof( mbedtls_des_context ) );
}

void mbedtls_des3_init( mbedtls_des3_context *ctx )
{
    memset( ctx, 0, sizeof( mbedtls_des3_context ) );
}

void mbedtls_des3_free( mbedtls_des3_context *ctx )
{
    if( ctx == NULL )
        return;

    mbedtls_zeroize( ctx, sizeof( mbedtls_des3_context ) );
}
static const unsigned char odd_parity_table[128] = { 1,  2,  4,  7,  8,
        11, 13, 14, 16, 19, 21, 22, 25, 26, 28, 31, 32, 35, 37, 38, 41, 42, 44,
        47, 49, 50, 52, 55, 56, 59, 61, 62, 64, 67, 69, 70, 73, 74, 76, 79, 81,
        82, 84, 87, 88, 91, 93, 94, 97, 98, 100, 103, 104, 107, 109, 110, 112,
        115, 117, 118, 121, 122, 124, 127, 128, 131, 133, 134, 137, 138, 140,
        143, 145, 146, 148, 151, 152, 155, 157, 158, 161, 162, 164, 167, 168,
        171, 173, 174, 176, 179, 181, 182, 185, 186, 188, 191, 193, 194, 196,
        199, 200, 203, 205, 206, 208, 211, 213, 214, 217, 218, 220, 223, 224,
        227, 229, 230, 233, 234, 236, 239, 241, 242, 244, 247, 248, 251, 253,
        254 };

void mbedtls_des_key_set_parity( unsigned char key[MBEDTLS_DES_KEY_SIZE] )
{
    int i;

    for( i = 0; i < MBEDTLS_DES_KEY_SIZE; i++ )
        key[i] = odd_parity_table[key[i] / 2];
}

/*
 * DES key schedule (56-bit, encryption)
 */
int mbedtls_des_setkey_enc( mbedtls_des_context *ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE] )
{
    ctx->mode = MBEDTLS_DES_ENCRYPT;
    memcpy(ctx->key, key, MBEDTLS_DES_KEY_SIZE);

    return( 0 );
}

/*
 * DES key schedule (56-bit, decryption)
 */
int mbedtls_des_setkey_dec( mbedtls_des_context *ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE] )
{
    ctx->mode = MBEDTLS_DES_DECRYPT;
    memcpy(ctx->key, key, MBEDTLS_DES_KEY_SIZE);

    return( 0 );
}

/*
 * Triple-DES key schedule (112-bit, encryption)
 */
int mbedtls_des3_set2key_enc( mbedtls_des3_context *ctx,
                      const unsigned char key[MBEDTLS_DES_KEY_SIZE * 2] )
{
    ctx->mode = MBEDTLS_DES_ENCRYPT;
    memcpy(ctx->key, key, (MBEDTLS_DES_KEY_SIZE * 2));
    memcpy(&(ctx->key[16]), key, MBEDTLS_DES_KEY_SIZE);

    return( 0 );
}

/*
 * Triple-DES key schedule (112-bit, decryption)
 */
int mbedtls_des3_set2key_dec( mbedtls_des3_context *ctx,
                      const unsigned char key[MBEDTLS_DES_KEY_SIZE * 2] )
{
    ctx->mode = MBEDTLS_DES_DECRYPT;
    memcpy(ctx->key, key, (MBEDTLS_DES_KEY_SIZE * 2));
    memcpy(&(ctx->key[16]), key, MBEDTLS_DES_KEY_SIZE);

    return( 0 );
}

/*
 * Triple-DES key schedule (168-bit, encryption)
 */
int mbedtls_des3_set3key_enc( mbedtls_des3_context *ctx,
                      const unsigned char key[MBEDTLS_DES_KEY_SIZE * 3] )
{
    ctx->mode = MBEDTLS_DES_ENCRYPT;
    memcpy(ctx->key, key, (MBEDTLS_DES_KEY_SIZE * 3));

    return( 0 );
}

/*
 * Triple-DES key schedule (168-bit, decryption)
 */
int mbedtls_des3_set3key_dec( mbedtls_des3_context *ctx,
                      const unsigned char key[MBEDTLS_DES_KEY_SIZE * 3] )
{
    ctx->mode = MBEDTLS_DES_DECRYPT;
    memcpy(ctx->key, key, (MBEDTLS_DES_KEY_SIZE * 3));

    return( 0 );
}

/*
 * DES-ECB block encryption/decryption
 */
int mbedtls_des_crypt_ecb( mbedtls_des_context *ctx,
                    const unsigned char input[8],
                    unsigned char output[8] )
{
    ErrStatus ret;
    cau_parameter_struct cau_parameter;
    if (MBEDTLS_DES_ENCRYPT == ctx->mode) {
        cau_parameter.alg_dir = CAU_ENCRYPT;
    } else {
        cau_parameter.alg_dir = CAU_DECRYPT;
    }
    cau_parameter.key = ctx->key;
    cau_parameter.input = (unsigned char*)input;
    cau_parameter.in_length = 8;
    
    ibl_get_mutex(MUTEX_TYPE_CRYP);
    ret = cau_des_ecb(&cau_parameter, output);
    ibl_put_mutex(MUTEX_TYPE_CRYP);

    return (ret == ERROR) ? 1 : 0;
}

#if defined(MBEDTLS_CIPHER_MODE_CBC)
/*
 * DES-CBC buffer encryption/decryption
 */
int mbedtls_des_crypt_cbc( mbedtls_des_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[8],
                    const unsigned char *input,
                    unsigned char *output )
{
    ErrStatus ret;
    unsigned char temp[8];
    cau_parameter_struct cau_parameter;
    if (MBEDTLS_DES_ENCRYPT == ctx->mode) {
        cau_parameter.alg_dir = CAU_ENCRYPT;
    } else {
        cau_parameter.alg_dir = CAU_DECRYPT;
    }
    cau_parameter.key = ctx->key;
    cau_parameter.iv = iv;
    cau_parameter.input = (unsigned char*)input;
    cau_parameter.in_length = length;

    memcpy(temp, (input + length - 8), 8);
    ibl_get_mutex(MUTEX_TYPE_CRYP);
    ret = cau_des_cbc(&cau_parameter, output);
    if(mode == MBEDTLS_DES_DECRYPT)
        memcpy(iv, temp, 8);
    else
        memcpy(iv, (output + length - 8), 8);
    ibl_put_mutex(MUTEX_TYPE_CRYP);

    return (ret == ERROR) ? 1 : 0;
}
#endif /* MBEDTLS_CIPHER_MODE_CBC */

/*
 * 3DES-ECB block encryption/decryption
 */
int mbedtls_des3_crypt_ecb( mbedtls_des3_context *ctx,
                     const unsigned char input[8],
                     unsigned char output[8] )
{
    cau_parameter_struct cau_parameter;
    if (MBEDTLS_DES_ENCRYPT == ctx->mode) {
        cau_parameter.alg_dir = CAU_ENCRYPT;
    } else {
        cau_parameter.alg_dir = CAU_DECRYPT;
    }
    cau_parameter.key = ctx->key;
    cau_parameter.input = (unsigned char*)input;
    cau_parameter.in_length = 8;
    
    ibl_get_mutex(MUTEX_TYPE_CRYP);
    cau_tdes_ecb(&cau_parameter, output);
    ibl_put_mutex(MUTEX_TYPE_CRYP);

    return( 0 );
}

#if defined(MBEDTLS_CIPHER_MODE_CBC)
/*
 * 3DES-CBC buffer encryption/decryption
 */
int mbedtls_des3_crypt_cbc( mbedtls_des3_context *ctx,
                     int mode,
                     size_t length,
                     unsigned char iv[8],
                     const unsigned char *input,
                     unsigned char *output )
{
    unsigned char temp[8];
    cau_parameter_struct cau_parameter;
    if (MBEDTLS_DES_ENCRYPT == mode) {
        cau_parameter.alg_dir = CAU_ENCRYPT;
    } else {
        cau_parameter.alg_dir = CAU_DECRYPT;
    }
    cau_parameter.key = ctx->key;
    cau_parameter.iv = iv;
    cau_parameter.input = (unsigned char*)input;
    cau_parameter.in_length = length;

    memcpy(temp, (input + length - 8), 8);
    ibl_get_mutex(MUTEX_TYPE_CRYP);
    cau_tdes_cbc(&cau_parameter, output);
    if(mode == MBEDTLS_DES_DECRYPT)
        memcpy(iv, temp, 8);
    else
        memcpy(iv, (output + length - 8), 8);
    ibl_put_mutex(MUTEX_TYPE_CRYP);

    return( 0 );
}
#endif /* MBEDTLS_CIPHER_MODE_CBC */

#endif /* MBEDTLS_DES_ALT */

#endif /* MBEDTLS_DES_C */


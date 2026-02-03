/*!
    \file    ibl_image_validate.c
    \brief   IBL image validate for GD32 SDK

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
#include "mbedtls/sha256.h"

#include "mbedtls/md.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/rsa.h"
#include "mbedtls/platform.h"

extern uint32_t img_hdr_size;

#if SIGN_ALGO_SET == IMG_SIG_ED25519
extern int ED25519_verify(const uint8_t *message, size_t message_len,
                   const uint8_t signature[IMG_SIG_LEN], const uint8_t public_key[IMG_PK_LEN]);
#endif /* SIGN_ALGO_SET */

uint32_t cal_checksum(uint8_t *ptr, uint32_t sz)
{
    uint32_t chksum = 0;
    int i;

    /* should be 4 bytes align */
    sz = ((sz >> 2) << 2);

    for (i = 0; i < sz; i += 4) {
        chksum ^= *(uint32_t *)(ptr + i);
    }

    return chksum;
}

#if SIGN_ALGO_SET == IMG_SIG_ECDSA256
#if defined(MBEDTLS_ECDSA_C)
int secp256r1_verify(const uint8_t *hash, size_t h_size, const uint8_t *sig, const uint8_t *pk)
{
    int ret = 0;
    int res = 0;//OK
    mbedtls_ecp_group grp;
    mbedtls_mpi inner_r, inner_s;
    mbedtls_ecp_point inner_Q;

    /* initialize and load inner r and inner s */
    mbedtls_mpi_init(&inner_r);
    mbedtls_mpi_init(&inner_s);
    mbedtls_ecp_group_init( &grp );
    mbedtls_ecp_point_init( &inner_Q);
    
    mbedtls_mpi_read_binary(&inner_r, (const unsigned char *)(sig), 32);
    mbedtls_mpi_read_binary(&inner_s, (const unsigned char *)(sig+32), 32);
    
    mbedtls_mpi_read_binary(&(inner_Q.X), (const unsigned char *)(pk), 32);
    mbedtls_mpi_read_binary(&(inner_Q.Y), (const unsigned char *)(pk+32), 32);
    mbedtls_mpi_lset( &(inner_Q.Z), 1 );
   
    /* load curve group */
    ret = mbedtls_ecp_group_load( &grp, MBEDTLS_ECP_DP_SECP256R1 );
    
    if(0 != ret){
        res = -1; //SBSTI_LIC_LL_CAL_ERR;
    }else{
        /* ecdsa verify */
        ret = mbedtls_ecdsa_verify(&grp, hash, h_size, &inner_Q, &inner_r, &inner_s);

        if(MBEDTLS_ERR_ECP_VERIFY_FAILED == ret){
            res = -2; //SBSTI_LIC_CHECK_ERR; 
        }else if(0 != ret){
            res = -1; //SBSTI_LIC_LL_CAL_ERR;
        }else{
            /* empty */
        }
    }
    
    /* free space */
    mbedtls_mpi_free(&inner_r);
    mbedtls_mpi_free(&inner_s);
    mbedtls_ecp_point_free(&inner_Q);
    mbedtls_ecp_group_free(&grp);

    return res;
}
#else
/* curve modulus p */
static const uint8_t secp256r1_p[]  = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
                                            };
/* coefficient a */
static const uint8_t secp256r1_a[]  = { 0x4B, 0x60, 0xD2, 0x27, 0x3E, 0x3C, 0xCE, 0x3B,
                                        0xF6, 0xB0, 0x53, 0xCC, 0xB0, 0x06, 0x1D, 0x65,
                                        0xBC, 0x86, 0x98, 0x76, 0x55, 0xBD, 0xEB, 0xB3,
                                        0xE7, 0x93, 0x3A, 0xAA, 0xD8, 0x35, 0xC6, 0x5A
                                       };
/* base point x */
static const uint8_t secp256r1_gx[] = { 0x96, 0xC2, 0x98, 0xD8, 0x45, 0x39, 0xA1, 0xF4,
                                        0xA0, 0x33, 0xEB, 0x2D, 0x81, 0x7D, 0x03, 0x77,
                                        0xF2, 0x40, 0xA4, 0x63, 0xE5, 0xE6, 0xBC, 0xF8,
                                        0x47, 0x42, 0x2C, 0xE1, 0xF2, 0xD1, 0x17, 0x6B,
                                       };
/* base point y */
static const uint8_t secp256r1_gy[] = { 0xF5, 0x51, 0xBF, 0x37, 0x68, 0x40, 0xB6, 0xCB,
                                        0xCE, 0x5E, 0x31, 0x6B, 0x57, 0x33, 0xCE, 0x2B,
                                        0x16, 0x9E, 0x0F, 0x7C, 0x4A, 0xEB, 0xE7, 0x8E,
                                        0x9B, 0x7F, 0x1A, 0xFE, 0xE2, 0x42, 0xE3, 0x4F,
                                       };
/* order n */
static const uint8_t secp256r1_n[]  = { 0x51, 0x25, 0x63, 0xFC, 0xC2, 0xCA, 0xB9, 0xF3,
                                        0x84, 0x9E, 0x17, 0xA7, 0xAD, 0xFA, 0xE6, 0xBC,
                                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
                                      };

void exchange_byte(uint8_t *data, uint8_t data_len)
{
    uint8_t i = 0, data_temp;
    for(i=0; i < data_len/2; i++) {
        data_temp = data[i];
        data[i] = data[data_len-i-1];
        data[data_len-i-1] = data_temp;
    }
}
#ifndef PLATFORM_GD32G5X3
int pkcau_secp256r1_verify(const uint8_t *hash, size_t h_size, const uint8_t *sig, const uint8_t *pk)
{
    uint8_t verify_res = 1;
    uint8_t ecc_verify_x[32] = {0}, ecc_verify_y[32] = {0};
    uint8_t ecc_verify_r[32] = {0}, ecc_verify_s[32] = {0};
    uint8_t ecc_verify_hash[32] = {0};
    uint8_t a = 0x03;
    /* ECC curve parameter structure */
    pkcau_ec_group_parameter_struct pkcau_curve_group;
    /* hash parameter structure */
    pkcau_hash_parameter_struct pkcau_hash_parameter;
    /* signature parameter structure */
    pkcau_signature_parameter_struct pkcau_signature_parameter;
    /* point parameter structure */
    pkcau_point_parameter_struct pkcau_point_parameter;

    /* initialize the ECC curve parameter, hash parameter, point parameter and signature parameter structure */
    pkcau_ec_group_struct_para_init(&pkcau_curve_group);
    pkcau_hash_struct_para_init(&pkcau_hash_parameter);
    pkcau_point_struct_para_init(&pkcau_point_parameter);
    pkcau_signature_struct_para_init(&pkcau_signature_parameter);
    
    memcpy(ecc_verify_r, sig, 32);
    memcpy(ecc_verify_s, sig+32, 32);
    exchange_byte(ecc_verify_r, 32);
    exchange_byte(ecc_verify_s, 32);

    /* initialize the input ECC signature parameters */
    pkcau_signature_parameter.sign_r     = (uint8_t *)ecc_verify_r;
    pkcau_signature_parameter.sign_r_len = 32;
    pkcau_signature_parameter.sign_s     = (uint8_t *)(ecc_verify_s);
    pkcau_signature_parameter.sign_s_len = 32;

    memcpy(ecc_verify_x, pk, 32);
    memcpy(ecc_verify_y, pk+32, 32);
    exchange_byte(ecc_verify_x, 32);
    exchange_byte(ecc_verify_y, 32);
    /* initialize the input point parameters */
    pkcau_point_parameter.point_x     = ecc_verify_x;
    pkcau_point_parameter.point_x_len = 32;
    pkcau_point_parameter.point_y     = ecc_verify_y;
    pkcau_point_parameter.point_y_len = 32;


    /* initialize the input ECC curve parameters */
    pkcau_curve_group.modulus_p        = (uint8_t *)secp256r1_p;
    pkcau_curve_group.modulus_p_len    = sizeof(secp256r1_p);
    pkcau_curve_group.coff_a           = &a;
    pkcau_curve_group.coff_a_len       = 1;
    pkcau_curve_group.coff_b           = (uint8_t *)secp256r1_a;
    pkcau_curve_group.coff_b_len       = sizeof(secp256r1_a);
    pkcau_curve_group.a_sign           = 1;
    pkcau_curve_group.base_point_x     = (uint8_t *)secp256r1_gx;
    pkcau_curve_group.base_point_x_len = sizeof(secp256r1_gx);
    pkcau_curve_group.base_point_y     = (uint8_t *)secp256r1_gy;
    pkcau_curve_group.base_point_y_len = sizeof(secp256r1_gy);
    pkcau_curve_group.order_n          = (uint8_t *)secp256r1_n,
    pkcau_curve_group.order_n_len      = sizeof(secp256r1_n);

    memcpy(ecc_verify_hash, hash, 32);
    exchange_byte(ecc_verify_hash, 32);
    /* initialize the input hash parameters */
    pkcau_hash_parameter.hash_z     = (uint8_t *)ecc_verify_hash;
    pkcau_hash_parameter.hash_z_len = h_size;

    /* execute ECDSA verification operation */
    verify_res = pkcau_ecdsa_verification_operation(&pkcau_point_parameter, &pkcau_hash_parameter, &pkcau_signature_parameter, &pkcau_curve_group);
    return verify_res;
}
#endif /* PLATFORM_GD32G5X3 */
#endif /* MBEDTLS_ECDSA_C */
#elif SIGN_ALGO_SET == IMG_SIG_RSA2048
#define RSA_MOD_N_LENGTH       2048            /* RSA module number N */
#define PADDING_STYLE          MBEDTLS_RSA_PKCS_V21
const uint8_t rsa_e[] = {0x00, 0x01, 0x00, 0x01};
int rsa2048_verify(const uint8_t *hash, size_t h_size, const uint8_t *sig, const uint8_t *pk)
{
    int ret = 0;
    const char *pers = "simple_rsa_sign";

    mbedtls_rsa_context ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    mbedtls_rsa_init(&ctx, PADDING_STYLE, MBEDTLS_MD_SHA256);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, 
                                (const uint8_t *) pers, strlen((const char *)pers));
    if(ret != 0) {
        goto cleanup;
    }

    mbedtls_mpi_read_binary(&(ctx.N), (const uint8_t *)(pk), IMG_PK_LEN);
    mbedtls_mpi_read_binary(&(ctx.E), (const uint8_t *)(rsa_e), sizeof(rsa_e));

    ctx.len = mbedtls_mpi_size(&(ctx.N));

    ret = mbedtls_rsa_pkcs1_verify(&ctx, mbedtls_ctr_drbg_random, &ctr_drbg, 
                                        MBEDTLS_RSA_PUBLIC, MBEDTLS_MD_SHA256, 
                                        h_size, hash, sig);
    if(ret != 0) {
        goto cleanup;
    }

cleanup:
    mbedtls_rsa_free(&ctx);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return ret;
}
#endif /* SIGN_ALGO_SET */

int img_verify_sign(IN uint8_t *hash,
                        IN uint32_t hlen,
                        IN uint8_t *pk,
                        IN uint8_t *sig)
{
    int rc = -1;

#if SIGN_ALGO_SET == IMG_SIG_ED25519
    rc = ED25519_verify(hash, hlen, sig, pk);
    if (rc == 0) {
        return -1;
    }
#elif SIGN_ALGO_SET == IMG_SIG_ECDSA256
#if defined(MBEDTLS_ECDSA_C)
    rc = secp256r1_verify(hash, hlen, sig, pk);
    if (rc != 0) {
        return -1;
    }
#else
    rc = pkcau_secp256r1_verify(hash, hlen, sig, pk);
    if (rc != 0) {
        return -1;
    }
#endif /* PLATFORM_GD32F5XX */
#elif SIGN_ALGO_SET == IMG_SIG_RSA2048
    rc = rsa2048_verify(hash, hlen, sig, pk);
    if (rc != 0) {
        return -1;
    }
#endif /* SIGN_ALGO_SET */

    return 0;
}

/*
 * Compute SHA256 over the image (Image Header + Image) and compare with the input hash.
 */
int img_verify_hash(IN uint32_t faddr,  /* Flash Adress */
                        IN uint32_t len,
                        IN uint8_t *hash,
                        IN uint32_t hlen,
                        IN uint8_t *seed,
                        IN int seed_len)
{
#define TEMP_BUF_SZ        256
    uint8_t hash_result[IMG_HASH_LEN];

#ifdef CONFIG_HW_SECURITY_ENGINE
    HASH_SHA256((unsigned char *)(FLASH_BASE + faddr), len, hash_result);
#else
    mbedtls_sha256_context sha256_ctx;
    uint8_t tmp_buf[TEMP_BUF_SZ];
    uint32_t tmp_buf_sz = TEMP_BUF_SZ, blk_sz, off;
    int ret;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0);

    /* in some cases (split image) the hash is seeded with data from
    * the loader image */
    if (seed && (seed_len > 0)) {
        mbedtls_sha256_update(&sha256_ctx, seed, seed_len);
    }

    for (off = 0; off < len; off += blk_sz) {
        blk_sz = len - off;
        if (blk_sz > tmp_buf_sz) {
            blk_sz = tmp_buf_sz;
        }
        ret = flash_read((faddr + off), tmp_buf, blk_sz);
        if (ret != 0) {
            return -1;
        }
        mbedtls_sha256_update(&sha256_ctx, tmp_buf, blk_sz);
    }

    mbedtls_sha256_finish(&sha256_ctx, hash_result);
#endif
    if (memcmp(hash, hash_result, hlen)) {
        ibl_print_data(IBL_INFO, "Hash:", hash, hlen);
        ibl_print_data(IBL_INFO, "Hash calculated:", hash_result, IMG_HASH_LEN);
        return -2;
    }
    return 0;
}

int img_verify_hdr(IN struct image_header *hdr,
                    IN uint8_t img_type)
{
    uint32_t ver_nv, ver_img;
    uint32_t chksum;
    int ret = 0;

    if (hdr->ih_magic != IMG_MAGIC_H) {
        return -1;
    }

    if ((hdr->ih_ver.iv_major > MAX_VER_MAJOR) || (hdr->ih_ver.iv_minor > MAX_VER_MINOR)) {
        return -2;
    }

    ver_img =  (hdr->ih_ver.iv_major << 24) | (hdr->ih_ver.iv_minor << 16) | hdr->ih_ver.iv_revision;
    ret = sys_status_get_internal(SYS_MBL_VER_COUNTER, LEN_SYS_VER_COUNTER, (uint8_t *)&ver_nv);
    ibl_trace(IBL_INFO, "MBL version: %d.%d.%d, Local: %d.%d.%d\r\n",
                    hdr->ih_ver.iv_major, hdr->ih_ver.iv_minor, hdr->ih_ver.iv_revision,
                    ((ver_nv >> 24) & 0xff), ((ver_nv >> 16) & 0xff), (ver_nv & 0xffff));
    if (ret != 0) {
        return -3;
    }
    if (ver_img < ver_nv) {
        ibl_trace(IBL_ERR, "The image version too low(0x%08x < 0x%08x).\r\n", ver_img, ver_nv);
        return -8;
    }

    return 0;
}

int img_verify_pkhash(IN uint8_t *pk,
                IN uint32_t klen,
                IN uint8_t *pkhash,
                IN uint32_t hlen)
{
    mbedtls_sha256_context ctx;
    uint8_t hash_result[IMG_PKHASH_LEN];

#ifdef CONFIG_HW_SECURITY_ENGINE
    hau_hash_sha_256((unsigned char *)pk, klen, hash_result);
#else
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, pk, klen);
    mbedtls_sha256_finish(&ctx, hash_result);
#endif

    if (memcmp(pkhash, hash_result, hlen)) {
        ibl_print_data(IBL_INFO, "Hash:", pkhash, hlen);
        ibl_print_data(IBL_INFO, "Hash calculated:", hash_result, IMG_PKHASH_LEN);
        return -1;
    }

    return 0;
}

/*
 * Verify the integrity of the image.
 * Return non-zero if image could not be validated/does not validate.
 */
int img_validate(IN uint32_t img_offset,
                   IN uint8_t img_type,
                   IN uint8_t *pk,
                   OUT void *img_info)
{
    struct image_header hdr;
    struct image_tlv_info info;
    struct image_tlv tlv;
    struct sw_info_t *out_img_info = (struct sw_info_t *)img_info;
    uint8_t hash[IMG_HASH_LEN];
    uint8_t pkhash[IMG_PKHASH_LEN];
    uint8_t sig[IMG_SIG_LEN];
    uint32_t slen;
    uint32_t off = 0, addr;
    int sha256_valid = 0;
    int valid_signature = 0;
    int pk_valid = 0;
    int ret;
#ifdef BOOT_RECORD
    uint8_t buf[MAX_BOOT_RECORD_LEN];
    int record_len = 0;
    int boot_record_found = 0;
#endif

    ret = flash_read(img_offset, (void*)&hdr, sizeof(hdr));
    if (ret != 0) {
        goto Read_Flash_Fail;
    }
    ret = img_verify_hdr(&hdr, img_type);
    if (ret != 0) {
        ibl_trace(IBL_ERR, "Bad image header(%d).\r\n", ret);
        return IMG_ERR_BAD_HEADER;
    }
    img_hdr_size = hdr.ih_hdr_size;
    addr = img_offset + hdr.ih_hdr_size + hdr.ih_img_size;
    ret = flash_read(addr, (void*)&info, sizeof(info));
    if (ret != 0) {
        goto Read_Flash_Fail;
    }

    if (info.magic_tlv != IMG_MAGIC_PTLV) {
        return IMG_ERR_BAD_PTLV;
    }
    off += sizeof(info);
    addr += off;

    while (off < info.tlv_sz) {
        ret = flash_read(addr, (void*)&tlv, sizeof(tlv));
        if (ret != 0) {
            goto Read_Flash_Fail;
        }
        addr += sizeof(tlv);
        if (tlv.type == IMG_TLV_SHA256_DIGEST) {
            /*
            * Verify the SHA256 image hash.  This must always be
            * present.
            */
            if (tlv.len != IMG_HASH_LEN) {
                return IMG_ERR_BAD_HASH;
            }
            ret = flash_read(addr, (void*)hash, IMG_HASH_LEN);
            if (ret != 0) {
                goto Read_Flash_Fail;
            }
            if (img_verify_hash(img_offset, (hdr.ih_hdr_size + hdr.ih_img_size), hash, IMG_HASH_LEN, NULL, 0)) {
                return IMG_ERR_BAD_HASH;
            }

            sha256_valid = 1;
        } else if (tlv.type == IMG_TLV_PKHASH) {
            /*
            * Check Public Key HASH.
            */
            ret = flash_read(addr, (void*)pkhash, IMG_PKHASH_LEN);
            if (ret != 0) {
                goto Read_Flash_Fail;
            }
            if (img_verify_pkhash(pk, IMG_PK_LEN, pkhash, IMG_PKHASH_LEN)) {
                return IMG_ERR_BAD_PKHASH;
            }
            pk_valid = 1;
        }
#if SIGN_ALGO_SET == IMG_SIG_ED25519
        else if (tlv.type == IMG_TLV_ED25519_SIG) {
            /* Ignore this signature if it is out of bounds. */
            if ((tlv.len != ED25519_SIG_LEN)) {
                return IMG_ERR_BAD_SIG;
            }
            if (pk_valid == 0) {
                return IMG_ERR_BAD_PKHASH;
            }
            slen = tlv.len;
            ret = flash_read(addr, (void*)sig, slen);
            if (ret != 0) {
                goto Read_Flash_Fail;
            }
            if (img_verify_sign(hash, IMG_HASH_LEN, pk, sig)) {
                return IMG_ERR_BAD_SIG;
            }
            valid_signature = 1;
        }
#elif SIGN_ALGO_SET == IMG_SIG_ECDSA256
        else if (tlv.type == IMG_TLV_ECDSA256_SIG) {
            /* Ignore this signature if it is out of bounds. */
            if ((tlv.len != IMG_SIG_LEN)) {
                return IMG_ERR_BAD_SIG;
            }
            if (pk_valid == 0) {
                return IMG_ERR_BAD_PKHASH;
            }
            slen = tlv.len;
            ret = flash_read(addr, (void*)sig, slen);
            if (ret != 0) {
                goto Read_Flash_Fail;
            }
            if (img_verify_sign(hash, IMG_HASH_LEN, pk, sig)) {
                return IMG_ERR_BAD_SIG;
            }
            valid_signature = 1;
        }
#elif SIGN_ALGO_SET == IMG_SIG_RSA2048
        else if (tlv.type == IMG_TLV_RSA2048_PSS) {
            /* Ignore this signature if it is out of bounds. */
            if ((tlv.len != IMG_SIG_LEN)) {
                return IMG_ERR_BAD_SIG;
            }
            if (pk_valid == 0) {
                return IMG_ERR_BAD_PKHASH;
            }
            slen = tlv.len;
            ret = flash_read(addr, (void*)sig, slen);
            if (ret != 0) {
                goto Read_Flash_Fail;
            }
            if (img_verify_sign(hash, IMG_HASH_LEN, pk, sig)) {
                return IMG_ERR_BAD_SIG;
            }
            valid_signature = 1;
        }
#endif /* SIGN_ALGO_SET */

#ifdef BOOT_RECORD
        else if (tlv.type == IMG_TLV_BOOT_RECORD) {
            if (tlv.len > sizeof(buf)) {
                    return IMG_ERR_BAD_RECORD;
            }
            record_len = tlv.len;
            ret = flash_read(addr, buf, record_len);
            if (ret) {
                goto Read_Flash_Fail;
            }
            boot_record_found = 1;
        }
#endif
        addr += tlv.len;
        off += sizeof(tlv) + tlv.len;
    }

    if (!sha256_valid || !valid_signature
#ifdef BOOT_RECORD
        || !boot_record_found
#endif
    ) {
        return IMG_ERR_MISSING_TLV;
    }

    if (out_img_info) {
        out_img_info->type = img_type;
        out_img_info->version = (hdr.ih_ver.iv_major << 24) | (hdr.ih_ver.iv_minor << 16) | hdr.ih_ver.iv_revision;
#ifdef BOOT_RECORD
        /* Update the measurement value (hash of the image) data item in the
        * boot record. It is always the last item in the structure to make
        * it easy to calculate its position.
        * The image hash is computed over the image header, the image itself and
        * the protected TLV area (which should already include the image hash as
        * part of the boot record TLV). For this reason this field has been
        * filled with zeros during the image signing process.
        */
        off = record_len - sizeof(hash);
        if ((off + sizeof(hash)) > sizeof(buf)) {
            return IMG_ERR_BAD_RECORD;
        }
        memcpy(buf + off, hash, sizeof(hash));

        out_img_info->record_len = record_len;
        memcpy(out_img_info->record, buf, record_len);
#else
        memcpy(out_img_info->signer_id, pk, IMG_PK_LEN);
        memcpy(out_img_info->digest, hash, IMG_HASH_LEN);
#endif
    }
    return 0;

Read_Flash_Fail:
    return IMG_ERR_READ_FLASH;
}

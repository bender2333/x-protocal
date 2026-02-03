/*
 *  SSL client demonstration program
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#endif

#include "ibl_includes.h"

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_ENTROPY_C) || /* !defined(MBEDTLS_CERTS_C) ||*/ \
    !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_CLI_C) || \
    !defined(MBEDTLS_NET_C) || !defined(MBEDTLS_RSA_C) ||         \
    !defined(MBEDTLS_PEM_PARSE_C) || \
    !defined(MBEDTLS_CTR_DRBG_C) || !defined(MBEDTLS_X509_CRT_PARSE_C)
int ssl_self_test( void )
{
    mbedtls_printf("MBEDTLS_BIGNUM_C and/or MBEDTLS_ENTROPY_C and/or "
           "MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_CLI_C and/or "
           "MBEDTLS_NET_C and/or MBEDTLS_RSA_C and/or "
           "MBEDTLS_CTR_DRBG_C and/or MBEDTLS_X509_CRT_PARSE_C "
           "not defined.\n");
    return( 0 );
}
#else

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy.h"
#include "mbedtls/entropy_poll.h"
#include "mbedtls/hmac_drbg.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/dhm.h"
#include "mbedtls/gcm.h"
#include "mbedtls/ccm.h"
#include "mbedtls/cmac.h"
#include "mbedtls/md2.h"
#include "mbedtls/md4.h"
#include "mbedtls/md5.h"
#include "mbedtls/ripemd160.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"
#include "mbedtls/arc4.h"
#include "mbedtls/des.h"
#include "mbedtls/aes.h"
#include "mbedtls/camellia.h"
#include "mbedtls/base64.h"
#include "mbedtls/bignum.h"
#include "mbedtls/rsa.h"
#include "mbedtls/x509.h"
#include "mbedtls/xtea.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/ecp.h"
#include "mbedtls/ecjpake.h"
#include "mbedtls/timing.h"
#include "mbedtls/memory_buffer_alloc.h"

//extern int mbedtls_mpi_exp_mod_self_test_512( int verbose );
////extern int mbedtls_mpi_exp_mod_self_test_1024( int verbose );
static int mbedtls_ecdsa_self_test(int verbose);

void ssl_self_test(void *arg)
{
#if defined (MBEDTLS_SELF_TEST)

    int v, suites_tested = 0, suites_failed = 0;
#if 0//defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C) && defined(MBEDTLS_SELF_TEST)
    unsigned char buf[1000000];
#endif
    void *pointer;

    /*
     * The C standard doesn't guarantee that all-bits-0 is the representation
     * of a NULL pointer. We do however use that in our code for initializing
     * structures, which should work on every modern platform. Let's be sure.
     */
    memset( &pointer, 0, sizeof( void * ) );
    if( pointer != NULL )
    {
        mbedtls_printf( "all-bits-zero is not a NULL pointer\n" );
        mbedtls_exit( MBEDTLS_EXIT_FAILURE );
    }

    {
        v = 1;
        mbedtls_printf( "\n" );
    }


#if defined(MBEDTLS_SELF_TEST)
#if 0//defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
    mbedtls_memory_buffer_alloc_init( buf, sizeof(buf) );
#endif

#if defined(MBEDTLS_MD2_C)
    if( mbedtls_md2_self_test( v )  != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_MD4_C)
    if( mbedtls_md4_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_MD5_C)
    if( mbedtls_md5_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_RIPEMD160_C)
    if( mbedtls_ripemd160_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_SHA1_C)
    if( mbedtls_sha1_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_SHA256_C)
    if( mbedtls_sha256_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_SHA512_C)
    if( mbedtls_sha512_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_ARC4_C)
    if( mbedtls_arc4_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_DES_C)
    if( mbedtls_des_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_AES_C)
    if( mbedtls_aes_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_GCM_C) && defined(MBEDTLS_AES_C)
    if( mbedtls_gcm_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_CCM_C) && defined(MBEDTLS_AES_C)
    if( mbedtls_ccm_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_CMAC_C)
    if( ( mbedtls_cmac_self_test( v ) ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_BASE64_C)
    if( mbedtls_base64_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_BIGNUM_C)
    if( mbedtls_mpi_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#if 0//def CONFIG_HW_SECURITY_ENGINE
    mbedtls_mpi_exp_mod_self_test_512(1);
    mbedtls_mpi_exp_mod_self_test_1024(1);
#endif
#endif

#if defined(MBEDTLS_RSA_C)
    if( mbedtls_rsa_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_X509_USE_C)
/*
    if( mbedtls_x509_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
*/
#endif

#if defined(MBEDTLS_XTEA_C)
    if( mbedtls_xtea_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_CAMELLIA_C)
    if( mbedtls_camellia_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_CTR_DRBG_C)
    if( mbedtls_ctr_drbg_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_HMAC_DRBG_C)
    if( mbedtls_hmac_drbg_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif


#if defined(MBEDTLS_ECP_C)
    if( mbedtls_ecp_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_ECDSA_C)
#ifdef CONFIG_HW_SECURITY_ENGINE
    if( mbedtls_ecdsa_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif
#endif


#if defined(MBEDTLS_ECJPAKE_C)
/*
    if( mbedtls_ecjpake_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
*/
#endif

#if defined(MBEDTLS_DHM_C)
    if( mbedtls_dhm_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_ENTROPY_C)

#if defined(MBEDTLS_ENTROPY_NV_SEED) && !defined(MBEDTLS_NO_PLATFORM_ENTROPY)
    create_entropy_seed_file();
#endif

    if( mbedtls_entropy_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#if defined(MBEDTLS_PKCS5_C)
    if( mbedtls_pkcs5_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

/* Slow tests last */

#if defined(MBEDTLS_TIMING_C)
/*
    if( mbedtls_timing_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
*/
#endif

    if( v != 0 )
    {
#if 0//defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C) && defined(MBEDTLS_MEMORY_DEBUG)
        mbedtls_memory_buffer_alloc_status();
#endif
    }

#if 0//defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
    mbedtls_memory_buffer_alloc_free();
    if( mbedtls_memory_buffer_alloc_self_test( v ) != 0 )
    {
        suites_failed++;
    }
    suites_tested++;
#endif

#else
    mbedtls_printf( " MBEDTLS_SELF_TEST not defined.\r\n" );
#endif

    if( v != 0 )
    {
        mbedtls_printf( "  Executed %d test suites\r\n\r\n", suites_tested );

        if( suites_failed > 0)
        {
            mbedtls_printf( "  [ %d tests FAIL ]\r\n\r\n", suites_failed );
        }
        else
        {
            mbedtls_printf( "  [ All tests PASS ]\r\n\r\n" );
        }
    }

    if( suites_failed > 0)
        mbedtls_exit( MBEDTLS_EXIT_FAILURE );

#endif  //MBEDTLS_SELF_TEST
}
static void dump_buf( const char *title, unsigned char *buf, size_t len )
{
    size_t i;

    mbedtls_printf( "%s", title );
    for( i = 0; i < len; i++ )
        mbedtls_printf("%c%c", "0123456789ABCDEF" [buf[i] / 16],
                       "0123456789ABCDEF" [buf[i] % 16] );
    mbedtls_printf( "\n" );
}

static void dump_pubkey( const char *title, mbedtls_ecdsa_context *key )
{
    unsigned char buf[300];
    size_t len;

    if( mbedtls_ecp_point_write_binary( &key->grp, &key->Q,
                MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buf, sizeof buf ) != 0 )
    {
        mbedtls_printf("internal error\n");
        return;
    }

    dump_buf( title, buf, len );
}

//static const uint32_t ecc_verify_hash[] = { 0xb2c28465, 0x89375212, 0x00f3e301, 0x06d7cd0d, 0x3b23f176, 0x1e8dcd3d };

extern const mbedtls_ecp_curve_info ecp_supported_curves[];
static int mbedtls_ecdsa_self_test(int verbose)
{
//#define ECPARAMS    MBEDTLS_ECP_DP_SECP192R1
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;
    mbedtls_ecdsa_context ctx_sign, ctx_verify;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    unsigned char message[100];
    unsigned char hash[32];
    unsigned char sig[MBEDTLS_ECDSA_MAX_LEN];
    size_t sig_len;
    const char *pers = "ecdsa";
    unsigned int group_id, nb_curves = 0;

    mbedtls_ecdsa_init( &ctx_sign );
    mbedtls_ecdsa_init( &ctx_verify );
    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );

NextCurve:
    if (nb_curves < 11) {
        exit_code = MBEDTLS_EXIT_FAILURE;
        mbedtls_ecdsa_free( &ctx_verify );
        mbedtls_ecdsa_free( &ctx_sign );
        mbedtls_ctr_drbg_free( &ctr_drbg );
        mbedtls_entropy_free( &entropy );

        mbedtls_ecdsa_init( &ctx_sign );
        mbedtls_ecdsa_init( &ctx_verify );
        mbedtls_ctr_drbg_init( &ctr_drbg );
        mbedtls_entropy_init( &entropy );
    } else {
        goto exit;
    }
    if( verbose != 0 )
        mbedtls_printf( "  ====== ECC Group[%d] %s ====== \r\n", nb_curves, ecp_supported_curves[nb_curves].name);
    group_id = ecp_supported_curves[nb_curves].grp_id;

    memset( sig, 0, sizeof( sig ) );
    memset( message, 0x25, sizeof( message ) );

    /*
     * Generate a key pair for signing
     */
    mbedtls_printf( "\n  . Seeding the random number generator..." );
    //fflush( stdout );

    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n  . Generating key pair..." );
    //fflush( stdout );

    if( ( ret = mbedtls_ecdsa_genkey( &ctx_sign, group_id,
                              mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ecdsa_genkey returned %d\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok (key size: %d bits)\n", (int) ctx_sign.grp.pbits );

    dump_pubkey( "  + Public key: ", &ctx_sign );

    /*
     * Compute message hash
     */
    mbedtls_printf( "  . Computing message hash..." );
    //fflush( stdout );

    if( ( ret = mbedtls_sha256_ret( message, sizeof( message ), hash, 0 ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_sha256_ret returned %d\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

//memcpy(hash, ecc_verify_hash, sizeof(ecc_verify_hash));

    dump_buf( "  + Hash: ", hash, sizeof( hash ) );


    /*
     * Sign message hash
     */
    mbedtls_printf( "  . Signing message hash..." );
    //fflush( stdout );

    if( ( ret = mbedtls_ecdsa_write_signature( &ctx_sign, MBEDTLS_MD_SHA256,
                                       hash, sizeof( hash ),
                                       sig, &sig_len,
                                       mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ecdsa_genkey returned %d\n", ret );
        goto exit;
    }
    mbedtls_printf( " ok (signature length = %u)\n", (unsigned int) sig_len );

    dump_buf( "  + Signature: ", sig, sig_len );

    /*
     * Transfer public information to verifying context
     *
     * We could use the same context for verification and signatures, but we
     * chose to use a new one in order to make it clear that the verifying
     * context only needs the public key (Q), and not the private key (d).
     */
    mbedtls_printf( "  . Preparing verification context..." );
    //fflush( stdout );

    if( ( ret = mbedtls_ecp_group_copy( &ctx_verify.grp, &ctx_sign.grp ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ecp_group_copy returned %d\n", ret );
        goto exit;
    }

    if( ( ret = mbedtls_ecp_copy( &ctx_verify.Q, &ctx_sign.Q ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ecp_copy returned %d\n", ret );
        goto exit;
    }


    /*
     * Verify signature
     */
    mbedtls_printf( " ok\n  . Verifying signature..." );
    //fflush( stdout );

    if( ( ret = mbedtls_ecdsa_read_signature( &ctx_verify,
                                      hash, sizeof( hash ),
                                      sig, sig_len ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ecdsa_read_signature returned %d\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    exit_code = MBEDTLS_EXIT_SUCCESS;

    nb_curves++;
    goto NextCurve;

exit:

    mbedtls_ecdsa_free( &ctx_verify );
    mbedtls_ecdsa_free( &ctx_sign );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return( exit_code );
}

#endif


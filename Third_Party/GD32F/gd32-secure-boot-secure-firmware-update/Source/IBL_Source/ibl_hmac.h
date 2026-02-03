/*!
    \file    ibl_hmac.h
    \brief   IBL hmac header file for GD32 SDK

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

#ifndef __IBL_HMAC_H__
#define __IBL_HMAC_H__

#include "mbedtls/sha256.h"
typedef struct
{
    /** The hash context. */
    mbedtls_sha256_context hash_ctx;
    /** The HMAC part of the context. */
    uint8_t opad[SHA256_HASH_BLOCK_SIZE];
} hmac_internal_data;

int hmac_abort_internal(hmac_internal_data *hmac);

void hmac_init_internal(hmac_internal_data *hmac);

int hmac_setup_internal(hmac_internal_data *hmac,
                                             const uint8_t *key,
                                             size_t key_length);

int hmac_update_internal(hmac_internal_data *hmac,
                                    const uint8_t *input,
                                    size_t input_length );

int hmac_finish_internal(hmac_internal_data *hmac,
                                              uint8_t *mac,
                                              size_t mac_size);
#endif  /* __IBL_HMAC_H__ */

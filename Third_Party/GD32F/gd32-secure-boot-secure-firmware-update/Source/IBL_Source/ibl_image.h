/*!
    \file    ibl_image.h
    \brief   IBL image header file for GD32 SDK

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

#ifndef __IBL_IMAGE_H__
#define __IBL_IMAGE_H__

#include "ibl_def.h"
#include "ibl_state.h"

/* !!!!!Image signed with ED25519 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __packed
#define __packed __attribute__((__packed__))
#endif

#ifdef IBL_PROJECT 
/*
 * Image Magic
 */
#define IMG_MAGIC_NONE            0xffffffff
#define IMG_MAGIC_H               0x96f3b83d
#define IMG_MAGIC_PTLV            0x6907
#define IMG_MAGIC_T               {0xf395c277, 0x7fefd260, 0x0f505235, 0x8079b62c}
#endif /* IBL_PROJECT  */

/*
 * Image Manifest Format Version
 */
#define IMG_MANI_FORMAT_VER       0

/*
 * Image TYPE.
 */
#define IMG_TYPE_MBL              0x1
#define IMG_TYPE_PROT             0x2
#define IMG_TYPE_AROT             0x4
#define IMG_TYPE_NSPE             0x8
#define IMG_TYPE_APP              0x10
#define IMG_TYPE_IMG              IMG_TYPE_APP


/*
 * Image Hash Algorithm.
 */
#define IMG_HASH_SHA256           0x1
#define IMG_HASH_SHA512           0x2

/*
 * Image Protected TLV types.
 */
//#define IMG_TLV_EC256_PKHASH      0x20   /* ECDSA Public Key HASH*/
//#define IMG_TLV_EC256_SIG         0x30   /* ECDSA Signature */
// #define IMG_TLV_ED25519_PKHASH      0x20   /* ED25519 Signature */
// #define IMG_TLV_SECP256R1_PKHASH    0x20   /* ED25519 Signature */
#define IMG_TLV_PKHASH              0x01   /* ED25519 Signature or ECDSA256 all PKHASH are 32 bits */
#define IMG_TLV_SHA256_DIGEST       0x10   /* SHA256 of image hdr and body */
#define IMG_TLV_RSA2048_PSS         0x20   /* RSA2048 of hash output */
#define IMG_TLV_ED25519_SIG         0x30   /* ED25519 Signature */
#define IMG_TLV_BOOT_RECORD         0x60   /* Boot Record */
#define IMG_TLV_ECDSA256_SIG        0x22   /* ECDSA256 Signature */

/*
 * Image Related Length.
 */
#define IMG_HEADER_SIZE           32
#define IMG_VER_MAJOR_LEN         8
#define IMG_VER_MINOR_LEN         8
#define IMG_VER_REVISION_LEN      16
#define IMG_PKHASH_LEN            32
#define ED25519_PK_LEN            32
#define ED25519_SIG_LEN           64

//#define IMG_SIG_MAX_LEN           128   /* For EC256 */

/*
 * Image Status.
 */
#if 0
#define IMG_STATUS_NEW            0x1
#define IMG_STATUS_READY          0x2  /* The Image is verified by MBL, but not verified by the remote cloud server. */
#define IMG_STATUS_OK             0x3  /* The Image is verified by MBL and the remote cloud server. */
#define IMG_STATUS_BAD            0x4
#define IMG_STATUS_UNKNOWN        0x5
#endif

#define IMG_FLAG_NEWER_MASK       0x01
#define IMG_FLAG_VERIFY_MASK      0x06
#define IMG_FLAG_IA_MASK          0x18

#define IMG_FLAG_OLDER            0x0
#define IMG_FLAG_NEWER            0x1         /* The image with higher version will be set to NEWER.
                                                Default Newer after ISP. Set or Cleared when New image downloaded though OTA.
                                                Checked when MBL finding the boot image.
                                                Only one image is set to be NEWER at the same time. */

#define IMG_FLAG_VERIFY_NONE      (0x0 << 1)  /* Default None. Set after MBL verification finished. Checked when MBL finding the boot image. */
#define IMG_FLAG_VERIFY_OK        (0x1 << 1)
#define IMG_FLAG_VERIFY_FAIL      (0x2 << 1)
#define IMG_FLAG_IA_NONE          (0x0 << 3)  /* Default None. Set after IA finished. Checked when MBL finding the boot image. */
#define IMG_FLAG_IA_OK            (0x1 << 3)
#define IMG_FLAG_IA_FAIL          (0x2 << 3)

/*
* Image check error.
*/
enum img_validate_err_t {
    IMG_OK = 0,
    IMG_ERR_BAD_HEADER = -1,
    IMG_ERR_BAD_PTLV = -2,
    IMG_ERR_BAD_HASH = -3,
    IMG_ERR_BAD_PKHASH = -4,
    IMG_ERR_BAD_SIG = -5,
    IMG_ERR_MISSING_TLV = -6,
    IMG_ERR_READ_FLASH = -7,
    IMG_ERR_BAD_RECORD = -8,
};

/*
* Cert check error.
*/
enum cert_validate_err_t {
    CERT_OK = 0,
    CERT_ERR_BAD_ADDR = -1,
    CERT_ERR_BAD_SZ = -2,
    CERT_ERR_BAD_FORMAT = -3,
    CERT_ERR_BAD_SIG = -4,
    CERT_ERR_READ_FLASH = -5,
};

#ifdef IBL_PROJECT 
/** Image header.  All fields are in little endian byte order. Total 32 bytes. */
//struct image_header {
//    uint32_t magic_h;                /* Head Magic for boundary check. */
//    uint32_t tot_sz;                 /* The total size of Image, including header, TLVs and the cert if existed. */
//    uint8_t mani_ver;                /* The version of Image Manifest Format. */
//    uint8_t img_type;                /* The type of Image (Firmware). */
//    uint8_t algo_hash;               /* The Hash algorithm for Image digest */
//    uint8_t algo_sign;               /* The algorithm used to sign Image manifest. */
//    uint16_t hdr_sz;                 /* Size of Image Header (bytes). */
//    uint16_t ptlv_sz;                /* Size of PTLVs (bytes). */
//    uint32_t img_sz;                 /* Size of Image itself (bytes). */
//    uint8_t ver_major;               /* Major Version. */
//    uint8_t ver_minor;               /* Minor Version. */
//    uint16_t ver_rev;                /* Revision. */
//    uint32_t rsvd;                   /* Reserved. */
//    uint32_t chksum;                 /* Header check sum. */
//};
struct image_version {
    uint8_t iv_major;
    uint8_t iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
} __packed;

struct image_dependency {
    uint8_t image_id;                       /* Image index (from 0) */
    uint8_t _pad1;
    uint16_t _pad2;
    struct image_version image_min_version; /* Indicates at minimum which
                                             * version of firmware must be
                                             * available to satisfy compliance
                                             */
};

/** Image header.  All fields are in little endian byte order. */
struct image_header {
    uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size;           /* Size of image header (bytes). */
    uint16_t ih_protect_tlv_size;   /* Size of protected TLV area (bytes). */
    uint32_t ih_img_size;           /* Does not include header. */
    uint32_t ih_flags;              /* IMAGE_F_[...]. */
    struct image_version ih_ver;
    uint32_t _pad1;
} __packed;

/** Image TLV header.  All fields in little endian. */
struct image_tlv_info {
    uint16_t magic_tlv;
    uint16_t tlv_sz;  /* size of TLV area (including tlv_info header) */
};

/** Image trailer TLV format. All fields in little endian. */
struct image_tlv {
    uint8_t  type;   /* IMAGE_TLV_[...]. */
    uint8_t  _pad;
    uint16_t len;    /* Data length (not including TLV header). */
};

_Static_assert(sizeof(struct image_header) == IMG_HEADER_SIZE,
               "struct image_header not required size");

uint32_t cal_checksum(uint8_t *ptr, uint32_t sz);
int img_find(IN uint32_t start_faddr,
                IN uint8_t img_type,
                OUT uint32_t *img_faddr);
int img_verify_sign(IN uint8_t *hash,
                        IN uint32_t hlen,
                        IN uint8_t *pk,
                        IN uint8_t *sig);
int img_verify_hash(IN uint32_t faddr,  /* Flash Adress */
                        IN uint32_t len,
                        IN uint8_t *hash,
                        IN uint32_t hlen,
                        IN uint8_t *seed,
                        IN int seed_len);
int img_verify_hdr(IN struct image_header *hdr,
                    IN uint8_t img_type);

int img_verify_pkhash(IN uint8_t *pk,
                IN uint32_t klen,
                IN uint8_t *pkhash,
                IN uint32_t hlen);

int img_validate(IN uint32_t img_faddr,
                   IN uint8_t img_type,
                   IN uint8_t *pk,
                   OUT void *img_info);

int cert_validate(IN uint32_t crt_faddr,
                    IN size_t crt_sz,
                    IN uint8_t *verify_pk,
                    OUT uint8_t *img_pk);
#endif /* IBL_PROJECT  */

#ifdef __cplusplus
}
#endif

#endif /* __IBL_IMAGE_H__ */

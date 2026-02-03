/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MCUBOOT_CONFIG_H__
#define __MCUBOOT_CONFIG_H__

#include "Source/IBL_Source/ibl_def.h"

/*
 * This file is included by the simulator, but we don't want to
 * define almost anything here.
 *
 * Instead of using mcuboot_config.h, the simulator adds MCUBOOT_xxx
 * configuration flags to the compiler command lines based on the
 * values of environment variables. However, the file still must
 * exist, or bootutil won't build.
 */

#define MCUBOOT_WATCHDOG_FEED()         \
    do {                                \
        ibl_fwdg_reload();              \
    } while (0)

#define MCUBOOT_CPU_IDLE() \
    do {                                \
    } while (0)

#define MCUBOOT_USE_MBED_TLS

//#ifndef MCUBOOT_MAX_IMG_SECTORS
//#define MCUBOOT_MAX_IMG_SECTORS     4096
//#endif /* MCUBOOT_MAX_IMG_SECTORS */

#define MCUBOOT_USE_FLASH_AREA_GET_SECTORS 
#define MCUBOOT_IMAGE_NUMBER    1

#define MCUBOOT_HAVE_LOGGING
#define MCUBOOT_LOG_LEVEL   MCUBOOT_LOG_LEVEL_SIM
    
//#define MCUBOOT_BOOT_MAX_ALIGN  8

#define MCUBOOT_ENC_IMAGES
    
#if SIGN_ALGO_SET == IMG_SIG_RSA2048
/* Uncomment for RSA signature support */
#define MCUBOOT_SIGN_RSA
#define MCUBOOT_SIGN_RSA_LEN    2048

#if defined(MCUBOOT_ENC_IMAGES)
#define MCUBOOT_ENCRYPT_RSA
#endif

#elif  SIGN_ALGO_SET == IMG_SIG_ECDSA256
/* Uncomment for ECDSA signatures using curve P-256. */
#define MCUBOOT_SIGN_EC256

#if defined(MCUBOOT_ENC_IMAGES)
#define MCUBOOT_ENCRYPT_EC256
#endif
#endif /* SIGN_ALGO_SET */

//#define MCUBOOT_HW_KEY
//#define MCUBOOT_OVERWRITE_ONLY
#define MCUBOOT_VALIDATE_PRIMARY_SLOT

/*
 * Upgrade mode
 *
 * The default is to support A/B image swapping with rollback.  Other modes
 * with simpler code path, which only supports overwriting the existing image
 * with the update image or running the newest image directly from its flash
 * partition, are also available.
 *
 * You can enable only one mode at a time from the list below to override
 * the default upgrade mode.
 */

/* Uncomment to enable the overwrite-only code path. */
/* #define MCUBOOT_OVERWRITE_ONLY */

#ifdef MCUBOOT_OVERWRITE_ONLY
/* Uncomment to only erase and overwrite those primary slot sectors needed
 * to install the new image, rather than the entire image slot. */
/* #define MCUBOOT_OVERWRITE_ONLY_FAST */
#endif

//#define CONFIG_MCUBOOT_SERIAL

/* Serial extensions are not implemented
 */
//#define MCUBOOT_PERUSER_MGMT_GROUP_ENABLED 0

////#define MCUBOOT_SERIAL_UNALIGNED_BUFFER_SIZE 128

//#define MCUBOOT_SERIAL_MAX_RECEIVE_SIZE 4*1024

#define MCUBOOT_USE_IBL_VERIFY_SIGN

#define MCUBOOT_DOWNGRADE_PREVENTION    1
#define MCUBOOT_DOWNGRADE_PREVENTION_SECURITY_COUNTER    0

#endif /* __MCUBOOT_CONFIG_H__ */

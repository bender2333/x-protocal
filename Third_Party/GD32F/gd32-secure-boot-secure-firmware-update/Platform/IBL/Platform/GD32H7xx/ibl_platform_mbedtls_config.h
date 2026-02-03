#include "Source/IBL_Source/ibl_def.h"

/* hardware acceleration engine define */
//#define CONFIG_HW_SECURITY_ENGINE

#ifdef CONFIG_HW_SECURITY_ENGINE
#define MBEDTLS_AES_ALT
#define MBEDTLS_DES_ALT
#else
#define MBEDTLS_AES_ROM_TABLES
#endif /* CONFIG_HW_SECURITY_ENGINE */

//#define MBEDTLS_ECDSA_VERIFY_ALT
//#define MBEDTLS_ECDSA_SIGN_ALT

/* whether all functions of mbedtls are implemented */
//#define CONFIG_HW_LIMITED_SIZE

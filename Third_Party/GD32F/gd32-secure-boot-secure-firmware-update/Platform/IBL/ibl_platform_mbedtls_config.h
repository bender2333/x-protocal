#ifndef __IBL_PLATFORM_MBEDTLS_CONFIG_H__
#define __IBL_PLATFORM_MBEDTLS_CONFIG_H__

#if defined(PLATFORM_GD32F5XX)
#include "Platform/GD32F5xx/ibl_platform_mbedtls_config.h"
#elif defined(PLATFORM_GD32H7XX)
#include "Platform/GD32H7xx/ibl_platform_mbedtls_config.h"
#elif defined(PLATFORM_GD32A513)
#include "Platform/GD32A513/ibl_platform_mbedtls_config.h"
#elif defined(PLATFORM_GD32G5X3)
#include "Platform/GD32G5x3/ibl_platform_mbedtls_config.h"
#elif defined(PLATFORM_GD32E50X)
#include "Platform/GD32E50x/ibl_platform_mbedtls_config.h"
#else
#error Unknown platform.
#endif

#endif /* __IBL_PLATFORM_MBEDTLS_CONFIG_H__ */

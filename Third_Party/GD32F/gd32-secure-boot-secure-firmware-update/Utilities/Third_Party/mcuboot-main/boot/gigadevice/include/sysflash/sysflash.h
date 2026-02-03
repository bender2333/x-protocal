/* Manual version of auto-generated version. */

#ifndef __SYSFLASH_H__
#define __SYSFLASH_H__

#include "../flash_hal/flash_layout.h"
#include "mcuboot_config/mcuboot_config.h"

//#define PRIMARY_ID      FLASH_AREA_0_ID
//#define SECONDARY_ID    FLASH_AREA_2_ID
//#define SCRATCH_ID      FLASH_AREA_SCRATCH_ID

//#define FLASH_AREA_IMAGE_PRIMARY(x)    PRIMARY_ID
//#define FLASH_AREA_IMAGE_SECONDARY(x)  SECONDARY_ID
//#define FLASH_AREA_IMAGE_SCRATCH       SCRATCH_ID

#if (MCUBOOT_IMAGE_NUMBER == 1)
/*
 * NOTE: the definition below returns the same values for true/false on
 * purpose, to avoid having to mark x as non-used by all callers when
 * running in single image mode.
 */
#if !defined(GD32G5X3_BANKSWAP)
#define FLASH_AREA_IMAGE_PRIMARY(x)     (((x) == 0) ? FLASH_AREA_0_ID : \
                                                      FLASH_AREA_1_ID)
#define FLASH_AREA_IMAGE_SECONDARY(x)   (((x) == 0) ? FLASH_AREA_1_ID : \
                                                      FLASH_AREA_0_ID)
#else
extern uint8_t run_imgt;
#define FLASH_AREA_IMAGE_PRIMARY(x)     (((x) == 0) ? run_imgt: \
                                                      run_imgt)
#define FLASH_AREA_IMAGE_SECONDARY(x)   (((x) == 0) ? (1 - run_imgt) : \
                                                      (1 - run_imgt))
#endif
#elif (MCUBOOT_IMAGE_NUMBER == 2)
/* MCUBoot currently supports only up to 2 updatable firmware images.
 * If the number of the current image is greater than MCUBOOT_IMAGE_NUMBER - 1
 * then a dummy value will be assigned to the flash area macros.
 */
#define FLASH_AREA_IMAGE_PRIMARY(x)     (((x) == 0) ? FLASH_AREA_0_ID : \
                                         ((x) == 1) ? FLASH_AREA_1_ID : \
                                                      255 )
#define FLASH_AREA_IMAGE_SECONDARY(x)   (((x) == 0) ? FLASH_AREA_2_ID : \
                                         ((x) == 1) ? FLASH_AREA_3_ID : \
                                                      255 )
#else
#error "Image slot and flash area mapping is not defined"
#endif

#define FLASH_AREA_IMAGE_SCRATCH        FLASH_AREA_SCRATCH_ID

#endif /* __SYSFLASH_H__ */

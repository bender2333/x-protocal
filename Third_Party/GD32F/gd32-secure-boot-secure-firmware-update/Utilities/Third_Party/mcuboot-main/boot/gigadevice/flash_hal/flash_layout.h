/*
 * Copyright (c) 2018 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

#include "../../../../../../config/config_gdm32.h"
#include "gd32f5xx.h"
/* Sector size of the flash hardware */
#if defined(PLATFORM_GD32A513)
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x400)         /* 1KB */
#elif defined(PLATFORM_GD32E50X)
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x2000)         /* 8KB */
#elif defined(PLATFORM_GD32F5XX)
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x1000)         /* 4KB */
#else
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x400)         /* 4KB */
#endif

/* Flash layout info for main bootloader */
#define FLASH_BASE_ADDRESS              (0x08000000)
/* Offset and size definitions of the flash partitions that are handled by the
 * bootloader. The image swapping is done between IMAGE_0 and IMAGE_1, SCRATCH
 * is used as a temporary storage during image swapping.
 */

/* scratch area */
#if defined PLATFORM_GD32A513 || PLATFORM_GD32G5X3
#define FLASH_AREA_SCRATCH_OFFSET       (RE_FLASH_BASE-FLASH_BASE_ADDRESS\
                                        +RE_SYS_STATUS_OFFSET+0x800)          /* the size of system status is 2KB */
#define FLASH_AREA_SCRATCH_SIZE         (0x800)
#else
#define FLASH_AREA_SCRATCH_OFFSET       (RE_FLASH_BASE-FLASH_BASE_ADDRESS\
                                        +RE_SYS_STATUS_OFFSET+0x2000)          /* the size of system status is 8KB */
#define FLASH_AREA_SCRATCH_SIZE         (0x2000)
#endif

#if defined GD32G5X3_BANKSWAP
#define FLASH_PARTITION_SIZE            (RE_IMG_SIZE)
/* image primary slot */
#define FLASH_AREA_0_ID                 (0)
#define FLASH_AREA_0_OFFSET             (RE_FLASH_BASE-FLASH_BASE+RE_IMG_0_APP_OFFSET)
#define FLASH_AREA_0_SIZE               (0x10000)

#define FLASH_AREA_1_ID                 (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_1_OFFSET             (RE_FLASH_BASE-FLASH_BASE+RE_IMG_1_APP_OFFSET)
#define FLASH_AREA_1_SIZE               (0x10000)
#define FLASH_AREA_SCRATCH_ID           (FLASH_AREA_1_ID + 1)
#else

#if defined(GD32F5XX_BANKS_SWP_USED)
#define FLASH_PARTITION_SIZE            (RE_IMG_SIZE)
#else
#define FLASH_PARTITION_SIZE            (RE_IMG_1_APP_OFFSET - RE_IMG_0_APP_OFFSET)
#endif
/* image primary slot */
#define FLASH_AREA_0_ID                 (0)
#define FLASH_AREA_0_OFFSET             (RE_FLASH_BASE-FLASH_BASE+RE_IMG_0_APP_OFFSET)
#define FLASH_AREA_0_SIZE               (FLASH_PARTITION_SIZE)

#define FLASH_AREA_1_ID                 (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_1_OFFSET             (RE_FLASH_BASE-FLASH_BASE+RE_IMG_1_APP_OFFSET)
#define FLASH_AREA_1_SIZE               (FLASH_PARTITION_SIZE)

#define FLASH_AREA_SCRATCH_ID           (FLASH_AREA_1_ID + 1)
#endif
/*
 * The maximum number of status entries supported by the bootloader.
 */
#define MCUBOOT_STATUS_MAX_ENTRIES       ((2 * FLASH_PARTITION_SIZE) / FLASH_AREA_SCRATCH_SIZE)
/** Maximum number of image sectors supported by the bootloader. */
#define MCUBOOT_MAX_IMG_SECTORS          ((2 * FLASH_PARTITION_SIZE) / FLASH_AREA_IMAGE_SECTOR_SIZE)

#endif /* __FLASH_LAYOUT_H__ */

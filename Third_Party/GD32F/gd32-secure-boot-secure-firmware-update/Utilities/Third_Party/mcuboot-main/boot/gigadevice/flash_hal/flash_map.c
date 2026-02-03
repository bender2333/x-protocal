#ifdef MCUBOOT_HAVE_ASSERT_H
#include "mcuboot_config/mcuboot_assert.h"
#else
#include <assert.h>
#endif

#include "string.h"
#include "storage/flash_map.h"
#include "include/flash_map_backend/flash_map_backend.h"
//#include "flash_map_backend/flash_map_backend.h"
#include "flash_layout.h"
#include "mcuboot_config/mcuboot_config.h"
#include "sysflash/sysflash.h"
#include "bootutil/bootutil_log.h"
#include "Source/IBL_Source/ibl_export.h"

static const struct flash_area flash_map[] = {
    {
        .fa_id = FLASH_AREA_0_ID,
        .fa_device_id = FLASH_DEVICE_ID,
        .fa_off = FLASH_AREA_0_OFFSET,
        .fa_size = FLASH_AREA_0_SIZE,
    },
    {
        .fa_id = FLASH_AREA_1_ID,
        .fa_device_id = FLASH_DEVICE_ID,
        .fa_off = FLASH_AREA_1_OFFSET,
        .fa_size = FLASH_AREA_1_SIZE,
    },
    {
        .fa_id = FLASH_AREA_SCRATCH_ID,
        .fa_device_id = FLASH_DEVICE_ID,
        .fa_off = FLASH_AREA_SCRATCH_OFFSET,
        .fa_size = FLASH_AREA_SCRATCH_SIZE,
    },
};

static const int flash_map_entry_num = sizeof(flash_map) / sizeof(struct flash_area);

int flash_device_base(uint8_t fd_id, uintptr_t *ret)
{
    if (fd_id != FLASH_DEVICE_ID)
    {
        BOOT_LOG_ERR("invalid flash ID %d; expected %d",
                     fd_id, FLASH_DEVICE_ID);
        return -22;
    }
    *ret = FLASH_BASE;
    return 0;
}

/*
 * `open` a flash area.  The `area` in this case is not the individual
 * sectors, but describes the particular flash area in question.
 */
int flash_area_open(uint8_t id, const struct flash_area **area)
{
    int i;

    for (i = 0; i < flash_map_entry_num; i++)
    {
        if (id == flash_map[i].fa_id)
        {
            break;
        }
    }
    if (i == flash_map_entry_num)
    {
        return -1;
    }

    *area = &flash_map[i];
    return 0;
}

void flash_area_close(const struct flash_area *area)
{
    /* Nothing to do. */
}

int flash_area_read(const struct flash_area *area, uint32_t off, void *dst, uint32_t len)
{
    return ibl_flash_read(area->fa_off + off, dst, len);
}

int flash_area_write(const struct flash_area *area, uint32_t off,
                     const void *src, uint32_t len)
{
    return ibl_flash_write(area->fa_off + off, src, len);
}

int flash_area_erase(const struct flash_area *area, uint32_t off, uint32_t len)
{
    return ibl_flash_erase(area->fa_off + off, len);
}

uint32_t flash_area_align(const struct flash_area *area)
{
#if defined PLATFORM_GD32G5X3
    return 8;
#else
    return 4;
#endif
}

uint8_t flash_area_erased_val(const struct flash_area *fap)
{
    return 0xFF;
}

int flash_area_id_to_multi_image_slot(int image_index, int area_id)
{
    if (area_id == FLASH_AREA_IMAGE_PRIMARY(image_index))
    {
        return 0;
    }
    if (area_id == FLASH_AREA_IMAGE_SECONDARY(image_index))
    {
        return 1;
    }

    return -1;
}

int flash_area_id_from_image_slot(int slot)
{
    return flash_area_id_from_multi_image_slot(0, slot);
}

int flash_area_id_from_multi_image_slot(int image_index, int slot)
{
    switch (slot)
    {
    case 0:
        return FLASH_AREA_IMAGE_PRIMARY(image_index);
    case 1:
        return FLASH_AREA_IMAGE_SECONDARY(image_index);
    case 2:
        return FLASH_AREA_IMAGE_SCRATCH;
    }
    return -1; /* flash_area_open will fail on that */
}

#ifdef MCUBOOT_USE_FLASH_AREA_GET_SECTORS
int flash_area_get_sectors(int idx, uint32_t *cnt, struct flash_sector *ret)
{
    int rc = 0;
    *cnt = 1;
    uint32_t i = 0;
    struct flash_area *fa = NULL;

    while (NULL != &flash_map[i])
    {
        if (idx == (&(flash_map[i]))->fa_id)
        {
            fa = (struct flash_area *)&(flash_map[i]);
            break;
        }
        i++;
    }

    if (NULL != &flash_map[i])
    {
        size_t sector_size = 0;

        if (fa->fa_device_id == FLASH_DEVICE_ID)
        {
            sector_size = FLASH_AREA_IMAGE_SECTOR_SIZE;
        }
        else
        {
            rc = -1;
        }

        if (0 == rc)
        {
            uint32_t addr = 0;
            size_t sectors_n = 0;

            sectors_n = (fa->fa_size + (sector_size - 1)) / sector_size;

            addr = fa->fa_off;
            for (i = 0; i < sectors_n; i++)
            {
                ret[i].fs_size = sector_size;
                ret[i].fs_off = addr;
                addr += sector_size;
            }

            *cnt = sectors_n;
        }
    }
    else
    {
        rc = -1;
    }
    return rc;
}
#else
//__attribute__((deprecated))
int flash_area_to_sectors(int idx, int *cnt, struct flash_area *ret)
{
    printf("%s %d\r\n", __FILE__, __LINE__);
    return -1;
}
#endif /* MCUBOOT_USE_FLASH_AREA_GET_SECTORS */

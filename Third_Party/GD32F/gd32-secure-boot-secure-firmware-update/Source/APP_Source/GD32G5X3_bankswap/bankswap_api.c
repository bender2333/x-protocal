#include "Platform/APP/app_region.h"
#include "bootutil/bootutil_public.h"
#include "flash_hal/flash_layout.h"
#include "include/storage/flash_map.h"
#include <stdio.h>
#include "gd32g5x3.h"
#include "gd32g553q_eval.h"

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

void com_usart_init(void)
{
    /* enable COM GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USART TX */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);
    /* connect port to USART RX */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);

    /* configure USART TX as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_9);

    /* configure USART RX as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_word_length_set(USART0, USART_WL_8BIT);
    
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_baudrate_set(USART0, 115200U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

    usart_enable(USART0);
}

int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART0, (uint8_t) ch);

    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));

    return ch;
}

int flash_area_open(uint8_t id, const struct flash_area **area)
{
    int i;

    for (i = 0; i < flash_map_entry_num; i++) {
        if (id == flash_map[i].fa_id) {
            break;
        }
    }
    if (i == flash_map_entry_num) {
        return -1;
    }

    *area = &flash_map[i];
    return 0;
}

static inline uint32_t
boot_magic_off(const struct flash_area *fap)
{
    return flash_area_get_size(fap) - BOOT_MAGIC_SZ;
}

static inline uint32_t
boot_image_ok_off(const struct flash_area *fap)
{
    return ALIGN_DOWN(boot_magic_off(fap) - BOOT_MAX_ALIGN, BOOT_MAX_ALIGN);
}

int
boot_write_trailer(const struct flash_area *fap, uint32_t off,
        const uint8_t *inbuf, uint8_t inlen)
{
    uint8_t buf[BOOT_MAX_ALIGN];
    uint8_t erased_val;
    uint32_t align;
    int rc;

    align = 8;
    align = ALIGN_UP(inlen, align);
    if (align > BOOT_MAX_ALIGN) {
        return -1;
    }
    erased_val = 0xFF;

    memcpy(buf, inbuf, inlen);
    memset(&buf[inlen], erased_val, align - inlen);

    rc = flash_area_write(fap, off, buf, align);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    return 0;
}

int
boot_write_trailer_flag(const struct flash_area *fap, uint32_t off,
        uint8_t flag_val)
{
    const uint8_t buf[1] = { flag_val };
    return boot_write_trailer(fap, off, buf, 1);
}

int
boot_write_image_ok(const struct flash_area *fap)
{
    uint32_t off;

    off = boot_image_ok_off(fap);
    return boot_write_trailer_flag(fap, off, BOOT_FLAG_SET);
}

int flash_area_write(const struct flash_area *area, uint32_t off,
                     const void *src, uint32_t len)
{
    uint8_t *data_u8 = (uint8_t *)src;
    uint32_t base_addr;
    uint64_t data_temp;
    uint32_t offset = area->fa_off + off;
    
    if (1) {
        uint32_t offset_align;
        uint64_t val32;
        int vb, act_len, i;
        uint8_t val[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        base_addr = FLASH_BASE;
        /* unlock the flash program erase controller */
        fmc_unlock();
        /* clear pending flags */
        fmc_flag_clear(FMC_FLAG_ENDF);
        fmc_flag_clear(FMC_FLAG_PGERR);
        fmc_flag_clear(FMC_FLAG_WPERR);
        fmc_flag_clear(FMC_FLAG_OPRERR);
        fmc_flag_clear(FMC_FLAG_PGSERR);
        fmc_flag_clear(FMC_FLAG_PGMERR);
        fmc_flag_clear(FMC_FLAG_PGAERR);
        fmc_flag_clear(FMC_OPRERR);
        fmc_flag_clear(FMC_TOERR);
        
        offset_align = (offset & ~0x7);

        /* if offset is not 4-byte alignment */
        vb = (offset & 0x7);
        if (vb > 0) {
            act_len = ((8 - vb) > len) ? len : (8 - vb);
            for (i = 0; i < act_len; i++) {
                val[vb + i] = *(data_u8 + i);
            }
#if 0
            if (act_len == 1) {
                    d0 = (~(*data_u8) & 0xFF);
                    val = ~(d0 << (vb * 8));
            } else if (act_len == 2) {
                    d0 = (~(*data_u8) & 0xFF);
                    d1 = (~(*(data_u8 + 1)) & 0xFF);
                    val = ~((d0 << (vb * 8)) | (d1 << ((vb + 1) * 8)));
            } else if (act_len == 3) {
                    d0 = (~(*data_u8) & 0xFF);
                    d1 = (~(*(data_u8 + 1)) & 0xFF);
                    d2 = (~(*(data_u8 + 2)) & 0xFF);
                    val = ~((d0 << 8) | (d1 << 16) | (d2 << 24));
            }
#endif
            fmc_doubleword_program((base_addr + offset_align), *(uint64_t *)val);
            offset_align += 8;
            data_u8 += act_len;
            len -= act_len;
        }

        /* word program */
        while (len >= 8) {
            memcpy((uint8_t *)(&data_temp), data_u8, sizeof(uint64_t));
            fmc_doubleword_program((base_addr + offset_align), data_temp);
            offset_align += 8;
            data_u8 += 8;
            len -= 8;
        }

        /* if len is not 4-byte alignment */
        val32 = 0xFFFFFFFFFFFFFFFF;
        if (len > 0) {
            while (len-- > 0) {
                val32 = (val32 << 8);
                val32 |= *(data_u8 + len);
            }
            fmc_doubleword_program((base_addr + offset_align), val32);
        }

        /* lock the flash program erase controller */
        fmc_lock();
    } else {
#if defined(PLATFORM_GDM32)
        uint32_t page_offset;
        uint32_t size_to_program;

        base_addr = FLASH_START_QSPI;
        page_offset = (offset & (FLASH_QSPI_PAGE_SIZE - 1));
        if (page_offset != 0) {
            size_to_program = (len > FLASH_QSPI_PAGE_SIZE - page_offset) ? (FLASH_QSPI_PAGE_SIZE - page_offset) : len;
            qspi_flash_program_page((base_addr + offset), data_u8, size_to_program);
            offset += size_to_program;
            data_u8 += size_to_program;
            len -= size_to_program;
        }

        while (len > 0) {
            size_to_program = (len > FLASH_QSPI_PAGE_SIZE) ? FLASH_QSPI_PAGE_SIZE : len;
            qspi_flash_program_page((base_addr + offset), data_u8, size_to_program);
            offset += size_to_program;
            data_u8 += size_to_program;
            len -= size_to_program;
        }
#endif /* PLATFORM_GDM32 */
    }
    return 0;
}
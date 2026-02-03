/*!
    \file    ibl_flash.c
    \brief   IBL flash configure for GD32 SDK

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

#include "Source/IBL_Source/ibl_includes.h"

uint32_t flash_tot_sz;

static void qspi_flash_gpio_config(void)
{
}

static void qspi_flash_write_enable(void)
{
}

static void qspi_flash_autopolling_ready(void)
{
}

void qspi_flash_config(uint32_t clock_prescaler)
{
}

int32_t qspi_flash_erase_sector(uint32_t address)
{
    return 0;
}

int32_t qspi_flash_erase_chip(void)
{
    return 0;
}

int32_t qspi_flash_read(uint32_t address, void *data, uint32_t size)
{
    return 0;
}

int32_t qspi_flash_program_page(uint32_t address, const uint8_t *data, uint32_t size)
{
    return 0;
}

int is_sip_flash(void)
{
//    return (OBSTAT_NQSPI() == SET);
    return 1;
}

int is_valid_flash_offset(uint32_t offset)
{
    if (offset < flash_total_size()) {
        return 1;
    }
    return 0;
}

int is_valid_flash_addr(uint32_t addr)
{
    int bvalid = 0;
    if (((addr >= FLASH_BASE) && (addr < (FLASH_BASE + flash_total_size())))
        || ((addr >= FLASH_BASE) && (addr < (FLASH_BASE + flash_total_size())))) {
        bvalid = 1;
    }
    return bvalid;
}

uint32_t flash_total_size(void)
{
    if (flash_tot_sz)
        return flash_tot_sz;
    else
       return FLASH_TOTAL_SIZE;
}

uint32_t flash_erase_size(void)
{
    if (is_sip_flash()) {
        return FLASH_SIP_PAGE_SIZE;
    }
    return 0;
}

int flash_init(void)
{
#if defined(GD32F5XX)
    /* need to do nothing */
    return 0;
#else /* GD32F5xx */
    return 0;
#endif
}

void flash_nodec_config(uint32_t nd_idx, uint32_t start_page, uint32_t end_page)
{

}

int flash_read(uint32_t offset, void *data, int len)
{
    uint32_t i;
    uint32_t dst = (uint32_t)data;
    uint32_t left;

    if ((offset & 3) || (dst & 3)) {
        for (i = 0; i < len; i++) {
            ((uint8_t *)data)[i] = *(uint8_t *)(FLASH_BASE + offset + i);
        }
    } else {
        left = (len & 3);
        len -= left;
        for (i = 0; i < len; i += 4) {
            *(uint32_t *)(dst + i) = *(uint32_t *)(FLASH_BASE + offset + i);
        }
        if (left > 0)
            ibl_memcpy((uint8_t *)(dst + len), (uint8_t *)(FLASH_BASE+ offset + len), left);
    }
    return 0;
}

/* consider encrypted image, two flash read api will be needed,
   one is directly read and flash value is decrypted, another is indirect
   read and no decryption is done.
*/
int flash_read_indirect(uint32_t offset, void *data, int len)
{
    uint32_t i;
    uint32_t dst = (uint32_t)data;
    uint32_t left;

    if (!is_valid_flash_offset(offset) || data == NULL
        || len <= 0 || !is_valid_flash_offset(offset + len - 1)) {
        return -1;
    }

    if (is_sip_flash()) {
        if ((offset & 3) || (dst & 3)) {
            for (i = 0; i < len; i++) {
                ((uint8_t *)data)[i] = *(uint8_t *)(FLASH_BASE + offset + i);
            }
        } else {
            left = (len & 3);
            len -= left;
            for (i = 0; i < len; i += 4) {
                *(uint32_t *)(dst + i) = *(uint32_t *)(FLASH_BASE + offset + i);
            }
            if (left > 0)
                ibl_memcpy((uint8_t *)(dst + len), (uint8_t *)(FLASH_BASE + offset + len), left);
        }
    } else {
    }
    return 0;
}

int flash_write_fast(uint32_t offset, const void *data, int len)
{
    int ret = 0;

    return ret;
}

int flash_write(uint32_t offset, const void *data, int len)
{
    uint8_t *data_u8 = (uint8_t *)data;
    uint32_t base_addr;

    if (is_sip_flash()) {
        uint32_t offset_align, val32;
        int vb, act_len, i;
        uint8_t val[4] = {0xFF, 0xFF, 0xFF, 0xFF};

        base_addr = FLASH_BASE;
        /* unlock the flash program erase controller */
        fmc_unlock();
        /* clear pending flags */
        fmc_flag_clear(FMC_FLAG_END);
        fmc_flag_clear(FMC_FLAG_PGAERR);
        fmc_flag_clear(FMC_FLAG_WPERR);
        fmc_flag_clear(FMC_FLAG_PGSERR);

        offset_align = (offset & ~0x3);

        /* if offset is not 4-byte alignment */
        vb = (offset & 0x3);
        if (vb > 0) {
            act_len = ((4 - vb) > len) ? len : (4 - vb);
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
            fmc_word_program((base_addr + offset_align), *(uint32_t *)val);
            offset_align += 4;
            data_u8 += act_len;
            len -= act_len;
        }

        /* word program */
        while (len >= 4) {
            fmc_word_program((base_addr + offset_align), *(uint32_t *)data_u8);
            offset_align += 4;
            data_u8 += 4;
            len -= 4;
        }

        /* if len is not 4-byte alignment */
        val32 = 0xFFFFFFFF;
        if (len > 0) {
            while (len-- > 0) {
                val32 = (val32 << 8);
                val32 |= *(data_u8 + len);
            }
            fmc_word_program((base_addr + offset_align), val32);
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

int flash_erase(uint32_t offset, int len)
{
    int ret;
    uint32_t erase_sz = flash_erase_size();
    uint32_t page_start, sector_start;

//    if (!is_valid_flash_offset(offset)
//        || len <= 0 || !is_valid_flash_offset(offset + len - 1)) {
//        return -1;
//    }
#if defined GD32F5XX_BANKS_SWP_USED
    uint8_t run_img;
    /* bank switch set */
    sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &run_img);
    rcu_periph_clock_enable(RCU_SYSCFG);
    if (1 == run_img && ((SYSCFG_CFG0&SYSCFG_CFG0_FMC_SWP)!=0)) {
        if(offset<0x200000) {
            /* bank swap not effect when erase */
            offset = offset + 0x200000;
        } else if(offset<0x400000) {
            /* bank swap not effect when erase */
            offset = offset - 0x200000;
        } else {
        }
    }
#endif /* GD32F5XX_BANKS_SWP_USED */

    if (is_sip_flash()) {
        /* Get page start */
        page_start = FLASH_BASE + (offset & (~(erase_sz - 1)));

        /* unlock the flash program erase controller */
        fmc_unlock();

        /* clear pending flags */
        fmc_flag_clear(FMC_FLAG_END);
        fmc_flag_clear(FMC_FLAG_PGAERR);
        fmc_flag_clear(FMC_FLAG_WPERR);
        fmc_flag_clear(FMC_FLAG_PGSERR);

        while (len > 0) {
            /* erase page */
            ret = fmc_page_erase(page_start);
            /* clear pending flags */
            fmc_flag_clear(FMC_FLAG_END);
            fmc_flag_clear(FMC_FLAG_PGAERR);
            fmc_flag_clear(FMC_FLAG_WPERR);
            fmc_flag_clear(FMC_FLAG_PGSERR);

            if (ret != FMC_READY)
                return -2;
            page_start += erase_sz;
            len -= erase_sz;
        }
        /* lock the flash program erase controller */
        fmc_lock();
    }
    return 0;
}

int flash_erase_chip(void)
{
    int ret;

    return ret;
}

int flash_get_obstat(void *stat)
{
    return 0;
}

void flash_set_ob(uint32_t ob)
{
}

uint32_t flash_get_ob(void)
{
    return 0;
}

#if (IBL_VERSION >= V_1_1)
void flash_set_obusr(uint32_t obusr)
{
}

uint32_t flash_get_obusr(void)
{
//    return fmc_get_obusr();
    return 0;
}
#endif
void flash_set_secwm(uint32_t wmidx, uint32_t start_page, uint32_t end_page)
{
}

uint32_t flash_get_secwm(uint32_t wmidx)
{
    return 0;
}
#if (IBL_VERSION >= V_1_1)
void flash_set_hdpwm(uint32_t idx, uint32_t enable, uint32_t end_page)
{
}

uint32_t flash_get_hdpwm(uint32_t idx)
{
    return 0;
}

void flash_set_wrp(uint32_t idx, uint32_t start_page, uint32_t end_page)
{
}

uint32_t flash_get_wrp(uint32_t idx)
{
    return 0;
}
#endif

#ifdef ROM_SELF_TEST
void flash_self_test(void)
{
    uint32_t offset, offset_orig = 0x100000;
    uint8_t val_in[16], val_out[16];
    int i, ret, sum;

    /* erase page or sector */
    offset = offset_orig;
    ibl_printf("========== Erase 1st page at 0x%08x ===========\r\n", offset);
    ret = flash_erase(offset, flash_erase_size());
    if (ret != 0) {
        ibl_printf("Flash erase failed (%d)\r\n", ret);
        return;
    } else {
        ibl_printf("Flash erase OK\r\n");
    }

    /* write 1,2,4,15 bytes with aligned address */
    /* read 1,2,4,15 bytes with aligned address */
    ibl_printf("========== Write 1 byte (aligned addr) ===========\r\n");
    val_in[0] = 0x5A;
    ret = flash_write(offset, val_in, 1);
    if (ret != 0) {
        ibl_printf("Flash write 1 byte failed (%d)\r\n", ret);
        return;
    }
    ret = flash_read(offset, val_out, 1);
    if (ret != 0) {
        ibl_printf("Flash read 1 byte failed (%d)\r\n", ret);
        return;
    }
    if (val_in[0] != val_out[0]) {
        ibl_printf("Flash write 1 byte failed (0x%x vs 0x%x)\r\n", val_in[0], val_out[0]);
        return;
    } else {
        ibl_printf("Flash write 1 byte OK\r\n");
    }

    ibl_printf("========== Write 2 bytes (aligned addr) ===========\r\n");
    offset = offset_orig + 4;
    *(uint16_t*)val_in = 0x962d;
    ret = flash_write(offset, val_in, 2);
    if (ret != 0) {
        ibl_printf("Flash write 2 bytes failed (%d)\r\n", ret);
        return;
    }
    ret = flash_read(offset, val_out, 2);
    if (ret != 0) {
        ibl_printf("Flash read 2  bytes failed (%d)\r\n", ret);
        return;
    }
    if (*(uint16_t*)val_in != *(uint16_t*)val_out) {
        ibl_printf("Flash write 2 bytes failed (0x%x vs 0x%x)\r\n", *(uint16_t*)val_in, *(uint16_t*)val_out);
        return;
    } else {
        ibl_printf("Flash write 2 bytes OK\r\n");
    }

    ibl_printf("========== Write 4 bytes (aligned addr) ===========\r\n");
    offset = offset_orig + 8;
    *(uint32_t*)val_in = 0x3cb41e87;
    ret = flash_write(offset, val_in, 4);
    if (ret != 0) {
        ibl_printf("Flash write 4 bytes failed (%d)\r\n", ret);
        return;
    }
    ret = flash_read(offset, val_out, 4);
    if (ret != 0) {
        ibl_printf("Flash read 4 bytes failed (%d)\r\n", ret);
        return;
    }
    if (*(uint32_t*)val_in != *(uint32_t*)val_out) {
        ibl_printf("Flash write 4 bytes failed (0x%x vs 0x%x)\r\n", *(uint32_t*)val_in, *(uint32_t*)val_out);
        return;
    } else {
        ibl_printf("Flash write 4 bytes OK\r\n");
    }

    ibl_printf("========== Write 15 bytes (aligned addr) ===========\r\n");
    offset = offset_orig + 0x10;
    for (i = 0; i < 15; i++) {
        val_in[i] = i;
    }
    ret = flash_write(offset, val_in, 15);
    if (ret != 0) {
        ibl_printf("Flash write 15 bytes failed (%d)\r\n", ret);
        return;
    }
    ret = flash_read(offset, val_out, 15);
    if (ret != 0) {
        ibl_printf("Flash read 15  bytes failed (%d)\r\n", ret);
        return;
    }
    if (memcmp(val_in, val_out, 15)) {
        ibl_printf("Flash write 15  bytes failed\r\n");
        ibl_print_data(IBL_ALWAYS, "Value in:", val_in, 15);
        ibl_print_data(IBL_ALWAYS, "Value out:", val_out, 15);
        return;
    } else {
        ibl_printf("Flash write 15 bytes OK\r\n");
    }

    /* write 1,2,4,15 bytes with unaligned address */
    /* read 1,2,4,15 bytes with unaligned address */
    ibl_printf("========== Write 1 byte (unaligned addr) ===========\r\n");
    offset = offset_orig + 0x22;
    val_in[0] = 0x5A;
    ret = flash_write(offset, val_in, 1);
    if (ret != 0) {
        ibl_printf("Flash write 1 byte failed (%d)\r\n", ret);
        return;
    }
    ret = flash_read(offset, val_out, 1);
    if (ret != 0) {
        ibl_printf("Flash read 1 byte failed (%d)\r\n", ret);
        return;
    }
    if (val_in[0] != val_out[0]) {
        ibl_printf("Flash write 1 byte failed (0x%x vs 0x%x)\r\n", val_in[0], val_out[0]);
        return;
    } else {
        ibl_printf("Flash write 1 byte OK\r\n");
    }

    ibl_printf("========== Write 2 bytes (unaligned addr) ===========\r\n");
    offset = offset_orig + 0x23;
    *(uint16_t*)val_in = 0x962d;
    ret = flash_write(offset, val_in, 2);
    if (ret != 0) {
        ibl_printf("Flash write 2 bytes failed (%d)\r\n", ret);
        return;
    }
    ret = flash_read(offset, val_out, 2);
    if (ret != 0) {
        ibl_printf("Flash read 2 bytes failed (%d)\r\n", ret);
        return;
    }
    if (*(uint16_t*)val_in != *(uint16_t*)val_out) {
        ibl_printf("Flash write 2 bytes failed (0x%x vs 0x%x)\r\n", *(uint16_t*)val_in, *(uint16_t*)val_out);
        return;
    } else {
        ibl_printf("Flash write 2 bytes OK\r\n");
    }

    ibl_printf("========== Write 4 bytes (unaligned addr) ===========\r\n");
    offset = offset_orig + 0x25;
    *(uint32_t*)val_in = 0x3cb41e87;
    ret = flash_write(offset, val_in, 4);
    if (ret != 0) {
        ibl_printf("Flash write 4 bytes failed (%d)\r\n", ret);
        return;
    }
    ret = flash_read(offset, val_out, 4);
    if (ret != 0) {
        ibl_printf("Flash read 4 bytes failed (%d)\r\n", ret);
        return;
    }
    if (*(uint32_t*)val_in != *(uint32_t*)val_out) {
        ibl_printf("Flash write 4 bytes failed (0x%x vs 0x%x)\r\n", *(uint32_t*)val_in, *(uint32_t*)val_out);
        return;
    } else {
        ibl_printf("Flash write 4 bytes OK\r\n");
    }

    ibl_printf("========== Write 15 bytes (unaligned addr) ===========\r\n");
    offset = offset_orig + 0x29;
    for (i = 0; i < 15; i++) {
        val_in[i] = i;
    }
    ret = flash_write(offset, val_in, 15);
    if (ret != 0) {
        ibl_printf("Flash write 15 bytes failed (%d)\r\n", ret);
        return;
    }
    ret = flash_read(offset, val_out, 15);
    if (ret != 0) {
        ibl_printf("Flash read 15 bytes failed (%d)\r\n", ret);
        return;
    }
    if (memcmp(val_in, val_out, 15)) {
        ibl_printf("Flash write 15 bytes failed\r\n");
        ibl_print_data(IBL_ALWAYS, "Value in:", val_in, 15);
        ibl_print_data(IBL_ALWAYS, "Value out:", val_out, 15);
        return;
    } else {
        ibl_printf("Flash write 15 bytes OK\r\n");
    }

    ret = flash_erase(offset_orig, flash_erase_size());
    if (ret != 0) {
        ibl_printf("Flash erase failed (%d)\r\n", ret);
        return;
    } else {
        ibl_printf("Flash erase OK\r\n");
    }

    ibl_printf("========== Program non-secure page use SECCTL ===========\r\n");
    offset = offset_orig;
    *(uint32_t*)val_in = 0x3cb41e87;
    ret = flash_write(offset, val_in, 4);
    if (ret != 0) {
        ibl_printf("Program failed (%d)\r\n", ret);
        return;
    } else {
        ibl_printf("Program OK.\r\n");
    }
    ibl_printf("========== Erase non-secure page use SECCTL ===========\r\n");
    ret = flash_erase(offset, flash_erase_size());
    if (ret != 0) {
        ibl_printf("Erase failed (%d)\r\n", ret);
        return;
    } else {
        ibl_printf("Erase OK\r\n");
    }

}

void flash_speed_test(void)
{
    const uint32_t offset = 0x100000;
    const uint32_t erase_size = 0x100000;
    int i, ret;
    uint8_t data[512];

    /* erase page or sector */
    ibl_printf("Erase\r\n");
    ret = flash_erase(offset, erase_size);
    if (ret != 0) {
        ibl_printf("Flash erase failed (%d)\r\n", ret);
        return;
    } else {
        ibl_printf("OK\r\n");
    }
    /* write with aligned address */
    ibl_printf("Write aligned\r\n");
    for (i = offset; i < offset + erase_size; i += sizeof(data)) {
        ret = flash_write_fast(i, data, sizeof(data));
        if (ret != 0) {
            ibl_printf("Flash write failed (%d)\r\n", ret);
            return;
        }
    }
    ibl_printf("OK\r\n");

    /* read with aligned address */
    ibl_printf("Read aligned\r\n");
    for (i = offset; i < offset + erase_size; i += sizeof(data)) {
        ret = flash_read(i, data, sizeof(data));
        if (ret != 0) {
            ibl_printf("Flash read failed (%d)\r\n", ret);
            return;
        }
    }
    ibl_printf("OK\r\n");

    /* read indirect with aligned address */
    ibl_printf("Read indirect aligned\r\n");
    for (i = offset; i < offset + erase_size; i += sizeof(data)) {
        ret = flash_read_indirect(i, data, sizeof(data));
        if (ret != 0) {
            ibl_printf("Flash read indirect failed (%d)\r\n", ret);
            return;
        }
    }
    ibl_printf("OK\r\n");

    /* erase page or sector */
    ibl_printf("Erase\r\n");
    ret = flash_erase(offset, erase_size);
    if (ret != 0) {
        ibl_printf("Flash erase failed (%d)\r\n", ret);
        return;
    } else {
        ibl_printf("OK\r\n");
    }

    /* write with unaligned address */
    ibl_printf("Write unaligned\r\n");
    for (i = offset; i < offset + erase_size; i += sizeof(data)) {
        ret = flash_write_fast(i - 1, data, sizeof(data));
        if (ret != 0) {
            ibl_printf("Flash write failed (%d)\r\n", ret);
            return;
        }
    }
    ibl_printf("OK\r\n");

    /* read with unaligned address */
    ibl_printf("Read unaligned\r\n");
    for (i = offset; i < offset + erase_size; i += sizeof(data)) {
        ret = flash_read(i - 1, data, sizeof(data));
        if (ret != 0) {
            ibl_printf("Flash read failed (%d)\r\n", ret);
            return;
        }
    }
    ibl_printf("OK\r\n");

    /* read indirect with unaligned address */
    ibl_printf("Read indirect unaligned\r\n");
    for (i = offset; i < offset + erase_size; i += sizeof(data)) {
        ret = flash_read_indirect(i - 1, data, sizeof(data));
        if (ret != 0) {
            ibl_printf("Flash read indirect failed (%d)\r\n", ret);
            return;
        }
    }
    ibl_printf("OK\r\n");
}
#endif


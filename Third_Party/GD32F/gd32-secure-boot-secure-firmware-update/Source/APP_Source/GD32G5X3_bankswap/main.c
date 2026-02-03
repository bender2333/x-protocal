/*!
    \file    main.c
    \brief   running led

    \version 2023-10-16, V0.0.0, firmware for applicatiom
*/

/*
    Copyright (c) 2023, GigaDevice Semiconductor Inc.

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

#include "gd32g5x3.h"
#include "gd32g553q_eval.h"

#include "Platform/APP/app_region.h"
#include "bootutil/bootutil_public.h"
#include "flash_hal/flash_layout.h"
#include "include/storage/flash_map.h"
#include <stdio.h>

extern void shell_com_init(void);
extern void com_usart_init(void);
extern int flash_area_open(uint8_t id, const struct flash_area **area);
extern int boot_write_image_ok(const struct flash_area *fap);
extern int boot_write_trailer_flag(const struct flash_area *fap, uint32_t off, uint8_t flag_val);
static inline uint32_t boot_magic_off(const struct flash_area *fap);
extern int boot_write_trailer(const struct flash_area *fap, uint32_t off, const uint8_t *inbuf, uint8_t inlen);
extern  void shell_task(void);
/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/


int main(void)
{
    const struct flash_area *fap;
    SCB->VTOR = APP_CODE_START;
    com_usart_init();
    printf("jump success!");
    flash_area_open(FLASH_AREA_0_ID, &fap);
    boot_write_image_ok(fap);
    shell_com_init();

    while(1) {
        shell_task();
    }
}

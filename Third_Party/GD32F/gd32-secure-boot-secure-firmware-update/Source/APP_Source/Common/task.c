/*!
    \file    task.c
    \brief   task

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

#include "task.h"
#include "Source/IBL_Source/ibl_export.h"
#include "Utilities/Third_Party/letter-shell-shell2.x/shell.h"

uint32_t shell_task_flag = 0;
extern SHELL_TypeDef shell;

void ymodem_update(void);
void app_test_suit_init(void);
void tftp_update(void);
void can_update(void);

void shell_task(void)
{
#ifdef APP_TEST_SUIT
    if(shell_task_flag & TASK_FLAG_APPTEST) {
        app_test_suit_init();
        shell_task_flag &= ~TASK_FLAG_APPTEST;
        shellDisplay(&shell, shell.command);
    }
#endif /* APP_TEST_SUIT */

    if(shell_task_flag & TASK_FLAG_UPDATE) {
        ymodem_update();
        shell_task_flag &= ~TASK_FLAG_UPDATE;
    }

#ifdef USE_IAP_TFTP
    if(shell_task_flag & TASK_FLAG_TFTP_UPDATE) {
        tftp_update();
        shell_task_flag &= ~TASK_FLAG_TFTP_UPDATE;
    }
#endif
    
#ifdef USE_IAP_CAN
    if(shell_task_flag & TASK_FLAG_CAN_UPDATE) {
        can_update();
        shell_task_flag &= ~TASK_FLAG_CAN_UPDATE;
    }
#endif
}


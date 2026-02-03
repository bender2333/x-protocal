/*!
    \file    gd32g5x3_it.c
    \brief   interrupt service routines

    \version 2024-04-12, V0.1.2, firmware for GD32G5x3
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

#include "gd32g5x3_it.h"
#include "systick.h"

#include "Utilities/Third_Party/letter-shell-shell2.x/shell.h"
#include "Source/IBL_Source/ibl_export.h"
#include "Source/IBL_Source/ibl_def.h"
#include "Source/APP_Source/Common/task.h"

extern SHELL_TypeDef shell;
extern uint32_t shell_task_flag;
extern ymodem_packet_str ymodem_packet;

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
    /* if NMI exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{

}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
    /* if SVC exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
    /* if DebugMon exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
    /* if PendSV exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
    delay_decrement();
}

void USART0_IRQHandler()
{
    static uint32_t flag_cnt = 0;
    char getValue;
    if((shell_task_flag&TASK_FLAG_UPDATE) == 0){
        if (usart_flag_get(LOG_UART, USART_FLAG_RBNE) != RESET){
            getValue = usart_data_receive(LOG_UART);
            shellInput(&shell, getValue);
        }
    } else {
#if !defined GD32G5X3_BANKSWAP
        if(RESET != usart_interrupt_flag_get(LOG_UART, USART_INT_FLAG_RBNE)) {
            ((uint8_t *)(&ymodem_packet))[flag_cnt] = (uint8_t)usart_data_receive(LOG_UART);
            flag_cnt++;
            if (flag_cnt>YMODEM_PACKET_1K_SIZE+5) {
                flag_cnt = 0;
            }
            (&ymodem_packet)->ymodem_flag |= YMODEM_FLAG_ACT;
        }

        if(RESET != usart_interrupt_flag_get(LOG_UART, USART_INT_FLAG_IDLE)) {
            usart_interrupt_flag_clear(LOG_UART, USART_INT_FLAG_IDLE);
            flag_cnt = 0;
            if(((&ymodem_packet)->ymodem_flag&YMODEM_FLAG_STR) == 0) {
                if(((&ymodem_packet)->packet_flag==0x01||(&ymodem_packet)->packet_flag==0x02)){
                    (&ymodem_packet)->ymodem_flag |= YMODEM_FLAG_INTGET;
                }
            } else {
                (&ymodem_packet)->ymodem_flag |= YMODEM_FLAG_INTGET;
            }
        }
#endif
    }
}

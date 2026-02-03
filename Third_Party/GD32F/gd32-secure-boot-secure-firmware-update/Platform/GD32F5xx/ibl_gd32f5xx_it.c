/*!
    \file    gd32f5xx_it.c
    \brief   interrupt service routines

    \version 2023-10-16, V0.0.0, firmware for GD32F5xx
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

#include "ibl_gd32f5xx_it.h"
#include "Platform/IBL/ibl_systick.h"

#include "Source/IBL_Source/ibl_includes.h"
#include "Platform/IBL/ymodem.h"

extern ymodem_packet_str *ymodem_packet;

#define MULTI_ECC_ERROR_HANDLE(s)    do{ibl_trace(IBL_ERR, s);}while(1)
#define SINGLE_ECC_ERROR_HANDLE(s)   do{ibl_trace(IBL_ERR, s);}while(1)

/*!
    \brief    this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
    if((SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME0)) ||
            (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME1)) ||
            (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME2)) ||
            (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME3)) ||
            (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME4)) ||
            (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME5)) ||
            (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME6))) {
        MULTI_ECC_ERROR_HANDLE("SRAM or FLASH multi-bits non-correction ECC error\r\n");
    } else if((SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE0)) ||
              (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE1)) ||
              (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE2)) ||
              (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE3)) ||
              (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE4)) ||
              (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE5)) ||
              (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE6))) {
        SINGLE_ECC_ERROR_HANDLE("SRAM or FLASH single bit correction ECC error\r\n");
    } else {
        /* if NMI exception occurs, go to infinite loop */
        /* HXTAL clock monitor NMI error */
        while(1) {
        }
    }
}

void HardFault_Handler_c(unsigned int stack[])
{
    ibl_trace(IBL_ERR, "[Hard Fault Handler]\r\n");
    ibl_trace(IBL_ERR, "    R0 = 0x%08X\r\n", stack[0]);
    ibl_trace(IBL_ERR, "    R1 = 0x%08X\r\n", stack[1]);
    ibl_trace(IBL_ERR, "    R2 = 0x%08X\r\n", stack[2]);
    ibl_trace(IBL_ERR, "    R3 = 0x%08X\r\n", stack[3]);
    ibl_trace(IBL_ERR, "    R12 = 0x%08X\r\n", stack[4]);
    ibl_trace(IBL_ERR, "    LR = 0x%08X\r\n", stack[5]);
    ibl_trace(IBL_ERR, "    PC = 0x%08X\r\n", stack[6]);
    ibl_trace(IBL_ERR, "    PSR = 0x%08X\r\n", stack[7]);
    ibl_trace(IBL_ERR, "    BFAR = 0x%08X\r\n", SCB->BFAR);
    ibl_trace(IBL_ERR, "    CFSR = 0x%08X\r\n", SCB->CFSR);
    ibl_trace(IBL_ERR, "    HFSR = 0x%08X\r\n", SCB->HFSR);
    ibl_trace(IBL_ERR, "    DFSR = 0x%08X\r\n", SCB->DFSR);
    ibl_trace(IBL_ERR, "    AFSR = 0x%08X\r\n", SCB->AFSR);

    __ASM volatile("BKPT #01");

    while(1);
}

/*!
    \brief    this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    __asm(/*"PUBLIC Hard_Fault_Handler\n"*/
    "TST lr, #4\n"
    "ITE EQ\n"
    "MRSEQ r0, MSP\n"
    "MRSNE r0, PSP\n"
    "B HardFault_Handler_c");
}

/*!
    \brief    this function handles MemManage exception
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
    \brief    this function handles BusFault exception
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
    \brief    this function handles UsageFault exception
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
    \brief    this function handles SVC exception
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
    \brief    this function handles DebugMon exception
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
    \brief    this function handles PendSV exception
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
    \brief    this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
    delay_decrement();
}

/*!
    \brief      this function handles USART interrupt exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UART6_IRQHandler(void)
{
    static uint32_t flag_cnt = 0;
    if(RESET != usart_interrupt_flag_get(LOG_UART, USART_INT_FLAG_RBNE)) {
        ((uint8_t *)(ymodem_packet))[flag_cnt] = (uint8_t)usart_data_receive(LOG_UART);
        flag_cnt++;
        if (flag_cnt>YMODEM_PACKET_1K_SIZE+5) {
            flag_cnt = 0;
        }
        ymodem_packet->ymodem_flag |= YMODEM_FLAG_ACT;
    }
    if(RESET != usart_interrupt_flag_get(LOG_UART, USART_INT_FLAG_IDLE)) {
        usart_data_receive(LOG_UART);
        flag_cnt = 0;
        if((ymodem_packet->ymodem_flag&YMODEM_FLAG_STR) == 0) {
            if((ymodem_packet->packet_flag==0x01||ymodem_packet->packet_flag==0x02)){
                ymodem_packet->ymodem_flag |= YMODEM_FLAG_INTGET;
            }
        } else {
            ymodem_packet->ymodem_flag |= YMODEM_FLAG_INTGET;
        }
    }
}

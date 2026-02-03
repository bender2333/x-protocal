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

#include "gd32f5xx.h"
#include "gd32f5xx_it.h"
#include "../main.h"
#include "../systick.h"

#include "Utilities/Third_Party/letter-shell-shell2.x/shell.h"
#include "Source/IBL_Source/ibl_export.h"
#include "Source/IBL_Source/ibl_def.h"
#include "Source/APP_Source/Common/task.h"

#include "main.h"

#include "can_update.h"


extern void lwip_frame_recv(void);
extern void time_update(void);

extern SHELL_TypeDef shell;
extern uint32_t shell_task_flag;
extern ymodem_packet_str ymodem_packet;

#define MULTI_ECC_ERROR_HANDLE(s)    do{}while(1)
#define SINGLE_ECC_ERROR_HANDLE(s)   do{}while(1)

/*!
    \brief    this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
    if((SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME0)) ||
         (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME1)) ||
           (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME2)) ||
             (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME3)) ||
               (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME4)) ||
                 (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME5)) ||
                   (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME6))) {
                     MULTI_ECC_ERROR_HANDLE("SRAM or FLASH multi-bits non-correction ECC error\r\n");
    }else if((SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE0)) ||
               (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE1)) ||
                 (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE2)) ||
                   (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE3)) ||
                     (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE4)) ||
                       (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE5)) ||
                         (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE6))) {
                           SINGLE_ECC_ERROR_HANDLE("SRAM or FLASH single bit correction ECC error\r\n");
    }else{
        /* if NMI exception occurs, go to infinite loop */
        /* HXTAL clock monitor NMI error */
        while(1) {
        }
    }
}

/*!
    \brief    this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while(1) {
    }
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
    time_update();
}

/*!
    \brief      this function handles USART interrupt exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
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
        if(RESET != usart_interrupt_flag_get(LOG_UART, USART_INT_FLAG_RBNE)) {
            ((uint8_t *)(&ymodem_packet))[flag_cnt] = (uint8_t)usart_data_receive(LOG_UART);
            flag_cnt++;
            if (flag_cnt>YMODEM_PACKET_1K_SIZE+5) {
                flag_cnt = 0;
            }
            (&ymodem_packet)->ymodem_flag |= YMODEM_FLAG_ACT;
        }
        if(RESET != usart_interrupt_flag_get(LOG_UART, USART_INT_FLAG_IDLE)) {
            usart_data_receive(LOG_UART);
            flag_cnt = 0;
            if(((&ymodem_packet)->ymodem_flag&YMODEM_FLAG_STR) == 0) {
                if(((&ymodem_packet)->packet_flag==0x01||(&ymodem_packet)->packet_flag==0x02)){
                    (&ymodem_packet)->ymodem_flag |= YMODEM_FLAG_INTGET;
                }
            } else {
                (&ymodem_packet)->ymodem_flag |= YMODEM_FLAG_INTGET;
            }
        }
    }
}

#ifdef USE_ENET_INTERRUPT
/*!
    \brief      this function handles ethernet interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ENET_IRQHandler(void)
{
    uint32_t reval;

    /* clear the enet DMA Rx interrupt pending bits */
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_RS_CLR);
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_NI_CLR);
    /* handles all the received frames */
    do {
        reval = enet_rxframe_size_get();

        if(reval > 1) {
            lwip_frame_recv();
        }
    } while(reval != 0);


}
#endif /* USE_ENET_INTERRUPT */

extern can_receive_message_struct receive_message;
extern FlagStatus can0_receive_flag;

void CAN0_RX0_IRQHandler(void)
{
    can_message_receive(CAN0,CAN_FIFO0, &receive_message);
    //对接收到的数据先做简单的判断，去掉错误的数据
    if(MSG_ID_TYPE == CAN_FF_STANDARD){
        if((receive_message.rx_ff == CAN_FF_STANDARD)&&     //消息类型必须匹配
        (receive_message.rx_sfid == MSG_RECEIVE_ID)&&       //消息ID必须匹配
        (receive_message.rx_dlen == 8)&&                      //数据长度必须为8
        ((receive_message.rx_data[0]==GetNAD()))||(receive_message.rx_data[0]==NAD_BROADCAST)){//接点地址必须相等或者为广播地址
            can0_receive_flag = SET; 
        }
    }else{
        if((receive_message.rx_ff == CAN_FF_EXTENDED)&&     //消息类型必须匹配
        (receive_message.rx_efid == MSG_RECEIVE_ID)&&       //消息ID必须匹配
        (receive_message.rx_dlen == 8)&&                      //数据长度必须为8
        ((receive_message.rx_data[0]==GetNAD())||(receive_message.rx_data[0]==NAD_BROADCAST))){//接点地址必须相等或者为广播地址
            can0_receive_flag = SET; 
        }
    }
}

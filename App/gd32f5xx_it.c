#ifdef GD32F527

/*!
    \file    gd32f5xx_it.c
    \brief   interrupt service routines

    \version 2024-12-20, V1.2.0, firmware for GD32F5xx
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

#include "gd32f5xx_it.h"
#include "drv_usbd_int.h"
//#include "systick.h"
//#include "mekiLED.h"
//#include "mekiTimer.h"
//extern usb_core_driver cdc_acm;
extern void usb_timer_irq(void);

#define MULTI_ECC_ERROR_HANDLE(s) \
    do {                          \
    } while (1)
#define SINGLE_ECC_ERROR_HANDLE(s) \
    do {                           \
    } while (1)

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
    if ((SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME0)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME1)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME2)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME3)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME4)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME5)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME6))) {
        MULTI_ECC_ERROR_HANDLE("SRAM or FLASH multi-bits non-correction ECC error\r\n");
    } else {
        /* if NMI exception occurs, go to infinite loop */
        /* HXTAL clock monitor NMI error */
        while (1) {
        }
    }
}

/*!
    \brief      this function handles SRAM and Flash single bit ECC non-correction exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SYSCFG_SINGLE_BIT_ECC_ER_IRQHandler(void)
{
    if ((SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE0)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE1)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE2)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE3)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE4)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE5)) ||
        (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE6))) {
        MULTI_ECC_ERROR_HANDLE("SRAM or FLASH single bit ECC error\r\n");
    }
}

/*!
    \brief      this function handles FPU exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void FPU_IRQHandler(void)
{
    /* if FPU exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
/* 简易 HardFault 调试输出 (轮询方式, 不依赖中断) */
static void hardfault_putc(char ch) {
    while ((USART_STAT0(USART2) & USART_STAT0_TBE) == 0);
    USART_DATA(USART2) = ch;
}
static void hardfault_puts(const char *s) {
    while (*s) hardfault_putc(*s++);
}

void HardFault_Handler(void)
{
    /* 尝试输出调试信息 */
    hardfault_puts("\r\n!!! HARDFAULT !!!\r\n");
    
    /* 获取故障地址 (如果可用) */
    volatile uint32_t *sp;
    __asm volatile ("mrs %0, msp" : "=r" (sp));
    
    /* 尝试打印 PC 值 */
    hardfault_puts("SP: 0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t nib = ((uint32_t)sp >> (i * 4)) & 0xF;
        hardfault_putc(nib < 10 ? '0' + nib : 'A' + nib - 10);
    }
    hardfault_puts("\r\n");
    
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1) {
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
    while (1) {
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
    while (1) {
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
    while (1) {
    }
}

/*!
    \brief    this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/

// void SVC_Handler(void)
// {
//     /* if SVC exception occurs, go to infinite loop */
//     while (1) {
//     }
// }

/*!
    \brief    this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
    /* if DebugMon exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief    this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/

// void PendSV_Handler(void)
// {
//     /* if PendSV exception occurs, go to infinite loop */
//     while (1) {
//     }
// }

/*!
    \brief    this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void dev_systick_IntHandler();
void vApplicationTickHook(void)
{
    dev_systick_IntHandler();
}
// void SysTick_Handler(void)
// {
//     delay_decrement();
// }

void TIMER1_IRQHandler(void)
{
//    if (Timer_Callbacks.TIMER1_Callback != NULL) {
//        Timer_Callbacks.TIMER1_Callback();
//    }
}

void TIMER2_IRQHandler(void)
{
    // if (Timer_Callbacks.TIMER2_Callback != NULL) {
    //     Timer_Callbacks.TIMER2_Callback();
    // }
    usb_timer_irq();
}

void dev_timer3_interrupt();
void TIMER3_IRQHandler(void)
{
    dev_timer3_interrupt();
}

void dev_timer6_interrupt();
void TIMER6_IRQHandler(void)
{
    dev_timer6_interrupt();
}

void dev_ai_isr(void);
void DMA1_Channel0_IRQHandler(void)
{
    dev_ai_isr();
}

void dev_uart_isr1();
void USART1_IRQHandler(void)
{
  dev_uart_isr1();
}

void dev_uart_isr5();
void USART5_IRQHandler(void)
{
  dev_uart_isr5();
}

/* TPMesh 调试输出使用 USART2 */
extern void tpmesh_debug_irq_handler(void);
void USART2_IRQHandler(void)
{
  tpmesh_debug_irq_handler();
}

/* TPMesh AT 命令使用 UART6 (中断RX) */
extern void tpmesh_uart6_irq_handler(void);
void UART6_IRQHandler(void)
{
  tpmesh_uart6_irq_handler();
}


#ifdef USE_USB_FS

/*!
    \brief      this function handles USBFS global interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern usb_core_driver cdc_acm;
extern usb_core_driver msc_udisk;
void USBFS_IRQHandler(void)
{
    usbd_isr(&cdc_acm);
    // usbd_isr(&msc_udisk);
}
#endif

#include "gd32f5xx_enet.h"
extern void lwip_frame_recv();
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

#include "PulseAccum.h"
static int i = 0;
#ifdef DEBUG_AI_RESPONSE_TIME
void dev_timer4_interrupt();
#endif
void TIMER4_IRQHandler(void)
{
#ifndef DEBUG_AI_RESPONSE_TIME
    timer_flag_clear(TIMER4, TIMER_FLAG_UP);

    i++;
    if(i<=100)
      return;
    
    gpio_bit_toggle(GPIOC, GPIO_PIN_15);
    if(i>=20100)
    {
      i=0;
      timer_interrupt_disable(TIMER4, TIMER_INT_UP);
    }
#else
    dev_timer4_interrupt();
#endif
}
#endif
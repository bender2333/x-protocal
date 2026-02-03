/*!
    \file    ibl_uart.c
    \brief   IBL uart configure for for GD32 SDK

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

void uart_putc(uint8_t c)
{
    while (RESET == usart_flag_get(LOG_UART, USART_FLAG_TBE));
    usart_data_transmit(LOG_UART, (uint8_t)c);
    while (RESET == usart_flag_get(LOG_UART, USART_FLAG_TBE));
}

char uart_rx_ch(void)
{
    uint8_t rx_char = '\0';
    if ((RESET != usart_interrupt_flag_get(LOG_UART, USART_INT_FLAG_RBNE)) &&
        (RESET != usart_flag_get(LOG_UART, USART_FLAG_RBNE))) {
        rx_char = (uint8_t)usart_data_receive(LOG_UART);
        if (RESET != usart_flag_get(LOG_UART, USART_FLAG_ORERR)) {
            usart_flag_clear(LOG_UART, USART_FLAG_ORERR);
        }
    }
    return (char)rx_char;
}

//uint8_t uart_rx_to(uint8_t *buf, uint16_t len, uint32_t timeout)
//{
//    uint32_t i = 0, uart_state = 0;
//    uint8_t get_state = 0;
//    uint32_t time_start = 0, time_end = timeout, time_cnt = 0;

//    for(i=0; i<len; i++) {
//        time_start = i;
//        time_end = timeout;
//        do {
//#if defined(PLATFORM_GD32F5XX) 
//            uart_state = USART_STAT0(LOG_UART);
//#elif defined(PLATFORM_GD32H7XX)
//            uart_state = USART_STAT(LOG_UART);
//#endif
//            if (0 == get_state) {
//                if(time_end<=0) {
//                    return 0;
//                } else {
//                    time_end--;
//                }
//            } else {
//                if (RESET != (uart_state&USART_CTL0_IDLEIE)) {
//#if defined(PLATFORM_GD32F5XX) 
//                    get_state = (uint8_t)usart_data_receive(LOG_UART);
//#elif defined(PLATFORM_GD32H7XX)
//                    get_state = (uint8_t)usart_data_receive(LOG_UART);
//                    usart_flag_clear(LOG_UART, USART_FLAG_IDLE);
//#endif /* PLATFORM_GD32H7XX */
//                    return 1;
//                }
//                if (time_start == i) {
//                    time_cnt++;
//                    if(time_cnt>=time_end ) {
//                        return 0;
//                    }
//                } else {
//                    time_cnt = 0;
//                }
//            }
//        } while(RESET == (uart_state&USART_CTL0_RBNEIE));

//        buf[i] = (uint8_t)usart_data_receive(LOG_UART);
//        get_state = 1;
//        if (RESET != usart_flag_get(LOG_UART, USART_FLAG_ORERR)) {
//            usart_flag_clear(LOG_UART, USART_FLAG_ORERR);
//        }
//    }

//    return 1;
//}

uint8_t uart_rx_to(ymodem_packet_str *ymodem_packet, uint16_t len, uint32_t timeout)
{
    static uint32_t i = 0;
    uint8_t get_state = 0;
    uint32_t time_end = timeout;
    
    if(i==0 && (ymodem_packet->ymodem_flag &YMODEM_FLAG_ACT)!=0) {
        i = 1;
        return 1;
    }

    while((ymodem_packet->ymodem_flag&YMODEM_FLAG_INTGET) == 0 && time_end>0) {
        time_end--;
    }
    
    if (time_end>0) {
        ymodem_packet->ymodem_flag &= ~YMODEM_FLAG_INTGET;
        return 1;
    } else {
        return 0;
    }
}

void uart_config(uint32_t usart_periph)
{
    uint32_t gpio_speed = GPIO_OSPEED_50MHZ;

    if (usart_periph == UART6) {
        rcu_periph_clock_enable(RCU_UART6);
        rcu_periph_clock_enable(RCU_GPIOF);

        gpio_af_set(GPIOF, GPIO_AF_8, GPIO_PIN_7); //Todo 
        gpio_af_set(GPIOF, GPIO_AF_8, GPIO_PIN_6);

        gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
        gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, gpio_speed, GPIO_PIN_7);
        gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
        gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, gpio_speed, GPIO_PIN_6);

				gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
        gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, gpio_speed, GPIO_PIN_8);
				gpio_bit_set(GPIOF, GPIO_PIN_8);
			
        nvic_irq_enable(UART6_IRQn, 0, 0);
    } else {
        return;
    }

    usart_deinit(usart_periph);
    usart_baudrate_set(usart_periph, 115200U);
    usart_parity_config(usart_periph, USART_PM_NONE);
    usart_word_length_set(usart_periph, USART_WL_8BIT);
    usart_stop_bit_set(usart_periph, USART_STB_1BIT);
    usart_receive_config(usart_periph, USART_RECEIVE_ENABLE);
    usart_transmit_config(usart_periph, USART_TRANSMIT_ENABLE);

//    usart_interrupt_enable(usart_periph, USART_INT_RBNE);
//    usart_interrupt_enable(usart_periph, USART_INT_IDLE);
    usart_enable(usart_periph);
}

void log_uart_init(void)
{
    uart_config(LOG_UART);
}

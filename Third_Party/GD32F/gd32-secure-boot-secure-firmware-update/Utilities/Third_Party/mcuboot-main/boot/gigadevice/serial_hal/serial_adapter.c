/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include <bootutil/bootutil_log.h>
#include "mcuboot_config/mcuboot_config.h"

#if defined GD32W515PI
#include "gd32w51x.h"
#elif defined GD32F527 
#include "gd32f5xx.h"
#endif /* GD32F527 */

//#include <esp_rom_uart.h>
//#include <esp_rom_gpio.h>
//#include <esp_rom_sys.h>
//#include <esp_rom_caps.h>
//#include <soc/uart_periph.h>
//#include <soc/gpio_struct.h>
//#include <soc/io_mux_reg.h>
//#include <soc/rtc.h>
//#include <hal/gpio_types.h>
//#include <hal/gpio_ll.h>
//#include <hal/uart_ll.h>
//#include <hal/clk_gate_ll.h>
//#include <hal/gpio_hal.h>

#if defined CONFIG_MCUBOOT_SERIAL

#define SERIAL_COM USART0

//static uart_dev_t *serial_boot_uart_dev = (SERIAL_BOOT_UART_NUM == 0) ?
//                                          &UART0 :
//                                          &UART1;

void console_write(const char *str, int cnt)
{
    uint32_t tx_cnt;

    for(tx_cnt=0; tx_cnt<cnt; tx_cnt++) {
        usart_data_transmit(SERIAL_COM, (uint8_t)str[tx_cnt]);
        while(RESET == usart_flag_get(SERIAL_COM, USART_FLAG_TBE)){
        }
    }
}

int console_read(char *str, int cnt, int *newline)
{
    uint32_t rx_cnt = 0;

    for(rx_cnt=0; rx_cnt<cnt; ) {
        while(usart_flag_get(SERIAL_COM, USART_FLAG_RBNE) != RESET){
            str[rx_cnt] = usart_data_receive(SERIAL_COM);
            rx_cnt++;
        }
        if('\n' == str[rx_cnt-1] || rx_cnt == cnt - 1) {
            break;
        }
    }
    *newline = (str[rx_cnt - 1] == '\n') ? 1 : 0;
    return rx_cnt;
}

int boot_console_init(void)
{
//    BOOT_LOG_INF("Initializing serial boot pins");

    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(SERIAL_COM);
    usart_baudrate_set(SERIAL_COM, 115200U);
    usart_transmit_config(SERIAL_COM, USART_TRANSMIT_ENABLE);
    usart_receive_config(SERIAL_COM, USART_RECEIVE_ENABLE);
    
////    usart_interrupt_enable(SHELL_COM, USART_INT_RBNE);  /* enable recive not empty interrupt */
//    usart_interrupt_enable(SHELL_COM, USART_INTEN_IDLEIE);  /* enable idle interrupt */
////    nvic_irq_enable(USART0_IRQn, 2, 0);
    
    usart_enable(SERIAL_COM);

    return 0;
}

/* CRC-16/XMODEM */
uint16_t crc16_ccitt(uint16_t crc_init, char *addr, int len)
{
    uint8_t data;
    uint16_t crc = crc_init;
    int i;
    for (; len > 0; len--) {
        data = *addr++;

        crc = crc ^ (data << 8);
        for (i = 0; i < 8; i++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }

    crc = crc^0x00;
    return crc; 
}

//bool boot_serial_detect_pin(void)
//{
//    bool detected = false;
//    int pin_value = 0;

//    esp_rom_gpio_pad_select_gpio(SERIAL_BOOT_GPIO_DETECT);
//    gpio_ll_input_enable(&GPIO, SERIAL_BOOT_GPIO_DETECT);
//    switch (SERIAL_BOOT_GPIO_INPUT_TYPE) {
//        // Pull-down
//        case 0:
//            gpio_ll_pulldown_en(&GPIO, SERIAL_BOOT_GPIO_DETECT);
//            break;
//        // Pull-up
//        case 1:
//            gpio_ll_pullup_en(&GPIO, SERIAL_BOOT_GPIO_DETECT);
//            break;
//    }
//    esp_rom_delay_us(50000);

//    pin_value = gpio_ll_get_level(&GPIO, SERIAL_BOOT_GPIO_DETECT);
//    detected = (pin_value == SERIAL_BOOT_GPIO_DETECT_VAL);
//    esp_rom_delay_us(50000);

//    if (detected) {
//        if (SERIAL_BOOT_DETECT_DELAY_S > 0) {
//            /* The delay time is an approximation */
//            for (int i = 0; i < (SERIAL_BOOT_DETECT_DELAY_S * 100); i++) {
//                esp_rom_delay_us(10000);
//                pin_value = gpio_ll_get_level(&GPIO, SERIAL_BOOT_GPIO_DETECT);
//                detected = (pin_value == SERIAL_BOOT_GPIO_DETECT_VAL);
//                if (!detected) {
//                    break;
//                }
//            }
//        }
//    }
//    return detected;
//}

uint16_t serial_htons(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

uint16_t serial_ntohs(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

void serial_delay_us(uint32_t cnt)
{
}

void serial_system_reset(void)
{
    /* generate a system reset */
    NVIC_SystemReset();
}

#endif /* CONFIG_MCUBOOT_SERIAL */

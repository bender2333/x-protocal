/**
 * @file tpmesh_debug.c
 * @brief TPMesh 调试输出模块实现
 * 
 * 将 printf 重定向到 USART2 (轮询 + 中断混合模式)
 * 
 * @version 0.6.5
 */

#include "tpmesh_debug.h"
#include "gd32f5xx.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* ============================================================================
 * 配置
 * ============================================================================ */

/** 调试 UART 波特率 */
#define DEBUG_UART_BAUD     115200

/** USART2 外设地址 */
#define DEBUG_USART         USART2

/** 使用轮询模式 (1=轮询, 0=中断) - 调试阶段建议用轮询 */
#define USE_POLLING_MODE    1

/** 发送超时计数 */
#define TX_TIMEOUT          0x1FFFF

/* ============================================================================
 * 私有变量
 * ============================================================================ */

/** 初始化状态 */
static volatile int s_initialized = 0;

/* ============================================================================
 * 轮询发送 (简单可靠)
 * ============================================================================ */

/**
 * @brief 轮询发送单个字节
 */
static void usart2_send_byte_polling(uint8_t ch)
{
    uint32_t timeout = TX_TIMEOUT;
    
    /* 等待发送缓冲区空 */
    while ((RESET == usart_flag_get(DEBUG_USART, USART_FLAG_TBE)) && (timeout > 0)) {
        timeout--;
    }
    
    if (timeout > 0) {
        usart_data_transmit(DEBUG_USART, ch);
    }
}

/* ============================================================================
 * 公共函数
 * ============================================================================ */

int tpmesh_debug_init(void)
{
    if (s_initialized) {
        return 0;
    }

    /* 使能时钟 */
    rcu_periph_clock_enable(RCU_USART2);
    rcu_periph_clock_enable(RCU_GPIOC);

    /* 配置 GPIO: PC10=TX, PC11=RX */
    gpio_af_set(GPIOC, GPIO_AF_7, GPIO_PIN_10);
    gpio_af_set(GPIOC, GPIO_AF_7, GPIO_PIN_11);
    
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_11);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* 复位 USART */
    usart_deinit(DEBUG_USART);

    /* 配置 USART: 115200, 8N1 */
    usart_baudrate_set(DEBUG_USART, DEBUG_UART_BAUD);
    usart_word_length_set(DEBUG_USART, USART_WL_8BIT);
    usart_stop_bit_set(DEBUG_USART, USART_STB_1BIT);
    usart_parity_config(DEBUG_USART, USART_PM_NONE);
    usart_hardware_flow_rts_config(DEBUG_USART, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(DEBUG_USART, USART_CTS_DISABLE);
    usart_receive_config(DEBUG_USART, USART_RECEIVE_ENABLE);
    usart_transmit_config(DEBUG_USART, USART_TRANSMIT_ENABLE);

    /* 使能 USART */
    usart_enable(DEBUG_USART);

    s_initialized = 1;
    
    /* 发送启动消息验证 */
    const char *msg = "\r\n[DBG] USART2 Ready\r\n";
    while (*msg) {
        usart2_send_byte_polling((uint8_t)*msg++);
    }
    
    return 0;
}

void tpmesh_debug_putc(char ch)
{
    if (!s_initialized) {
        return;
    }
    
    /* 处理换行 */
    if (ch == '\n') {
        usart2_send_byte_polling('\r');
    }
    usart2_send_byte_polling((uint8_t)ch);
}

void tpmesh_debug_puts(const char *str)
{
    if (!s_initialized || str == NULL) {
        return;
    }
    
    while (*str) {
        tpmesh_debug_putc(*str++);
    }
}

int tpmesh_debug_printf(const char *fmt, ...)
{
    if (!s_initialized) {
        return 0;
    }

    char buf[256];
    va_list args;
    
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len > 0) {
        for (int i = 0; i < len; i++) {
            tpmesh_debug_putc(buf[i]);
        }
    }

    return len;
}

void tpmesh_debug_flush(void)
{
    if (!s_initialized) {
        return;
    }
    
    /* 等待发送完成 */
    uint32_t timeout = TX_TIMEOUT;
    while ((RESET == usart_flag_get(DEBUG_USART, USART_FLAG_TC)) && (timeout > 0)) {
        timeout--;
    }
}

/**
 * @brief USART2 中断处理函数 (当前轮询模式下为空)
 */
void tpmesh_debug_irq_handler(void)
{
    /* 接收中断 - 读取并丢弃，防止溢出 */
    if (usart_interrupt_flag_get(DEBUG_USART, USART_INT_FLAG_RBNE) != RESET) {
        (void)usart_data_receive(DEBUG_USART);
    }
    
    /* 清除溢出错误 */
    if (usart_interrupt_flag_get(DEBUG_USART, USART_INT_FLAG_ERR_ORERR) != RESET) {
        usart_interrupt_flag_clear(DEBUG_USART, USART_INT_FLAG_ERR_ORERR);
    }
}

/* ============================================================================
 * printf 重定向 (ARM GCC / Keil / IAR)
 * ============================================================================ */

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
/* GCC: 使用 __io_putchar (与 syscalls.c 配合) */
int __io_putchar(int ch)
{
    tpmesh_debug_putc((char)ch);
    return ch;
}

/* 备用: 某些配置使用 _putchar */
int _putchar(int ch)
{
    tpmesh_debug_putc((char)ch);
    return ch;
}

#elif defined(__ARMCC_VERSION)
/* Keil MDK: 重定向 fputc */
int fputc(int ch, FILE *f)
{
    (void)f;
    tpmesh_debug_putc((char)ch);
    return ch;
}

#elif defined(__ICCARM__)
/* IAR: 重定向 __write */
size_t __write(int handle, const unsigned char *buf, size_t bufSize)
{
    (void)handle;
    
    if (!s_initialized) {
        return bufSize;
    }
    
    for (size_t i = 0; i < bufSize; i++) {
        tpmesh_debug_putc((char)buf[i]);
    }
    
    return bufSize;
}
#endif

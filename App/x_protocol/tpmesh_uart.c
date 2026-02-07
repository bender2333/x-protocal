/**
 * @file tpmesh_uart.c
 * @brief TPMesh UART6 驱动实现 (轮询TX + 中断RX)
 *
 * 使用 GD32 HAL, 无 RTOS 依赖
 *
 * @version 1.0.0
 */

#include "tpmesh_uart.h"
#include "tpmesh_debug.h"
#include "gd32f5xx.h"
#include <string.h>

/* ============================================================================
 * 硬件配置
 * ============================================================================ */

#define UART6_PERIPH        UART6
#define UART6_CLK           RCU_UART6
#define UART6_GPIO_CLK      RCU_GPIOF
#define UART6_GPIO_PORT     GPIOF
#define UART6_TX_PIN        GPIO_PIN_7
#define UART6_RX_PIN        GPIO_PIN_6
#define UART6_GPIO_AF       GPIO_AF_8
#define UART6_IRQ           UART6_IRQn
#define UART6_IRQ_PRIO      5

/* ============================================================================
 * 环形缓冲区
 * ============================================================================ */

#define RX_BUF_MASK         (TPMESH_UART6_RX_BUF_SIZE - 1)

static volatile uint8_t  s_rx_buf[TPMESH_UART6_RX_BUF_SIZE];
static volatile uint16_t s_rx_head;   /* ISR 写入位置 */
static volatile uint16_t s_rx_tail;   /* 用户读取位置 */

static volatile bool s_initialized = false;

/* ============================================================================
 * 内部函数
 * ============================================================================ */

static inline uint16_t rx_count(void)
{
    return (uint16_t)((s_rx_head - s_rx_tail) & RX_BUF_MASK);
}

/* ============================================================================
 * 公共函数
 * ============================================================================ */

int tpmesh_uart6_init(void)
{
    if (s_initialized) {
        return 0;
    }

    /* ---- GPIO ---- */
    rcu_periph_clock_enable(UART6_GPIO_CLK);

    /* TX: PF7 - AF, PP, 50MHz */
    gpio_af_set(UART6_GPIO_PORT, UART6_GPIO_AF, UART6_TX_PIN);
    gpio_mode_set(UART6_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, UART6_TX_PIN);
    gpio_output_options_set(UART6_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, UART6_TX_PIN);

    /* RX: PF6 - AF, PU */
    gpio_af_set(UART6_GPIO_PORT, UART6_GPIO_AF, UART6_RX_PIN);
    gpio_mode_set(UART6_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, UART6_RX_PIN);

    /* ---- UART 外设 ---- */
    rcu_periph_clock_enable(UART6_CLK);
    usart_deinit(UART6_PERIPH);
    usart_baudrate_set(UART6_PERIPH, TPMESH_UART6_BAUD);
    usart_word_length_set(UART6_PERIPH, USART_WL_8BIT);
    usart_stop_bit_set(UART6_PERIPH, USART_STB_1BIT);
    usart_parity_config(UART6_PERIPH, USART_PM_NONE);
    usart_hardware_flow_rts_config(UART6_PERIPH, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(UART6_PERIPH, USART_CTS_DISABLE);
    usart_receive_config(UART6_PERIPH, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART6_PERIPH, USART_TRANSMIT_ENABLE);
    usart_enable(UART6_PERIPH);

    /* ---- RX 缓冲区 ---- */
    s_rx_head = 0;
    s_rx_tail = 0;

    /* ---- 中断: 仅 RBNE + ERR ---- */
    nvic_irq_enable(UART6_IRQ, UART6_IRQ_PRIO, 0);
    usart_interrupt_enable(UART6_PERIPH, USART_INT_RBNE);
    usart_interrupt_enable(UART6_PERIPH, USART_INT_ERR);

    s_initialized = true;

    /* ---- 寄存器诊断 (打印到 USART2 调试口) ---- */
    {
        uint32_t apb1 = rcu_clock_freq_get(CK_APB1);
        uint32_t baud_reg = USART_BAUD(UART6_PERIPH);
        uint32_t ctl0_reg = USART_CTL0(UART6_PERIPH);
        uint32_t ctl1_reg = USART_CTL1(UART6_PERIPH);

        /* 反算实际波特率 */
        uint32_t intdiv  = baud_reg & 0xFFF0U;
        uint32_t fradiv  = baud_reg & 0x000FU;
        uint32_t actual_baud = 0;
        if (ctl0_reg & (1U << 15U)) {
            /* OVSMOD=1: 8x oversampling */
            uint32_t udiv = intdiv | ((fradiv & 0x7U) << 1);
            if (udiv > 0) actual_baud = (2U * apb1) / udiv;
        } else {
            /* OVSMOD=0: 16x oversampling */
            uint32_t udiv = intdiv + fradiv;
            if (udiv > 0) actual_baud = apb1 / udiv;
        }

        tpmesh_debug_printf("\n==== UART6 Diagnostics ====\n");
        tpmesh_debug_printf("  APB1 clock:  %lu Hz\n", (unsigned long)apb1);
        tpmesh_debug_printf("  BAUD reg:    0x%04lX\n", (unsigned long)baud_reg);
        tpmesh_debug_printf("  CTL0 reg:    0x%08lX\n", (unsigned long)ctl0_reg);
        tpmesh_debug_printf("  CTL1 reg:    0x%08lX\n", (unsigned long)ctl1_reg);
        tpmesh_debug_printf("  OVSMOD:      %s\n",
                            (ctl0_reg & (1U << 15U)) ? "8x" : "16x");
        tpmesh_debug_printf("  Target baud: %lu\n", (unsigned long)TPMESH_UART6_BAUD);
        tpmesh_debug_printf("  Actual baud: %lu\n", (unsigned long)actual_baud);
        tpmesh_debug_printf("  INTDIV=%lu FRADIV=%lu\n",
                            (unsigned long)(intdiv >> 4), (unsigned long)fradiv);
        tpmesh_debug_printf("===========================\n\n");
    }

    /* 发送测试: 先发 0x55('U') 10次方便示波器/逻辑分析仪验证波特率 */
    for (int i = 0; i < 10; i++) {
        uint32_t t = 0xFFFF;
        while ((RESET == usart_flag_get(UART6_PERIPH, USART_FLAG_TBE)) && (--t > 0));
        usart_data_transmit(UART6_PERIPH, 0x55U);
    }
    /* 等待最后一字节发完 */
    {
        uint32_t t = 0xFFFF;
        while ((RESET == usart_flag_get(UART6_PERIPH, USART_FLAG_TC)) && (--t > 0));
    }

    /* 发送可读测试字符串 */
    tpmesh_uart6_puts("\r\n[UART6] Ready\r\n");
    tpmesh_debug_printf("UART6: Init OK (PF7/PF6, %d baud)\n", TPMESH_UART6_BAUD);

    return 0;
}

void tpmesh_uart6_deinit(void)
{
    if (!s_initialized) {
        return;
    }
    usart_interrupt_disable(UART6_PERIPH, USART_INT_RBNE);
    usart_interrupt_disable(UART6_PERIPH, USART_INT_ERR);
    nvic_irq_disable(UART6_IRQ);
    usart_disable(UART6_PERIPH);
    s_initialized = false;
}

int tpmesh_uart6_send(const uint8_t *data, uint16_t len)
{
    if (!s_initialized) {
        return -3;
    }
    if (data == NULL || len == 0) {
        return -2;
    }

    for (uint16_t i = 0; i < len; i++) {
        uint32_t timeout = TPMESH_UART6_TX_TIMEOUT;
        while ((RESET == usart_flag_get(UART6_PERIPH, USART_FLAG_TBE)) && (--timeout > 0));
        if (timeout == 0) {
            return -1;
        }
        usart_data_transmit(UART6_PERIPH, data[i]);
    }

    /* 等待最后一字节发送完成 */
    uint32_t timeout = TPMESH_UART6_TX_TIMEOUT;
    while ((RESET == usart_flag_get(UART6_PERIPH, USART_FLAG_TC)) && (--timeout > 0));

    return 0;
}

int tpmesh_uart6_puts(const char *str)
{
    if (str == NULL) {
        return -2;
    }
    return tpmesh_uart6_send((const uint8_t *)str, (uint16_t)strlen(str));
}

int tpmesh_uart6_getc(uint8_t *out)
{
    if (!s_initialized || out == NULL) {
        return -1;
    }
    if (s_rx_head == s_rx_tail) {
        return -1;  /* 无数据 */
    }
    *out = s_rx_buf[s_rx_tail];
    s_rx_tail = (s_rx_tail + 1) & RX_BUF_MASK;
    return 0;
}

int tpmesh_uart6_getc_timeout(uint8_t *out, uint32_t timeout_ms)
{
    if (timeout_ms == 0) {
        return tpmesh_uart6_getc(out);
    }

    /*
     * 简易超时: 每次循环约 1us (取决于主频),
     * timeout_ms * 1000 次循环 ≈ timeout_ms 毫秒
     */
    uint32_t loops = timeout_ms * 1000U;
    while (loops-- > 0) {
        if (tpmesh_uart6_getc(out) == 0) {
            return 0;
        }
        /* 短延时 (~1us @200MHz: 约 200 个 nop) */
        for (volatile int d = 0; d < 50; d++);
    }
    return -1;  /* 超时 */
}

uint16_t tpmesh_uart6_rx_available(void)
{
    if (!s_initialized) {
        return 0;
    }
    return rx_count();
}

void tpmesh_uart6_rx_flush(void)
{
    if (!s_initialized) {
        return;
    }
    s_rx_tail = s_rx_head;
}

/* ============================================================================
 * 中断处理
 * ============================================================================ */

void tpmesh_uart6_irq_handler(void)
{
    if (!s_initialized) {
        return;
    }

    /* ---- RBNE: 收到数据 ---- */
    if (usart_interrupt_flag_get(UART6_PERIPH, USART_INT_FLAG_RBNE) != RESET) {
        uint8_t ch = (uint8_t)usart_data_receive(UART6_PERIPH);
        uint16_t next = (s_rx_head + 1) & RX_BUF_MASK;
        if (next != s_rx_tail) {
            s_rx_buf[s_rx_head] = ch;
            s_rx_head = next;
        }
        /* 若缓冲区满则丢弃 */
    }

    /* ---- 溢出错误: 必须读 DR 清除 ---- */
    if (usart_flag_get(UART6_PERIPH, USART_FLAG_ORERR) != RESET) {
        (void)usart_data_receive(UART6_PERIPH);
    }

    /* ---- 帧错误 / 噪声错误 ---- */
    if (usart_flag_get(UART6_PERIPH, USART_FLAG_FERR) != RESET) {
        (void)usart_data_receive(UART6_PERIPH);
    }
    if (usart_flag_get(UART6_PERIPH, USART_FLAG_NERR) != RESET) {
        (void)usart_data_receive(UART6_PERIPH);
    }
}

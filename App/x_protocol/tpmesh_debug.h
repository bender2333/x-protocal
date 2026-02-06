/**
 * @file tpmesh_debug.h
 * @brief TPMesh 调试输出模块
 *
 * 将 printf 重定向到 USART2 (中断发送模式)
 *
 * 硬件配置:
 * - USART2: PC10(TX), PC11(RX)
 * - 波特率: 115200
 * - 格式: 8N1
 *
 * @version 0.6.4
 */

#ifndef TPMESH_DEBUG_H
#define TPMESH_DEBUG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化调试 UART (USART2)
 *
 * 配置 USART2 为中断发送模式
 *
 * @return 0=成功
 */
int tpmesh_debug_init(void);

/**
 * @brief 发送单个字符到调试口
 * @param ch 字符 (自动处理 \n -> \r\n)
 */
void tpmesh_debug_putc(char ch);

/**
 * @brief 发送字符串到调试口
 * @param str 以 NULL 结尾的字符串
 */
void tpmesh_debug_puts(const char *str);

/**
 * @brief 格式化输出到调试口
 * @param fmt 格式字符串
 * @return 输出字符数
 */
int tpmesh_debug_printf(const char *fmt, ...);

/**
 * @brief 等待发送缓冲区清空
 */
void tpmesh_debug_flush(void);

/**
 * @brief USART2 中断处理函数
 *
 * 从 gd32f5xx_it.c 的 USART2_IRQHandler 调用
 */
void tpmesh_debug_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* TPMESH_DEBUG_H */

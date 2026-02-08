/**
 * @file tpmesh_uart.h
 * @brief TPMesh UART6 驱动 (轮询TX + 中断RX)
 *
 * 简洁可靠的串口驱动:
 * - TX: 轮询发送 (无 RTOS 依赖, 调度器启动前后均可用)
 * - RX: 中断接收到环形缓冲区, 无 RTOS 依赖
 *
 * 硬件配置:
 * - UART6: PF7(TX), PF6(RX), AF8
 * - 波特率: 115200, 8N1
 *
 * @version 1.0.0
 */

#ifndef TPMESH_UART_H
#define TPMESH_UART_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 配置
 * ============================================================================
 */

/** UART6 波特率 */
#define TPMESH_UART6_BAUD 460800

/** RX 环形缓冲区大小 (必须是 2 的幂) */
#define TPMESH_UART6_RX_BUF_SIZE 1024

/** TX 超时 (每字节等待循环数) */
#define TPMESH_UART6_TX_TIMEOUT 0xFFFF

/* ============================================================================
 * API
 * ============================================================================
 */

/**
 * @brief 初始化 UART6 (GPIO + 外设 + 中断)
 *
 * 无 RTOS 依赖, 可在 main() 最早期调用
 *
 * @return 0=成功
 */
int tpmesh_uart6_init(void);

/**
 * @brief 反初始化 UART6
 */
void tpmesh_uart6_deinit(void);

/**
 * @brief 发送数据 (轮询阻塞, 无 RTOS 依赖)
 * @param data 数据指针
 * @param len 长度
 * @return 0=成功, -1=超时, -2=参数错误
 */
int tpmesh_uart6_send(const uint8_t *data, uint16_t len);

/**
 * @brief 发送字符串 (便捷函数)
 * @param str 以 NULL 结尾的字符串
 * @return 0=成功
 */
int tpmesh_uart6_puts(const char *str);

/**
 * @brief 从 RX 缓冲区读取一个字节 (非阻塞)
 * @param out [out] 读取的字节
 * @return 0=成功, -1=无数据
 */
int tpmesh_uart6_getc(uint8_t *out);

/**
 * @brief 从 RX 缓冲区读取一个字节 (带超时)
 * @param out [out] 读取的字节
 * @param timeout_ms 超时 (ms), 0=不等待
 * @return 0=成功, -1=超时
 */
int tpmesh_uart6_getc_timeout(uint8_t *out, uint32_t timeout_ms);

/**
 * @brief 获取 RX 缓冲区中可读字节数
 * @return 可读字节数
 */
uint16_t tpmesh_uart6_rx_available(void);

/**
 * @brief 清空 RX 缓冲区
 */
void tpmesh_uart6_rx_flush(void);

/**
 * @brief UART6 中断处理函数 (从 UART6_IRQHandler 调用)
 */
void tpmesh_uart6_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* TPMESH_UART_H */

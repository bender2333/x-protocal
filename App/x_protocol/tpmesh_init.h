/**
 * @file tpmesh_init.h
 * @brief TPMesh 桥接模块初始化接口
 *
 * 用于在 main.c 中初始化 TPMesh 模块
 *
 * @version 0.6.2
 */

#ifndef TPMESH_INIT_H
#define TPMESH_INIT_H

#include "lwip/netif.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 配置
 * ============================================================================
 */

/**
 * TPMesh 模式配置
 *
 * TPMESH_MODE:
 *   0 = 禁用 TPMesh (仅使用 PHY 以太网, 正常模式)
 *   1 = Top Node 模式 (以太网桥接到 Mesh)
 *   2 = DDC 节点模式 (通过 Mesh 连接)
 */
#ifndef TPMESH_MODE
#define TPMESH_MODE 2 /* 默认禁用 TPMesh */
#endif

#define TPMESH_MODE_DISABLED 0
#define TPMESH_MODE_TOP_NODE 1
#define TPMESH_MODE_DDC 2

/* 兼容旧宏 */
#if defined(TPMESH_IS_TOP_NODE) && !defined(TPMESH_MODE)
#if TPMESH_IS_TOP_NODE
#define TPMESH_MODE TPMESH_MODE_TOP_NODE
#else
#define TPMESH_MODE TPMESH_MODE_DDC
#endif
#endif

/** Top Node Mesh ID */
#define TPMESH_TOP_NODE_MESH_ID 0xFFFE

/** DDC Mesh ID (需要根据实际配置修改) */
#define TPMESH_DDC_MESH_ID 0x0002

/** Top Node IP (需要根据实际配置修改) */
#define TPMESH_TOP_NODE_IP "192.168.10.1"

/** DDC IP (需要根据实际配置修改) */
#define TPMESH_DDC_IP "192.168.10.2"

/* ============================================================================
 * 任务配置
 * ============================================================================
 */

/** AT 接收任务栈大小 */
#define TPMESH_AT_RX_TASK_STACK 256

/** AT 接收任务优先级 */
#define TPMESH_AT_RX_TASK_PRIO (configMAX_PRIORITIES - 2)

/** 桥接任务栈大小 */
#define TPMESH_BRIDGE_TASK_STACK 512

/** 桥接任务优先级 */
#define TPMESH_BRIDGE_TASK_PRIO (configMAX_PRIORITIES - 3)

/** DDC 心跳任务栈大小 */
#define TPMESH_DDC_HB_TASK_STACK 256

/** DDC 心跳任务优先级 */
#define TPMESH_DDC_HB_TASK_PRIO (tskIDLE_PRIORITY + 2)

/* ============================================================================
 * API
 * ============================================================================
 */

/**
 * @brief 初始化 TPMesh 模块 (Top Node 模式) — 仅硬件/内存
 *
 * 调用时机: 在 EnetInit() 之后, vTaskStartScheduler() 之前
 * 不发送任何 AT 命令 (AT 命令在 bridge_task 中执行)
 *
 * @param eth_netif 以太网 netif 指针
 * @return 0=成功
 */
int tpmesh_module_init_top(struct netif *eth_netif);

/**
 * @brief 初始化 TPMesh 模块 (DDC 模式) — 仅硬件/内存
 *
 * 不发送任何 AT 命令 (AT 命令在 bridge_task 中执行)
 *
 * @return 0=成功
 */
int tpmesh_module_init_ddc(void);

/**
 * @brief 创建 TPMesh 相关任务
 *
 * 调用时机: 在 tpmesh_module_init_xxx() 成功后, vTaskStartScheduler() 之前
 * bridge_task 启动后会自动完成: 回调注册 + 模组AT命令配置
 */
void tpmesh_create_tasks(void);

/**
 * @brief 以太网输入钩子 (在 ethernetif_input 中调用)
 *
 * 在将帧传给 LwIP 之前调用此函数判断是否需要桥接
 *
 * @param netif 网络接口
 * @param p 收到的 pbuf
 * @return true=已处理(桥接),false=交给 LwIP
 */
bool tpmesh_eth_input_hook(struct netif *netif, struct pbuf *p);

/**
 * @brief 获取 TPMesh 桥接状态
 * @return true=已初始化
 */
bool tpmesh_is_initialized(void);

/**
 * @brief 请求 x_protocol 释放 UART6，供外部模块接管配置
 * @return 0=成功, <0=失败
 *
 * 行为说明:
 * - 该接口在 busy 场景下可阻塞等待，直到 UART6 可安全释放。
 * - 当前集成模型为“释放 -> 外部配置 -> 进程/模块重启”，
 *   不提供运行中 reclaim 接口。
 */
int tpmesh_request_uart6_takeover(void);

/**
 * @brief 打印 TPMesh 状态 (调试用)
 */
void tpmesh_print_status(void);

#ifdef __cplusplus
}
#endif

#endif /* TPMESH_INIT_H */

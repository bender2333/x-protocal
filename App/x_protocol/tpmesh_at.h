/**
 * @file tpmesh_at.h
 * @brief TPMesh AT 命令模块 (RTOS 队列 + 互斥锁)
 *
 * 设计原则:
 * 1. 所有 AT 操作均有超时, 永不死等
 * 2. 所有 AT 命令必须在 FreeRTOS Task 中调用 (调度器已启动)
 * 3. tpmesh_at_init() 可在调度器前调用 (仅创建 UART + RTOS 对象)
 * 4. TX 互斥锁保证多任务并发安全
 * 5. RX Task 解析响应 → 队列通知发送者
 *
 * 初始化阶段 (main, 调度器前):
 *   tpmesh_at_init();   // 创建 UART6 + 响应队列 + TX 互斥锁 (仅分配, 不阻塞)
 *
 * 任务阶段 (Task, 调度器后):
 *   tpmesh_module_init(0x0002, false); // 发 AT 命令序列 (RX Task 已运行)
 *   tpmesh_at_cmd("AT+SEND=...", 2000);
 *
 * @version 1.2.0
 */

#ifndef TPMESH_AT_H
#define TPMESH_AT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 配置
 * ============================================================================
 */

/** AT 命令最大长度 (含 AT+SEND 的 hex payload) */
#ifndef TPMESH_AT_CMD_MAX_LEN
#define TPMESH_AT_CMD_MAX_LEN 600
#endif

/** URC 行缓冲区大小 */
#define TPMESH_URC_LINE_SIZE 600

/** 默认 AT 响应超时 (ms) */
#define TPMESH_AT_TIMEOUT_MS 2000

/** AT+SEND TYPE 默认值 */
#ifndef TPMESH_AT_SEND_TYPE_DEFAULT
#define TPMESH_AT_SEND_TYPE_DEFAULT 0
#endif

/** Top Node 默认功耗模式 LP (Type D, 常接收) */
#ifndef TPMESH_AT_LP_TOP_DEFAULT
#define TPMESH_AT_LP_TOP_DEFAULT 3
#endif

/** DDC 默认功耗模式 LP (Type C, 低功耗) */
#ifndef TPMESH_AT_LP_DDC_DEFAULT
#define TPMESH_AT_LP_DDC_DEFAULT 2
#endif

/** AT+SEND 后最大有效载荷 */
#ifndef TPMESH_MTU
#define TPMESH_MTU 200
#endif

/** 分片发送间隔 (ms) */
#define TPMESH_FRAG_DELAY_MS 50

/* ============================================================================
 * AT 响应类型
 * ============================================================================
 */

typedef enum {
  AT_RESP_NONE = 0, /**< 无响应 / 超时 */
  AT_RESP_OK,       /**< OK */
  AT_RESP_ERROR,    /**< ERROR */
  AT_RESP_TIMEOUT,  /**< 超时 */
} at_resp_t;

/* ============================================================================
 * 回调函数类型
 * ============================================================================
 */

/**
 * @brief 数据接收回调 (+NNMI)
 * @param src_mesh_id 源 Mesh ID
 * @param data 数据
 * @param len 长度
 * @note 在 RX Task 上下文中执行, 应尽快返回
 */
typedef void (*tpmesh_data_cb_t)(uint16_t src_mesh_id, const uint8_t *data,
                                 uint16_t len);

/**
 * @brief 路由事件回调 (+ROUTE)
 * @param event 事件类型 ("CREATE", "DELETE", etc.)
 * @param addr 相关地址
 */
typedef void (*tpmesh_route_cb_t)(const char *event, uint16_t addr);

/* ============================================================================
 * API - 初始化
 * ============================================================================
 */

/**
 * @brief 初始化 AT 模块 (硬件 + RTOS 对象)
 *
 * 内部操作:
 * 1. tpmesh_uart6_init() — GPIO/USART/中断
 * 2. xQueueCreate(1) — 响应队列 (仅分配内存, 调度器前安全)
 * 3. xSemaphoreCreateMutex() — TX 互斥锁 (仅分配内存, 调度器前安全)
 *
 * @note 可在调度器启动前调用, 不发送任何 AT 命令
 *
 * @return 0=成功, -1=UART失败, -2=RTOS对象创建失败
 */
int tpmesh_at_init(void);

/**
 * @brief 反初始化
 */
void tpmesh_at_deinit(void);

/* ============================================================================
 * API - 命令发送 (必须在 Task 中调用)
 * ============================================================================
 */

/**
 * @brief 发送 AT 命令并等待 OK/ERROR
 *
 * 内部流程:
 * 1. xSemaphoreTake(tx_mutex) — 获取发送权
 * 2. xQueueReset(resp_queue) — 清空旧响应
 * 3. uart6_send("cmd\r\n") — 发送
 * 4. xQueueReceive(resp_queue, timeout) — 等 RX Task 写入响应
 * 5. xSemaphoreGive(tx_mutex) — 释放
 *
 * @param cmd 命令字符串 (不含 \r\n)
 * @param timeout_ms 超时 (ms)
 * @return AT_RESP_OK / AT_RESP_ERROR / AT_RESP_TIMEOUT
 */
at_resp_t tpmesh_at_cmd(const char *cmd, uint32_t timeout_ms);

/**
 * @brief 发送 AT 命令, 不等待回复
 * @param cmd 命令字符串 (不含 \r\n)
 * @return 0=发送成功, <0=发送失败
 */
int tpmesh_at_cmd_no_wait(const char *cmd);

/**
 * @brief 发送 AT+SEND 数据命令
 * @param dest_mesh_id 目标 Mesh ID
 * @param data 数据
 * @param len 数据长度 (最大 TPMESH_MTU)
 * @return AT_RESP_OK / AT_RESP_ERROR / AT_RESP_TIMEOUT
 */
at_resp_t tpmesh_at_send_data(uint16_t dest_mesh_id, const uint8_t *data,
                              uint16_t len);

/* ============================================================================
 * API - 接收处理
 * ============================================================================
 */

/**
 * @brief 轮询处理 UART RX 数据 (仅由 RX Task 调用)
 *
 * 从环形缓冲区读取字节, 按行解析:
 * - OK/ERROR → xQueueOverwrite(resp_queue) 通知发送者
 * - +NNMI → data callback
 * - +ROUTE → route callback
 *
 * @return 处理的字节数
 */
int tpmesh_at_poll(void);

/**
 * @brief 设置数据接收回调 (+NNMI)
 */
void tpmesh_at_set_data_cb(tpmesh_data_cb_t cb);

/**
 * @brief 设置路由事件回调 (+ROUTE)
 */
void tpmesh_at_set_route_cb(tpmesh_route_cb_t cb);

/* ============================================================================
 * API - 模组初始化 (必须在 Task 中调用)
 * ============================================================================
 */

/**
 * @brief 初始化 TPMesh 模组 (发送 AT 命令序列)
 *
 * 发送: AT -> AT+ADDR -> AT+CELL -> AT+LP
 *
 * @param mesh_id 本机 Mesh ID
 * @param is_top_node 节点角色信息 (true=Top, false=DDC, 用于 LP 配置)
 * @return 0=成功, <0=失败(-1=AT,-2=ADDR,-3=CELL,-4=LP)
 */
int tpmesh_module_init(uint16_t mesh_id, bool is_top_node);

/**
 * @brief 重置模组
 */
void tpmesh_module_reset(void);

/* ============================================================================
 * API - 任务 (FreeRTOS)
 * ============================================================================
 */

/**
 * @brief AT 接收处理任务 (唯一的 ring buffer 消费者)
 * @param arg 未使用
 */
void tpmesh_at_rx_task(void *arg);

/**
 * @brief 解析 +NNMI URC
 * @param urc 完整 URC 字符串
 * @param src_mesh_id [out]
 * @param data [out]
 * @param len [in/out]
 * @return 0=成功
 */
int tpmesh_at_parse_nnmi(const char *urc, uint16_t *src_mesh_id, uint8_t *data,
                         uint16_t *len);

#ifdef __cplusplus
}
#endif

#endif /* TPMESH_AT_H */

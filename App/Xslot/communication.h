/**
 * @file communication.h
 * @brief DDC MQTT 透传通信模块
 *
 * 功能：
 * 1. mDNS 监听 - 被动发现 PC 端发布的 MQTT Broker 服务
 * 2. MQTT 客户端 - 连接 Broker，收发消息
 * 3. AT 透传 - UART 与 AT 模组通信
 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 常量定义
 * ============================================================================
 */

/* mDNS 服务类型 */
#define MDNS_SERVICE_TYPE "_ddc-mqtt._tcp"
#define MDNS_SERVICE_DOMAIN "local"

/* MQTT Topic 前缀 */
#define MQTT_TOPIC_PREFIX "ddc"

/* AT 透传 UART 端口 */
#define AT_UART_PORT 2

/* 缓冲区大小 */
#define AT_RX_BUFFER_SIZE 512
#define AT_TX_BUFFER_SIZE 512
#define MQTT_MSG_BUFFER_SIZE 1024

/* ============================================================================
 * 数据结构
 * ============================================================================
 */

/**
 * @brief Broker 发现状态
 */
typedef enum {
  BROKER_STATE_IDLE = 0,   /* 空闲，等待发现 */
  BROKER_STATE_DISCOVERED, /* 已发现 Broker */
  BROKER_STATE_CONNECTING, /* 正在连接 */
  BROKER_STATE_CONNECTED,  /* 已连接 */
  BROKER_STATE_ERROR       /* 错误状态 */
} broker_state_t;

/**
 * @brief Broker 信息
 */
typedef struct {
  char ip[16];   /* Broker IP 地址 */
  uint16_t port; /* Broker 端口 (默认 1883) */
  bool valid;    /* 信息是否有效 */
} broker_info_t;

/**
 * @brief AT 透传配置
 */
typedef struct {
  char topic_cmd[64];  /* 命令 Topic: ddc/{id}/at/cmd */
  char topic_resp[64]; /* 响应 Topic: ddc/{id}/at/resp */
  char device_id[32];  /* 设备 ID (基于 MAC 地址) */
} at_config_t;

/**
 * @brief 通信模块统计信息
 */
typedef struct {
  uint32_t mqtt_rx_count; /* MQTT 接收消息数 */
  uint32_t mqtt_tx_count; /* MQTT 发送消息数 */
  uint32_t uart_rx_bytes; /* UART 接收字节数 */
  uint32_t uart_tx_bytes; /* UART 发送字节数 */
  uint32_t error_count;   /* 错误计数 */
} comm_stats_t;

/* ============================================================================
 * API 接口 - mDNS 发现
 * ============================================================================
 */

/**
 * @brief 初始化 mDNS 监听模块
 */
void mdns_discovery_init(void);

/**
 * @brief 获取 Broker 发现状态
 * @return 当前状态
 */
broker_state_t mdns_discovery_get_state(void);

/**
 * @brief 获取发现的 Broker 信息
 * @param[out] info Broker 信息
 * @return true 如果有有效信息
 */
bool mdns_discovery_get_broker(broker_info_t *info);

/**
 * @brief mDNS 服务消失回调（Broker 停止）
 */
void mdns_discovery_service_removed(void);

/**
 * @brief 执行带退避算法的主动查询
 *
 * 退避算法: 初始 1 秒，每次翻倍，最大 5 分钟
 * 退避序列: 1s -> 2s -> 4s -> 8s -> 16s -> 32s -> 64s -> 128s -> 256s ->
 * 300s(max)
 *
 * @param current_tick_ms 当前系统 tick (毫秒)
 * @return true 如果发送了查询, false 如果还在退避等待
 */
bool mdns_discovery_query_with_backoff(uint32_t current_tick_ms);

/**
 * @brief 重置退避算法
 * 在发现服务或连接成功后调用
 */
void mdns_discovery_reset_backoff(void);

/**
 * @brief 获取当前退避间隔
 * @return 当前退避间隔 (毫秒)
 */
uint32_t mdns_discovery_get_backoff_interval(void);

/**
 * @brief 获取查询计数
 * @return 已发送的查询次数
 */
uint32_t mdns_discovery_get_query_count(void);

/* ============================================================================
 * API 接口 - MQTT 客户端
 * ============================================================================
 */

/**
 * @brief 初始化 MQTT 客户端（不连接）
 */
void ddc_mqtt_init(void);

/**
 * @brief 设置 Broker 地址并连接
 * @param ip Broker IP 地址
 * @param port Broker 端口
 */
void ddc_mqtt_connect(const char *ip, uint16_t port);

/**
 * @brief 断开 MQTT 连接
 */
void ddc_mqtt_disconnect(void);

/**
 * @brief 检查 MQTT 是否已连接
 * @return true 如果已连接
 */
bool ddc_mqtt_is_connected(void);

/**
 * @brief 发布消息
 * @param topic Topic 名称
 * @param payload 消息内容
 * @param len 消息长度
 */
void ddc_mqtt_publish(const char *topic, const uint8_t *payload, uint16_t len);

/**
 * @brief 订阅 Topic
 * @param topic Topic 名称
 */
void ddc_mqtt_subscribe(const char *topic);

/* ============================================================================
 * API 接口 - AT 透传
 * ============================================================================
 */

/**
 * @brief 初始化 AT 透传模块
 */
void at_passthrough_init(void);

/**
 * @brief AT 透传处理（在主循环中调用）
 * 检查 UART 接收缓冲区，如果有数据则通过 MQTT 发送
 */
void at_passthrough_process(void);

/**
 * @brief 处理从 MQTT 收到的 AT 命令
 * @param data AT 命令数据
 * @param len 数据长度
 */
void at_passthrough_on_mqtt_cmd(const uint8_t *data, uint16_t len);

/**
 * @brief 获取设备 ID（基于 MAC 地址）
 * @param[out] id 设备 ID 字符串缓冲区
 * @param len 缓冲区长度
 */
void at_passthrough_get_device_id(char *id, uint16_t len);

/* ============================================================================
 * API 接口 - 主任务
 * ============================================================================
 */

/**
 * @brief 通信主任务（FreeRTOS 任务函数）
 * @param pvParameters 任务参数（未使用）
 */
void communication_task(void *pvParameters);

/**
 * @brief 获取通信统计信息
 * @param[out] stats 统计信息
 */
void communication_get_stats(comm_stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif /* COMMUNICATION_H */

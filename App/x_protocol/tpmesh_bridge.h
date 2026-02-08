/**
 * @file tpmesh_bridge.h
 * @brief TPMesh L2 隧道桥接模块 - 主头文件
 *
 * 基于设计文档 V0.6.2
 * 功能：BACnet over Sub-G (TPMesh L2 Tunnel Bridge)
 *
 * @version 0.6.2
 * @date 2026-02-03
 */

#ifndef TPMESH_BRIDGE_H
#define TPMESH_BRIDGE_H

#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include <stdbool.h>
#include <stdint.h>

/* 包含节点表定义 (避免重复定义 node_entry_t) */
#include "node_table.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 配置常量
 * ============================================================================
 */

/** 最大节点数量 */
#define TPMESH_MAX_NODE_ENTRIES 16

/** 节点超时时间 (ms) */
#define TPMESH_NODE_TIMEOUT_MS 90000

/** 心跳间隔 (ms) */
#define TPMESH_HEARTBEAT_MS 30000

/** 注册重试间隔 (ms) */
#define TPMESH_REGISTER_RETRY_MS 5000

/** 注册最大重试次数 */
#define TPMESH_REGISTER_MAX_RETRIES 10

/** 分片发送延时 (ms) */
#define TPMESH_FRAG_DELAY_MS 50

/** 广播限速周期 (ms) */
#define TPMESH_BROADCAST_RATE_MS 1000

/** 广播突发允许数量 */
#define TPMESH_BROADCAST_BURST_MAX 3

/** AT命令最大长度 */
#ifndef TPMESH_AT_CMD_MAX_LEN
#define TPMESH_AT_CMD_MAX_LEN 600
#endif

/** AT响应最大长度 */
#ifndef TPMESH_AT_RESP_MAX_LEN
#define TPMESH_AT_RESP_MAX_LEN 512
#endif

/** Mesh数据最大长度 (AT+SEND MTU) */
#ifndef TPMESH_MTU
#define TPMESH_MTU 200
#endif

/** 隧道头部长度 (L2_HDR + FRAG_HDR + Rule) */
#define TPMESH_TUNNEL_HDR_LEN 3

/** 有效载荷MTU */
#define TPMESH_PAYLOAD_MTU (TPMESH_MTU - TPMESH_TUNNEL_HDR_LEN)

/** BACnet/IP 端口 */
#define TPMESH_PORT_BACNET 47808

/* ============================================================================
 * Mesh 地址定义
 * ============================================================================
 */

/** Top Node Mesh 地址 */
#define MESH_ADDR_TOP_NODE 0xFFFE

/** 广播 Mesh 地址 */
#define MESH_ADDR_BROADCAST 0x0000

/** 无效 Mesh 地址 */
#define MESH_ADDR_INVALID 0xFFFF

/* ============================================================================
 * 帧类型定义
 * ============================================================================
 */

/** 注册帧类型 */
typedef enum {
  REG_FRAME_REGISTER = 0x01,      /**< 注册请求 */
  REG_FRAME_REGISTER_ACK = 0x02,  /**< 注册确认 */
  REG_FRAME_HEARTBEAT = 0x03,     /**< 心跳 */
  REG_FRAME_HEARTBEAT_ACK = 0x04, /**< 心跳响应 */
} reg_frame_type_t;

/** SCHC 规则 ID */
typedef enum {
  SCHC_RULE_NO_COMPRESS = 0x00, /**< 不压缩 */
  SCHC_RULE_BACNET_IP = 0x01,   /**< BACnet/IP 压缩 (IP+UDP头) */
  SCHC_RULE_IP_ONLY = 0x02,     /**< 仅压缩 IP 头 */
  SCHC_RULE_REGISTER = 0x10,    /**< 注册/心跳帧 */
} schc_rule_t;

/** 桥接动作 */
typedef enum {
  BRIDGE_LOCAL,     /**< 交给本机 LwIP 处理 */
  BRIDGE_TO_MESH,   /**< 转发到 Mesh 网络 */
  BRIDGE_PROXY_ARP, /**< 代理 ARP 回复 */
  BRIDGE_DROP,      /**< 丢弃 */
} bridge_action_t;

/** 过滤动作 */
typedef enum {
  FILTER_DROP,    /**< 丢弃 */
  FILTER_FORWARD, /**< 转发 */
  FILTER_LOCAL,   /**< 本地处理 */
} filter_action_t;

/** DDC 状态 */
typedef enum {
  DDC_STATE_INIT,        /**< 初始化 */
  DDC_STATE_REGISTERING, /**< 注册中 */
  DDC_STATE_ONLINE,      /**< 在线 */
} ddc_state_t;

/* ============================================================================
 * 数据结构定义
 * ============================================================================
 */

/**
 * @brief 注册帧结构
 */
typedef struct __attribute__((packed)) {
  uint8_t frame_type; /**< 帧类型 */
  uint8_t mac[6];     /**< MAC 地址 */
  uint32_t ip;        /**< IP 地址 (网络字节序) */
  uint16_t mesh_id;   /**< Mesh ID */
  uint16_t checksum;  /**< CRC16 校验 */
} reg_frame_t;

/* node_entry_t 定义在 node_table.h 中 */

/**
 * @brief Top Node 配置
 */
typedef struct {
  uint8_t mac_addr[6]; /**< MAC 地址 */
  ip4_addr_t ip_addr;  /**< IP 地址 */
  uint16_t mesh_id;    /**< Mesh ID (通常 0xFFFE) */
  uint8_t cell_id;     /**< Cell ID */
} top_config_t;

/**
 * @brief DDC 配置
 */
typedef struct {
  uint8_t mac_addr[6]; /**< MAC 地址 */
  ip4_addr_t ip_addr;  /**< IP 地址 */
  uint16_t mesh_id;    /**< Mesh ID */
  uint8_t cell_id;     /**< Cell ID */
} ddc_config_t;

/**
 * @brief 隧道帧头部 (V0.6.2)
 */
typedef struct __attribute__((packed)) {
  uint8_t l2_hdr;   /**< L2 Header: bit7=广播 */
  uint8_t frag_hdr; /**< Frag Header: bit7=最后一片, bit6-0=seq */
  uint8_t rule_id;  /**< SCHC Rule ID */
} tunnel_hdr_t;

/**
 * @brief 广播限速器
 */
typedef struct {
  uint32_t last_tick;  /**< 最后广播时间 */
  uint8_t burst_count; /**< 突发计数 */
} rate_limiter_t;

/**
 * @brief 分片重组会话
 */
typedef struct {
  uint16_t src_mesh_id; /**< 源 Mesh ID */
  uint8_t buffer[1600]; /**< 重组缓冲区 */
  uint16_t total_len;   /**< 当前总长度 */
  uint8_t expected_seq; /**< 期望的下一个序号 */
  uint32_t last_tick;   /**< 最后接收时间 */
  bool active;          /**< 会话活跃 */
} reassembly_session_t;

/* ============================================================================
 * 公共 API - 初始化
 * ============================================================================
 */

/**
 * @brief 初始化 TPMesh 桥接模块 (Top Node)
 * @param eth_netif 以太网 netif
 * @param config Top Node 配置
 * @return 0=成功
 */
int tpmesh_top_init(struct netif *eth_netif, const top_config_t *config);

/**
 * @brief 初始化 TPMesh 桥接模块 (DDC)
 * @param config DDC 配置
 * @return 0=成功
 */
int tpmesh_ddc_init(const ddc_config_t *config);

/* ============================================================================
 * 公共 API - AT 命令
 * ============================================================================
 */

/* AT 模块 API 定义在 tpmesh_at.h, 此处仅声明 bridge 使用的兼容函数 */

/**
 * @brief 发送 AT+SEND (兼容 int 返回值)
 * @param dest_mesh_id 目标 Mesh ID
 * @param data 数据
 * @param len 数据长度 (最大 200)
 * @return 0=成功, <0=失败
 */
int tpmesh_at_send(uint16_t dest_mesh_id, const uint8_t *data, uint16_t len);

/* ============================================================================
 * 公共 API - 节点映射表
 * ============================================================================
 */

/**
 * @brief 初始化节点映射表
 */
void node_table_init(void);

/**
 * @brief 添加/更新节点 (注册)
 * @param mac MAC 地址
 * @param ip IP 地址
 * @param mesh_id Mesh ID
 * @return 0=成功
 */
int node_table_register(const uint8_t *mac, const ip4_addr_t *ip,
                        uint16_t mesh_id);

/**
 * @brief 动态学习节点
 * @param mac MAC 地址
 * @param ip IP 地址
 * @param mesh_id Mesh ID
 * @return 0=成功
 */
int node_table_learn(const uint8_t *mac, const ip4_addr_t *ip,
                     uint16_t mesh_id);

/**
 * @brief 通过 Mesh ID 学习 MAC
 * @param mesh_id Mesh ID
 * @param mac MAC 地址
 * @return 0=成功
 */
int node_table_learn_by_mesh(uint16_t mesh_id, const uint8_t *mac);

/**
 * @brief 通过 MAC 获取 Mesh ID
 * @param mac MAC 地址
 * @return Mesh ID，失败返回 MESH_ADDR_INVALID
 */
uint16_t node_table_get_mesh_by_mac(const uint8_t *mac);

/**
 * @brief 通过 IP 获取 Mesh ID
 * @param ip IP 地址
 * @return Mesh ID，失败返回 MESH_ADDR_INVALID
 */
uint16_t node_table_get_mesh_by_ip(const ip4_addr_t *ip);

/**
 * @brief 通过 Mesh ID 获取 MAC
 * @param mesh_id Mesh ID
 * @param mac [out] MAC 地址
 * @return 0=成功
 */
int node_table_get_mac_by_mesh(uint16_t mesh_id, uint8_t *mac);

/**
 * @brief 通过 IP 获取 MAC
 * @param ip IP 地址
 * @param mac [out] MAC 地址
 * @return 0=成功
 */
int node_table_get_mac_by_ip(const ip4_addr_t *ip, uint8_t *mac);

/**
 * @brief 通过 Mesh ID 获取 IP
 * @param mesh_id Mesh ID
 * @param ip [out] IP 地址
 * @return 0=成功
 */
int node_table_get_ip_by_mesh(uint16_t mesh_id, ip4_addr_t *ip);

/**
 * @brief 检查 IP 是否为 DDC
 * @param ip IP 地址
 * @return true=是DDC
 */
bool node_table_is_ddc_ip(const ip4_addr_t *ip);

/**
 * @brief 检查 MAC 是否为 DDC
 * @param mac MAC 地址
 * @return true=是DDC
 */
bool node_table_is_ddc_mac(const uint8_t *mac);

/**
 * @brief 检查节点是否已注册
 * @param mesh_id Mesh ID
 * @return true=已注册
 */
bool node_table_is_registered(uint16_t mesh_id);

/**
 * @brief 节点表维护任务
 * @param arg 任务参数
 */
void node_table_maint_task(void *arg);

/* ============================================================================
 * 公共 API - SCHC 压缩/解压
 * ============================================================================
 */

/**
 * @brief SCHC 压缩
 * @param eth_frame 以太网帧
 * @param eth_len 帧长度
 * @param out_data [out] 输出缓冲区
 * @param out_len [out] 输出长度
 * @param is_broadcast 是否广播
 * @return 0=成功
 */
int schc_compress(const uint8_t *eth_frame, uint16_t eth_len, uint8_t *out_data,
                  uint16_t *out_len, bool is_broadcast);

/**
 * @brief SCHC 解压
 * @param mesh_data Mesh 数据
 * @param mesh_len 数据长度
 * @param out_frame [out] 输出以太网帧
 * @param out_len [out] 输出长度
 * @param src_mesh_id 源 Mesh ID (从 AT URC 获取)
 * @param dst_mesh_id 目标 Mesh ID
 * @return 0=成功
 */
int schc_decompress(const uint8_t *mesh_data, uint16_t mesh_len,
                    uint8_t *out_frame, uint16_t out_frame_size,
                    uint16_t *out_len, uint16_t src_mesh_id,
                    uint16_t dst_mesh_id);

/* ============================================================================
 * 公共 API - 桥接
 * ============================================================================
 */

/**
 * @brief 检查以太网帧的桥接动作
 * @param p pbuf
 * @return 桥接动作
 */
bridge_action_t tpmesh_bridge_check(struct pbuf *p);

/**
 * @brief 转发以太网帧到 Mesh
 * @param p pbuf
 * @return 0=成功
 */
int tpmesh_bridge_forward_to_mesh(struct pbuf *p);

/**
 * @brief 发送代理 ARP 回复
 * @param p 收到的 ARP 请求 pbuf
 * @return 0=成功
 */
int tpmesh_bridge_send_proxy_arp(struct pbuf *p);

/**
 * @brief 在 DDC 模式下挂接 netif->linkoutput 到 TPMesh bridge
 * @param netif 目标 netif
 * @return 0=成功
 */
int tpmesh_bridge_attach_ddc_netif(struct netif *netif);

/**
 * @brief 处理来自 Mesh 的数据帧
 * @param src_mesh_id 源 Mesh ID
 * @param data 数据
 * @param len 长度
 */
void tpmesh_bridge_handle_mesh_data(uint16_t src_mesh_id, const uint8_t *data,
                                    uint16_t len);

/**
 * @brief 桥接协议处理任务
 * @param arg 任务参数
 */
void tpmesh_bridge_task(void *arg);

/* ============================================================================
 * 公共 API - DDC 注册/心跳
 * ============================================================================
 */

/**
 * @brief DDC 发送注册请求
 * @param config DDC 配置
 * @return 0=成功
 */
int ddc_send_register(const ddc_config_t *config);

/**
 * @brief DDC 发送心跳
 * @param config DDC 配置
 * @return 0=成功
 */
int ddc_send_heartbeat(const ddc_config_t *config);

/**
 * @brief DDC 心跳任务
 * @param arg 任务参数
 */
void ddc_heartbeat_task(void *arg);

/* ============================================================================
 * 公共 API - 广播限速
 * ============================================================================
 */

/**
 * @brief 检查广播是否允许发送
 * @return true=允许
 */
bool broadcast_rate_check(void);

/* ============================================================================
 * 公共 API - 工具函数
 * ============================================================================
 */

/**
 * @brief 获取当前 tick (ms)
 * @return tick 值
 */
uint32_t tpmesh_get_tick_ms(void);

/**
 * @brief 计算 CRC16
 * @param data 数据
 * @param len 长度
 * @return CRC16 值
 */
uint16_t tpmesh_calc_crc16(const void *data, uint16_t len);

/**
 * @brief 检查是否为广播 MAC
 * @param mac MAC 地址
 * @return true=广播
 */
bool tpmesh_is_broadcast_mac(const uint8_t *mac);

/**
 * @brief 获取本机 MAC 地址
 * @param mac [out] MAC 地址
 */
void tpmesh_get_local_mac(uint8_t *mac);

#ifdef __cplusplus
}
#endif

#endif /* TPMESH_BRIDGE_H */

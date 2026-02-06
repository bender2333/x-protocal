/**
 * @file node_table.h
 * @brief TPMesh 节点映射表模块
 *
 * 管理 DDC 节点的 MAC/IP/MeshID 映射关系
 *
 * @version 0.6.2
 */

#ifndef NODE_TABLE_H
#define NODE_TABLE_H

#include "lwip/ip4_addr.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 配置
 * ============================================================================
 */

/** 最大节点数量 */
#ifndef NODE_TABLE_MAX_ENTRIES
#define NODE_TABLE_MAX_ENTRIES 16
#endif

/** 节点超时时间 (ms) */
#ifndef NODE_TABLE_TIMEOUT_MS
#define NODE_TABLE_TIMEOUT_MS 90000
#endif

/* ============================================================================
 * 节点来源类型
 * ============================================================================
 */

typedef enum {
  NODE_SOURCE_STATIC = 0,   /**< 静态配置 */
  NODE_SOURCE_LEARNED = 1,  /**< 动态学习 (从数据帧) */
  NODE_SOURCE_REGISTER = 2, /**< 注册 (DDC主动注册) */
} node_source_t;

/* ============================================================================
 * 节点条目结构
 * ============================================================================
 */

typedef struct {
  uint8_t valid;      /**< 条目有效 */
  uint8_t mac[6];     /**< MAC 地址 */
  ip4_addr_t ip;      /**< IP 地址 */
  uint16_t mesh_id;   /**< Mesh ID */
  uint32_t last_seen; /**< 最后活跃时间 (tick) */
  uint8_t source;     /**< 来源类型 (node_source_t) */
  uint8_t online;     /**< 在线状态 */
} node_entry_t;

/* ============================================================================
 * API
 * ============================================================================
 */

/**
 * @brief 初始化节点映射表
 */
void node_table_init(void);

/**
 * @brief 清空节点映射表
 */
void node_table_clear(void);

/**
 * @brief 添加静态节点
 * @param mac MAC 地址
 * @param ip IP 地址
 * @param mesh_id Mesh ID
 * @return 0=成功, -1=表满
 */
int node_table_add_static(const uint8_t *mac, const ip4_addr_t *ip,
                          uint16_t mesh_id);

/**
 * @brief 节点注册 (DDC主动注册)
 * @param mac MAC 地址
 * @param ip IP 地址
 * @param mesh_id Mesh ID
 * @return 0=成功
 */
int node_table_register(const uint8_t *mac, const ip4_addr_t *ip,
                        uint16_t mesh_id);

/**
 * @brief 动态学习节点 (从数据帧学习)
 * @param mac MAC 地址
 * @param ip IP 地址
 * @param mesh_id Mesh ID
 * @return 0=成功
 */
int node_table_learn(const uint8_t *mac, const ip4_addr_t *ip,
                     uint16_t mesh_id);

/**
 * @brief 通过 Mesh ID 学习 MAC (从接收数据中学习)
 * @param mesh_id Mesh ID
 * @param mac MAC 地址
 * @return 0=成功
 */
int node_table_learn_by_mesh(uint16_t mesh_id, const uint8_t *mac);

/**
 * @brief 更新节点活跃时间
 * @param mesh_id Mesh ID
 */
void node_table_touch(uint16_t mesh_id);

/**
 * @brief 删除节点
 * @param mesh_id Mesh ID
 */
void node_table_remove(uint16_t mesh_id);

/* ============================================================================
 * 查询 API
 * ============================================================================
 */

/**
 * @brief 通过 MAC 获取 Mesh ID
 * @param mac MAC 地址
 * @return Mesh ID, 失败返回 0xFFFF
 */
uint16_t node_table_get_mesh_by_mac(const uint8_t *mac);

/**
 * @brief 通过 IP 获取 Mesh ID
 * @param ip IP 地址
 * @return Mesh ID, 失败返回 0xFFFF
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
 * @brief 获取节点条目
 * @param mesh_id Mesh ID
 * @return 节点条目指针, NULL=未找到
 */
const node_entry_t *node_table_get_entry(uint16_t mesh_id);

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
 * @brief 检查节点是否在线
 * @param mesh_id Mesh ID
 * @return true=在线
 */
bool node_table_is_online(uint16_t mesh_id);

/* ============================================================================
 * 维护 API
 * ============================================================================
 */

/**
 * @brief 超时检查 (清理过期节点)
 * 应该周期性调用
 */
void node_table_check_timeout(void);

/**
 * @brief 打印节点表 (调试用)
 */
void node_table_dump(void);

/**
 * @brief 获取当前节点数量
 * @return 节点数量
 */
uint16_t node_table_count(void);

/**
 * @brief 遍历所有节点
 * @param callback 回调函数, 返回 false 停止遍历
 * @param arg 用户参数
 */
void node_table_foreach(bool (*callback)(const node_entry_t *entry, void *arg),
                        void *arg);

#ifdef __cplusplus
}
#endif

#endif /* NODE_TABLE_H */

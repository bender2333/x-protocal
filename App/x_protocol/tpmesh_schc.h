/**
 * @file tpmesh_schc.h
 * @brief TPMesh SCHC (Static Context Header Compression) 模块
 *
 * 简化版 SCHC，针对 BACnet/IP over Sub-G 优化
 *
 * 压缩规则:
 * - Rule 0x00: 不压缩
 * - Rule 0x01: BACnet/IP (压缩 IP+UDP 头, 28字节)
 * - Rule 0x02: 其他 IP (仅压缩 IP 头, 20字节)
 * - Rule 0x10: 注册/心跳帧
 *
 * @version 0.6.2
 */

#ifndef TPMESH_SCHC_H
#define TPMESH_SCHC_H

#include "lwip/ip4_addr.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 常量定义
 * ============================================================================
 */

/* SCHC 规则 ID 定义在 tpmesh_bridge.h 的 schc_rule_t enum 中 */

/** 以太网头部长度 */
#define ETH_HDR_LEN 14

/** IP 头部长度 (无选项) */
#define IP_HDR_LEN 20

/** UDP 头部长度 */
#define UDP_HDR_LEN 8

/** BACnet/IP 端口 */
#define PORT_BACNET_IP 47808

/** EtherType */
#define ETHERTYPE_IP 0x0800
#define ETHERTYPE_ARP 0x0806

/* ============================================================================
 * 压缩上下文结构
 * ============================================================================
 */

/**
 * @brief SCHC 压缩上下文
 * 存储被压缩的头部字段
 */
typedef struct {
  /* 以太网头部 */
  uint8_t eth_src[6]; /**< 源 MAC */
  uint8_t eth_dst[6]; /**< 目标 MAC */
  uint16_t eth_type;  /**< EtherType */

  /* IP 头部 */
  uint32_t ip_src;  /**< 源 IP (网络序) */
  uint32_t ip_dst;  /**< 目标 IP (网络序) */
  uint8_t ip_proto; /**< IP 协议 */
  uint8_t ip_ttl;   /**< TTL */

  /* UDP 头部 */
  uint16_t udp_src_port; /**< 源端口 (网络序) */
  uint16_t udp_dst_port; /**< 目标端口 (网络序) */

  /* 规则信息 */
  uint8_t rule_id;   /**< 使用的规则 */
  bool is_broadcast; /**< 是否广播 */
} schc_context_t;

/* ============================================================================
 * 压缩 API
 * ============================================================================
 */

/**
 * @brief 压缩以太网帧为 Mesh 隧道帧
 *
 * 根据帧类型自动选择压缩规则:
 * - BACnet/IP (UDP:47808) -> Rule 0x01, 压缩28字节
 * - 其他 IP/UDP -> Rule 0x02, 压缩20字节
 * - ARP/其他 -> Rule 0x00, 不压缩
 *
 * @param eth_frame 输入以太网帧
 * @param eth_len 以太网帧长度
 * @param out_data 输出缓冲区 (隧道帧)
 * @param out_len [out] 输出长度
 * @param is_broadcast 是否广播
 * @return 0=成功
 */
int schc_compress(const uint8_t *eth_frame, uint16_t eth_len, uint8_t *out_data,
                  uint16_t *out_len, bool is_broadcast);

/**
 * @brief 解压 Mesh 隧道帧为以太网帧
 *
 * @param mesh_data 输入隧道帧 (从 L2 Header 开始)
 * @param mesh_len 隧道帧长度
 * @param out_frame 输出以太网帧
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
 * 辅助 API
 * ============================================================================
 */

/**
 * @brief 判断压缩规则
 * @param eth_frame 以太网帧
 * @param eth_len 帧长度
 * @return 规则 ID
 */
uint8_t schc_get_rule(const uint8_t *eth_frame, uint16_t eth_len);

/**
 * @brief 获取规则压缩节省的字节数
 * @param rule_id 规则 ID
 * @return 节省字节数
 */
uint16_t schc_get_compression_savings(uint8_t rule_id);

/**
 * @brief 解析隧道头部
 * @param data 隧道数据
 * @param l2_hdr [out] L2 头部
 * @param frag_hdr [out] 分片头部
 * @param rule_id [out] 规则 ID
 * @return 头部长度
 */
int schc_parse_tunnel_header(const uint8_t *data, uint16_t data_len,
                             uint8_t *l2_hdr, uint8_t *frag_hdr,
                             uint8_t *rule_id);

/**
 * @brief 检查是否为广播 MAC
 * @param mac MAC 地址
 * @return true=广播
 */
bool schc_is_broadcast_mac(const uint8_t *mac);

/**
 * @brief 计算 IP 校验和
 * @param data IP 头部数据
 * @param len 头部长度
 * @return 校验和
 */
uint16_t schc_ip_checksum(const uint8_t *data, uint16_t len);

/**
 * @brief 计算 UDP 校验和 (可选)
 * @param ip_src 源 IP
 * @param ip_dst 目标 IP
 * @param udp_hdr UDP 头部 + 数据
 * @param udp_len UDP 长度
 * @return 校验和
 */
uint16_t schc_udp_checksum(uint32_t ip_src, uint32_t ip_dst,
                           const uint8_t *udp_hdr, uint16_t udp_len);

#ifdef __cplusplus
}
#endif

#endif /* TPMESH_SCHC_H */

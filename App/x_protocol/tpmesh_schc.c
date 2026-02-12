/**
 * @file tpmesh_schc.c
 * @brief TPMesh SCHC 压缩模块实现
 *
 * @version 0.6.2
 */

#include "tpmesh_bridge.h"
/* node_table.h 和 schc_rule_t 已通过 tpmesh_bridge.h 定义 */
#include "lwip/inet_chksum.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "tpmesh_schc.h"
#include <stdio.h>
#include <string.h>


/* ============================================================================
 * 私有常量
 * ============================================================================
 */

/** 广播 MAC */
static const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define IPV4_IHL_MIN_BYTES 20
#define IPV4_IHL_MAX_BYTES 60

/* ============================================================================
 * 私有函数
 * ============================================================================
 */

/**
 * @brief 从 16位数组读取大端值
 */
static inline uint16_t read_be16(const uint8_t *p) {
  return ((uint16_t)p[0] << 8) | p[1];
}

/**
 * @brief 写入16位大端值
 */
static inline void write_be16(uint8_t *p, uint16_t val) {
  p[0] = (val >> 8) & 0xFF;
  p[1] = val & 0xFF;
}

/**
 * @brief 从 32位数组读取大端值
 */
static inline uint32_t read_be32(const uint8_t *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8) | p[3];
}

/**
 * @brief 写入32位大端值
 */
static inline void write_be32(uint8_t *p, uint32_t val) {
  p[0] = (val >> 24) & 0xFF;
  p[1] = (val >> 16) & 0xFF;
  p[2] = (val >> 8) & 0xFF;
  p[3] = val & 0xFF;
}

/* ============================================================================
 * 公共函数
 * ============================================================================
 */

bool schc_is_broadcast_mac(const uint8_t *mac) {
  return memcmp(mac, BROADCAST_MAC, 6) == 0;
}

uint8_t schc_get_rule(const uint8_t *eth_frame, uint16_t eth_len) {
  if (eth_len < ETH_HDR_LEN) {
    return SCHC_RULE_NO_COMPRESS;
  }

  uint16_t ethertype = read_be16(eth_frame + 12);
  if (ethertype != ETHERTYPE_IP) {
    return SCHC_RULE_NO_COMPRESS;
  }

  if (eth_len < (ETH_HDR_LEN + IPV4_IHL_MIN_BYTES)) {
    return SCHC_RULE_NO_COMPRESS;
  }

  const uint8_t *ip_hdr = eth_frame + ETH_HDR_LEN;
  uint8_t ihl = (uint8_t)((ip_hdr[0] & 0x0F) * 4);
  if (ihl < IPV4_IHL_MIN_BYTES || ihl > IPV4_IHL_MAX_BYTES) {
    return SCHC_RULE_NO_COMPRESS;
  }
  if (eth_len < (uint16_t)(ETH_HDR_LEN + ihl)) {
    return SCHC_RULE_NO_COMPRESS;
  }

  /* Current rule set only supports IPv4 without options. */
  if (ihl != IP_HDR_LEN) {
    return SCHC_RULE_NO_COMPRESS;
  }

  uint8_t ip_proto = ip_hdr[9];
  if (ip_proto != IP_PROTO_UDP) {
    return SCHC_RULE_NO_COMPRESS;
  }

  if (eth_len < (uint16_t)(ETH_HDR_LEN + ihl + UDP_HDR_LEN)) {
    return SCHC_RULE_NO_COMPRESS;
  }

  const uint8_t *udp_hdr = ip_hdr + ihl;
  uint16_t src_port = read_be16(udp_hdr + 0);
  uint16_t dst_port = read_be16(udp_hdr + 2);
  if (src_port == PORT_BACNET_IP && dst_port == PORT_BACNET_IP) {
    return SCHC_RULE_BACNET_IP;
  }
  return SCHC_RULE_IP_ONLY;
}

uint16_t schc_get_compression_savings(uint8_t rule_id) {
  switch (rule_id) {
  case SCHC_RULE_BACNET_IP:
    return IP_HDR_LEN + UDP_HDR_LEN; /* 28 bytes */
  case SCHC_RULE_IP_ONLY:
    return IP_HDR_LEN; /* 20 bytes */
  default:
    return 0;
  }
}

int schc_compress(const uint8_t *eth_frame, uint16_t eth_len, uint8_t *out_data,
                  uint16_t *out_len, bool is_broadcast) {
  if (eth_frame == NULL || out_data == NULL || out_len == NULL ||
      eth_len < ETH_HDR_LEN) {
    return -1;
  }

  uint8_t rule_id = schc_get_rule(eth_frame, eth_len);

  out_data[0] = is_broadcast ? 0x80 : 0x00;
  out_data[1] = 0x80;
  out_data[2] = rule_id;

  uint8_t *payload = out_data + TPMESH_TUNNEL_HDR_LEN;
  uint16_t payload_len = 0;

  switch (rule_id) {
  case SCHC_RULE_BACNET_IP: {
    const uint8_t *ip_hdr = eth_frame + ETH_HDR_LEN;
    uint8_t ihl = (uint8_t)((ip_hdr[0] & 0x0F) * 4);
    if (ihl != IP_HDR_LEN ||
        eth_len < (uint16_t)(ETH_HDR_LEN + ihl + UDP_HDR_LEN)) {
      return -2;
    }

    memcpy(payload, eth_frame + 6, 6);
    payload += 6;

    uint16_t apdu_offset = (uint16_t)(ETH_HDR_LEN + ihl + UDP_HDR_LEN);
    uint16_t apdu_len = (uint16_t)(eth_len - apdu_offset);
    if (apdu_len > 0) {
      memcpy(payload, eth_frame + apdu_offset, apdu_len);
    }
    payload_len = (uint16_t)(6 + apdu_len);
    break;
  }

  case SCHC_RULE_IP_ONLY: {
    const uint8_t *ip_hdr = eth_frame + ETH_HDR_LEN;
    uint8_t ihl = (uint8_t)((ip_hdr[0] & 0x0F) * 4);
    if (ihl != IP_HDR_LEN || eth_len < (uint16_t)(ETH_HDR_LEN + ihl)) {
      return -2;
    }

    memcpy(payload, eth_frame + 6, 6);
    payload += 6;

    uint16_t rest_offset = (uint16_t)(ETH_HDR_LEN + ihl);
    uint16_t rest_len = (uint16_t)(eth_len - rest_offset);
    if (rest_len > 0) {
      memcpy(payload, eth_frame + rest_offset, rest_len);
    }
    payload_len = (uint16_t)(6 + rest_len);
    break;
  }

  case SCHC_RULE_NO_COMPRESS:
  default:
    if (eth_len < 12) {
      return -2;
    }
    memcpy(payload, eth_frame + 6, 6);
    memcpy(payload + 6, eth_frame, 6);
    memcpy(payload + 12, eth_frame + 12, (size_t)(eth_len - 12));
    payload_len = eth_len;
    break;
  }

  *out_len = (uint16_t)(TPMESH_TUNNEL_HDR_LEN + payload_len);
  return 0;
}

int schc_decompress(const uint8_t *mesh_data, uint16_t mesh_len,
                    uint8_t *out_frame, uint16_t out_frame_size,
                    uint16_t *out_len, uint16_t src_mesh_id,
                    uint16_t dst_mesh_id) {
  if (mesh_data == NULL || out_frame == NULL || out_len == NULL) {
    return -1;
  }

  if (mesh_len < TPMESH_TUNNEL_HDR_LEN || out_frame_size < ETH_HDR_LEN) {
    return -1;
  }

  uint8_t l2_hdr = mesh_data[0];
  uint8_t frag_hdr = mesh_data[1];
  uint8_t rule_id = mesh_data[2];

  (void)l2_hdr;
  (void)frag_hdr;

  const uint8_t *payload = mesh_data + TPMESH_TUNNEL_HDR_LEN;
  uint16_t payload_len = (uint16_t)(mesh_len - TPMESH_TUNNEL_HDR_LEN);

  if (payload_len < 6) {
    return -2;
  }

  uint8_t src_mac[6];
  memcpy(src_mac, payload, 6);
  payload += 6;
  payload_len = (uint16_t)(payload_len - 6);

  uint8_t *eth = out_frame;
  uint16_t eth_len = 0;

  switch (rule_id) {
  case SCHC_RULE_BACNET_IP: {
    uint16_t needed_len =
        (uint16_t)(ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN + payload_len);
    if (needed_len > out_frame_size) {
      return -4;
    }

    uint8_t dst_mac[6];
    if (dst_mesh_id == MESH_ADDR_BROADCAST) {
      memcpy(dst_mac, BROADCAST_MAC, 6);
    } else if (node_table_get_mac_by_mesh(dst_mesh_id, dst_mac) != 0) {
      memcpy(dst_mac, BROADCAST_MAC, 6);
    }

    memcpy(eth, dst_mac, 6);
    memcpy(eth + 6, src_mac, 6);
    write_be16(eth + 12, ETHERTYPE_IP);
    eth += ETH_HDR_LEN;

    uint16_t ip_total_len = (uint16_t)(IP_HDR_LEN + UDP_HDR_LEN + payload_len);
    eth[0] = 0x45;
    eth[1] = 0x00;
    write_be16(eth + 2, ip_total_len);
    write_be16(eth + 4, 0);
    write_be16(eth + 6, 0);
    eth[8] = 64;
    eth[9] = IP_PROTO_UDP;
    write_be16(eth + 10, 0);

    ip4_addr_t src_ip, dst_ip;
    if (node_table_get_ip_by_mesh(src_mesh_id, &src_ip) != 0) {
      IP4_ADDR(&src_ip, 192, 168, 10, 100);
    }
    if (dst_mesh_id == MESH_ADDR_BROADCAST) {
      IP4_ADDR(&dst_ip, 255, 255, 255, 255);
    } else if (node_table_get_ip_by_mesh(dst_mesh_id, &dst_ip) != 0) {
      IP4_ADDR(&dst_ip, 192, 168, 10, 1);
    }
    eth[12] = ip4_addr1(&src_ip);
    eth[13] = ip4_addr2(&src_ip);
    eth[14] = ip4_addr3(&src_ip);
    eth[15] = ip4_addr4(&src_ip);
    eth[16] = ip4_addr1(&dst_ip);
    eth[17] = ip4_addr2(&dst_ip);
    eth[18] = ip4_addr3(&dst_ip);
    eth[19] = ip4_addr4(&dst_ip);

    uint16_t ip_cksum = schc_ip_checksum(eth, IP_HDR_LEN);
    write_be16(eth + 10, ip_cksum);
    eth += IP_HDR_LEN;

    uint16_t udp_len = (uint16_t)(UDP_HDR_LEN + payload_len);
    write_be16(eth, PORT_BACNET_IP);
    write_be16(eth + 2, PORT_BACNET_IP);
    write_be16(eth + 4, udp_len);
    write_be16(eth + 6, 0);
    eth += UDP_HDR_LEN;

    if (payload_len > 0) {
      memcpy(eth, payload, payload_len);
    }

    eth_len = needed_len;
    break;
  }

  case SCHC_RULE_IP_ONLY: {
    uint16_t needed_len = (uint16_t)(ETH_HDR_LEN + IP_HDR_LEN + payload_len);
    if (needed_len > out_frame_size) {
      return -4;
    }

    uint8_t dst_mac[6];
    if (dst_mesh_id == MESH_ADDR_BROADCAST) {
      memcpy(dst_mac, BROADCAST_MAC, 6);
    } else if (node_table_get_mac_by_mesh(dst_mesh_id, dst_mac) != 0) {
      memcpy(dst_mac, BROADCAST_MAC, 6);
    }

    memcpy(eth, dst_mac, 6);
    memcpy(eth + 6, src_mac, 6);
    write_be16(eth + 12, ETHERTYPE_IP);
    eth += ETH_HDR_LEN;

    uint16_t ip_total_len = (uint16_t)(IP_HDR_LEN + payload_len);
    eth[0] = 0x45;
    eth[1] = 0x00;
    write_be16(eth + 2, ip_total_len);
    write_be16(eth + 4, 0);
    write_be16(eth + 6, 0);
    eth[8] = 64;
    eth[9] = IP_PROTO_UDP;
    write_be16(eth + 10, 0);

    ip4_addr_t src_ip, dst_ip;
    if (node_table_get_ip_by_mesh(src_mesh_id, &src_ip) != 0) {
      IP4_ADDR(&src_ip, 192, 168, 10, 100);
    }
    if (dst_mesh_id == MESH_ADDR_BROADCAST) {
      IP4_ADDR(&dst_ip, 255, 255, 255, 255);
    } else if (node_table_get_ip_by_mesh(dst_mesh_id, &dst_ip) != 0) {
      IP4_ADDR(&dst_ip, 192, 168, 10, 1);
    }
    eth[12] = ip4_addr1(&src_ip);
    eth[13] = ip4_addr2(&src_ip);
    eth[14] = ip4_addr3(&src_ip);
    eth[15] = ip4_addr4(&src_ip);
    eth[16] = ip4_addr1(&dst_ip);
    eth[17] = ip4_addr2(&dst_ip);
    eth[18] = ip4_addr3(&dst_ip);
    eth[19] = ip4_addr4(&dst_ip);

    uint16_t ip_cksum = schc_ip_checksum(eth, IP_HDR_LEN);
    write_be16(eth + 10, ip_cksum);
    eth += IP_HDR_LEN;

    if (payload_len > 0) {
      memcpy(eth, payload, payload_len);
    }

    eth_len = needed_len;
    break;
  }

  case SCHC_RULE_NO_COMPRESS:
  default: {
    if (payload_len < 8) {
      return -3;
    }
    uint16_t needed_len = (uint16_t)(12 + payload_len);
    if (needed_len > out_frame_size) {
      return -4;
    }

    uint8_t dst_mac[6];
    memcpy(dst_mac, payload, 6);
    payload += 6;
    payload_len = (uint16_t)(payload_len - 6);

    memcpy(eth, dst_mac, 6);
    memcpy(eth + 6, src_mac, 6);
    memcpy(eth + 12, payload, payload_len);
    eth_len = (uint16_t)(12 + payload_len);
    break;
  }
  }

  *out_len = eth_len;
  return 0;
}

int schc_parse_tunnel_header(const uint8_t *data, uint16_t data_len,
                             uint8_t *l2_hdr, uint8_t *frag_hdr,
                             uint8_t *rule_id) {
  if (data == NULL || data_len < TPMESH_TUNNEL_HDR_LEN) {
    return -1;
  }

  if (l2_hdr) {
    *l2_hdr = data[0];
  }
  if (frag_hdr) {
    *frag_hdr = data[1];
  }
  if (rule_id) {
    *rule_id = data[2];
  }
  return TPMESH_TUNNEL_HDR_LEN;
}

uint16_t schc_ip_checksum(const uint8_t *data, uint16_t len) {
  uint32_t sum = 0;

  /* 16位字求和 */
  for (uint16_t i = 0; i < len; i += 2) {
    uint16_t word = ((uint16_t)data[i] << 8);
    if (i + 1 < len) {
      word |= data[i + 1];
    }
    sum += word;
  }

  /* 折叠到16位 */
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  return ~sum;
}

uint16_t schc_udp_checksum(uint32_t ip_src, uint32_t ip_dst,
                           const uint8_t *udp_hdr, uint16_t udp_len) {
  uint32_t sum = 0;

  /* 伪首部 */
  sum += (ip_src >> 16) & 0xFFFF;
  sum += ip_src & 0xFFFF;
  sum += (ip_dst >> 16) & 0xFFFF;
  sum += ip_dst & 0xFFFF;
  sum += IP_PROTO_UDP;
  sum += udp_len;

  /* UDP 头 + 数据 */
  for (uint16_t i = 0; i < udp_len; i += 2) {
    uint16_t word = ((uint16_t)udp_hdr[i] << 8);
    if (i + 1 < udp_len) {
      word |= udp_hdr[i + 1];
    }
    sum += word;
  }

  /* 折叠 */
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  return (sum == 0xFFFF) ? sum : ~sum;
}

/**
 * @file tpmesh_bridge.c
 * @brief TPMesh L2 隧道桥接模块实现
 *
 * 核心功能:
 * 1. Top Node: 以太网 ↔ Mesh 桥接,代理 ARP,广播过滤
 * 2. DDC: 注册/心跳,Mesh 收发
 *
 * @version 0.6.2
 */

#include "tpmesh_bridge.h"
#include "tpmesh_at.h"
#include "tpmesh_debug.h"
#include "tpmesh_schc.h"

/* node_table.h 已通过 tpmesh_bridge.h 包含 */
#include "FreeRTOS.h"
#include "lwip/etharp.h"
#include "lwip/ip.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/prot/etharp.h"
#include "lwip/prot/ethernet.h"
#include "lwip/prot/ip4.h"
#include "lwip/prot/udp.h"
#include "lwip/tcpip.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>
#include <string.h>


/* ============================================================================
 * 类型定义
 * ============================================================================
 */

/** 分片发送上下文 */
typedef struct {
  uint16_t dest_mesh_id;
  uint8_t buffer[1600];
  uint16_t total_len;
  uint16_t offset;
  uint8_t seq;
} frag_send_ctx_t;

/** Mesh 消息 (队列用) */
typedef struct {
  uint16_t src_mesh_id;
  uint16_t len;
  uint8_t data[TPMESH_MTU];
} mesh_msg_t;

/* ============================================================================
 * 私有变量
 * ============================================================================
 */

/** 以太网 netif */
static struct netif *s_eth_netif = NULL;

/** Top Node 配置 */
static top_config_t s_top_config;

/** DDC 配置 */
static ddc_config_t s_ddc_config;

/** 是否为 Top Node */
static bool s_is_top_node = false;

/** DDC 状态 */
static ddc_state_t s_ddc_state = DDC_STATE_INIT;

/** 注册重试计数 */
static uint8_t s_register_retry_count = 0;

/** 注册最后时间 */
static uint32_t s_last_register_tick = 0;

/** 心跳最后时间 */
static uint32_t s_last_heartbeat_tick = 0;

/** 广播限速器 */
static rate_limiter_t s_rate_limiter = {0, 0};

/** 分片重组会话 (Top Node 支持多个) */
#define MAX_REASSEMBLY_SESSIONS 10
static reassembly_session_t s_reassembly_sessions[MAX_REASSEMBLY_SESSIONS];

/** DDC 重组会话 (单个) */
static reassembly_session_t s_ddc_reassembly;

/** Mesh 消息队列 */
static QueueHandle_t s_mesh_msg_queue = NULL;

/** 初始化状态 */
static bool s_initialized = false;

/* ============================================================================
 * 私有函数声明
 * ============================================================================
 */

static void mesh_data_callback(uint16_t src_mesh_id, const uint8_t *data,
                               uint16_t len);
static void route_event_callback(const char *event, uint16_t addr);
static int fragment_and_send(uint16_t dest_mesh_id, const uint8_t *data,
                             uint16_t len);
static int reassemble_packet(uint16_t src_mesh_id, const uint8_t *data,
                             uint16_t len, uint8_t **out_data,
                             uint16_t *out_len);
static void process_register_frame(uint16_t src_mesh_id, const uint8_t *data,
                                   uint16_t len);
static void process_data_frame(uint16_t src_mesh_id, const uint8_t *data,
                               uint16_t len);
static void send_garp(const uint8_t *mac, const ip4_addr_t *ip);
static bridge_action_t check_arp_request(struct pbuf *p,
                                         const uint8_t **target_ip);
static int send_proxy_arp_reply_internal(struct pbuf *arp_request,
                                         const uint8_t *target_mac,
                                         const ip4_addr_t *target_ip);

/* ============================================================================
 * 公共函数 - 初始化
 * ============================================================================
 */

int tpmesh_top_init(struct netif *eth_netif, const top_config_t *config) {
  if (s_initialized) {
    return 0;
  }

  s_eth_netif = eth_netif;
  memcpy(&s_top_config, config, sizeof(top_config_t));
  s_is_top_node = true;

  /* ---- 仅硬件/内存初始化 (调度器前安全) ---- */
  node_table_init();

  if (tpmesh_at_init() != 0) {
    tpmesh_debug_printf("TPMesh: AT init failed\n");
    return -1;
  }

  /* 创建消息队列 (xQueueCreate 仅分配内存, 调度器前安全) */
  s_mesh_msg_queue = xQueueCreate(20, sizeof(mesh_msg_t));
  if (s_mesh_msg_queue == NULL) {
    tpmesh_debug_printf("TPMesh: Queue create failed\n");
    return -2;
  }

  /* 初始化重组会话 */
  memset(s_reassembly_sessions, 0, sizeof(s_reassembly_sessions));

  s_initialized = true;
  tpmesh_debug_printf("TPMesh Top: HW init done (Mesh ID: 0x%04X)\n",
                      config->mesh_id);
  tpmesh_debug_printf("  Note: module_init will run in bridge_task\n");

  return 0;
}

int tpmesh_ddc_init(const ddc_config_t *config) {
  if (s_initialized) {
    return 0;
  }

  tpmesh_debug_printf("DDC init: start\n");

  memcpy(&s_ddc_config, config, sizeof(ddc_config_t));
  s_is_top_node = false;
  s_ddc_state = DDC_STATE_INIT;

  /* ---- 仅硬件/内存初始化 (调度器前安全) ---- */
  node_table_init();

  if (tpmesh_at_init() != 0) {
    tpmesh_debug_printf("TPMesh: AT init failed\n");
    return -1;
  }

  /* 创建消息队列 (xQueueCreate 仅分配内存, 调度器前安全) */
  s_mesh_msg_queue = xQueueCreate(10, sizeof(mesh_msg_t));
  if (s_mesh_msg_queue == NULL) {
    tpmesh_debug_printf("TPMesh: Queue create failed\n");
    return -2;
  }

  memset(&s_ddc_reassembly, 0, sizeof(s_ddc_reassembly));

  s_initialized = true;
  tpmesh_debug_printf("DDC: HW init done (Mesh ID: 0x%04X)\n", config->mesh_id);
  tpmesh_debug_printf("  Note: module_init will run in bridge_task\n");

  return 0;
}

/* ============================================================================
 * 公共函数 - 桥接
 * ============================================================================
 */

bridge_action_t tpmesh_bridge_check(struct pbuf *p) {
  if (!s_initialized || !s_is_top_node || p->len < ETH_HDR_LEN) {
    return BRIDGE_LOCAL;
  }

  struct eth_hdr *eth = (struct eth_hdr *)p->payload;
  uint16_t ethertype = lwip_ntohs(eth->type);

  /* 检查目标 MAC */
  bool is_broadcast = schc_is_broadcast_mac((uint8_t *)&eth->dest);

  if (is_broadcast) {
    /* 广播帧处理 */
    if (ethertype == ETHTYPE_ARP) {
      /* ARP 广播 - 检查是否需要代理应答 */
      const uint8_t *target_ip;
      bridge_action_t action = check_arp_request(p, &target_ip);
      return action;
    }

    if (ethertype == ETHTYPE_IP) {
      /* IP 广播 - 检查 UDP 端口 */
      if (p->len >= ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN) {
        struct ip_hdr *iph =
            (struct ip_hdr *)((uint8_t *)p->payload + ETH_HDR_LEN);
        if (IPH_PROTO(iph) == IP_PROTO_UDP) {
          struct udp_hdr *udph =
              (struct udp_hdr *)((uint8_t *)iph + IPH_HL_BYTES(iph));
          uint16_t dst_port = lwip_ntohs(udph->dest);

          if (dst_port == TPMESH_PORT_BACNET) {
            /* BACnet 广播 (Who-Is) - 转发到 Mesh */
            return BRIDGE_TO_MESH;
          }
        }
      }
    }

    /* 其他广播 - 丢弃 (DHCP, NetBIOS 等) */
    return BRIDGE_DROP;
  }

  /* 单播帧 */
  uint8_t *dst_mac = (uint8_t *)&eth->dest;

  /* 检查是否发给 DDC */
  if (node_table_is_ddc_mac(dst_mac)) {
    return BRIDGE_TO_MESH;
  }

  /* 发给本机 */
  return BRIDGE_LOCAL;
}

int tpmesh_bridge_forward_to_mesh(struct pbuf *p) {
  if (!s_initialized || !s_is_top_node) {
    return -1;
  }

  struct eth_hdr *eth = (struct eth_hdr *)p->payload;
  bool is_broadcast = schc_is_broadcast_mac((uint8_t *)&eth->dest);

  /* SCHC 压缩 */
  uint8_t tunnel_buf[1600];
  uint16_t tunnel_len;

  if (schc_compress((uint8_t *)p->payload, p->tot_len, tunnel_buf, &tunnel_len,
                    is_broadcast) != 0) {
    return -2;
  }

  /* 确定目标 Mesh ID */
  uint16_t dest_mesh_id;
  if (is_broadcast) {
    /* 广播限速检查 */
    if (!broadcast_rate_check()) {
      tpmesh_debug_printf("TPMesh: Broadcast rate limited\n");
      return -3;
    }
    dest_mesh_id = MESH_ADDR_BROADCAST;
  } else {
    dest_mesh_id = node_table_get_mesh_by_mac((uint8_t *)&eth->dest);
    if (dest_mesh_id == MESH_ADDR_INVALID) {
      tpmesh_debug_printf("TPMesh: Unknown destination MAC\n");
      return -4;
    }
  }

  /* 分片发送 */
  return fragment_and_send(dest_mesh_id, tunnel_buf, tunnel_len);
}

int tpmesh_bridge_send_proxy_arp(struct pbuf *p) {
  if (!s_initialized || !s_is_top_node) {
    return -1;
  }

  /* 解析 ARP 请求 */
  struct etharp_hdr *arp =
      (struct etharp_hdr *)((uint8_t *)p->payload + SIZEOF_ETH_HDR);

  ip4_addr_t target_ip;
  SMEMCPY(&target_ip, &arp->dipaddr, sizeof(ip4_addr_t));

  /* 获取目标 DDC 的 MAC */
  uint8_t target_mac[6];
  if (node_table_get_mac_by_ip(&target_ip, target_mac) != 0) {
    return -2;
  }

  return send_proxy_arp_reply_internal(p, target_mac, &target_ip);
}

void tpmesh_bridge_handle_mesh_data(uint16_t src_mesh_id, const uint8_t *data,
                                    uint16_t len) {
  if (!s_initialized) {
    return;
  }

  /* 检查是否为注册帧 */
  if (len >= 1 && data[2] == SCHC_RULE_REGISTER) {
    process_register_frame(src_mesh_id, data + TPMESH_TUNNEL_HDR_LEN,
                           len - TPMESH_TUNNEL_HDR_LEN);
    return;
  }

  /* 数据帧处理 */
  process_data_frame(src_mesh_id, data, len);
}

void tpmesh_bridge_task(void *arg) {
  (void)arg;

  tpmesh_debug_printf("Bridge Task: started\n");

  /* ================================================================
   * Phase 1: 业务初始化 (调度器已启动, RX Task 已运行)
   * ================================================================ */

  /* 设置回调 */
  tpmesh_at_set_data_cb(mesh_data_callback);
  tpmesh_at_set_route_cb(route_event_callback);

  /* 模组 AT 命令初始化 (支持重试) */
  {
    uint16_t mesh_id =
        s_is_top_node ? s_top_config.mesh_id : s_ddc_config.mesh_id;
    int max_retries = 10;
    int retry = 0;

    while (tpmesh_module_init(mesh_id, s_is_top_node) != 0) {
      retry++;
      if (retry >= max_retries) {
        tpmesh_debug_printf("Bridge: Module init FAILED after %d retries!\n",
                            retry);
        /* 继续运行, 但功能受限 */
        break;
      }
      tpmesh_debug_printf("Bridge: Module init retry %d/%d ...\n", retry,
                          max_retries);
      vTaskDelay(pdMS_TO_TICKS(3000));
    }
  }

  tpmesh_debug_printf(
      "Bridge Task: business init complete, entering main loop\n");

  /* ================================================================
   * Phase 2: 主循环
   * ================================================================ */

  mesh_msg_t msg;

  while (1) {
    /* 从队列获取消息 */
    if (xQueueReceive(s_mesh_msg_queue, &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
      tpmesh_bridge_handle_mesh_data(msg.src_mesh_id, msg.data, msg.len);
    }

    /* 节点表维护 */
    node_table_check_timeout();
  }
}

/* ============================================================================
 * 公共函数 - DDC 注册/心跳
 * ============================================================================
 */

int ddc_send_register(const ddc_config_t *config) {
  reg_frame_t frame;
  frame.frame_type = REG_FRAME_REGISTER;
  memcpy(frame.mac, config->mac_addr, 6);
  frame.ip = ip4_addr_get_u32(&config->ip_addr);
  frame.mesh_id = config->mesh_id;
  frame.checksum = tpmesh_calc_crc16(&frame, sizeof(frame) - 2);

  /* 封装为隧道帧 */
  uint8_t tunnel_buf[32];
  tunnel_buf[0] = 0x00; /* L2 HDR: 单播 */
  tunnel_buf[1] = 0x80; /* FRAG HDR: 单片 */
  tunnel_buf[2] = SCHC_RULE_REGISTER;
  memcpy(tunnel_buf + 3, &frame, sizeof(frame));

  tpmesh_debug_printf("TPMesh DDC: Sending register to Top Node\n");
  return tpmesh_at_send(MESH_ADDR_TOP_NODE, tunnel_buf, 3 + sizeof(frame));
}

int ddc_send_heartbeat(const ddc_config_t *config) {
  reg_frame_t frame;
  frame.frame_type = REG_FRAME_HEARTBEAT;
  memcpy(frame.mac, config->mac_addr, 6);
  frame.ip = ip4_addr_get_u32(&config->ip_addr);
  frame.mesh_id = config->mesh_id;
  frame.checksum = tpmesh_calc_crc16(&frame, sizeof(frame) - 2);

  uint8_t tunnel_buf[32];
  tunnel_buf[0] = 0x00;
  tunnel_buf[1] = 0x80;
  tunnel_buf[2] = SCHC_RULE_REGISTER;
  memcpy(tunnel_buf + 3, &frame, sizeof(frame));

  return tpmesh_at_send(MESH_ADDR_TOP_NODE, tunnel_buf, 3 + sizeof(frame));
}

void ddc_heartbeat_task(void *arg) {
  (void)arg;

  while (1) {
    uint32_t now = tpmesh_get_tick_ms();

    switch (s_ddc_state) {
    case DDC_STATE_INIT:
      /* 等待路由建立 */
      vTaskDelay(pdMS_TO_TICKS(1000));
      break;

    case DDC_STATE_REGISTERING:
      /* 注册重试 */
      if (now - s_last_register_tick > TPMESH_REGISTER_RETRY_MS) {
        if (s_register_retry_count < TPMESH_REGISTER_MAX_RETRIES) {
          ddc_send_register(&s_ddc_config);
          s_last_register_tick = now;
          s_register_retry_count++;
        } else {
          tpmesh_debug_printf("TPMesh DDC: Register timeout, reset module\n");
          tpmesh_module_reset();
          s_ddc_state = DDC_STATE_INIT;
          s_register_retry_count = 0;
        }
      }
      break;

    case DDC_STATE_ONLINE:
      /* 心跳 */
      if (now - s_last_heartbeat_tick > TPMESH_HEARTBEAT_MS) {
        ddc_send_heartbeat(&s_ddc_config);
        s_last_heartbeat_tick = now;
      }
      break;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

/* ============================================================================
 * 公共函数 - 广播限速
 * ============================================================================
 */

bool broadcast_rate_check(void) {
  uint32_t now = tpmesh_get_tick_ms();
  uint32_t elapsed = now - s_rate_limiter.last_tick;

  if (elapsed > TPMESH_BROADCAST_RATE_MS) {
    /* 新周期 */
    s_rate_limiter.last_tick = now;
    s_rate_limiter.burst_count = 1;
    return true;
  }

  if (s_rate_limiter.burst_count < TPMESH_BROADCAST_BURST_MAX) {
    s_rate_limiter.burst_count++;
    return true;
  }

  return false;
}

/* ============================================================================
 * 公共函数 - 工具
 * ============================================================================
 */

uint32_t tpmesh_get_tick_ms(void) {
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

uint16_t tpmesh_calc_crc16(const void *data, uint16_t len) {
  const uint8_t *p = (const uint8_t *)data;
  uint16_t crc = 0xFFFF;

  for (uint16_t i = 0; i < len; i++) {
    crc ^= p[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}

bool tpmesh_is_broadcast_mac(const uint8_t *mac) {
  return schc_is_broadcast_mac(mac);
}

void tpmesh_get_local_mac(uint8_t *mac) {
  if (s_is_top_node) {
    memcpy(mac, s_top_config.mac_addr, 6);
  } else {
    memcpy(mac, s_ddc_config.mac_addr, 6);
  }
}

/* ============================================================================
 * 私有函数 - 回调
 * ============================================================================
 */

static void mesh_data_callback(uint16_t src_mesh_id, const uint8_t *data,
                               uint16_t len) {
  if (!s_initialized) {
    return;
  }

  /* 放入队列,由任务处理 */
  if (s_mesh_msg_queue) {
    mesh_msg_t msg;
    msg.src_mesh_id = src_mesh_id;
    msg.len = (len > TPMESH_MTU) ? TPMESH_MTU : len;
    memcpy(msg.data, data, msg.len);
    xQueueSend(s_mesh_msg_queue, &msg, 0);
  }
}

static void route_event_callback(const char *event, uint16_t addr) {
  tpmesh_debug_printf("TPMesh Route: %s 0x%04X\n", event, addr);

  if (!s_is_top_node && strncmp(event, "CREATE", 6) == 0 &&
      addr == MESH_ADDR_TOP_NODE) {
    /* DDC 发现 Top Node,开始注册 */
    tpmesh_debug_printf("TPMesh DDC: Top Node discovered, start registering\n");
    s_ddc_state = DDC_STATE_REGISTERING;
    s_register_retry_count = 0;
    s_last_register_tick = 0;
  }
}

/* ============================================================================
 * 私有函数 - 分片发送
 * ============================================================================
 */

static int fragment_and_send(uint16_t dest_mesh_id, const uint8_t *data,
                             uint16_t len) {
  if (len <= TPMESH_MTU) {
    /* 无需分片 */
    return tpmesh_at_send(dest_mesh_id, data, len);
  }

  /* 分片发送 */
  uint8_t seq = 0;
  uint16_t offset = 0;

  while (offset < len) {
    uint8_t packet[TPMESH_MTU];
    uint16_t chunk_len;

    if (seq == 0) {
      /* 第一片: 包含完整隧道头 */
      chunk_len = (len > TPMESH_MTU) ? TPMESH_MTU : len;
      memcpy(packet, data, chunk_len);
      /* 更新分片头: 非最后一片 */
      if (len > TPMESH_MTU) {
        packet[1] = seq & 0x7F; /* Last=0, Seq=0 */
      }
    } else {
      /* 后续片: 简化头 (仅 FRAG_HDR) */
      uint16_t remain = len - offset;
      chunk_len = (remain > TPMESH_MTU - 1) ? (TPMESH_MTU - 1) : remain;

      if (remain <= TPMESH_MTU - 1) {
        packet[0] = 0x80 | (seq & 0x7F); /* Last=1 */
      } else {
        packet[0] = seq & 0x7F; /* Last=0 */
      }
      memcpy(packet + 1, data + offset, chunk_len);
      chunk_len += 1;
    }

    if (tpmesh_at_send(dest_mesh_id, packet, chunk_len) != 0) {
      return -1;
    }

    if (seq == 0) {
      offset = TPMESH_MTU;
    } else {
      offset += TPMESH_MTU - 1;
    }
    seq++;
  }

  return 0;
}

/* ============================================================================
 * 私有函数 - 分片重组
 * ============================================================================
 */

static int reassemble_packet(uint16_t src_mesh_id, const uint8_t *data,
                             uint16_t len, uint8_t **out_data,
                             uint16_t *out_len) {
  reassembly_session_t *session = NULL;

  if (s_is_top_node) {
    /* Top Node: 查找或创建会话 */
    for (int i = 0; i < MAX_REASSEMBLY_SESSIONS; i++) {
      if (s_reassembly_sessions[i].active &&
          s_reassembly_sessions[i].src_mesh_id == src_mesh_id) {
        session = &s_reassembly_sessions[i];
        break;
      }
    }
    if (!session) {
      /* 新会话 */
      for (int i = 0; i < MAX_REASSEMBLY_SESSIONS; i++) {
        if (!s_reassembly_sessions[i].active) {
          session = &s_reassembly_sessions[i];
          break;
        }
      }
      if (!session) {
        /* 替换最老的 */
        uint32_t oldest = 0xFFFFFFFF;
        for (int i = 0; i < MAX_REASSEMBLY_SESSIONS; i++) {
          if (s_reassembly_sessions[i].last_tick < oldest) {
            oldest = s_reassembly_sessions[i].last_tick;
            session = &s_reassembly_sessions[i];
          }
        }
      }
    }
  } else {
    /* DDC: 单会话 */
    session = &s_ddc_reassembly;
  }

  if (!session) {
    return -1;
  }

  /* 解析分片头 */
  uint8_t frag_hdr = data[1];
  bool is_last = (frag_hdr & 0x80) != 0;
  uint8_t seq = frag_hdr & 0x7F;

  if (seq == 0) {
    /* 第一片: 重置会话 */
    session->active = true;
    session->src_mesh_id = src_mesh_id;
    session->total_len = 0;
    session->expected_seq = 0;
    session->last_tick = tpmesh_get_tick_ms();
  }

  /* 序号检查 */
  if (seq != session->expected_seq) {
    tpmesh_debug_printf("TPMesh: Fragment seq mismatch (got %d, expect %d)\n",
                        seq, session->expected_seq);
    session->active = false;
    return -2;
  }

  /* 超时检查 */
  if (tpmesh_get_tick_ms() - session->last_tick > 5000) {
    session->active = false;
    return -3;
  }

  /* 复制数据 */
  if (seq == 0) {
    /* 第一片: 完整数据 */
    if (session->total_len + len > sizeof(session->buffer)) {
      session->active = false;
      return -4;
    }
    memcpy(session->buffer + session->total_len, data, len);
    session->total_len += len;
  } else {
    /* 后续片: 跳过 FRAG_HDR */
    if (session->total_len + len - 1 > sizeof(session->buffer)) {
      session->active = false;
      return -4;
    }
    memcpy(session->buffer + session->total_len, data + 1, len - 1);
    session->total_len += len - 1;
  }

  session->expected_seq++;
  session->last_tick = tpmesh_get_tick_ms();

  if (is_last) {
    /* 重组完成 */
    *out_data = session->buffer;
    *out_len = session->total_len;
    session->active = false;
    return 1; /* 完成 */
  }

  return 0; /* 继续等待 */
}

/* ============================================================================
 * 私有函数 - 注册帧处理
 * ============================================================================
 */

static void process_register_frame(uint16_t src_mesh_id, const uint8_t *data,
                                   uint16_t len) {
  if (len < sizeof(reg_frame_t)) {
    return;
  }

  const reg_frame_t *frame = (const reg_frame_t *)data;

  /* 校验 CRC */
  uint16_t crc = tpmesh_calc_crc16(frame, sizeof(reg_frame_t) - 2);
  if (crc != frame->checksum) {
    tpmesh_debug_printf("TPMesh: Register frame CRC error\n");
    return;
  }

  if (s_is_top_node) {
    /* Top Node 处理注册/心跳 */
    switch (frame->frame_type) {
    case REG_FRAME_REGISTER:
    case REG_FRAME_HEARTBEAT: {
      ip4_addr_t ip;
      ip4_addr_set_u32(&ip, frame->ip);

      if (frame->frame_type == REG_FRAME_REGISTER) {
        /* 注册: 添加到节点表 */
        node_table_register(frame->mac, &ip, src_mesh_id);

        /* 发送 GARP */
        send_garp(frame->mac, &ip);

        /* 发送 ACK */
        reg_frame_t ack;
        ack.frame_type = REG_FRAME_REGISTER_ACK;
        memcpy(ack.mac, s_top_config.mac_addr, 6);
        ack.ip = ip4_addr_get_u32(&s_top_config.ip_addr);
        ack.mesh_id = s_top_config.mesh_id;
        ack.checksum = tpmesh_calc_crc16(&ack, sizeof(ack) - 2);

        uint8_t buf[32];
        buf[0] = 0x00;
        buf[1] = 0x80;
        buf[2] = SCHC_RULE_REGISTER;
        memcpy(buf + 3, &ack, sizeof(ack));
        tpmesh_at_send(src_mesh_id, buf, 3 + sizeof(ack));

        tpmesh_debug_printf("TPMesh Top: DDC 0x%04X registered\n", src_mesh_id);
      } else {
        /* 心跳: 更新活跃时间 */
        node_table_touch(src_mesh_id);

        /* 发送 ACK */
        reg_frame_t ack;
        ack.frame_type = REG_FRAME_HEARTBEAT_ACK;
        memcpy(ack.mac, s_top_config.mac_addr, 6);
        ack.ip = ip4_addr_get_u32(&s_top_config.ip_addr);
        ack.mesh_id = s_top_config.mesh_id;
        ack.checksum = tpmesh_calc_crc16(&ack, sizeof(ack) - 2);

        uint8_t buf[32];
        buf[0] = 0x00;
        buf[1] = 0x80;
        buf[2] = SCHC_RULE_REGISTER;
        memcpy(buf + 3, &ack, sizeof(ack));
        tpmesh_at_send(src_mesh_id, buf, 3 + sizeof(ack));
      }
      break;
    }
    default:
      break;
    }
  } else {
    /* DDC 处理 ACK */
    switch (frame->frame_type) {
    case REG_FRAME_REGISTER_ACK:
      tpmesh_debug_printf("TPMesh DDC: Register ACK received\n");
      s_ddc_state = DDC_STATE_ONLINE;
      s_last_heartbeat_tick = tpmesh_get_tick_ms();
      break;

    case REG_FRAME_HEARTBEAT_ACK:
      /* 心跳确认,无需特殊处理 */
      break;

    default:
      break;
    }
  }
}

/* ============================================================================
 * 私有函数 - 数据帧处理
 * ============================================================================
 */

static void process_data_frame(uint16_t src_mesh_id, const uint8_t *data,
                               uint16_t len) {
  /* 分片重组 */
  uint8_t *complete_data;
  uint16_t complete_len;

  int ret =
      reassemble_packet(src_mesh_id, data, len, &complete_data, &complete_len);
  if (ret != 1) {
    return; /* 未完成 */
  }

  /* 更新节点活跃时间 */
  node_table_touch(src_mesh_id);

  /* SCHC 解压 */
  uint8_t eth_frame[1600];
  uint16_t eth_len;

  uint16_t dst_mesh_id =
      s_is_top_node ? s_top_config.mesh_id : s_ddc_config.mesh_id;

  if (schc_decompress(complete_data, complete_len, eth_frame, &eth_len,
                      src_mesh_id, dst_mesh_id) != 0) {
    tpmesh_debug_printf("TPMesh: Decompress failed\n");
    return;
  }

  if (s_is_top_node) {
    /* Top Node: 转发到以太网 */
    if (s_eth_netif && s_eth_netif->linkoutput) {
      struct pbuf *p = pbuf_alloc(PBUF_RAW, eth_len, PBUF_RAM);
      if (p) {
        memcpy(p->payload, eth_frame, eth_len);
        s_eth_netif->linkoutput(s_eth_netif, p);
        pbuf_free(p);
      }
    }
  } else {
    /* DDC: 交给本地协议栈处理 */
    /* TODO: 调用 LwIP 处理 */
  }
}

/* ============================================================================
 * 私有函数 - ARP 处理
 * ============================================================================
 */

static bridge_action_t check_arp_request(struct pbuf *p,
                                         const uint8_t **target_ip) {
  if (p->len < SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR) {
    return BRIDGE_LOCAL;
  }

  struct etharp_hdr *arp =
      (struct etharp_hdr *)((uint8_t *)p->payload + SIZEOF_ETH_HDR);

  /* 只处理 ARP 请求 */
  if (arp->opcode != PP_HTONS(ARP_REQUEST)) {
    return BRIDGE_LOCAL;
  }

  /* 检查目标 IP 是否为 DDC */
  ip4_addr_t target;
  SMEMCPY(&target, &arp->dipaddr, sizeof(ip4_addr_t));

  if (node_table_is_ddc_ip(&target)) {
    *target_ip = (const uint8_t *)&arp->dipaddr;
    return BRIDGE_PROXY_ARP;
  }

  return BRIDGE_LOCAL;
}

static int send_proxy_arp_reply_internal(struct pbuf *arp_request,
                                         const uint8_t *target_mac,
                                         const ip4_addr_t *target_ip) {
  if (!s_eth_netif) {
    return -1;
  }

  /* 原始 ARP 请求 */
  struct eth_hdr *req_eth = (struct eth_hdr *)arp_request->payload;
  struct etharp_hdr *req_arp =
      (struct etharp_hdr *)((uint8_t *)arp_request->payload + SIZEOF_ETH_HDR);

  /* 分配 ARP 回复 */
  struct pbuf *p =
      pbuf_alloc(PBUF_RAW, SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR, PBUF_RAM);
  if (!p) {
    return -2;
  }

  struct eth_hdr *eth = (struct eth_hdr *)p->payload;
  struct etharp_hdr *arp =
      (struct etharp_hdr *)((uint8_t *)p->payload + SIZEOF_ETH_HDR);

  /* 以太网头 */
  SMEMCPY(&eth->dest, &req_eth->src, ETH_HWADDR_LEN);
  SMEMCPY(&eth->src, target_mac, ETH_HWADDR_LEN);
  eth->type = PP_HTONS(ETHTYPE_ARP);

  /* ARP 回复 */
  arp->hwtype = PP_HTONS(1);
  arp->proto = PP_HTONS(ETHTYPE_IP);
  arp->hwlen = ETH_HWADDR_LEN;
  arp->protolen = sizeof(ip4_addr_t);
  arp->opcode = PP_HTONS(ARP_REPLY);

  /* 发送方: DDC */
  SMEMCPY(&arp->shwaddr, target_mac, ETH_HWADDR_LEN);
  SMEMCPY(&arp->sipaddr, target_ip, sizeof(ip4_addr_t));

  /* 目标: 原请求者 */
  SMEMCPY(&arp->dhwaddr, &req_arp->shwaddr, ETH_HWADDR_LEN);
  SMEMCPY(&arp->dipaddr, &req_arp->sipaddr, sizeof(ip4_addr_t));

  /* 发送 */
  s_eth_netif->linkoutput(s_eth_netif, p);
  pbuf_free(p);

  tpmesh_debug_printf("TPMesh: Proxy ARP reply sent for %d.%d.%d.%d\n",
                      ip4_addr1(target_ip), ip4_addr2(target_ip),
                      ip4_addr3(target_ip), ip4_addr4(target_ip));

  return 0;
}

/* ============================================================================
 * 私有函数 - GARP
 * ============================================================================
 */

static void send_garp(const uint8_t *mac, const ip4_addr_t *ip) {
  if (!s_eth_netif) {
    return;
  }

  /* 分配 GARP */
  struct pbuf *p =
      pbuf_alloc(PBUF_RAW, SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR, PBUF_RAM);
  if (!p) {
    return;
  }

  struct eth_hdr *eth = (struct eth_hdr *)p->payload;
  struct etharp_hdr *arp =
      (struct etharp_hdr *)((uint8_t *)p->payload + SIZEOF_ETH_HDR);

  /* 以太网头: 广播 */
  memset(&eth->dest, 0xFF, ETH_HWADDR_LEN);
  SMEMCPY(&eth->src, mac, ETH_HWADDR_LEN);
  eth->type = PP_HTONS(ETHTYPE_ARP);

  /* ARP: GARP */
  arp->hwtype = PP_HTONS(1);
  arp->proto = PP_HTONS(ETHTYPE_IP);
  arp->hwlen = ETH_HWADDR_LEN;
  arp->protolen = sizeof(ip4_addr_t);
  arp->opcode = PP_HTONS(ARP_REPLY);

  SMEMCPY(&arp->shwaddr, mac, ETH_HWADDR_LEN);
  SMEMCPY(&arp->sipaddr, ip, sizeof(ip4_addr_t));
  SMEMCPY(&arp->dhwaddr, mac, ETH_HWADDR_LEN);
  SMEMCPY(&arp->dipaddr, ip, sizeof(ip4_addr_t));

  s_eth_netif->linkoutput(s_eth_netif, p);
  pbuf_free(p);

  tpmesh_debug_printf("TPMesh: GARP sent for %d.%d.%d.%d\n", ip4_addr1(ip),
                      ip4_addr2(ip), ip4_addr3(ip), ip4_addr4(ip));
}

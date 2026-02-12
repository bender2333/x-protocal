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
#include "tpmesh_init.h"
#include "tpmesh_schc.h"

/* node_table.h 已通过 tpmesh_bridge.h 包含 */
#include "FreeRTOS.h"
#include "lwip/err.h"
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
static struct netif *s_ddc_netif = NULL;
static netif_linkoutput_fn s_ddc_phy_linkoutput = NULL;

/** Top Node 配置 */
static top_config_t s_top_config;

/** DDC 配置 */
static ddc_config_t s_ddc_config;

/** 是否为 Top Node */
static bool s_is_top_node = false;

/** DDC 状态 */
static volatile ddc_state_t s_ddc_state = DDC_STATE_INIT;

/** 注册重试计数 */
static volatile uint8_t s_register_retry_count = 0;

/** 注册最后时间 */
static volatile uint32_t s_last_register_tick = 0;

/** 心跳最后时间 */
static volatile uint32_t s_last_heartbeat_tick = 0;

/** 心跳 ACK 最后时间 */
static volatile uint32_t s_last_heartbeat_ack_tick = 0;

/** 心跳发送时间 (等待 ACK 用) */
static volatile uint32_t s_last_heartbeat_send_tick = 0;

/** 心跳 ACK 连续丢失计数 */
static volatile uint8_t s_heartbeat_miss_count = 0;

/** 当前是否等待心跳 ACK */
static volatile bool s_waiting_heartbeat_ack = false;
static volatile uint32_t s_last_ddc_preonline_drop_log_tick = 0;
static volatile uint32_t s_last_self_loop_drop_log_tick = 0;

/** 广播限速器 */
static rate_limiter_t s_rate_limiter = {0, 0};

/** 分片重组会话 (Top Node 支持多个) */
#define MAX_REASSEMBLY_SESSIONS 10
static reassembly_session_t s_reassembly_sessions[MAX_REASSEMBLY_SESSIONS];

/** DDC 重组会话 (单个) */
static reassembly_session_t s_ddc_reassembly;
/* Reuse in bridge task only; avoids large stack frame in process_data_frame().
 */
static uint8_t s_rebuild_eth_frame[1600];

/** Mesh 消息队列 */
static QueueHandle_t s_mesh_msg_queue = NULL;

/** 初始化状态 */
static bool s_initialized = false;

/* DDC 注入本地协议栈使用的默认网卡 */
extern struct netif *netif_default;

/* ============================================================================
 * 私有函数声明
 * ============================================================================
 */

static void mesh_data_callback(uint16_t src_mesh_id, const uint8_t *data,
                               uint16_t len);
static void route_event_callback(const char *event, uint16_t addr);
static int fragment_and_send(uint16_t dest_mesh_id, const uint8_t *data,
                             uint16_t len, const char *source);
static int reassemble_packet(uint16_t src_mesh_id, const uint8_t *data,
                             uint16_t len, uint8_t **out_data,
                             uint16_t *out_len);
static void process_register_frame(uint16_t src_mesh_id, const uint8_t *data,
                                   uint16_t len);
static void process_data_frame(uint16_t src_mesh_id, const uint8_t *data,
                               uint16_t len);
static void send_garp(const uint8_t *mac, const ip4_addr_t *ip);
static bool pbuf_copy_exact(struct pbuf *p, uint16_t offset, void *dst,
                            uint16_t len);
static bool clone_pbuf_contiguous(struct pbuf *src, struct pbuf **out);
static int build_no_compress_tunnel(const uint8_t *eth_frame, uint16_t eth_len,
                                    bool is_broadcast, uint8_t *out_data,
                                    uint16_t *out_len);
static err_t ddc_netif_linkoutput_hook(struct netif *netif, struct pbuf *p);
static bool ddc_should_forward_arp_to_mesh(const struct etharp_hdr *arp);
static bool ddc_should_forward_to_mesh(struct pbuf *p);
static bridge_action_t check_arp_request(struct pbuf *p);
static int send_proxy_arp_reply_internal(struct pbuf *arp_request,
                                         const uint8_t *target_mac,
                                         const ip4_addr_t *target_ip);
static const char *ddc_state_to_str(ddc_state_t state);
static void trace_tunnel_packet_summary(const char *source, const uint8_t *data,
                                        uint16_t len);
static void trace_ddc_inject_frame(uint16_t src_mesh_id, const uint8_t *frame,
                                   uint16_t len);
static int at_send_with_trace(const char *source, uint16_t dest_mesh_id,
                              const uint8_t *data, uint16_t len);
static uint16_t ip_checksum16(const uint8_t *buf, uint16_t len);
static void top_fix_bacnet_src_ip(uint16_t src_mesh_id, uint8_t *eth_frame,
                                  uint16_t eth_len);

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
  if (node_table_add_static(config->mac_addr, &config->ip_addr,
                            config->mesh_id) != 0) {
    tpmesh_debug_printf("TPMesh Top: add self static node failed\n");
  } else {
    node_table_touch(config->mesh_id);
  }

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
  if (node_table_add_static(config->mac_addr, &config->ip_addr,
                            config->mesh_id) != 0) {
    tpmesh_debug_printf("TPMesh DDC: add self static node failed\n");
  } else {
    node_table_touch(config->mesh_id);
  }

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
  s_last_heartbeat_tick = 0;
  s_last_heartbeat_ack_tick = 0;
  s_last_heartbeat_send_tick = 0;
  s_heartbeat_miss_count = 0;
  s_waiting_heartbeat_ack = false;

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
  if (!s_initialized || !s_is_top_node || p == NULL ||
      p->tot_len < SIZEOF_ETH_HDR) {
    return BRIDGE_LOCAL;
  }
  struct eth_hdr eth_hdr;
  if (!pbuf_copy_exact(p, 0, &eth_hdr, SIZEOF_ETH_HDR)) {
    return BRIDGE_LOCAL;
  }
  uint16_t ethertype = lwip_ntohs(eth_hdr.type);
  bool is_broadcast = schc_is_broadcast_mac((const uint8_t *)&eth_hdr.dest);
  if (is_broadcast) {
    if (ethertype == ETHTYPE_ARP) {
      return check_arp_request(p);
    }
    if (ethertype == ETHTYPE_IP) {
      if (p->tot_len >= (SIZEOF_ETH_HDR + IP_HLEN + UDP_HLEN)) {
        uint8_t l3_hdr_buf[SIZEOF_ETH_HDR + IP_HLEN_MAX + UDP_HLEN];
        uint16_t copy_len = (uint16_t)sizeof(l3_hdr_buf);
        if (p->tot_len < copy_len) {
          copy_len = p->tot_len;
        }
        if (!pbuf_copy_exact(p, 0, l3_hdr_buf, copy_len)) {
          return BRIDGE_DROP;
        }
        struct ip_hdr *iph = (struct ip_hdr *)(l3_hdr_buf + SIZEOF_ETH_HDR);
        uint16_t ihl = IPH_HL_BYTES(iph);
        if (ihl >= IP_HLEN && ihl <= IP_HLEN_MAX &&
            (SIZEOF_ETH_HDR + ihl + UDP_HLEN) <= copy_len &&
            IPH_PROTO(iph) == IP_PROTO_UDP) {
          struct udp_hdr *udph = (struct udp_hdr *)((uint8_t *)iph + ihl);
          if (lwip_ntohs(udph->dest) == TPMESH_PORT_BACNET) {
            return BRIDGE_TO_MESH;
          }
        }
      }
    }
    return BRIDGE_DROP;
  }
  uint16_t dst_mesh_id =
      node_table_get_mesh_by_mac((const uint8_t *)&eth_hdr.dest);
  if (dst_mesh_id == MESH_ADDR_INVALID) {
    return BRIDGE_LOCAL;
  }
  return node_table_is_online(dst_mesh_id) ? BRIDGE_TO_MESH : BRIDGE_DROP;
}

int tpmesh_bridge_forward_to_mesh(struct pbuf *p) {
  if (!s_initialized || !tpmesh_data_plane_enabled() || p == NULL ||
      p->tot_len < SIZEOF_ETH_HDR) {
    return -1;
  }
  if (!tpmesh_at_is_uart6_active()) {
    return -5;
  }
  int ret = 0;
  struct pbuf *contig = NULL;
  struct pbuf *frame = p;
  if (p->len < p->tot_len) {
    if (!clone_pbuf_contiguous(p, &contig)) {
      return -6;
    }
    frame = contig;
  }
  struct eth_hdr *eth = (struct eth_hdr *)frame->payload;
  bool is_broadcast = schc_is_broadcast_mac((uint8_t *)&eth->dest);
  uint8_t tunnel_buf[1600];
  uint16_t tunnel_len;
  uint16_t dest_mesh_id;
  const char *send_source;

  if (s_is_top_node) {
    if (schc_compress((uint8_t *)frame->payload, frame->tot_len, tunnel_buf,
                      &tunnel_len, is_broadcast) != 0) {
      ret = -2;
      goto out;
    }

    if (is_broadcast) {
      if (!broadcast_rate_check()) {
        tpmesh_debug_printf("TPMesh: Broadcast rate limited\n");
        ret = -3;
        goto out;
      }
      dest_mesh_id = MESH_ADDR_BROADCAST;
      send_source = "bridge_top_bcast";
    } else {
      dest_mesh_id = node_table_get_mesh_by_mac((uint8_t *)&eth->dest);
      if (dest_mesh_id == MESH_ADDR_INVALID ||
          !node_table_is_online(dest_mesh_id)) {
        tpmesh_debug_printf("TPMesh: destination node unavailable\n");
        ret = -4;
        goto out;
      }
      send_source = "bridge_top_ucast";
    }
  } else {
    if (build_no_compress_tunnel((const uint8_t *)frame->payload,
                                 frame->tot_len, is_broadcast, tunnel_buf,
                                 &tunnel_len) != 0) {
      ret = -2;
      goto out;
    }
    dest_mesh_id = MESH_ADDR_TOP_NODE;
    send_source = "bridge_ddc_to_top";
  }

  ret = fragment_and_send(dest_mesh_id, tunnel_buf, tunnel_len, send_source);
out:
  if (contig != NULL) {
    pbuf_free(contig);
  }
  return ret;
}

int tpmesh_bridge_attach_ddc_netif(struct netif *netif) {
  if (netif == NULL || netif->linkoutput == NULL) {
    return -1;
  }
  if (netif->linkoutput == ddc_netif_linkoutput_hook && s_ddc_netif == netif) {
    return 0;
  }

  if (netif->linkoutput != ddc_netif_linkoutput_hook) {
    s_ddc_phy_linkoutput = netif->linkoutput;
  }
  s_ddc_netif = netif;
  netif->linkoutput = ddc_netif_linkoutput_hook;
  tpmesh_debug_printf("TPMesh DDC: netif linkoutput hooked to Mesh bridge\n");
  return 0;
}

int tpmesh_bridge_send_proxy_arp(struct pbuf *p) {
  if (!s_initialized || !s_is_top_node || p == NULL ||
      p->tot_len < (SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR)) {
    return -1;
  }
  struct pbuf *contig = NULL;
  struct pbuf *frame = p;
  if (p->len < p->tot_len) {
    if (!clone_pbuf_contiguous(p, &contig)) {
      return -3;
    }
    frame = contig;
  }
  int ret = 0;
  struct etharp_hdr *arp =
      (struct etharp_hdr *)((uint8_t *)frame->payload + SIZEOF_ETH_HDR);
  if (arp->opcode != PP_HTONS(ARP_REQUEST)) {
    ret = -2;
    goto out;
  }
  ip4_addr_t target_ip;
  SMEMCPY(&target_ip, &arp->dipaddr, sizeof(ip4_addr_t));
  uint16_t mesh_id = node_table_get_mesh_by_ip(&target_ip);
  if (mesh_id == MESH_ADDR_INVALID || !node_table_is_online(mesh_id)) {
    ret = -2;
    goto out;
  }
  uint8_t target_mac[6];
  if (node_table_get_mac_by_ip(&target_ip, target_mac) != 0) {
    ret = -2;
    goto out;
  }
  ret = send_proxy_arp_reply_internal(frame, target_mac, &target_ip);
out:
  if (contig != NULL) {
    pbuf_free(contig);
  }
  return ret;
}

void tpmesh_bridge_handle_mesh_data(uint16_t src_mesh_id, const uint8_t *data,
                                    uint16_t len) {
  if (!s_initialized || !tpmesh_data_plane_enabled() || data == NULL ||
      len == 0) {
    return;
  }

  /* Top node may receive its own broadcast echo from module; ignore it. */
  if (s_is_top_node && src_mesh_id == s_top_config.mesh_id) {
    uint32_t now = tpmesh_get_tick_ms();
    if (now - s_last_self_loop_drop_log_tick > 1000) {
      s_last_self_loop_drop_log_tick = now;
      tpmesh_debug_printf(
          "TPMesh: drop self-loop mesh frame src=0x%04X len=%u\n", src_mesh_id,
          len);
    }
    return;
  }

  if (len < TPMESH_TUNNEL_HDR_LEN) {
    tpmesh_debug_printf("TPMesh: Drop short mesh frame len=%u\n", len);
    return;
  }

  /* 检查是否为注册帧/心跳帧 */
  if (data[2] == SCHC_RULE_REGISTER) {
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
  TickType_t last_stack_log_tick = 0;

  /* ================================================================
   * Phase 1: 业务初始化 (调度器已启动, RX Task 已运行)
   * ================================================================ */

  /* 设置回调 */
  tpmesh_at_set_data_cb(mesh_data_callback);
  tpmesh_at_set_route_cb(route_event_callback);

/* 模组初始化策略 */
#if (TPMESH_MODULE_INIT_POLICY == TPMESH_MODULE_INIT_BY_X_PROTOCOL)
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
#else
  tpmesh_debug_printf(
      "Bridge: external config mode, skip module AT init sequence\n");
#endif

  tpmesh_debug_printf(
      "Bridge Task: business init complete, entering main loop\n");

  /* ================================================================
   * Phase 2: 主循环
   * ================================================================ */

  mesh_msg_t msg;

  while (1) {
#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
    TickType_t now_tick = xTaskGetTickCount();
    if ((last_stack_log_tick == 0) ||
        ((now_tick - last_stack_log_tick) >= pdMS_TO_TICKS(5000))) {
      last_stack_log_tick = now_tick;
      UBaseType_t free_words = uxTaskGetStackHighWaterMark(NULL);
      tpmesh_debug_printf(
          "TPMesh Stack: task=Bridge free=%lu words (%lu bytes)\n",
          (unsigned long)free_words,
          (unsigned long)(free_words * sizeof(StackType_t)));
    }
#endif

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
  if (!tpmesh_at_is_uart6_active()) {
    return -1;
  }

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

  return at_send_with_trace("ddc_register", MESH_ADDR_TOP_NODE, tunnel_buf,
                            3 + sizeof(frame));
}

int ddc_send_heartbeat(const ddc_config_t *config) {
  if (!tpmesh_at_is_uart6_active()) {
    return -1;
  }

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

  return at_send_with_trace("ddc_heartbeat", MESH_ADDR_TOP_NODE, tunnel_buf,
                            3 + sizeof(frame));
}

void ddc_heartbeat_task(void *arg) {
  (void)arg;
  TickType_t last_stack_log_tick = 0;

  while (1) {
#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
    TickType_t now_tick = xTaskGetTickCount();
    if ((last_stack_log_tick == 0) ||
        ((now_tick - last_stack_log_tick) >= pdMS_TO_TICKS(5000))) {
      last_stack_log_tick = now_tick;
      UBaseType_t free_words = uxTaskGetStackHighWaterMark(NULL);
      tpmesh_debug_printf("TPMesh Stack: task=HB free=%lu words (%lu bytes)\n",
                          (unsigned long)free_words,
                          (unsigned long)(free_words * sizeof(StackType_t)));
    }
#endif

    if (!tpmesh_at_is_uart6_active()) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    uint32_t now = tpmesh_get_tick_ms();

    switch (s_ddc_state) {
    case DDC_STATE_INIT:
      /* 等待路由建立 */
      vTaskDelay(pdMS_TO_TICKS(1000));
      break;

    case DDC_STATE_REGISTERING:
      /* 注册重试 */
      if (s_last_register_tick == 0 ||
          (now - s_last_register_tick > TPMESH_REGISTER_RETRY_MS)) {
        if (s_register_retry_count < TPMESH_REGISTER_MAX_RETRIES) {
          if (ddc_send_register(&s_ddc_config) != 0) {
            tpmesh_debug_printf("TPMesh DDC: register send failed\n");
          } else {
            tpmesh_debug_printf("TPMesh DDC: register sent, current tick "
                                "is:%x,  waiting ACK (try=%u/%u)\n",
                                now, (unsigned)(s_register_retry_count + 1),
                                (unsigned)TPMESH_REGISTER_MAX_RETRIES);
          }
          s_last_register_tick = now;
          s_register_retry_count++;

        } else {
          tpmesh_debug_printf("TPMesh DDC: Register timeout, reset module\n");
          tpmesh_module_reset();
          s_ddc_state = DDC_STATE_INIT;
          s_register_retry_count = 0;
          s_waiting_heartbeat_ack = false;
          s_heartbeat_miss_count = 0;
          s_last_heartbeat_send_tick = 0;
          s_last_heartbeat_ack_tick = 0;
        }
      }
      break;

    case DDC_STATE_ONLINE:
      if (s_waiting_heartbeat_ack && s_last_heartbeat_send_tick > 0 &&
          (now - s_last_heartbeat_send_tick >
           TPMESH_HEARTBEAT_ACK_TIMEOUT_MS)) {
        s_waiting_heartbeat_ack = false;
        s_heartbeat_miss_count++;
        tpmesh_debug_printf("TPMesh DDC: heartbeat ACK timeout (%u/%u)\n",
                            (unsigned)s_heartbeat_miss_count,
                            (unsigned)TPMESH_HEARTBEAT_MISS_MAX);
        if (s_heartbeat_miss_count >= TPMESH_HEARTBEAT_MISS_MAX) {
          tpmesh_debug_printf(
              "TPMesh DDC: heartbeat lost, back to REGISTERING\n");
          s_ddc_state = DDC_STATE_REGISTERING;
          s_register_retry_count = 0;
          s_last_register_tick = 0;
          break;
        }
      }

      /* 心跳 */
      if (now - s_last_heartbeat_tick > TPMESH_HEARTBEAT_MS) {
        if (ddc_send_heartbeat(&s_ddc_config) == 0) {
          s_last_heartbeat_tick = now;
          s_last_heartbeat_send_tick = now;
          s_waiting_heartbeat_ack = true;
        } else {
          tpmesh_debug_printf("TPMesh DDC: heartbeat send failed\n");
        }
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
  TickType_t ticks = xTaskGetTickCount();
  return (uint32_t)(((uint64_t)ticks * 1000ULL) / (uint64_t)configTICK_RATE_HZ);
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

static uint16_t ip_checksum16(const uint8_t *buf, uint16_t len) {
  uint32_t sum = 0;
  uint16_t i = 0;

  while ((i + 1) < len) {
    sum += (uint32_t)(((uint16_t)buf[i] << 8) | buf[i + 1]);
    i += 2;
  }
  if (i < len) {
    sum += (uint32_t)((uint16_t)buf[i] << 8);
  }
  while ((sum >> 16) != 0) {
    sum = (sum & 0xFFFFU) + (sum >> 16);
  }
  return (uint16_t)(~sum);
}

static void top_fix_bacnet_src_ip(uint16_t src_mesh_id, uint8_t *eth_frame,
                                  uint16_t eth_len) {
  if (!s_is_top_node || src_mesh_id == s_top_config.mesh_id ||
      eth_frame == NULL || eth_len < (ETH_HDR_LEN + IP_HLEN + UDP_HLEN)) {
    return;
  }

  uint16_t ethertype =
      (uint16_t)(((uint16_t)eth_frame[12] << 8) | eth_frame[13]);
  if (ethertype != ETHTYPE_IP) {
    return;
  }

  uint8_t *ip = eth_frame + ETH_HDR_LEN;
  uint8_t version = (uint8_t)((ip[0] >> 4) & 0x0F);
  uint8_t ihl = (uint8_t)((ip[0] & 0x0F) * 4U);
  if (version != 4 || ihl < IP_HLEN || ihl > IP_HLEN_MAX ||
      eth_len < (ETH_HDR_LEN + ihl + UDP_HLEN) || ip[9] != IP_PROTO_UDP) {
    return;
  }

  uint8_t *udp = ip + ihl;
  uint16_t sport = (uint16_t)(((uint16_t)udp[0] << 8) | udp[1]);
  uint16_t dport = (uint16_t)(((uint16_t)udp[2] << 8) | udp[3]);
  if (sport != TPMESH_PORT_BACNET && dport != TPMESH_PORT_BACNET) {
    return;
  }

  ip4_addr_t src_ip_expected;
  if (node_table_get_ip_by_mesh(src_mesh_id, &src_ip_expected) != 0) {
    return;
  }

  uint8_t expected[4] = {
      ip4_addr1(&src_ip_expected), ip4_addr2(&src_ip_expected),
      ip4_addr3(&src_ip_expected), ip4_addr4(&src_ip_expected)};
  if (memcmp(ip + 12, expected, sizeof(expected)) == 0) {
    return;
  }

  tpmesh_debug_printf(
      "TPMesh: fix uplink src ip mesh=0x%04X %u.%u.%u.%u -> %u.%u.%u.%u\n",
      src_mesh_id, ip[12], ip[13], ip[14], ip[15], expected[0], expected[1],
      expected[2], expected[3]);

  memcpy(ip + 12, expected, sizeof(expected));
  ip[10] = 0;
  ip[11] = 0;
  {
    uint16_t cksum = ip_checksum16(ip, ihl);
    ip[10] = (uint8_t)(cksum >> 8);
    ip[11] = (uint8_t)(cksum & 0xFF);
  }

  /* IPv4 UDP allows checksum=0. Clear stale checksum after source IP rewrite.
   */
  udp[6] = 0;
  udp[7] = 0;
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
  if (!s_initialized || data == NULL || len == 0) {
    return;
  }
  if (len > TPMESH_MTU) {
    tpmesh_debug_printf(
        "TPMesh: Drop oversize mesh frame src=0x%04X len=%u (MTU=%u)\n",
        src_mesh_id, len, (unsigned)TPMESH_MTU);
    return;
  }

  /* 放入队列,由任务处理 */
  if (s_mesh_msg_queue) {
    mesh_msg_t msg;
    msg.src_mesh_id = src_mesh_id;
    msg.len = len;
    memcpy(msg.data, data, msg.len);
    if (xQueueSend(s_mesh_msg_queue, &msg, 0) != pdTRUE) {
      tpmesh_debug_printf(
          "TPMesh: mesh msg queue full, drop src=0x%04X len=%u\n", src_mesh_id,
          msg.len);
    }
  }
}

static void route_event_callback(const char *event, uint16_t addr) {
  const char *ev = event;
  if (ev == NULL) {
    return;
  }
  while (*ev == ' ')
    ev++;
  char ev_token[16];
  uint8_t n = 0;
  while (ev[n] != '\0' && ev[n] != ' ' && ev[n] != ',' && ev[n] != '[' &&
         n < (sizeof(ev_token) - 1)) {
    ev_token[n] = ev[n];
    n++;
  }
  ev_token[n] = '\0';
  tpmesh_debug_printf("TPMesh Route: %s 0x%04X\n",
                      (n > 0) ? ev_token : "UNKNOWN", addr);

  if (!s_is_top_node && strncmp(ev, "CREATE", 6) == 0 &&
      addr == MESH_ADDR_TOP_NODE) {
    if (s_ddc_state != DDC_STATE_INIT) {
      tpmesh_debug_printf(
          "TPMesh DDC: ignore duplicate CREATE while state=%s\n",
          ddc_state_to_str(s_ddc_state));
      return;
    }
    /* DDC 发现 Top Node,开始注册 */
    tpmesh_debug_printf("TPMesh DDC: Top Node discovered, start registering\n");
    s_ddc_state = DDC_STATE_REGISTERING;
    s_register_retry_count = 0;
    s_last_register_tick = 0;
    s_waiting_heartbeat_ack = false;
    s_heartbeat_miss_count = 0;
    s_last_heartbeat_send_tick = 0;
    s_last_heartbeat_ack_tick = 0;
  } else if (!s_is_top_node && strncmp(ev, "DELETE", 6) == 0 &&
             addr == MESH_ADDR_TOP_NODE) {
    /* Top 路由失效,回到等待态 */
    tpmesh_debug_printf("TPMesh DDC: Top Node route lost, back to INIT\n");
    s_ddc_state = DDC_STATE_INIT;
    s_waiting_heartbeat_ack = false;
    s_heartbeat_miss_count = 0;
  }
}

/* ============================================================================
 * 私有函数 - 分片发送
 * ============================================================================
 */

static const char *ddc_state_to_str(ddc_state_t state) {
  switch (state) {
  case DDC_STATE_INIT:
    return "INIT";
  case DDC_STATE_REGISTERING:
    return "REGISTERING";
  case DDC_STATE_ONLINE:
    return "ONLINE";
  default:
    return "UNKNOWN";
  }
}

static void trace_tunnel_packet_summary(const char *source, const uint8_t *data,
                                        uint16_t len) {
  const char *tag = (source != NULL) ? source : "unknown";

  if (data == NULL || len < TPMESH_TUNNEL_HDR_LEN) {
    return;
  }

  uint8_t rule = data[2];
  uint8_t frag_hdr = data[1];
  uint8_t seq = frag_hdr & 0x7F;
  bool is_last = (frag_hdr & 0x80) != 0;
  const uint8_t *payload = data + TPMESH_TUNNEL_HDR_LEN;
  uint16_t payload_len = len - TPMESH_TUNNEL_HDR_LEN;

  if (rule != SCHC_RULE_NO_COMPRESS) {
    tpmesh_debug_printf("TPMesh AT+SEND [%s]: decoded rule=0x%02X seq=%u%s "
                        "(compressed/control)\n",
                        tag, (unsigned)rule, (unsigned)seq, is_last ? "L" : "");
    return;
  }

  if (seq != 0 || payload_len < (ETH_HWADDR_LEN * 2 + 2)) {
    tpmesh_debug_printf("TPMesh AT+SEND [%s]: decoded rule=0x00 seq=%u%s "
                        "payload=%u (wait first fragment)\n",
                        tag, (unsigned)seq, is_last ? "L" : "",
                        (unsigned)payload_len);
    return;
  }

  const uint8_t *src_mac = payload;
  const uint8_t *dst_mac = payload + ETH_HWADDR_LEN;
  uint16_t ethertype = (uint16_t)(((uint16_t)payload[12] << 8) | payload[13]);
  const char *l2 = schc_is_broadcast_mac(dst_mac) ? "broadcast" : "unicast";

  if (ethertype == ETHTYPE_ARP) {
    tpmesh_debug_printf(
        "TPMesh AT+SEND [%s]: decoded %s ARP src=%02X:%02X:%02X:%02X:%02X:%02X "
        "dst=%02X:%02X:%02X:%02X:%02X:%02X\n",
        tag, l2, src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4],
        src_mac[5], dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4],
        dst_mac[5]);
    return;
  }

  if (ethertype != ETHTYPE_IP || payload_len < (ETH_HDR_LEN + IP_HLEN)) {
    tpmesh_debug_printf(
        "TPMesh AT+SEND [%s]: decoded %s eth=0x%04X payload=%u\n", tag, l2,
        ethertype, (unsigned)payload_len);
    return;
  }

  const uint8_t *ip = payload + ETH_HDR_LEN;
  uint8_t version = (uint8_t)((ip[0] >> 4) & 0x0F);
  uint8_t ihl = (uint8_t)((ip[0] & 0x0F) * 4U);

  if (version != 4 || ihl < IP_HLEN || payload_len < (ETH_HDR_LEN + ihl)) {
    tpmesh_debug_printf(
        "TPMesh AT+SEND [%s]: decoded %s IPv4 malformed ihl=%u payload=%u\n",
        tag, l2, (unsigned)ihl, (unsigned)payload_len);
    return;
  }

  uint8_t proto = ip[9];
  const uint8_t *src_ip = ip + 12;
  const uint8_t *dst_ip = ip + 16;

  if (proto == IP_PROTO_UDP && payload_len >= (ETH_HDR_LEN + ihl + UDP_HLEN)) {
    const uint8_t *udp = ip + ihl;
    uint16_t sport = (uint16_t)(((uint16_t)udp[0] << 8) | udp[1]);
    uint16_t dport = (uint16_t)(((uint16_t)udp[2] << 8) | udp[3]);
    tpmesh_debug_printf("TPMesh AT+SEND [%s]: decoded %s IPv4 UDP "
                        "%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u\n",
                        tag, l2, src_ip[0], src_ip[1], src_ip[2], src_ip[3],
                        (unsigned)sport, dst_ip[0], dst_ip[1], dst_ip[2],
                        dst_ip[3], (unsigned)dport);
    return;
  }

  tpmesh_debug_printf("TPMesh AT+SEND [%s]: decoded %s IPv4 proto=%u "
                      "%u.%u.%u.%u -> %u.%u.%u.%u\n",
                      tag, l2, (unsigned)proto, src_ip[0], src_ip[1], src_ip[2],
                      src_ip[3], dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);
}

static int at_send_with_trace(const char *source, uint16_t dest_mesh_id,
                              const uint8_t *data, uint16_t len) {
  const char *tag = (source != NULL) ? source : "unknown";
  uint8_t rule =
      (data != NULL && len >= TPMESH_TUNNEL_HDR_LEN) ? data[2] : 0xFF;
  const char *role = s_is_top_node ? "TOP" : "DDC";
  const char *state = s_is_top_node ? "-" : ddc_state_to_str(s_ddc_state);
  bool compact_control_rule = (rule == SCHC_RULE_REGISTER);

  if (compact_control_rule) {
    if (data != NULL && len >= TPMESH_TUNNEL_HDR_LEN) {
      uint8_t frag_hdr = data[1];
      uint8_t seq = frag_hdr & 0x7F;
      bool is_last = (frag_hdr & 0x80) != 0;
      const char *rule_desc =
          (rule != SCHC_RULE_NO_COMPRESS) ? " (compressed/control)" : "";
      tpmesh_debug_printf(
          "TPMesh AT+SEND [%s]: role=%s dst=0x%04X rule=0x%02X seq=%u%s%s\n",
          tag, role, dest_mesh_id, (unsigned)rule, (unsigned)seq,
          is_last ? "L" : "", rule_desc);
    } else {
      tpmesh_debug_printf(
          "TPMesh AT+SEND [%s]: role=%s dst=0x%04X rule=0x%02X\n", tag, role,
          dest_mesh_id, (unsigned)rule);
    }
  } else {
    tpmesh_debug_printf(
        "TPMesh AT+SEND [%s]: role=%s state=%s dst=0x%04X len=%u rule=0x%02X\n",
        tag, role, state, dest_mesh_id, (unsigned)len, (unsigned)rule);
    trace_tunnel_packet_summary(tag, data, len);
  }

  int ret = tpmesh_at_send(dest_mesh_id, data, len);
  if (ret != 0) {
    tpmesh_debug_printf("TPMesh AT+SEND [%s]: send failed ret=%d\n", tag, ret);
  }
  return ret;
}

static void trace_ddc_inject_frame(uint16_t src_mesh_id, const uint8_t *frame,
                                   uint16_t len) {
  if (frame == NULL || len < ETH_HDR_LEN) {
    tpmesh_debug_printf("TPMesh DDC RX inject: invalid frame src=0x%04X len=%u\n",
                        src_mesh_id, (unsigned)len);
    return;
  }

  const uint8_t *dst_mac = frame;
  const uint8_t *src_mac = frame + ETH_HWADDR_LEN;
  uint16_t ethertype = (uint16_t)(((uint16_t)frame[12] << 8) | frame[13]);
  const char *l2 = schc_is_broadcast_mac(dst_mac) ? "broadcast" : "unicast";

  tpmesh_debug_printf(
      "TPMesh DDC RX inject: src=0x%04X len=%u %s eth=0x%04X src=%02X:%02X:%02X:%02X:%02X:%02X dst=%02X:%02X:%02X:%02X:%02X:%02X\n",
      src_mesh_id, (unsigned)len, l2, (unsigned)ethertype, src_mac[0],
      src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5], dst_mac[0],
      dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);

  if (ethertype == ETHTYPE_ARP) {
    if (len < (ETH_HDR_LEN + SIZEOF_ETHARP_HDR)) {
      tpmesh_debug_printf(
          "TPMesh DDC RX inject: ARP short frame len=%u need=%u\n",
          (unsigned)len, (unsigned)(ETH_HDR_LEN + SIZEOF_ETHARP_HDR));
      return;
    }

    const struct etharp_hdr *arp =
        (const struct etharp_hdr *)(frame + ETH_HDR_LEN);
    const ip4_addr_t *sip = (const ip4_addr_t *)&arp->sipaddr;
    const ip4_addr_t *dip = (const ip4_addr_t *)&arp->dipaddr;
    tpmesh_debug_printf(
        "TPMesh DDC RX inject: ARP op=%u %u.%u.%u.%u -> %u.%u.%u.%u\n",
        (unsigned)lwip_ntohs(arp->opcode), ip4_addr1(sip), ip4_addr2(sip),
        ip4_addr3(sip), ip4_addr4(sip), ip4_addr1(dip), ip4_addr2(dip),
        ip4_addr3(dip), ip4_addr4(dip));
    return;
  }

  if (ethertype != ETHTYPE_IP) {
    return;
  }

  if (len < (ETH_HDR_LEN + IP_HLEN)) {
    tpmesh_debug_printf("TPMesh DDC RX inject: IPv4 short frame len=%u\n",
                        (unsigned)len);
    return;
  }

  const uint8_t *ip = frame + ETH_HDR_LEN;
  uint8_t version = (uint8_t)((ip[0] >> 4) & 0x0F);
  uint8_t ihl = (uint8_t)((ip[0] & 0x0F) * 4U);
  uint16_t ip_tot_len = (uint16_t)(((uint16_t)ip[2] << 8) | ip[3]);
  uint8_t proto = ip[9];
  const uint8_t *src_ip = ip + 12;
  const uint8_t *dst_ip = ip + 16;

  tpmesh_debug_printf(
      "TPMesh DDC RX inject: IPv4 ver=%u ihl=%u tot=%u proto=%u src=%u.%u.%u.%u dst=%u.%u.%u.%u\n",
      (unsigned)version, (unsigned)ihl, (unsigned)ip_tot_len, (unsigned)proto,
      src_ip[0], src_ip[1], src_ip[2], src_ip[3], dst_ip[0], dst_ip[1],
      dst_ip[2], dst_ip[3]);

  if (version != 4 || ihl < IP_HLEN || proto != IP_PROTO_UDP ||
      len < (uint16_t)(ETH_HDR_LEN + ihl + UDP_HLEN)) {
    return;
  }

  const uint8_t *udp = ip + ihl;
  uint16_t sport = (uint16_t)(((uint16_t)udp[0] << 8) | udp[1]);
  uint16_t dport = (uint16_t)(((uint16_t)udp[2] << 8) | udp[3]);
  uint16_t ulen = (uint16_t)(((uint16_t)udp[4] << 8) | udp[5]);
  tpmesh_debug_printf("TPMesh DDC RX inject: UDP sport=%u dport=%u len=%u\n",
                      (unsigned)sport, (unsigned)dport, (unsigned)ulen);
}

static int fragment_and_send(uint16_t dest_mesh_id, const uint8_t *data,
                             uint16_t len, const char *source) {
  if (!tpmesh_at_is_uart6_active()) {
    return -2;
  }
  if (data == NULL || len < TPMESH_TUNNEL_HDR_LEN) {
    return -1;
  }

  /* 统一分片格式:
   * 所有分片均保留 [L2_HDR][FRAG_HDR][RULE_ID]，仅切分 payload。 */
  uint8_t seq = 0;
  uint16_t offset = 0;

  while (offset < len) {
    uint8_t packet[TPMESH_MTU];
    uint16_t chunk_len = 0;

    if (seq == 0) {
      /* 第一片: 拷贝完整前段数据 */
      chunk_len = (len > TPMESH_MTU) ? TPMESH_MTU : len;
      memcpy(packet, data, chunk_len);
      offset += chunk_len;
    } else {
      /* 后续片: 保留隧道头，仅追加 payload */
      memcpy(packet, data, TPMESH_TUNNEL_HDR_LEN);
      uint16_t remain = len - offset;
      uint16_t max_payload = TPMESH_MTU - TPMESH_TUNNEL_HDR_LEN;
      uint16_t payload_len = (remain > max_payload) ? max_payload : remain;
      memcpy(packet + TPMESH_TUNNEL_HDR_LEN, data + offset, payload_len);
      chunk_len = TPMESH_TUNNEL_HDR_LEN + payload_len;
      offset += payload_len;
    }

    if (seq > 0x7F) {
      return -1;
    }
    bool is_last = (offset >= len);
    packet[1] = (is_last ? 0x80 : 0x00) | (seq & 0x7F);

    char trace_tag[48];
    const char *base = (source != NULL) ? source : "bridge_data";
    snprintf(trace_tag, sizeof(trace_tag), "%s:frag%u%s", base, (unsigned)seq,
             is_last ? "L" : "");
    if (at_send_with_trace(trace_tag, dest_mesh_id, packet, chunk_len) != 0) {
      return -1;
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
  if (data == NULL || len < TPMESH_TUNNEL_HDR_LEN) {
    return -5;
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
  } else {
    if (!session->active) {
      return -2;
    }
    if (tpmesh_get_tick_ms() - session->last_tick > 5000) {
      session->active = false;
      return -3;
    }
    if (session->total_len < TPMESH_TUNNEL_HDR_LEN ||
        data[0] != session->buffer[0] || data[2] != session->buffer[2]) {
      session->active = false;
      return -6;
    }
  }

  /* 序号检查 */
  if (seq != session->expected_seq) {
    tpmesh_debug_printf("TPMesh: Fragment seq mismatch (got %d, expect %d)\n",
                        seq, session->expected_seq);
    session->active = false;
    return -2;
  }

  /* 复制数据 */
  if (seq == 0) {
    /* 第一片: 拷贝完整隧道头 + 首段 payload */
    if (session->total_len + len > sizeof(session->buffer)) {
      session->active = false;
      return -4;
    }
    memcpy(session->buffer + session->total_len, data, len);
    session->total_len += len;
  } else {
    /* 后续片: 仅追加 payload，跳过固定隧道头 */
    uint16_t payload_len = len - TPMESH_TUNNEL_HDR_LEN;
    if (session->total_len + payload_len > sizeof(session->buffer)) {
      session->active = false;
      return -4;
    }
    memcpy(session->buffer + session->total_len, data + TPMESH_TUNNEL_HDR_LEN,
           payload_len);
    session->total_len += payload_len;
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

  // TODO: if
  // else逻辑太大了，并且有重复逻辑，需要进行函数拆分和合并优化，简洁逻辑
  if (s_is_top_node) {
    /* Top Node 处理注册/心跳 */
    switch (frame->frame_type) {
    case REG_FRAME_REGISTER:
    case REG_FRAME_HEARTBEAT: {
      ip4_addr_t ip;
      ip4_addr_set_u32(&ip, frame->ip);

      /* 注册/心跳统一刷新映射，支持 Top 重启后由心跳自恢复 */
      if (node_table_register(frame->mac, &ip, src_mesh_id) != 0) {
        tpmesh_debug_printf("TPMesh Top: table update failed src=0x%04X\n",
                            src_mesh_id);
        break;
      }

      if (frame->frame_type == REG_FRAME_REGISTER) {
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
        if (at_send_with_trace("top_register_ack", src_mesh_id, buf,
                               3 + sizeof(ack)) != 0) {
          tpmesh_debug_printf(
              "TPMesh Top: register ACK send failed dst=0x%04X\n", src_mesh_id);
          break;
        }

        tpmesh_debug_printf("TPMesh Top: DDC 0x%04X registered\n", src_mesh_id);
      } else {
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
        if (at_send_with_trace("top_heartbeat_ack", src_mesh_id, buf,
                               3 + sizeof(ack)) != 0) {
          tpmesh_debug_printf(
              "TPMesh Top: heartbeat ACK send failed dst=0x%04X\n",
              src_mesh_id);
        }
      }
      break;
    }
    default:
      break;
    }
  } else {
    /* DDC 处理 ACK */
    if (frame->frame_type == REG_FRAME_REGISTER_ACK ||
        frame->frame_type == REG_FRAME_HEARTBEAT_ACK) {
      tpmesh_debug_printf(
          "TPMesh DDC: ACK frame type=%u from src=0x%04X (expect=0x%04X)\n",
          (unsigned)frame->frame_type, src_mesh_id, MESH_ADDR_TOP_NODE);
    }
    switch (frame->frame_type) {
    case REG_FRAME_REGISTER_ACK:
      if (src_mesh_id != MESH_ADDR_TOP_NODE) {
        tpmesh_debug_printf("TPMesh DDC: Ignore register ACK from 0x%04X\n",
                            src_mesh_id);
        break;
      }

      {
        ip4_addr_t top_ip;
        ip4_addr_set_u32(&top_ip, frame->ip);
        if (node_table_register(frame->mac, &top_ip, src_mesh_id) != 0) {
          tpmesh_debug_printf(
              "TPMesh DDC: update top node entry failed src=0x%04X\n",
              src_mesh_id);
        }
      }

      tpmesh_debug_printf("TPMesh DDC: Register ACK received\n");
      s_ddc_state = DDC_STATE_ONLINE;
      s_last_heartbeat_tick = tpmesh_get_tick_ms();
      s_last_heartbeat_ack_tick = s_last_heartbeat_tick;
      s_last_heartbeat_send_tick = 0;
      s_waiting_heartbeat_ack = false;
      s_heartbeat_miss_count = 0;
      break;

    case REG_FRAME_HEARTBEAT_ACK:
      if (src_mesh_id != MESH_ADDR_TOP_NODE) {
        tpmesh_debug_printf("TPMesh DDC: Ignore heartbeat ACK from 0x%04X\n",
                            src_mesh_id);
        break;
      }
      {
        ip4_addr_t top_ip;
        ip4_addr_set_u32(&top_ip, frame->ip);
        if (node_table_register(frame->mac, &top_ip, src_mesh_id) != 0) {
          tpmesh_debug_printf(
              "TPMesh DDC: refresh top node entry failed src=0x%04X\n",
              src_mesh_id);
        }
      }
      s_last_heartbeat_ack_tick = tpmesh_get_tick_ms();
      s_waiting_heartbeat_ack = false;
      s_heartbeat_miss_count = 0;
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
  uint16_t eth_len;

  uint16_t dst_mesh_id =
      s_is_top_node ? s_top_config.mesh_id : s_ddc_config.mesh_id;

  if (schc_decompress(complete_data, complete_len, s_rebuild_eth_frame,
                      (uint16_t)sizeof(s_rebuild_eth_frame), &eth_len,
                      src_mesh_id, dst_mesh_id) != 0) {
    tpmesh_debug_printf("TPMesh: Decompress failed\n");
    return;
  }

  top_fix_bacnet_src_ip(src_mesh_id, s_rebuild_eth_frame, eth_len);

  if (s_is_top_node) {
    /* Top Node: 转发到以太网 */
    if (s_eth_netif && s_eth_netif->linkoutput) {
      struct pbuf *p = pbuf_alloc(PBUF_RAW, eth_len, PBUF_RAM);
      if (p) {
        memcpy(p->payload, s_rebuild_eth_frame, eth_len);
        s_eth_netif->linkoutput(s_eth_netif, p);
        pbuf_free(p);
      }
    }
  } else {
    /* DDC: 交给本地协议栈处理 */
    struct netif *ddc_netif = netif_default;
    if (ddc_netif && ddc_netif->input) {
      trace_ddc_inject_frame(src_mesh_id, s_rebuild_eth_frame, eth_len);
      struct pbuf *p = pbuf_alloc(PBUF_RAW, eth_len, PBUF_RAM);
      if (p) {
        memcpy(p->payload, s_rebuild_eth_frame, eth_len);
        if (ddc_netif->input(p, ddc_netif) != ERR_OK) {
          tpmesh_debug_printf(
              "TPMesh DDC: netif input failed len=%u src=0x%04X\n", eth_len,
              src_mesh_id);
          pbuf_free(p);
        }
      }
    }
  }
}

/* ============================================================================
 * 私有函数 - ARP 处理
 * ============================================================================
 */

static bool pbuf_copy_exact(struct pbuf *p, uint16_t offset, void *dst,
                            uint16_t len) {
  if (p == NULL || dst == NULL) {
    return false;
  }
  return pbuf_copy_partial(p, dst, len, offset) == len;
}

static bool clone_pbuf_contiguous(struct pbuf *src, struct pbuf **out) {
  if (src == NULL || out == NULL || src->tot_len == 0) {
    return false;
  }
  struct pbuf *cpy = pbuf_alloc(PBUF_RAW, src->tot_len, PBUF_RAM);
  if (cpy == NULL) {
    return false;
  }
  if (!pbuf_copy_exact(src, 0, cpy->payload, src->tot_len)) {
    pbuf_free(cpy);
    return false;
  }
  *out = cpy;
  return true;
}

static int build_no_compress_tunnel(const uint8_t *eth_frame, uint16_t eth_len,
                                    bool is_broadcast, uint8_t *out_data,
                                    uint16_t *out_len) {
  if (eth_frame == NULL || out_data == NULL || out_len == NULL ||
      eth_len < ETH_HDR_LEN) {
    return -1;
  }

  out_data[0] = is_broadcast ? 0x80 : 0x00;
  out_data[1] = 0x80;
  out_data[2] = SCHC_RULE_NO_COMPRESS;

  /* Keep full L2 identity: [SRC_MAC][DST_MAC][EtherType + Payload] */
  memcpy(out_data + TPMESH_TUNNEL_HDR_LEN, eth_frame + ETH_HWADDR_LEN,
         ETH_HWADDR_LEN);
  memcpy(out_data + TPMESH_TUNNEL_HDR_LEN + ETH_HWADDR_LEN, eth_frame,
         ETH_HWADDR_LEN);
  memcpy(out_data + TPMESH_TUNNEL_HDR_LEN + (ETH_HWADDR_LEN * 2),
         eth_frame + (ETH_HWADDR_LEN * 2), eth_len - (ETH_HWADDR_LEN * 2));

  *out_len = TPMESH_TUNNEL_HDR_LEN + eth_len;
  return 0;
}

static bool ddc_should_forward_arp_to_mesh(const struct etharp_hdr *arp) {
  if (arp == NULL || arp->opcode != PP_HTONS(ARP_REQUEST)) {
    return false;
  }

  ip4_addr_t sip;
  ip4_addr_t dip;
  SMEMCPY(&sip, &arp->sipaddr, sizeof(ip4_addr_t));
  SMEMCPY(&dip, &arp->dipaddr, sizeof(ip4_addr_t));

  uint32_t sip_u32 = ip4_addr_get_u32(&sip);
  uint32_t dip_u32 = ip4_addr_get_u32(&dip);

  if (dip_u32 == 0U || dip_u32 == 0xFFFFFFFFUL) {
    return false;
  }
  if (sip_u32 == 0U || sip_u32 == dip_u32) {
    return false;
  }
  return true;
}

static bool ddc_should_forward_to_mesh(struct pbuf *p) {
  if (p == NULL || p->tot_len < SIZEOF_ETH_HDR) {
    return false;
  }

  bool forward = false;
  struct pbuf *contig = NULL;
  struct pbuf *frame = p;

  if (p->len < p->tot_len) {
    if (!clone_pbuf_contiguous(p, &contig)) {
      return false;
    }
    frame = contig;
  }

  const struct eth_hdr *eth = (const struct eth_hdr *)frame->payload;
  uint16_t ethertype = lwip_ntohs(eth->type);

  if (ethertype == ETHTYPE_ARP &&
      frame->tot_len >= (SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR)) {
    const struct etharp_hdr *arp =
        (const struct etharp_hdr *)((const uint8_t *)frame->payload +
                                    SIZEOF_ETH_HDR);
    forward = ddc_should_forward_arp_to_mesh(arp);
    goto out;
  }

  if (ethertype == ETHTYPE_IP &&
      frame->tot_len >= (SIZEOF_ETH_HDR + IP_HLEN + UDP_HLEN)) {
    const struct ip_hdr *iph =
        (const struct ip_hdr *)((const uint8_t *)frame->payload +
                                SIZEOF_ETH_HDR);
    uint16_t ihl = IPH_HL_BYTES(iph);
    if (ihl >= IP_HLEN && ihl <= IP_HLEN_MAX &&
        frame->tot_len >= (SIZEOF_ETH_HDR + ihl + UDP_HLEN) &&
        IPH_PROTO(iph) == IP_PROTO_UDP) {
      const struct udp_hdr *udph =
          (const struct udp_hdr *)((const uint8_t *)iph + ihl);
      uint16_t dport = lwip_ntohs(udph->dest);
      uint16_t sport = lwip_ntohs(udph->src);
      if (dport == TPMESH_PORT_BACNET || sport == TPMESH_PORT_BACNET) {
        forward = true;
      }
    }
  }

out:
  if (contig != NULL) {
    pbuf_free(contig);
  }
  return forward;
}

static err_t ddc_netif_linkoutput_hook(struct netif *netif, struct pbuf *p) {
  if (netif == NULL || netif != s_ddc_netif || !s_initialized ||
      s_is_top_node) {
    return ERR_IF;
  }

  if (!ddc_should_forward_to_mesh(p)) {
    return s_ddc_phy_linkoutput ? s_ddc_phy_linkoutput(netif, p) : ERR_IF;
  }

  if (!tpmesh_data_plane_enabled() || !tpmesh_at_is_uart6_active()) {
    return ERR_IF;
  }

  if (s_ddc_state != DDC_STATE_ONLINE) {
    uint32_t now = tpmesh_get_tick_ms();
    if ((s_last_ddc_preonline_drop_log_tick == 0) ||
        (now - s_last_ddc_preonline_drop_log_tick > 1000)) {
      s_last_ddc_preonline_drop_log_tick = now;
      tpmesh_debug_printf(
          "TPMesh DDC: drop linkoutput while state=%s (not registered)\n",
          ddc_state_to_str(s_ddc_state));
    }
    return ERR_IF;
  }

  return (tpmesh_bridge_forward_to_mesh(p) == 0) ? ERR_OK : ERR_IF;
}

static bridge_action_t check_arp_request(struct pbuf *p) {
  if (p == NULL || p->tot_len < (SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR)) {
    return BRIDGE_LOCAL;
  }
  uint8_t arp_hdr_buf[SIZEOF_ETHARP_HDR];
  if (!pbuf_copy_exact(p, SIZEOF_ETH_HDR, arp_hdr_buf, SIZEOF_ETHARP_HDR)) {
    return BRIDGE_LOCAL;
  }
  const struct etharp_hdr *arp = (const struct etharp_hdr *)arp_hdr_buf;
  if (arp->opcode != PP_HTONS(ARP_REQUEST)) {
    return BRIDGE_LOCAL;
  }
  ip4_addr_t target;
  SMEMCPY(&target, &arp->dipaddr, sizeof(ip4_addr_t));
  uint16_t mesh_id = node_table_get_mesh_by_ip(&target);
  if (mesh_id == MESH_ADDR_INVALID) {
    return BRIDGE_LOCAL;
  }
  return node_table_is_online(mesh_id) ? BRIDGE_PROXY_ARP : BRIDGE_DROP;
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

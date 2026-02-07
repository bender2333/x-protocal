/**
 * @file tpmesh_init.c
 * @brief TPMesh 桥接模块初始化实现
 *
 * 初始化分为两阶段:
 * 1. main() 阶段 (调度器前): tpmesh_module_init_xxx() → 硬件/内存/RTOS对象
 * 2. Task 阶段 (调度器后): tpmesh_bridge_task() → AT命令/回调/业务逻辑
 *
 * @version 0.7.0
 */

#include "tpmesh_init.h"
#include "AppConfig.h"
#include "tpmesh_at.h"
#include "tpmesh_bridge.h"
#include "tpmesh_debug.h"

/* node_table.h 已通过 tpmesh_bridge.h 包含 */
#include "FreeRTOS.h"
#include "lwip/pbuf.h"
#include "lwip/prot/etharp.h"
#include "lwip/prot/ethernet.h"
#include "lwip/prot/ip4.h"
#include "lwip/prot/udp.h"
#include "task.h"
#include <string.h>

/* ============================================================================
 * 私有变量
 * ============================================================================
 */

/** 初始化状态 */
static bool s_tpmesh_initialized = false;

/** 是否为 Top Node */
static bool s_is_top_node = false;
static bool s_uart6_taken_over = false;

/** 任务句柄 */
static TaskHandle_t s_at_rx_task_handle = NULL;
static TaskHandle_t s_bridge_task_handle = NULL;
static TaskHandle_t s_heartbeat_task_handle = NULL;

static bool tpmesh_clone_pbuf_contiguous(struct pbuf *src, struct pbuf **out);
static bridge_action_t tpmesh_eth_bridge_check_safe(struct pbuf *p);

static bool tpmesh_clone_pbuf_contiguous(struct pbuf *src, struct pbuf **out) {
  if (src == NULL || out == NULL || src->tot_len == 0) {
    return false;
  }

  struct pbuf *cpy = pbuf_alloc(PBUF_RAW, src->tot_len, PBUF_RAM);
  if (cpy == NULL) {
    return false;
  }

  if (pbuf_copy_partial(src, cpy->payload, src->tot_len, 0) != src->tot_len) {
    pbuf_free(cpy);
    return false;
  }

  *out = cpy;
  return true;
}

static bridge_action_t tpmesh_eth_bridge_check_safe(struct pbuf *p) {
  if (p == NULL || p->tot_len < SIZEOF_ETH_HDR) {
    return BRIDGE_LOCAL;
  }

  uint8_t eth_hdr_buf[SIZEOF_ETH_HDR];
  if (pbuf_copy_partial(p, eth_hdr_buf, SIZEOF_ETH_HDR, 0) != SIZEOF_ETH_HDR) {
    return BRIDGE_LOCAL;
  }

  struct eth_hdr *eth = (struct eth_hdr *)eth_hdr_buf;
  uint16_t ethertype = lwip_ntohs(eth->type);
  bool is_broadcast = tpmesh_is_broadcast_mac((const uint8_t *)&eth->dest);

  if (is_broadcast) {
    if (ethertype == ETHTYPE_ARP) {
      if (p->tot_len < (SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR)) {
        return BRIDGE_LOCAL;
      }

      uint8_t arp_hdr_buf[SIZEOF_ETHARP_HDR];
      if (pbuf_copy_partial(p, arp_hdr_buf, SIZEOF_ETHARP_HDR, SIZEOF_ETH_HDR) !=
          SIZEOF_ETHARP_HDR) {
        return BRIDGE_LOCAL;
      }

      struct etharp_hdr *arp = (struct etharp_hdr *)arp_hdr_buf;
      if (arp->opcode != PP_HTONS(ARP_REQUEST)) {
        return BRIDGE_LOCAL;
      }

      ip4_addr_t target_ip;
      memcpy(&target_ip, &arp->dipaddr, sizeof(ip4_addr_t));
      uint16_t mesh_id = node_table_get_mesh_by_ip(&target_ip);
      if (mesh_id == MESH_ADDR_INVALID) {
        return BRIDGE_LOCAL;
      }

      return node_table_is_online(mesh_id) ? BRIDGE_PROXY_ARP : BRIDGE_DROP;
    }

    if (ethertype == ETHTYPE_IP) {
      if (p->tot_len >= (SIZEOF_ETH_HDR + IP_HDR_LEN + UDP_HDR_LEN)) {
        uint8_t l3_hdr_buf[SIZEOF_ETH_HDR + 80];
        uint16_t copy_len = (uint16_t)sizeof(l3_hdr_buf);
        if (p->tot_len < copy_len) {
          copy_len = p->tot_len;
        }

        if (pbuf_copy_partial(p, l3_hdr_buf, copy_len, 0) != copy_len) {
          return BRIDGE_DROP;
        }

        struct ip_hdr *iph = (struct ip_hdr *)(l3_hdr_buf + SIZEOF_ETH_HDR);
        uint16_t ihl = IPH_HL_BYTES(iph);
        if (ihl >= IP_HDR_LEN &&
            (SIZEOF_ETH_HDR + ihl + UDP_HDR_LEN) <= copy_len &&
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

  uint16_t dst_mesh_id = node_table_get_mesh_by_mac((const uint8_t *)&eth->dest);
  if (dst_mesh_id == MESH_ADDR_INVALID) {
    return BRIDGE_LOCAL;
  }

  return node_table_is_online(dst_mesh_id) ? BRIDGE_TO_MESH : BRIDGE_DROP;
}

/* ============================================================================
 * 公共函数
 * ============================================================================
 */

int tpmesh_module_init_top(struct netif *eth_netif) {
  if (s_tpmesh_initialized) {
    return 0;
  }

  if (eth_netif == NULL) {
    tpmesh_debug_printf("TPMesh Init: eth_netif is NULL\n");
    return -1;
  }

  tpmesh_debug_printf("TPMesh Init: Initializing as Top Node...\n");

  /* 配置 Top Node */
  top_config_t config;

  /* 从 netif 获取 MAC */
  memcpy(config.mac_addr, eth_netif->hwaddr, 6);

  /* 设置 IP (从 netif 获取或使用默认值) */
  if (eth_netif->ip_addr.addr != 0) {
    ip4_addr_copy(config.ip_addr, *netif_ip4_addr(eth_netif));
  } else {
    IP4_ADDR(&config.ip_addr, 192, 168, 10, 1);
  }

  config.mesh_id = TPMESH_TOP_NODE_MESH_ID;
  config.cell_id = 0;

  /* 初始化 Top Node */
  int ret = tpmesh_top_init(eth_netif, &config);
  if (ret != 0) {
    tpmesh_debug_printf("TPMesh Init: tpmesh_top_init failed (%d)\n", ret);
    return ret;
  }

  s_is_top_node = true;
  s_tpmesh_initialized = true;
  s_uart6_taken_over = false;

  tpmesh_debug_printf(
      "TPMesh Init: Top Node HW init done (AT cmds deferred to task)\n");
  tpmesh_debug_printf("  MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                      config.mac_addr[0], config.mac_addr[1],
                      config.mac_addr[2], config.mac_addr[3],
                      config.mac_addr[4], config.mac_addr[5]);
  tpmesh_debug_printf("  IP: %d.%d.%d.%d\n", ip4_addr1(&config.ip_addr),
                      ip4_addr2(&config.ip_addr), ip4_addr3(&config.ip_addr),
                      ip4_addr4(&config.ip_addr));
  tpmesh_debug_printf("  Mesh ID: 0x%04X\n", config.mesh_id);

  return 0;
}

int tpmesh_module_init_ddc(void) {
  if (s_tpmesh_initialized) {
    return 0;
  }

  tpmesh_debug_printf("TPMesh Init: Initializing as Edge node...\n");

  /* 配置 DDC */
  ddc_config_t config;

  /* 引用 LwIP 的全局默认网卡指针 */
  extern struct netif *netif_default;

  if (netif_default != NULL) {
    /* ============================================================ */
    /* 1. 从 LwIP 获取 MAC 地址 */
    /* ============================================================ */

    /* LwIP 的 MAC 地址存储在 hwaddr 数组中，通常长度为 6 */
    /* 为了通用性，这里使用 memcpy (需包含 string.h) 或者手动赋值 */
    /* 这里为了安全，使用了 manual copy 或者确保 memcpy 可用 */

    config.mac_addr[0] = netif_default->hwaddr[0];
    config.mac_addr[1] = netif_default->hwaddr[1];
    config.mac_addr[2] = netif_default->hwaddr[2];
    config.mac_addr[3] = netif_default->hwaddr[3];
    config.mac_addr[4] = netif_default->hwaddr[4];
    config.mac_addr[5] = netif_default->hwaddr[5];

    tpmesh_debug_printf("LwIP MAC Copied: %02X:%02X:%02X:%02X:%02X:%02X\n",
                        config.mac_addr[0], config.mac_addr[1],
                        config.mac_addr[2], config.mac_addr[3],
                        config.mac_addr[4], config.mac_addr[5]);

    /* ============================================================ */
    /* 2. 从 LwIP 获取 IP 地址 */
    /* ============================================================ */

    /* 使用 ip4_addr_get_u32 宏提取 u32 格式的 IP */
    ip4_addr_set_u32(&config.ip_addr,
                     ip4_addr_get_u32(netif_ip4_addr(netif_default)));

    tpmesh_debug_printf("LwIP IP Copied: %s\n",
                        ip4addr_ntoa(netif_ip4_addr(netif_default)));

  } else {
    /* ============================================================ */
    /* 容错处理：LwIP 未初始化 */
    /* ============================================================ */
    tpmesh_debug_printf(
        "Error: LwIP netif not ready! Using default/zero config.\n");

    /* 清零 MAC */
    memset(config.mac_addr, 0, 6);

    /* 清零 IP */
    IP4_ADDR(&config.ip_addr, 0, 0, 0, 0);
  }
  config.mesh_id = TPMESH_DDC_MESH_ID;
  config.cell_id = 0;

  /* 初始化 DDC */
  int ret = tpmesh_ddc_init(&config);
  if (ret != 0) {
    tpmesh_debug_printf("TPMesh Init: tpmesh_ddc_init failed (%d)\n", ret);
    return ret;
  }

  s_is_top_node = false;
  s_tpmesh_initialized = true;
  s_uart6_taken_over = false;

  tpmesh_debug_printf(
      "TPMesh Init: DDC HW init done (AT cmds deferred to task)\n");

  return 0;
}

void tpmesh_create_tasks(void) {
  if (!s_tpmesh_initialized) {
    tpmesh_debug_printf("TPMesh: Not initialized, cannot create tasks\n");
    return;
  }

  /* AT 接收任务 */
  xTaskCreate(tpmesh_at_rx_task, "TPMesh_AT_RX", TPMESH_AT_RX_TASK_STACK, NULL,
              TPMESH_AT_RX_TASK_PRIO, &s_at_rx_task_handle);

  /* 桥接任务 */
  xTaskCreate(tpmesh_bridge_task, "TPMesh_Bridge", TPMESH_BRIDGE_TASK_STACK,
              NULL, TPMESH_BRIDGE_TASK_PRIO, &s_bridge_task_handle);

  if (!s_is_top_node) {
    /* DDC 心跳任务 */
    xTaskCreate(ddc_heartbeat_task, "TPMesh_HB", TPMESH_DDC_HB_TASK_STACK, NULL,
                TPMESH_DDC_HB_TASK_PRIO, &s_heartbeat_task_handle);
  }

  tpmesh_debug_printf("TPMesh: Tasks created\n");
}

bool tpmesh_eth_input_hook(struct netif *netif, struct pbuf *p) {
  (void)netif;

  if (!s_tpmesh_initialized || !s_is_top_node) {
    return false;
  }
  if (s_uart6_taken_over || !tpmesh_at_is_uart6_active()) {
    return false;
  }

  /* Safe ingress policy: no contiguous pbuf assumption + online-node filter. */
  bridge_action_t action = tpmesh_eth_bridge_check_safe(p);

  switch (action) {
  case BRIDGE_TO_MESH: {
    struct pbuf *tx = NULL;
    if (!tpmesh_clone_pbuf_contiguous(p, &tx)) {
      return false;
    }
    int ret = tpmesh_bridge_forward_to_mesh(tx);
    pbuf_free(tx);
    return (ret == 0);
  }

  case BRIDGE_PROXY_ARP: {
    struct pbuf *arp_req = NULL;
    if (!tpmesh_clone_pbuf_contiguous(p, &arp_req)) {
      return false;
    }
    int ret = tpmesh_bridge_send_proxy_arp(arp_req);
    pbuf_free(arp_req);
    return (ret == 0);
  }

  case BRIDGE_DROP:
    return true;

  case BRIDGE_LOCAL:
  default:
    return false;
  }
}

bool tpmesh_is_initialized(void) { return s_tpmesh_initialized; }

void tpmesh_request_uart6_takeover(void) {
  if (!s_tpmesh_initialized) {
    return;
  }
  int ret = tpmesh_at_release_uart6();
  if (ret == 0) {
    s_uart6_taken_over = true;
  } else {
    tpmesh_debug_printf("TPMesh: UART6 takeover request failed (%d)\n", ret);
  }
}

int tpmesh_reclaim_uart6_for_tpmesh(void) {
  if (!s_tpmesh_initialized) {
    return -1;
  }
  int ret = tpmesh_at_acquire_uart6();
  if (ret == 0) {
    s_uart6_taken_over = false;
  }
  return ret;
}

bool tpmesh_is_uart6_taken_over(void) { return s_uart6_taken_over; }

void tpmesh_print_status(void) {
  tpmesh_debug_printf("\n=== TPMesh Status ===\n");
  tpmesh_debug_printf("Initialized: %s\n", s_tpmesh_initialized ? "Yes" : "No");
  tpmesh_debug_printf("Mode: %s\n", s_is_top_node ? "Top Node" : "DDC");
  tpmesh_debug_printf("UART6 owner: %s\n",
                      s_uart6_taken_over ? "External" : "x_protocol");

  if (s_tpmesh_initialized) {
    tpmesh_debug_printf("Tasks:\n");
    tpmesh_debug_printf("  AT RX: %s\n",
                        s_at_rx_task_handle ? "Running" : "Not created");
    tpmesh_debug_printf("  Bridge: %s\n",
                        s_bridge_task_handle ? "Running" : "Not created");
    if (!s_is_top_node) {
      tpmesh_debug_printf("  Heartbeat: %s\n",
                          s_heartbeat_task_handle ? "Running" : "Not created");
    }

    tpmesh_debug_printf("\nNode Table:\n");
    node_table_dump();
  }
  tpmesh_debug_printf("=====================\n\n");
}

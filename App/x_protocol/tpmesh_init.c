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
static tpmesh_role_t s_role = TPMESH_ROLE_NODE;
static tpmesh_run_state_t s_run_state = TPMESH_RUN_STATE_RUN;

/** 任务句柄 */
static TaskHandle_t s_at_rx_task_handle = NULL;
static TaskHandle_t s_bridge_task_handle = NULL;
static TaskHandle_t s_heartbeat_task_handle = NULL;

#define TPMESH_UART6_TAKEOVER_WAIT_MS 20

static const char *tpmesh_role_to_str(tpmesh_role_t role) {
  switch (role) {
  case TPMESH_ROLE_TOP:
    return "Top";
  case TPMESH_ROLE_EDGE:
    return "Edge";
  case TPMESH_ROLE_NODE:
  default:
    return "Node";
  }
}

static const char *tpmesh_state_to_str(tpmesh_run_state_t state) {
  switch (state) {
  case TPMESH_RUN_STATE_CONFIG:
    return "Config";
  case TPMESH_RUN_STATE_RUN:
  default:
    return "Run";
  }
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
  // IP4_ADDR(&config.ip_addr, 192, 168, 10, 11);

  config.mesh_id = TPMESH_TOP_NODE_MESH_ID;
  config.cell_id = 0;

  /* 初始化 Top Node */
  int ret = tpmesh_top_init(eth_netif, &config);
  if (ret != 0) {
    tpmesh_debug_printf("TPMesh Init: tpmesh_top_init failed (%d)\n", ret);
    return ret;
  }

  s_is_top_node = true;
  s_role = TPMESH_ROLE_TOP;
  s_run_state = TPMESH_RUN_STATE_RUN;
  s_tpmesh_initialized = true;

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

int tpmesh_module_init_edge(void) {
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
    IP4_ADDR(&config.ip_addr, 192, 168, 10, 11);
    // ip4_addr_set_u32(&config.ip_addr,
    //                  ip4_addr_get_u32(netif_ip4_addr(netif_default)));

    tpmesh_debug_printf("LwIP IP Copied: %s\n", ip4addr_ntoa(&config.ip_addr));

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

  if (netif_default == NULL) {
    tpmesh_debug_printf("TPMesh Init: DDC netif is NULL, hook failed\n");
    return -3;
  }
  ret = tpmesh_bridge_attach_ddc_netif(netif_default);
  if (ret != 0) {
    tpmesh_debug_printf("TPMesh Init: DDC linkoutput hook failed (%d)\n", ret);
    return ret;
  }

  s_is_top_node = false;
  s_role = TPMESH_ROLE_EDGE;
  s_run_state = TPMESH_RUN_STATE_RUN;
  s_tpmesh_initialized = true;

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

  if (!tpmesh_eth_input_tap_enabled()) {
    return false;
  }

  /* Ingress policy is implemented in bridge module. */
  bridge_action_t action = tpmesh_bridge_check(p);

  switch (action) {
  case BRIDGE_TO_MESH: {
    int ret = tpmesh_bridge_forward_to_mesh(p);
    if (ret != 0) {
      tpmesh_debug_printf("TPMesh Hook: forward_to_mesh failed (%d), drop\n",
                          ret);
    }
    return true;
  }

  case BRIDGE_PROXY_ARP: {
    int ret = tpmesh_bridge_send_proxy_arp(p);
    if (ret != 0) {
      tpmesh_debug_printf("TPMesh Hook: proxy_arp failed (%d), drop\n", ret);
    }
    return true;
  }

  case BRIDGE_DROP:
    return true;

  case BRIDGE_LOCAL:
  default:
    return false;
  }
}

tpmesh_role_t tpmesh_get_role(void) { return s_role; }

tpmesh_run_state_t tpmesh_get_run_state(void) { return s_run_state; }

bool tpmesh_data_plane_enabled(void) {
  return s_tpmesh_initialized && (s_run_state == TPMESH_RUN_STATE_RUN) &&
         (s_role != TPMESH_ROLE_NODE);
}

bool tpmesh_eth_input_tap_enabled(void) {
  return tpmesh_data_plane_enabled() && (s_role == TPMESH_ROLE_TOP) &&
         tpmesh_at_is_uart6_active();
}

int tpmesh_set_run_state(tpmesh_run_state_t state) {
  if (state != TPMESH_RUN_STATE_RUN && state != TPMESH_RUN_STATE_CONFIG) {
    return -2;
  }
  if (state == s_run_state) {
    return 0;
  }

  if (state == TPMESH_RUN_STATE_CONFIG && s_tpmesh_initialized) {
    int ret = tpmesh_request_uart6_takeover();
    if (ret != 0) {
      return ret;
    }
  }

  s_run_state = state;
  tpmesh_debug_printf("TPMesh: runtime state -> %s\n",
                      tpmesh_state_to_str(s_run_state));
  return 0;
}

bool tpmesh_is_initialized(void) { return s_tpmesh_initialized; }

int tpmesh_request_uart6_takeover(void) {
  if (!s_tpmesh_initialized) {
    return -1;
  }

  while (1) {
    int ret = tpmesh_at_release_uart6();
    if (ret == 0) {
      return 0;
    }

    /* Busy path: block-retry until TX path drains and release succeeds. */
    if (ret == -3 && xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
      vTaskDelay(pdMS_TO_TICKS(TPMESH_UART6_TAKEOVER_WAIT_MS));
      continue;
    }

    tpmesh_debug_printf("TPMesh: UART6 takeover request failed (%d)\n", ret);
    return ret;
  }
}

void tpmesh_print_status(void) {
  tpmesh_debug_printf("\n=== TPMesh Status ===\n");
  tpmesh_debug_printf("Initialized: %s\n", s_tpmesh_initialized ? "Yes" : "No");
  tpmesh_debug_printf("Role: %s\n", tpmesh_role_to_str(s_role));
  tpmesh_debug_printf("State: %s\n", tpmesh_state_to_str(s_run_state));
  tpmesh_debug_printf("UART6 active: %s\n",
                      tpmesh_at_is_uart6_active() ? "Yes" : "No");

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

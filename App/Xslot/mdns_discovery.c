/**
 * @file mdns_discovery.c
 * @brief mDNS Broker 发现模块
 *
 * 监听 PC 端发布的 _ddc-mqtt._tcp.local 服务
 * 当发现服务时，解析 Broker IP 和端口
 */

#include "communication.h"
#include "lwip/apps/mdns.h"
#include "lwip/igmp.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/udp.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * 私有变量
 * ============================================================================ */

static broker_state_t s_broker_state = BROKER_STATE_IDLE;
static broker_info_t s_broker_info = {0};

/* 用于 DNS-SD 查询的 UDP PCB */
static struct udp_pcb *s_mdns_pcb = NULL;

/* mDNS 组播地址 */
#define MDNS_MULTICAST_IP "224.0.0.251"
#define MDNS_PORT 5353

/* ============================================================================
 * 退避算法参数
 * ============================================================================ */

/* 退避时间配置 (单位: 毫秒) */
#define BACKOFF_INITIAL_MS      1000      /* 初始间隔: 1 秒 */
#define BACKOFF_MAX_MS          300000    /* 最大间隔: 5 分钟 */
#define BACKOFF_MULTIPLIER      2         /* 退避倍数 */

/* 退避状态 */
static uint32_t s_backoff_interval_ms = BACKOFF_INITIAL_MS;
static uint32_t s_last_query_tick = 0;
static uint32_t s_query_count = 0;

/* DNS 报文相关常量 */
#define DNS_FLAG_QR (1 << 15)     /* Response */
#define DNS_FLAG_AA (1 << 10)     /* Authoritative */
#define DNS_TYPE_PTR 12
#define DNS_TYPE_SRV 33
#define DNS_TYPE_TXT 16
#define DNS_TYPE_A 1
#define DNS_CLASS_IN 1

/* ============================================================================
 * 私有函数声明
 * ============================================================================ */

static void mdns_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                               const ip_addr_t *addr, u16_t port);
static bool parse_mdns_response(struct pbuf *p, broker_info_t *info);
static void send_mdns_query(void);

/* ============================================================================
 * 公共函数实现
 * ============================================================================ */

void mdns_discovery_init(void) {
  err_t err;
  ip_addr_t multicast_addr;

  printf("mDNS Discovery: Initializing...\n");

  /* 创建 UDP PCB */
  s_mdns_pcb = udp_new();
  if (s_mdns_pcb == NULL) {
    printf("mDNS Discovery: Failed to create UDP PCB\n");
    return;
  }

  /* 绑定到 mDNS 端口 */
  err = udp_bind(s_mdns_pcb, IP_ADDR_ANY, MDNS_PORT);
  if (err != ERR_OK) {
    printf("mDNS Discovery: Failed to bind UDP port %d\n", MDNS_PORT);
    udp_remove(s_mdns_pcb);
    s_mdns_pcb = NULL;
    return;
  }

  /* 加入组播组 */
  ipaddr_aton(MDNS_MULTICAST_IP, &multicast_addr);
  struct netif *netif = netif_default;
  if (netif != NULL) {
    igmp_joingroup_netif(netif, ip_2_ip4(&multicast_addr));
  }

  /* 设置接收回调 */
  udp_recv(s_mdns_pcb, mdns_recv_callback, NULL);

  s_broker_state = BROKER_STATE_IDLE;
  memset(&s_broker_info, 0, sizeof(s_broker_info));

  printf("mDNS Discovery: Initialized, listening on port %d\n", MDNS_PORT);
}

broker_state_t mdns_discovery_get_state(void) { return s_broker_state; }

bool mdns_discovery_get_broker(broker_info_t *info) {
  if (info == NULL) {
    return false;
  }

  if (s_broker_info.valid) {
    memcpy(info, &s_broker_info, sizeof(broker_info_t));
    return true;
  }

  return false;
}

void mdns_discovery_service_removed(void) {
  printf("mDNS Discovery: Service removed, resetting state\n");
  s_broker_state = BROKER_STATE_IDLE;
  s_broker_info.valid = false;
  
  /* 重置退避算法 */
  s_backoff_interval_ms = BACKOFF_INITIAL_MS;
}

/**
 * @brief 执行带退避算法的主动查询
 * @param current_tick_ms 当前系统 tick (毫秒)
 * @return true 如果发送了查询, false 如果还在退避等待
 */
bool mdns_discovery_query_with_backoff(uint32_t current_tick_ms) {
  /* 如果已经发现 Broker，不需要查询 */
  if (s_broker_info.valid) {
    return false;
  }
  
  /* 检查是否到达退避时间 */
  uint32_t elapsed = current_tick_ms - s_last_query_tick;
  
  if (elapsed < s_backoff_interval_ms) {
    /* 还在退避等待中 */
    return false;
  }
  
  /* 发送查询 */
  send_mdns_query();
  s_query_count++;
  s_last_query_tick = current_tick_ms;
  
  /* 计算下一次退避时间 (指数退避) */
  s_backoff_interval_ms *= BACKOFF_MULTIPLIER;
  if (s_backoff_interval_ms > BACKOFF_MAX_MS) {
    s_backoff_interval_ms = BACKOFF_MAX_MS;
  }
  
  printf("mDNS Discovery: Query #%lu sent, next in %lu ms\n",
         (unsigned long)s_query_count,
         (unsigned long)s_backoff_interval_ms);
  
  return true;
}

/**
 * @brief 重置退避算法（发现服务后调用）
 */
void mdns_discovery_reset_backoff(void) {
  s_backoff_interval_ms = BACKOFF_INITIAL_MS;
  s_query_count = 0;
  printf("mDNS Discovery: Backoff reset\n");
}

/**
 * @brief 获取当前退避间隔
 * @return 当前退避间隔 (毫秒)
 */
uint32_t mdns_discovery_get_backoff_interval(void) {
  return s_backoff_interval_ms;
}

/**
 * @brief 获取查询计数
 * @return 已发送的查询次数
 */
uint32_t mdns_discovery_get_query_count(void) {
  return s_query_count;
}

/* ============================================================================
 * 私有函数实现
 * ============================================================================ */

/**
 * @brief mDNS UDP 接收回调
 */
static void mdns_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                               const ip_addr_t *addr, u16_t port) {
  (void)arg;
  (void)pcb;
  (void)port;

  if (p == NULL) {
    return;
  }

  /* 尝试解析 mDNS 响应 */
  broker_info_t temp_info = {0};

  if (parse_mdns_response(p, &temp_info)) {
    /* 发现有效的 Broker 服务 */
    if (temp_info.valid) {
      memcpy(&s_broker_info, &temp_info, sizeof(broker_info_t));
      s_broker_state = BROKER_STATE_DISCOVERED;

      /* 发现服务后重置退避算法 */
      s_backoff_interval_ms = BACKOFF_INITIAL_MS;
      s_query_count = 0;

      printf("mDNS Discovery: Found broker at %s:%d\n", s_broker_info.ip,
             s_broker_info.port);
    }
  }

  pbuf_free(p);
}

/**
 * @brief 解析 mDNS 响应报文
 *
 * 简化版解析，查找 _ddc-mqtt._tcp.local 服务的 SRV 和 A 记录
 */
static bool parse_mdns_response(struct pbuf *p, broker_info_t *info) {
  if (p->tot_len < 12) {
    return false; /* 报文太短 */
  }

  uint8_t *data = (uint8_t *)p->payload;
  uint16_t flags = (data[2] << 8) | data[3];

  /* 检查是否为响应报文 */
  if (!(flags & DNS_FLAG_QR)) {
    return false; /* 这是查询，不是响应 */
  }

  uint16_t answers = (data[6] << 8) | data[7];
  uint16_t additional = (data[10] << 8) | data[11];

  if (answers == 0 && additional == 0) {
    return false;
  }

  /* 简化解析：在报文中查找我们需要的服务名称和 IP */
  /* 查找 "_ddc-mqtt" 字符串 */
  const char *service_name = "_ddc-mqtt";
  size_t service_len = strlen(service_name);
  bool found_service = false;

  for (size_t i = 12; i < p->tot_len - service_len; i++) {
    if (memcmp(data + i, service_name, service_len) == 0) {
      found_service = true;
      break;
    }
  }

  if (!found_service) {
    return false;
  }

  /* 查找 A 记录（IPv4 地址）*/
  /* A 记录格式: ... type(2) class(2) ttl(4) rdlength(2) ip(4) */
  for (size_t i = 12; i < p->tot_len - 14; i++) {
    uint16_t type = (data[i] << 8) | data[i + 1];
    uint16_t class = (data[i + 2] << 8) | data[i + 3];

    if (type == DNS_TYPE_A && (class & 0x7FFF) == DNS_CLASS_IN) {
      /* 跳过 ttl(4) 和 rdlength(2) */
      size_t ip_offset = i + 10;
      if (ip_offset + 4 <= p->tot_len) {
        snprintf(info->ip, sizeof(info->ip), "%d.%d.%d.%d", data[ip_offset],
                 data[ip_offset + 1], data[ip_offset + 2], data[ip_offset + 3]);
        info->port = 1883; /* 默认 MQTT 端口 */
        info->valid = true;
        return true;
      }
    }
  }

  /* 如果没找到 A 记录，使用发送者的 IP */
  /* 这在很多情况下是有效的，因为 Broker 和 mDNS 服务通常在同一机器 */

  return false;
}

/**
 * @brief 发送 mDNS 查询（主动查询，可选）
 */
static void send_mdns_query(void) {
  if (s_mdns_pcb == NULL) {
    return;
  }

  /* 构造 DNS 查询报文 */
  /* 查询 _ddc-mqtt._tcp.local PTR 记录 */
  static const uint8_t query[] = {
      0x00, 0x00, /* Transaction ID */
      0x00, 0x00, /* Flags: Standard query */
      0x00, 0x01, /* Questions: 1 */
      0x00, 0x00, /* Answer RRs: 0 */
      0x00, 0x00, /* Authority RRs: 0 */
      0x00, 0x00, /* Additional RRs: 0 */
      /* Query: _ddc-mqtt._tcp.local */
      0x09, '_', 'd', 'd', 'c', '-', 'm', 'q', 't', 't', 0x04, '_', 't', 'c',
      'p', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00,
      0x00, 0x0C, /* Type: PTR */
      0x00, 0x01  /* Class: IN */
  };

  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(query), PBUF_RAM);
  if (p == NULL) {
    return;
  }

  memcpy(p->payload, query, sizeof(query));

  ip_addr_t multicast_addr;
  ipaddr_aton(MDNS_MULTICAST_IP, &multicast_addr);

  udp_sendto(s_mdns_pcb, p, &multicast_addr, MDNS_PORT);
  pbuf_free(p);

  printf("mDNS Discovery: Sent query for %s\n", MDNS_SERVICE_TYPE);
}

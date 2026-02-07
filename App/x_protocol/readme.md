
---

# 技术架构设计说明书：基于 TPMesh 的 LwIP/BACnet 无线网关

**项目名称:** BACnet over Sub-G (TPMesh L2 Tunnel Bridge)
**目标平台:** FreeRTOS + LwIP (Dual Stack)
**核心模块:** TPMesh 无线模组 (UART 接口)
**协议关键:** L2 隧道桥接 + 代理 ARP + 主动注册 + SCHC 压缩
**版本:** V0.7
**更新日期:** 2026-02-07

---

## 1. 系统概览

### 1.1 设计目标

| 目标 | 说明 |
|------|------|
| **即插即用** | BMS 无需任何配置，DDC 如同在本地网段 |
| **带宽优化** | 在 L2 桥接基础上保留 SCHC 压缩 |
| **可靠发现** | DDC 主动注册，解决冷启动问题 |
| **低延迟 ARP** | Top Node 代答 ARP，毫秒级响应 |

### 1.2 架构特点

| 特性 | V0.5 | V0.6.2 改进 |
|------|------|----------|
| ARP 处理 | 转发到 Mesh | **Top Node 代答** |
| DDC 发现 | 被动学习 | **主动注册 + 心跳** |
| 数据传输 | 透传模式 | **AT+SEND (URC 带源地址)** |
| 头部压缩 | 无 | **SCHC 压缩 IP/UDP** |
| 广播控制 | 过滤 | **过滤 + 限速** |
| 隧道头部 | 5 字节 | **3 字节 (源地址从 URC 获取)** |

### 1.4 V0.7 架构更新（实现对齐）

为确保设计与 TPMESH_V1-6 模组协议一致，V0.7 在接口层与数据面进行以下更新：

1. **AT+SEND 指令格式对齐**
   - 由 `AT+SEND=<ADDR>,<LEN>,<DATA>` 更新为 `AT+SEND=<ADDR>,<LEN>,<DATA>,<TYPE>`。
   - `TYPE` 作为链路策略参数，默认值为 `0`（普通模式），预留为编译期配置。

2. **NNMI URC 解析增强**
   - 优先解析文档格式：`+NNMI:<SRC>,<DEST>,<RSSI>,<LEN>,<DATA>`。
   - 保留对历史简化格式 `+NNMI:<SRC>,<LEN>,<DATA>` 的兼容解析。

3. **模组初始化序列补全**
   - 启用 `AT+TYPE`（CENTER/ORDINARY）和 `AT+NNMI` 配置，确保网关能够接收下行 URC。
   - 增加 `AT+LP` 默认配置，统一中心节点与边缘节点功耗行为。

4. **DDC 下行数据链路闭环**
   - DDC 接收到 Mesh 数据并解压后，直接注入本地 LwIP 输入路径（pbuf + netif->input）。
   - 保障“DDC 在 BMS 看起来等效本地以太网节点”的架构目标。

5. **Mesh 数据入口健壮性**
   - 增加隧道最小头长度检查，避免异常短帧导致规则字段越界访问。

### 1.3 网络拓扑

```
┌─────────────────────────────────────────────────────────────────┐
│                        以太网 (物理层)                           │
│                      192.168.10.0/24                            │
│                                                                 │
│  ┌─────────────┐              ┌─────────────┐                  │
│  │     BMS     │              │  Top Node   │                  │
│  │192.168.10.100│             │192.168.10.1 │                  │
│  │             │              │  ┌───────┐  │                  │
│  │             │              │  │ARP代理│  │                  │
│  └──────┬──────┘              │  │节点表 │  │                  │
│         │                     │  └───────┘  │                  │
│ ════════╧═════════════════════╧═══════════════════════════ ETH │
└─────────────────────────────────────────────────────────────────┘
                                       │
                                       │ L2 Tunnel (透传模式2)
                                       │
┌──────────────────────────────────────┼──────────────────────────┐
│                        Sub-G Mesh                                │
│                                      │                          │
│  ┌───────────────────────────────────┴───────────────────────┐ │
│  │                      Top Node (0xFFFE)                     │ │
│  │              ┌─────────────────────────┐                  │ │
│  │              │   - 代理 ARP           │                  │ │
│  │              │   - 节点映射表         │                  │ │
│  │              │   - 广播限速           │                  │ │
│  │              │   - SCHC 压缩/解压     │                  │ │
│  │              └─────────────────────────┘                  │ │
│  └───────────────────────────────────┬───────────────────────┘ │
│                                      │                          │
│          ┌───────────────────────────┼───────────────────────┐ │
│          │                           │                       │ │
│    ┌─────┴─────┐               ┌─────┴─────┐          ┌─────┴─────┐
│    │   DDC 1   │               │   DDC 2   │          │   DDC N   │
│    │  主动注册  │               │  主动注册  │          │  主动注册  │
│    │  心跳保活  │               │  心跳保活  │          │  心跳保活  │
│    │Mesh:0x0002│               │Mesh:0x0003│          │Mesh:0x000N│
│    └───────────┘               └───────────┘          └───────────┘
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. 核心机制

### 2.1 DDC 主动注册 (解决冷启动死锁)

**问题回顾：** V0.5 中，如果 Top Node 映射表为空，BMS 的 ARP 请求会被丢弃，DDC 永远无法被发现。

**解决方案：** DDC 上电后**主动向 Top Node 注册**。

```
DDC 启动流程:
┌─────────────────────────────────────────────────────────────────┐
│  1. DDC 上电                                                     │
│  2. Mesh 模组初始化 (AT+ADDR, AT+CELL, ...)                     │
│  3. 等待路由建立 (+ROUTE:CREATE ADDR[0xFFFE])                   │
│  4. ★ 发送注册帧给 Top Node ★                                   │
│  5. 启动心跳定时器                                               │
│  6. 进入正常工作状态                                             │
└─────────────────────────────────────────────────────────────────┘
```

#### 2.1.1 注册帧格式

```
注册帧 (DDC → Top Node):
┌──────────┬──────────┬──────────┬──────────┬──────────┐
│ Frame Type│   MAC    │    IP    │ Mesh ID  │ Checksum │
│  1 byte   │ 6 bytes  │ 4 bytes  │ 2 bytes  │  2 bytes │
└──────────┴──────────┴──────────┴──────────┴──────────┘

Frame Type: 0x01 = 注册请求
            0x02 = 注册确认 (Top Node 回复)
            0x03 = 心跳
            0x04 = 心跳响应

总长度: 15 bytes
```

#### 2.1.2 注册实现

```c
/* ==================== DDC 注册协议 ==================== */

#define REG_FRAME_TYPE_REGISTER     0x01
#define REG_FRAME_TYPE_REGISTER_ACK 0x02
#define REG_FRAME_TYPE_HEARTBEAT    0x03
#define REG_FRAME_TYPE_HEARTBEAT_ACK 0x04

typedef struct __attribute__((packed)) {
    uint8_t  frame_type;
    uint8_t  mac[6];
    uint32_t ip;          // 网络字节序
    uint16_t mesh_id;
    uint16_t checksum;    // CRC16
} reg_frame_t;

/**
 * @brief DDC 发送注册请求
 */
int ddc_send_register(ddc_config_t *config) {
    reg_frame_t frame;

    frame.frame_type = REG_FRAME_TYPE_REGISTER;
    memcpy(frame.mac, config->mac_addr, 6);
    frame.ip = config->ip_addr.addr;
    frame.mesh_id = config->mesh_id;
    frame.checksum = calc_crc16(&frame, sizeof(frame) - 2);

    // 发送到 Top Node (透传模式2: 前2字节是目标地址)
    uint8_t packet[2 + sizeof(reg_frame_t)];
    packet[0] = (MESH_ADDR_TOP_NODE >> 8) & 0xFF;
    packet[1] = MESH_ADDR_TOP_NODE & 0xFF;
    memcpy(&packet[2], &frame, sizeof(frame));

    return uart_write(packet, sizeof(packet));
}

/**
 * @brief DDC 发送心跳
 */
int ddc_send_heartbeat(ddc_config_t *config) {
    reg_frame_t frame;

    frame.frame_type = REG_FRAME_TYPE_HEARTBEAT;
    memcpy(frame.mac, config->mac_addr, 6);
    frame.ip = config->ip_addr.addr;
    frame.mesh_id = config->mesh_id;
    frame.checksum = calc_crc16(&frame, sizeof(frame) - 2);

    uint8_t packet[2 + sizeof(reg_frame_t)];
    packet[0] = (MESH_ADDR_TOP_NODE >> 8) & 0xFF;
    packet[1] = MESH_ADDR_TOP_NODE & 0xFF;
    memcpy(&packet[2], &frame, sizeof(frame));

    return uart_write(packet, sizeof(packet));
}

/**
 * @brief DDC 注册/心跳任务 (带重传状态机)
 *
 * 状态机:
 *   REGISTERING -> (收到ACK) -> ONLINE -> (心跳超时) -> REGISTERING
 */
#define REGISTER_RETRY_INTERVAL_MS   5000   // 注册重试间隔 5秒
#define REGISTER_MAX_RETRIES         10     // 最大重试次数
#define HEARTBEAT_INTERVAL_MS        30000  // 心跳间隔 30秒

typedef enum {
    DDC_STATE_INIT,
    DDC_STATE_REGISTERING,
    DDC_STATE_ONLINE,
} ddc_state_t;

static volatile ddc_state_t g_ddc_state = DDC_STATE_INIT;
static volatile bool g_register_ack_received = false;

/**
 * @brief DDC 收到注册确认时调用
 */
void ddc_on_register_ack(void) {
    g_register_ack_received = true;
}

/**
 * @brief DDC 注册/心跳任务
 */
void ddc_heartbeat_task(void *arg) {
    ddc_config_t *config = (ddc_config_t *)arg;
    uint8_t retry_count = 0;

    g_ddc_state = DDC_STATE_REGISTERING;

    while (1) {
        switch (g_ddc_state) {
            case DDC_STATE_REGISTERING: {
                // 发送注册请求
                ddc_send_register(config);
                retry_count++;

                // 等待 ACK (带超时)
                g_register_ack_received = false;
                for (int i = 0; i < REGISTER_RETRY_INTERVAL_MS / 100; i++) {
                    vTaskDelay(pdMS_TO_TICKS(100));
                    if (g_register_ack_received) {
                        // 收到 ACK，进入在线状态
                        g_ddc_state = DDC_STATE_ONLINE;
                        retry_count = 0;
                        break;
                    }
                }

                // 超时检查
                if (g_ddc_state == DDC_STATE_REGISTERING) {
                    if (retry_count >= REGISTER_MAX_RETRIES) {
                        // 重试次数用尽，重启模组
                        tpmesh_module_reset();
                        retry_count = 0;
                    }
                }
                break;
            }

            case DDC_STATE_ONLINE: {
                // 定时发送心跳
                vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_INTERVAL_MS));
                ddc_send_heartbeat(config);
                break;
            }

            default:
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;
        }
    }
}
```

#### 2.1.3 Top Node 注册处理

```c
/**
 * @brief Top Node 处理注册/心跳帧
 */
void top_handle_reg_frame(uint16_t src_mesh_id, const uint8_t *data, uint16_t len) {
    if (len < sizeof(reg_frame_t)) return;

    reg_frame_t *frame = (reg_frame_t *)data;

    // 校验 CRC
    if (calc_crc16(frame, sizeof(reg_frame_t) - 2) != frame->checksum) {
        return;
    }

    switch (frame->frame_type) {
        case REG_FRAME_TYPE_REGISTER:
        case REG_FRAME_TYPE_HEARTBEAT: {
            // 更新/添加到映射表
            ip4_addr_t ip;
            ip.addr = frame->ip;
            node_table_learn(frame->mac, &ip, frame->mesh_id);

            // 回复确认
            if (frame->frame_type == REG_FRAME_TYPE_REGISTER) {
                send_reg_ack(src_mesh_id);
            }
            break;
        }

        default:
            break;
    }
}

/**
 * @brief 发送注册确认
 */
void send_reg_ack(uint16_t dest_mesh_id) {
    reg_frame_t ack;
    ack.frame_type = REG_FRAME_TYPE_REGISTER_ACK;
    // ... 填充 Top Node 信息

    uint8_t packet[2 + sizeof(reg_frame_t)];
    packet[0] = (dest_mesh_id >> 8) & 0xFF;
    packet[1] = dest_mesh_id & 0xFF;
    memcpy(&packet[2], &ack, sizeof(ack));

    uart_write(packet, sizeof(packet));
}

/**
 * @brief 发送免费 ARP (Gratuitous ARP)
 *
 * 当 DDC 注册成功后，Top Node 主动向以太网发送 GARP
 * 作用: 刷新 BMS 的 ARP 缓存和交换机的 MAC 表
 */
void send_gratuitous_arp(const uint8_t *ddc_mac, const ip4_addr_t *ddc_ip) {
    uint8_t garp[SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR];
    struct eth_hdr *eth = (struct eth_hdr *)garp;
    struct etharp_hdr *arp = (struct etharp_hdr *)(garp + SIZEOF_ETH_HDR);

    // 以太网头: 广播
    memset(eth->dest.addr, 0xFF, 6);           // 广播
    memcpy(eth->src.addr, ddc_mac, 6);         // DDC MAC (让交换机学习)
    eth->type = PP_HTONS(ETHTYPE_ARP);

    // ARP: 免费 ARP (源 IP = 目标 IP)
    arp->hwtype = PP_HTONS(HWTYPE_ETHERNET);
    arp->proto = PP_HTONS(ETHTYPE_IP);
    arp->hwlen = 6;
    arp->protolen = 4;
    arp->opcode = PP_HTONS(ARP_REQUEST);  // 或 ARP_REPLY 都可以

    // 发送方 = DDC
    memcpy(arp->shwaddr.addr, ddc_mac, 6);
    SMEMCPY(&arp->sipaddr, ddc_ip, sizeof(ip4_addr_t));

    // 目标 = DDC (免费 ARP 的特征)
    memset(arp->dhwaddr.addr, 0x00, 6);        // 或广播
    SMEMCPY(&arp->dipaddr, ddc_ip, sizeof(ip4_addr_t));

    // 发送到以太网
    struct pbuf *p = pbuf_alloc(PBUF_RAW, sizeof(garp), PBUF_RAM);
    if (p) {
        pbuf_take(p, garp, sizeof(garp));
        g_eth_netif.linkoutput(&g_eth_netif, p);
        pbuf_free(p);
    }
}

/**
 * @brief Top Node 处理注册帧 (增强版)
 */
void top_handle_reg_frame_v2(uint16_t src_mesh_id, const uint8_t *data, uint16_t len) {
    if (len < sizeof(reg_frame_t)) return;

    reg_frame_t *frame = (reg_frame_t *)data;

    // 校验 CRC
    if (calc_crc16(frame, sizeof(reg_frame_t) - 2) != frame->checksum) {
        return;
    }

    ip4_addr_t ip;
    ip.addr = frame->ip;

    switch (frame->frame_type) {
        case REG_FRAME_TYPE_REGISTER: {
            // 注册请求
            bool is_new = !node_table_is_registered(frame->mesh_id);

            // 更新映射表
            node_table_learn(frame->mac, &ip, frame->mesh_id);

            // 回复确认
            send_reg_ack(src_mesh_id);

            // ★ 如果是新节点，发送免费 ARP ★
            if (is_new) {
                send_gratuitous_arp(frame->mac, &ip);
            }
            break;
        }

        case REG_FRAME_TYPE_HEARTBEAT: {
            // 心跳: 更新活跃时间
            node_table_learn(frame->mac, &ip, frame->mesh_id);
            break;
        }

        default:
            break;
    }
}
```

---

### 2.2 Top Node 代答 ARP (低延迟)

**问题回顾：** Mesh 网络延迟高（多跳可能 > 1秒），BMS 的 ARP 超时通常 1-2 秒，可能导致 ARP 失败。

**解决方案：** Top Node **代替 DDC 回复 ARP**，毫秒级响应。

```
ARP 流程对比:

V0.5 (转发 ARP):
BMS ──ARP Req──> Top ──转发──> Mesh ──> DDC
                                         │
BMS <──ARP Reply─ Top <──转发─ Mesh <───┘
RTT: 可能 > 1秒 (多跳)

V0.6 (代答 ARP):
BMS ──ARP Req──> Top (查表，直接回复)
                  │
BMS <──ARP Reply─┘
RTT: < 10ms
```

#### 2.2.1 代理 ARP 实现

```c
/**
 * @brief 处理 ARP 请求 (代答模式)
 */
filter_action_t handle_arp_request(const uint8_t *eth_frame, uint16_t len) {
    const struct eth_hdr *eth = (const struct eth_hdr *)eth_frame;
    const struct etharp_hdr *arp = (const struct etharp_hdr *)
        (eth_frame + SIZEOF_ETH_HDR);

    // 提取目标 IP
    ip4_addr_t target_ip;
    SMEMCPY(&target_ip, &arp->dipaddr, sizeof(ip4_addr_t));

    // 查找映射表
    uint8_t target_mac[6];
    if (node_table_get_mac_by_ip(&target_ip, target_mac) == 0) {
        // ★ 找到了！Top Node 代替 DDC 回复 ARP ★
        send_proxy_arp_reply(eth_frame, target_mac, &target_ip);
        return FILTER_DROP;  // 不转发到 Mesh
    }

    // 不在表中，丢弃 (等 DDC 注册后再试)
    return FILTER_DROP;
}

/**
 * @brief 发送代理 ARP 回复
 *
 * 关键: 回复 DDC 的真实 MAC 地址
 * 这样 BMS 的 ARP 缓存中是 DDC 的真实 MAC
 */
void send_proxy_arp_reply(const uint8_t *arp_request,
                          const uint8_t *target_mac,
                          const ip4_addr_t *target_ip) {
    const struct eth_hdr *req_eth = (const struct eth_hdr *)arp_request;
    const struct etharp_hdr *req_arp = (const struct etharp_hdr *)
        (arp_request + SIZEOF_ETH_HDR);

    // 构建 ARP 回复
    uint8_t reply[SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR];
    struct eth_hdr *rep_eth = (struct eth_hdr *)reply;
    struct etharp_hdr *rep_arp = (struct etharp_hdr *)(reply + SIZEOF_ETH_HDR);

    // 以太网头
    memcpy(rep_eth->dest.addr, req_eth->src.addr, 6);  // 发给请求者
    memcpy(rep_eth->src.addr, target_mac, 6);          // ★ DDC 的真实 MAC ★
    rep_eth->type = PP_HTONS(ETHTYPE_ARP);

    // ARP 回复
    rep_arp->hwtype = PP_HTONS(HWTYPE_ETHERNET);
    rep_arp->proto = PP_HTONS(ETHTYPE_IP);
    rep_arp->hwlen = 6;
    rep_arp->protolen = 4;
    rep_arp->opcode = PP_HTONS(ARP_REPLY);

    // 发送方 = DDC (Target)
    memcpy(rep_arp->shwaddr.addr, target_mac, 6);      // ★ DDC 的真实 MAC ★
    SMEMCPY(&rep_arp->sipaddr, target_ip, sizeof(ip4_addr_t));

    // 目标 = 请求者 (BMS)
    memcpy(rep_arp->dhwaddr.addr, req_arp->shwaddr.addr, 6);
    SMEMCPY(&rep_arp->dipaddr, &req_arp->sipaddr, sizeof(ip4_addr_t));

    // 发送到以太网
    struct pbuf *p = pbuf_alloc(PBUF_RAW, sizeof(reply), PBUF_RAM);
    if (p) {
        pbuf_take(p, reply, sizeof(reply));
        g_eth_netif.linkoutput(&g_eth_netif, p);
        pbuf_free(p);
    }
}
```

---

### 2.3 AT+SEND 模式 (URC 带源地址)

**设计选择：** 透传模式和 AT+SEND 模式的最大包长都是 200 字节，但 AT 模式接收时 URC 自带源地址，无需在 Payload 中携带，可节省 2 字节隧道头部。

| 方面 | 透传模式 2 | AT+SEND 模式 |
|------|-----------|--------------|
| **最大包长** | 200 字节 | 200 字节 |
| **接收格式** | 只有 Payload | `+NNMI:ADDR,LEN,DATA` ★带源地址★ |
| **有效载荷** | 198 字节 (需携带源地址) | 200 字节 |

#### 2.3.1 模组初始化

```c
/**
 * @brief Top Node 模组初始化 (AT 模式)
 */
const char* top_init_cmds[] = {
    "AT\r\n",
    "AT+ADDR=FFFE\r\n",           // 中心节点
    "AT+CELL=0\r\n",
    "AT+LP=3\r\n",                // TypeD (常接收)
    "AT+NR\r\n",                  // 新数据到达提醒
    "AT+NNMI=2\r\n",              // ★ 接收 URC: +NNMI:ADDR,LEN,DATA ★
    NULL
};

/**
 * @brief DDC 模组初始化 (AT 模式)
 */
const char* ddc_init_cmds[] = {
    "AT\r\n",
    "AT+ADDR=0002\r\n",           // 普通节点
    "AT+CELL=0\r\n",
    "AT+LP=2\r\n",                // TypeC (低功耗)
    "AT+WOR=500\r\n",
    "AT+NR\r\n",                  // 新数据到达提醒
    "AT+NNMI=2\r\n",              // ★ 接收 URC: +NNMI:ADDR,LEN,DATA ★
    NULL
};
```

#### 2.3.2 AT 模式数据格式

```
AT+SEND 发送:
┌─────────────────────────────────────────────────────────────────┐
│  AT+SEND=<DEST_ADDR>,<LEN>,<HEX_DATA>\r\n                      │
│                                                                 │
│  示例: AT+SEND=0002,10,0180010203040506070809\r\n              │
│        发送给 0x0002, 长度 10, 数据为 hex 字符串               │
└─────────────────────────────────────────────────────────────────┘

- DEST_ADDR: 4 位 hex 字符串 (目标 Mesh ID)
- LEN: 十进制数字 (1~200)
- HEX_DATA: 2*LEN 个 hex 字符

+NNMI 接收 URC:
┌─────────────────────────────────────────────────────────────────┐
│  +NNMI:<SRC_ADDR>,<LEN>,<HEX_DATA>\r\n                         │
│                                                                 │
│  示例: +NNMI:0002,10,0180010203040506070809\r\n                │
│        来自 0x0002, 长度 10, 数据为 hex 字符串                 │
└─────────────────────────────────────────────────────────────────┘

- ★ SRC_ADDR: 源地址！关键优势，无需在 Payload 携带 ★
- LEN: 十进制数字
- HEX_DATA: 2*LEN 个 hex 字符
```

#### 2.3.3 发送实现

```c
/**
 * @brief AT+SEND 模式发送
 *
 * @param dest_mesh_id 目标 Mesh ID
 * @param data 二进制数据
 * @param len 数据长度 (最大 200)
 * @return int 0=成功
 */
int tpmesh_send_at(uint16_t dest_mesh_id,
                   const uint8_t *data, uint16_t len) {
    char cmd[512];  // AT+SEND=XXXX,LEN, + 400 hex chars + \r\n
    char *p = cmd;

    if (len > 200) return -1;

    // 构建 AT 命令头
    p += sprintf(p, "AT+SEND=%04X,%d,", dest_mesh_id, len);

    // 转换为 Hex 字符串
    for (uint16_t i = 0; i < len; i++) {
        p += sprintf(p, "%02X", data[i]);
    }

    // 添加结束符
    *p++ = '\r';
    *p++ = '\n';
    *p = '\0';

    // 发送 AT 命令
    return uart_write_string(cmd);
}

/**
 * @brief 解析 +NNMI URC
 *
 * 输入: "+NNMI:0002,10,0180010203040506070809\r\n"
 * 输出: src_mesh_id=0x0002, data=[...], len=10
 */
int tpmesh_parse_nnmi(const char *urc,
                      uint16_t *src_mesh_id,
                      uint8_t *data, uint16_t *len) {
    // 跳过 "+NNMI:"
    if (strncmp(urc, "+NNMI:", 6) != 0) return -1;
    urc += 6;

    // 解析源地址 (4 位 hex)
    char addr_str[5];
    strncpy(addr_str, urc, 4);
    addr_str[4] = '\0';
    *src_mesh_id = (uint16_t)strtol(addr_str, NULL, 16);
    urc += 5;  // 跳过 "XXXX,"

    // 解析长度
    *len = (uint16_t)atoi(urc);
    while (*urc && *urc != ',') urc++;
    urc++;  // 跳过 ","

    // 解析 Hex 数据
    for (uint16_t i = 0; i < *len; i++) {
        char hex[3] = { urc[i*2], urc[i*2+1], '\0' };
        data[i] = (uint8_t)strtol(hex, NULL, 16);
    }

    return 0;
}
```

---

### 2.4 SCHC 压缩 (带宽优化)

**问题回顾：** 纯 L2 桥接开销太大（~25 字节头部）。

**解决方案：** 在 L2 隧道内部，对 **BACnet/IP 包** 进行 SCHC 压缩。

#### 2.4.1 压缩规则

| Rule ID | 场景 | 压缩内容 | 节省 |
|---------|------|----------|------|
| `0x00` | 非 IP 帧 | 不压缩 | 0 |
| `0x01` | BACnet/IP | IP头 + UDP头 | 28 字节 |
| `0x02` | 其他 IP | 仅 IP 头 | 20 字节 |
| `0x10` | 注册/心跳 | 专用帧 | - |

#### 2.4.2 隧道帧格式 (V0.6.2)

```
V0.6.2 隧道帧格式 (AT 模式):
┌──────────┬──────────┬──────────┬──────────────────┐
│ L2 HDR   │ FRAG HDR │ Rule ID  │    Payload       │
│ 1 byte   │ 1 byte   │ 1 byte   │    N bytes       │
└──────────┴──────────┴──────────┴──────────────────┘

★ 源地址从 AT URC +NNMI:SRC_ADDR,... 获取，无需在 Payload 携带 ★

L2 Header:
  Bit7: Broadcast (1=广播, 0=单播)
  Bit6-0: Reserved

Frag Header:
  Bit7: Last (1=最后一片)
  Bit6-0: Seq (0~127)

Rule ID:
  0x00: 不压缩，Payload = [SRC_MAC:6][DST_MAC:6][EtherType:2][Data]
  0x01: BACnet/IP 压缩，Payload = [SRC_MAC:6][BACnet APDU]
  0x02: IP 压缩，Payload = [SRC_MAC:6][UDP头:8][Data]
  0x10: 注册/心跳帧

总头部: 3 字节 (vs V0.6 的 5 字节，节省 2 字节)

有效 MTU: 200 - 3 = 197 字节
```

#### 2.4.3 压缩实现 (V0.6.2)

```c
/**
 * @brief 封装以太网帧 (带 SCHC 压缩)
 *
 * V0.6.2: 去掉 SRC_MESH 字段，源地址从 AT URC 获取
 * 帧格式: [L2_HDR:1][FRAG_HDR:1][Rule:1][Payload:N]
 */
int l2_encapsulate(const uint8_t *eth_frame, uint16_t eth_len,
                   uint8_t *out_data, uint16_t *out_len,
                   bool is_broadcast) {
    const struct eth_hdr *eth = (const struct eth_hdr *)eth_frame;
    uint16_t ethertype = lwip_ntohs(eth->type);

    // L2 Header (1 byte)
    out_data[0] = is_broadcast ? 0x80 : 0x00;

    // Frag Header (1 byte, 初始为最后一片)
    out_data[1] = 0x80;

    // ★ V0.6.2: 无 SRC_MESH，从 AT URC 获取 ★

    // 尝试 SCHC 压缩
    if (ethertype == ETHERTYPE_IP) {
        const struct ip_hdr *iph = (const struct ip_hdr *)
            (eth_frame + SIZEOF_ETH_HDR);

        if (IPH_PROTO(iph) == IP_PROTO_UDP) {
            const struct udp_hdr *udph = (const struct udp_hdr *)
                ((uint8_t *)iph + IPH_HL(iph) * 4);

            uint16_t dst_port = lwip_ntohs(udph->dest);

            if (dst_port == PORT_BACNET) {
                // ★ BACnet/IP: 压缩 IP + UDP 头 (28字节) ★
                out_data[2] = 0x01;  // Rule ID

                // 只保留 SRC MAC + BACnet APDU
                memcpy(&out_data[3], eth->src.addr, 6);

                uint16_t udp_payload_offset = SIZEOF_ETH_HDR +
                    IPH_HL(iph) * 4 + sizeof(struct udp_hdr);
                uint16_t udp_payload_len = eth_len - udp_payload_offset;

                memcpy(&out_data[9], eth_frame + udp_payload_offset, udp_payload_len);

                *out_len = 9 + udp_payload_len;
                return 0;
            }
        }

        // 其他 IP: 压缩 IP 头 (20字节)
        out_data[2] = 0x02;
        memcpy(&out_data[3], eth->src.addr, 6);

        uint16_t ip_payload_offset = SIZEOF_ETH_HDR + IPH_HL(iph) * 4;
        uint16_t ip_payload_len = eth_len - ip_payload_offset;

        memcpy(&out_data[9], eth_frame + ip_payload_offset, ip_payload_len);

        *out_len = 9 + ip_payload_len;
        return 0;
    }

    // 非 IP: 不压缩
    out_data[2] = 0x00;

    // 完整以太网帧 (跳过目标 MAC，因为可从 Mesh 地址恢复)
    memcpy(&out_data[3], eth->src.addr, 6);           // SRC MAC
    memcpy(&out_data[9], &eth->type, eth_len - 12);   // EtherType + Payload

    *out_len = 3 + 6 + (eth_len - 12);
    return 0;
}

/**
 * @brief 解封装 (带 SCHC 解压)
 *
 * V0.6.2: 源地址从 AT URC +NNMI:SRC_ADDR,... 获取
 *
 * @param mesh_data 隧道帧数据 [L2_HDR:1][FRAG_HDR:1][Rule:1][Payload:N]
 * @param mesh_len 隧道帧长度
 * @param out_frame 输出以太网帧
 * @param out_len 输出长度
 * @param src_mesh_id ★ 从 AT URC 解析得到的源 Mesh ID ★
 * @param dst_mesh_id 本机 Mesh ID (用于单播 MAC 恢复)
 */
int l2_decapsulate(const uint8_t *mesh_data, uint16_t mesh_len,
                   uint8_t *out_frame, uint16_t *out_len,
                   uint16_t src_mesh_id, uint16_t dst_mesh_id) {
    uint8_t l2_hdr = mesh_data[0];
    // uint8_t frag_hdr = mesh_data[1];  // 已在重组时处理
    uint8_t rule_id = mesh_data[2];
    const uint8_t *payload = &mesh_data[3];
    uint16_t payload_len = mesh_len - 3;

    struct eth_hdr *eth = (struct eth_hdr *)out_frame;

    // 恢复目标 MAC
    if (l2_hdr & 0x80) {
        memset(eth->dest.addr, 0xFF, 6);  // 广播
    } else {
        // 单播: 从映射表查找
        if (node_table_get_mac_by_mesh_id(dst_mesh_id, eth->dest.addr) < 0) {
            get_local_mac(eth->dest.addr);  // 默认本机
        }
    }

    // 恢复源 MAC
    memcpy(eth->src.addr, payload, 6);
    payload += 6;
    payload_len -= 6;

    // 动态学习 (使用 AT URC 提供的源地址)
    node_table_learn_by_mesh(src_mesh_id, eth->src.addr);

    switch (rule_id) {
        case 0x01: {
            // BACnet/IP: 重建 IP + UDP 头
            return rebuild_bacnet_frame(out_frame, out_len,
                                        payload, payload_len, src_mesh_id);
        }

        case 0x02: {
            // IP: 重建 IP 头
            return rebuild_ip_frame(out_frame, out_len,
                                    payload, payload_len, src_mesh_id);
        }

        case 0x00:
        default: {
            // 不压缩: 直接复制
            memcpy((uint8_t *)eth + 12, payload, payload_len);
            *out_len = 12 + payload_len;
            return 0;
        }
    }
}

/**
 * @brief 重建 BACnet/IP 帧
 */
int rebuild_bacnet_frame(uint8_t *out_frame, uint16_t *out_len,
                         const uint8_t *bacnet_apdu, uint16_t apdu_len,
                         uint16_t src_mesh_id) {
    struct eth_hdr *eth = (struct eth_hdr *)out_frame;
    struct ip_hdr *iph = (struct ip_hdr *)(out_frame + SIZEOF_ETH_HDR);
    struct udp_hdr *udph = (struct udp_hdr *)(out_frame + SIZEOF_ETH_HDR + 20);
    uint8_t *data = out_frame + SIZEOF_ETH_HDR + 20 + 8;

    // EtherType
    eth->type = PP_HTONS(ETHERTYPE_IP);

    // IP 头
    uint16_t total_len = 20 + 8 + apdu_len;
    IPH_VHL_SET(iph, 4, 5);
    IPH_TOS_SET(iph, 0);
    IPH_LEN_SET(iph, lwip_htons(total_len));
    IPH_ID_SET(iph, 0);
    IPH_OFFSET_SET(iph, 0);
    IPH_TTL_SET(iph, 64);
    IPH_PROTO_SET(iph, IP_PROTO_UDP);

    // 恢复 IP 地址 (从映射表)
    ip4_addr_t src_ip, dst_ip;
    node_table_get_ip_by_mesh_id(src_mesh_id, &src_ip);
    IP4_ADDR(&dst_ip, 192, 168, 10, 1);  // Top Node

    iph->src = src_ip;
    iph->dest = dst_ip;

    IPH_CHKSUM_SET(iph, 0);
    IPH_CHKSUM_SET(iph, inet_chksum(iph, 20));

    // UDP 头
    udph->src = lwip_htons(PORT_BACNET);
    udph->dest = lwip_htons(PORT_BACNET);
    udph->len = lwip_htons(8 + apdu_len);
    udph->chksum = 0;

    // BACnet APDU
    memcpy(data, bacnet_apdu, apdu_len);

    *out_len = SIZEOF_ETH_HDR + total_len;
    return 0;
}
```

---

### 2.5 广播限速 (防止 Mesh 拥塞)

```c
/* ==================== 广播限速 ==================== */

#define BROADCAST_RATE_LIMIT_MS  1000  // 每秒最多1个广播
#define BROADCAST_BURST_MAX      3     // 突发允许3个

typedef struct {
    uint32_t last_broadcast_tick;
    uint8_t  burst_count;
} rate_limiter_t;

static rate_limiter_t g_broadcast_limiter;

/**
 * @brief 检查广播是否允许发送
 */
bool broadcast_rate_check(void) {
    uint32_t now = get_tick_ms();
    uint32_t elapsed = now - g_broadcast_limiter.last_broadcast_tick;

    if (elapsed >= BROADCAST_RATE_LIMIT_MS) {
        // 超过限速周期，重置
        g_broadcast_limiter.burst_count = 0;
        g_broadcast_limiter.last_broadcast_tick = now;
        return true;
    }

    if (g_broadcast_limiter.burst_count < BROADCAST_BURST_MAX) {
        // 在突发允许范围内
        g_broadcast_limiter.burst_count++;
        return true;
    }

    // 超过限速
    return false;
}

/**
 * @brief 广播过滤 (带限速)
 */
filter_action_t filter_broadcast_frame_v6(const uint8_t *eth_frame,
                                          uint16_t len,
                                          uint16_t *dest_mesh) {
    // ... 原有过滤逻辑 ...

    // BACnet 广播检查限速
    if (is_bacnet_broadcast(eth_frame, len)) {
        if (!broadcast_rate_check()) {
            // 超过限速，丢弃
            return FILTER_DROP;
        }
        *dest_mesh = MESH_ADDR_BROADCAST;
        return FILTER_FORWARD;
    }

    // ... 其他逻辑 ...
}
```

---

### 2.6 分片发送延时 (防止模组溢出)

```c
/**
 * @brief 分片并发送 (带延时)
 *
 * V0.6.2: AT+SEND 模式，MTU=200 字节
 * 隧道头 3 字节 (L2_HDR + FRAG_HDR + Rule)，有效 MTU = 197 字节
 */
#define FRAG_SEND_DELAY_MS  50  // 分片间延时 50ms
#define FRAG_MTU            197 // 200 - 3 (L2_HDR + FRAG + RULE)

int l2_fragment_and_send(uint16_t dest_mesh_id,
                         const uint8_t *eth_frame, uint16_t eth_len,
                         bool is_broadcast) {
    uint8_t encap_buf[1600];
    uint16_t encap_len;

    // 封装 (带压缩, V0.6.2 无需 src_mesh_id)
    l2_encapsulate(eth_frame, eth_len, encap_buf, &encap_len, is_broadcast);

    // 分片发送
    uint8_t seq = 0;
    uint16_t offset = 0;

    while (offset < encap_len) {
        uint8_t packet[200];
        uint16_t chunk_len;

        if (seq == 0) {
            // 第一片: 完整头部
            chunk_len = MIN(200, encap_len);
            memcpy(packet, encap_buf, chunk_len);
        } else {
            // 后续片: 简化头部 (L2_HDR + FRAG_HDR)
            chunk_len = MIN(198, encap_len - offset);
            packet[0] = encap_buf[0];  // L2_HDR
            packet[1] = (seq & 0x7F);  // FRAG_HDR (暂时标记非最后)
            memcpy(&packet[2], encap_buf + offset, chunk_len);
            chunk_len += 2;
        }

        // 检查是否最后一片
        bool is_last = (offset + chunk_len >= encap_len) ||
                       (seq == 0 && chunk_len >= encap_len);
        if (is_last) {
            packet[1] |= 0x80;  // 设置 Last 标志
        }

        // ★ 发送 (AT+SEND 模式) ★
        tpmesh_send_at(dest_mesh_id, packet, chunk_len);

        offset += (seq == 0) ? chunk_len : (chunk_len - 2);
        seq++;

        // ★ 分片间延时，防止模组 buffer 溢出 ★
        if (!is_last) {
            vTaskDelay(pdMS_TO_TICKS(FRAG_SEND_DELAY_MS));
        }
    }

    return 0;
}
```

---

## 3. 节点映射表 (增强版)

### 3.1 数据结构

```c
typedef struct {
    uint8_t  valid;              // 条目有效
    uint8_t  mac[6];             // MAC 地址
    ip4_addr_t ip;               // IP 地址
    uint16_t mesh_id;            // Mesh ID
    uint32_t last_seen;          // 最后活跃时间
    uint8_t  source;             // 0=静态配置, 1=动态学习, 2=注册
    uint8_t  online;             // 在线状态
} node_entry_t;

#define MAX_NODE_ENTRIES  16
#define NODE_TIMEOUT_MS   90000  // 90秒无心跳视为离线

static node_entry_t g_node_table[MAX_NODE_ENTRIES];
```

### 3.2 超时检查

```c
/**
 * @brief 节点表维护任务 (检查超时)
 */
void node_table_maintenance_task(void *arg) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));  // 10秒检查一次

        uint32_t now = get_tick_ms();

        for (int i = 0; i < MAX_NODE_ENTRIES; i++) {
            node_entry_t *entry = &g_node_table[i];

            if (entry->valid && entry->source != 0) {
                // 动态条目检查超时
                if (now - entry->last_seen > NODE_TIMEOUT_MS) {
                    entry->online = 0;  // 标记离线
                    // 可选: 清除条目
                    // entry->valid = 0;
                }
            }
        }
    }
}
```

---

## 4. 完整数据流程

### 4.1 DDC 上线流程

```
┌─────────────────────────────────────────────────────────────────┐
│  DDC 上电                                                        │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  模组初始化 (AT+ADDR, AT+TRANS=2,0, AT+EXIT)                    │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  等待路由建立 (+ROUTE:CREATE)                                    │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  ★ 发送注册帧给 Top Node ★                                       │
│  [Mesh Dest: FFFE][Type:0x01][MAC][IP][MeshID][CRC]             │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Top Node 收到，更新映射表                                       │
│  回复注册确认                                                    │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  DDC 进入正常工作状态                                            │
│  启动心跳任务 (每 30 秒)                                         │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 BMS 发现 DDC 流程

```
┌─────────────────────────────────────────────────────────────────┐
│  BMS: 发送 ARP Request (Who is 192.168.10.2?)                   │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Top Node: 收到 ARP Request                                     │
│  → 查表: 192.168.10.2 → MAC=xx:xx, Mesh=0x0002                 │
│  → ★ 代答 ARP Reply (不转发到 Mesh) ★                          │
│  → 回复: "192.168.10.2 is at xx:xx:xx:xx:xx:xx"                │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  BMS: 收到 ARP Reply (< 10ms)                                   │
│  → 缓存 ARP                                                     │
│  → 后续直接发送 IP 单播                                          │
└─────────────────────────────────────────────────────────────────┘
```

### 4.3 BMS → DDC 数据流程 (单播)

```
┌─────────────────────────────────────────────────────────────────┐
│  BMS: 发送 BACnet/IP 包给 DDC (192.168.10.2)                    │
│  ETH: [DDC MAC][BMS MAC][0800][IP:UDP:BACnet APDU]              │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Top Node: 收到单播帧                                            │
│  → 查表: DDC MAC → Mesh 0x0002                                  │
│  → SCHC 压缩 (Rule 0x01): 去除 IP+UDP 头                        │
│  → L2 封装: [L2_HDR][FRAG][SRC_MESH:FFFE][Rule:01][SRC_MAC][APDU]│
│  → 分片 (如需要) + 透传发送                                      │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼ Mesh 传输
┌─────────────────────────────────────────────────────────────────┐
│  DDC: 收到 Mesh 数据                                             │
│  → 分片重组                                                      │
│  → L2 解封装 + SCHC 解压                                         │
│  → 重建 ETH 帧                                                   │
│  → 注入 LwIP                                                     │
└─────────────────────────────────────────────────────────────────┘
```

### 4.4 BACnet Who-Is 广播流程 (设备发现)

**场景：** BMS 发送 Who-Is 广播发现网络上的 BACnet 设备

```
┌─────────────────────────────────────────────────────────────────┐
│  BMS: 发送 BACnet Who-Is 广播                                    │
│  ETH: [FF:FF:FF:FF:FF:FF][BMS MAC][0800][IP][UDP:47808][Who-Is] │
│  目标 IP: 192.168.10.255 (子网广播)                              │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Top Node: 收到广播帧                                            │
│  → 检测: 目标 MAC=FF:FF:FF:FF:FF:FF (广播)                      │
│  → 识别: UDP 端口=47808 (BACnet/IP)                             │
│  → ★ 广播限速检查 (broadcast_rate_check) ★                      │
│     - 通过: 继续处理                                             │
│     - 拒绝: 丢弃 (防止 Mesh 风暴)                                │
└───────────────────────────────┬─────────────────────────────────┘
                                │ (限速通过)
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Top Node: 封装并转发到 Mesh                                     │
│  → SCHC 压缩 (Rule 0x01): 去除 IP+UDP 头 (节省 28 字节)         │
│  → L2 封装:                                                      │
│    [L2_HDR:0x80][FRAG][SRC_MESH:FFFE][Rule:01][BMS_MAC][Who-Is] │
│    (L2_HDR bit7=1 表示广播)                                      │
│  → 发送到 Mesh 广播地址 0x0000                                   │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼ Mesh 广播 (所有 DDC 收到)
┌─────────────────────────────────────────────────────────────────┐
│  DDC (所有在线节点): 收到 Mesh 广播                              │
│  → 分片重组 (如需要)                                             │
│  → L2 解封装 + SCHC 解压                                         │
│  → 重建完整以太网帧:                                             │
│    [FF:FF:FF:FF:FF:FF][BMS MAC][0800][IP][UDP:47808][Who-Is]    │
│  → 注入 LwIP 协议栈                                              │
│  → BACnet 应用层处理 Who-Is                                      │
│  → ★ 生成 I-Am 响应 (见 4.5) ★                                  │
└─────────────────────────────────────────────────────────────────┘
```

### 4.5 BACnet I-Am 响应流程 (DDC → BMS)

**场景 A：I-Am 广播响应 (最常见)**

大多数 BACnet 设备对 Who-Is 的响应是**广播 I-Am**，让所有设备都能发现它。

```
┌─────────────────────────────────────────────────────────────────┐
│  DDC: BACnet 栈生成 I-Am 广播响应                                │
│  ETH: [FF:FF:FF:FF:FF:FF][DDC MAC][0800][IP][UDP:47808][I-Am]   │
│  目标 IP: 192.168.10.255 (子网广播)                              │
│  内容: Device Instance, Vendor ID, Max APDU, Segmentation...    │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  DDC: L2 隧道封装 (★ DDC 侧的广播处理 ★)                        │
│  → 检测: 目标 MAC=FF:FF:FF:FF:FF:FF (广播)                      │
│  → SCHC 压缩 (Rule 0x01)                                        │
│  → L2 封装:                                                      │
│    [L2_HDR:0x80][FRAG][SRC_MESH:DDC_ID][Rule:01][DDC_MAC][I-Am] │
│    (L2_HDR bit7=1 表示这是广播帧)                                │
│  → 发送到 Top Node (透传模式: dest=0xFFFE)                       │
│    注意: 广播帧也发送给 Top Node，由 Top Node 转发到以太网       │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼ Mesh 单播 (DDC → Top Node)
┌─────────────────────────────────────────────────────────────────┐
│  Top Node: 收到来自 DDC 的帧                                     │
│  → 检测: L2_HDR bit7=1 (广播标记)                               │
│  → 分片重组 (如需要)                                             │
│  → L2 解封装 + SCHC 解压                                         │
│  → 重建以太网帧:                                                 │
│    [FF:FF:FF:FF:FF:FF][DDC MAC][0800][IP][UDP:47808][I-Am]      │
│  → ★ 转发到以太网 (linkoutput) ★                                │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  BMS: 收到 I-Am 广播                                             │
│  → BACnet 栈处理                                                 │
│  → 发现新设备 (Device Instance = DDC ID)                        │
│  → 后续可发起单播通信 (ReadProperty 等)                          │
└─────────────────────────────────────────────────────────────────┘
```

**场景 B：I-Am 单播响应 (可选)**

某些 BACnet 实现可能单播 I-Am 给请求者。此时 DDC 需要知道 BMS 的 MAC 地址。

```
┌─────────────────────────────────────────────────────────────────┐
│  DDC: BACnet 栈生成 I-Am 单播 (目标=BMS IP)                      │
│  → LwIP: 需要 ARP 解析 BMS IP                                    │
│  → DDC 发送 ARP Request: "Who is 192.168.10.100?"               │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  DDC ARP 请求处理:                                               │
│  → L2 封装 ARP Request (广播)                                    │
│  → 发送到 Top Node (dest=0xFFFE, L2_HDR bit7=1)                 │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Top Node: 收到 DDC 的 ARP Request                               │
│  → 检测: ARP 请求 192.168.10.100 (BMS)                          │
│  → 方案 1: Top Node 本地 ARP 缓存有 BMS MAC                     │
│           → 构建 ARP Reply 发回 DDC                              │
│  → 方案 2: Top Node 转发 ARP 到以太网                           │
│           → 等待 BMS 回复 → 转发给 DDC                          │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  DDC: 收到 ARP Reply                                             │
│  → 更新本地 ARP 缓存                                             │
│  → 发送 I-Am 单播给 BMS                                          │
│  → L2 封装 (单播，L2_HDR bit7=0)                                │
│  → 发送到 Top Node → 转发到以太网                                │
└─────────────────────────────────────────────────────────────────┘
```

### 4.6 DDC → Top Node 广播帧处理

DDC 发出的广播帧（如 I-Am）需要特殊处理：

```c
/**
 * @brief DDC 侧以太网输出处理 (拦截广播)
 *
 * DDC 的所有以太网输出都需要通过 Mesh 隧道
 */
err_t ddc_etharp_output(struct netif *netif, struct pbuf *p,
                        const ip4_addr_t *ipaddr) {
    // 标准 ARP 解析流程
    // ...

    // 发送前拦截
    return ddc_linkoutput(netif, p);
}

/**
 * @brief DDC 链路层输出 (所有帧都走隧道)
 *
 * V0.6.2: 使用 AT+SEND 模式
 */
err_t ddc_linkoutput(struct netif *netif, struct pbuf *p) {
    struct eth_hdr *eth = (struct eth_hdr *)p->payload;
    bool is_broadcast = is_broadcast_mac(eth->dest.addr);

    // 封装并发送到 Top Node (V0.6.2: 无需 src_mesh_id)
    uint8_t encap_buf[1600];
    uint16_t encap_len;

    l2_encapsulate(p->payload, p->tot_len,
                   encap_buf, &encap_len,
                   is_broadcast);  // ★ 标记广播 ★

    // 广播帧也发送给 Top Node，由 Top Node 转发
    // (DDC 不直接 Mesh 广播，避免多 DDC 时的冲突)
    // ★ V0.6.2: 使用 AT+SEND 模式 ★
    return tpmesh_send_at(MESH_ADDR_TOP_NODE, encap_buf, encap_len);
}
```

### 4.7 Top Node 转发 DDC 广播帧

```c
/**
 * @brief Top Node 处理来自 DDC 的数据帧
 *
 * V0.6.2: src_mesh_id 从 AT URC +NNMI:SRC,... 获取
 */
void top_handle_data_frame(uint16_t src_mesh_id,
                           const uint8_t *data, uint16_t len) {
    // 分片重组
    uint8_t *complete_data;
    uint16_t complete_len;

    int ret = reassemble_l2_packet(src_mesh_id, data, len,
                                   &complete_data, &complete_len);
    if (ret != 1) return;  // 未完成重组

    // 解封装 (V0.6.2: src_mesh_id 从 AT URC 获取)
    uint8_t eth_frame[1600];
    uint16_t eth_len;

    l2_decapsulate(complete_data, complete_len,
                   eth_frame, &eth_len,
                   src_mesh_id, MESH_ADDR_TOP_NODE);

    // 检查是否广播帧
    uint8_t l2_hdr = complete_data[0];
    bool is_broadcast = (l2_hdr & 0x80) != 0;

    if (is_broadcast) {
        // ★ 广播帧: 转发到以太网 ★
        forward_broadcast_to_eth(eth_frame, eth_len);
    } else {
        // 单播帧: 直接转发
        forward_unicast_to_eth(eth_frame, eth_len);
    }
}

/**
 * @brief 转发广播帧到以太网
 */
void forward_broadcast_to_eth(const uint8_t *eth_frame, uint16_t len) {
    struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
    if (p) {
        pbuf_take(p, eth_frame, len);
        g_eth_netif.linkoutput(&g_eth_netif, p);
        pbuf_free(p);
    }
}

/**
 * @brief 转发单播帧到以太网
 */
void forward_unicast_to_eth(const uint8_t *eth_frame, uint16_t len) {
    struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
    if (p) {
        pbuf_take(p, eth_frame, len);
        g_eth_netif.linkoutput(&g_eth_netif, p);
        pbuf_free(p);
    }
}
```

### 4.8 Top Node 代答 DDC 的 ARP 请求

当 DDC 需要与 BMS 单播通信时，需要知道 BMS 的 MAC 地址。Top Node 代答此 ARP。

```c
/**
 * @brief 处理来自 DDC 的 ARP 请求
 *
 * DDC 发送 ARP 请求询问以太网设备 (如 BMS) 的 MAC
 * Top Node 从本地 ARP 缓存回复，或转发到以太网
 */
void top_handle_ddc_arp_request(uint16_t src_mesh_id,
                                const uint8_t *arp_frame, uint16_t len) {
    const struct eth_hdr *eth = (const struct eth_hdr *)arp_frame;
    const struct etharp_hdr *arp = (const struct etharp_hdr *)
        (arp_frame + SIZEOF_ETH_HDR);

    // 提取目标 IP
    ip4_addr_t target_ip;
    SMEMCPY(&target_ip, &arp->dipaddr, sizeof(ip4_addr_t));

    // 检查本地 ARP 缓存
    struct eth_addr *target_mac;
    if (etharp_find_addr(&g_eth_netif, &target_ip, &target_mac, NULL) >= 0) {
        // ★ 缓存命中: 直接回复 DDC ★
        send_arp_reply_to_ddc(src_mesh_id, arp_frame,
                              target_mac->addr, &target_ip);
    } else {
        // 缓存未命中: 转发到以太网，记录等待
        forward_arp_to_eth(arp_frame, len);
        pending_arp_add(src_mesh_id, &target_ip);
    }
}

/**
 * @brief 发送 ARP Reply 给 DDC
 */
void send_arp_reply_to_ddc(uint16_t dest_mesh_id,
                           const uint8_t *arp_request,
                           const uint8_t *target_mac,
                           const ip4_addr_t *target_ip) {
    const struct eth_hdr *req_eth = (const struct eth_hdr *)arp_request;
    const struct etharp_hdr *req_arp = (const struct etharp_hdr *)
        (arp_request + SIZEOF_ETH_HDR);

    // 构建 ARP Reply
    uint8_t reply[SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR];
    struct eth_hdr *rep_eth = (struct eth_hdr *)reply;
    struct etharp_hdr *rep_arp = (struct etharp_hdr *)
        (reply + SIZEOF_ETH_HDR);

    // 以太网头
    memcpy(rep_eth->dest.addr, req_eth->src.addr, 6);   // → DDC
    memcpy(rep_eth->src.addr, target_mac, 6);           // ← BMS MAC
    rep_eth->type = PP_HTONS(ETHTYPE_ARP);

    // ARP Reply
    rep_arp->hwtype = PP_HTONS(HWTYPE_ETHERNET);
    rep_arp->proto = PP_HTONS(ETHTYPE_IP);
    rep_arp->hwlen = 6;
    rep_arp->protolen = 4;
    rep_arp->opcode = PP_HTONS(ARP_REPLY);

    // 发送方 = 目标 (BMS)
    memcpy(rep_arp->shwaddr.addr, target_mac, 6);
    SMEMCPY(&rep_arp->sipaddr, target_ip, sizeof(ip4_addr_t));

    // 接收方 = 请求者 (DDC)
    memcpy(rep_arp->dhwaddr.addr, req_arp->shwaddr.addr, 6);
    SMEMCPY(&rep_arp->dipaddr, &req_arp->sipaddr, sizeof(ip4_addr_t));

    // 封装并发送到 DDC
    uint8_t encap_buf[256];
    uint16_t encap_len;

    l2_encapsulate_v6(reply, sizeof(reply), encap_buf, &encap_len,
                      MESH_ADDR_TOP_NODE, false);

    tpmesh_send_transparent(dest_mesh_id, encap_buf, encap_len);
}
```

---

## 5. 配置管理

### 5.1 Top Node 配置

```c
typedef struct {
    uint8_t  mac_addr[6];
    ip4_addr_t ip_addr;
    uint16_t mesh_id;           // 0xFFFE
    uint8_t  cell_id;
} top_config_t;

top_config_t g_top_config = {
    .mac_addr = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},
    .ip_addr  = IPADDR4_INIT_BYTES(192, 168, 10, 1),
    .mesh_id  = 0xFFFE,
    .cell_id  = 0,
};
```

### 5.2 DDC 配置

```c
typedef struct {
    uint8_t  mac_addr[6];       // 真实/固定 MAC
    ip4_addr_t ip_addr;         // 配置的 IP
    uint16_t mesh_id;           // 配置的 Mesh ID
    uint8_t  cell_id;
} ddc_config_t;

// 示例: DDC 1
ddc_config_t g_ddc_config = {
    .mac_addr = {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x01},
    .ip_addr  = IPADDR4_INIT_BYTES(192, 168, 10, 2),
    .mesh_id  = 0x0002,
    .cell_id  = 0,
};
```

---

## 6. 接口定义

```c
#ifndef TPMESH_L2_BRIDGE_H
#define TPMESH_L2_BRIDGE_H

#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include <stdint.h>
#include <stdbool.h>

/* ==================== 配置 ==================== */

#define TPMESH_MAX_NODE_ENTRIES    16
#define TPMESH_NODE_TIMEOUT_MS     90000
#define TPMESH_HEARTBEAT_MS        30000
#define TPMESH_FRAG_DELAY_MS       50
#define TPMESH_BROADCAST_RATE_MS   1000

/* ==================== Mesh 地址 ==================== */

#define MESH_ADDR_TOP_NODE         0xFFFE
#define MESH_ADDR_BROADCAST        0x0000

/* ==================== 注册帧类型 ==================== */

#define REG_FRAME_REGISTER         0x01
#define REG_FRAME_REGISTER_ACK     0x02
#define REG_FRAME_HEARTBEAT        0x03
#define REG_FRAME_HEARTBEAT_ACK    0x04

/* ==================== SCHC 规则 ==================== */

#define SCHC_RULE_NO_COMPRESS      0x00
#define SCHC_RULE_BACNET_IP        0x01
#define SCHC_RULE_IP_ONLY          0x02
#define SCHC_RULE_REGISTER         0x10

/* ==================== API ==================== */

// 初始化
err_t top_node_init_v6(struct netif *eth_netif, const top_config_t *config);
err_t ddc_node_init_v6(struct netif *mesh_netif, const ddc_config_t *config);

// 节点映射表
int node_table_register(const uint8_t *mac, const ip4_addr_t *ip, uint16_t mesh_id);
int node_table_learn(const uint8_t *mac, const ip4_addr_t *ip, uint16_t mesh_id);
uint16_t node_table_get_mesh_id_by_mac(const uint8_t *mac);
uint16_t node_table_get_mesh_id_by_ip(const ip4_addr_t *ip);
int node_table_get_mac_by_mesh_id(uint16_t mesh_id, uint8_t *mac);
int node_table_get_mac_by_ip(const ip4_addr_t *ip, uint8_t *mac);
int node_table_get_ip_by_mesh_id(uint16_t mesh_id, ip4_addr_t *ip);

// DDC 注册/心跳
int ddc_send_register(const ddc_config_t *config);
int ddc_send_heartbeat(const ddc_config_t *config);
void ddc_heartbeat_task(void *arg);

// 代理 ARP (Top Node 代答 BMS 的 ARP)
void send_proxy_arp_reply(const uint8_t *arp_request,
                          const uint8_t *target_mac,
                          const ip4_addr_t *target_ip);

// 代答 DDC 的 ARP (Top Node 代答 DDC 询问 BMS)
void send_arp_reply_to_ddc(uint16_t dest_mesh_id,
                           const uint8_t *arp_request,
                           const uint8_t *target_mac,
                           const ip4_addr_t *target_ip);

// DDC 广播帧处理
err_t ddc_linkoutput(struct netif *netif, struct pbuf *p);

// Top Node 转发 DDC 帧
void top_handle_data_frame(uint16_t src_mesh_id,
                           const uint8_t *data, uint16_t len);
void forward_broadcast_to_eth(const uint8_t *eth_frame, uint16_t len);
void forward_unicast_to_eth(const uint8_t *eth_frame, uint16_t len);

// L2 隧道 (带压缩, V0.6.2: 无 src_mesh_id)
int l2_encapsulate(const uint8_t *eth_frame, uint16_t eth_len,
                   uint8_t *out_data, uint16_t *out_len,
                   bool is_broadcast);
int l2_decapsulate(const uint8_t *mesh_data, uint16_t mesh_len,
                   uint8_t *out_frame, uint16_t *out_len,
                   uint16_t src_mesh_id,   // ★ 从 AT URC 获取 ★
                   uint16_t dst_mesh_id);

// 分片发送 (V0.6.2: 无 src_mesh_id)
int l2_fragment_and_send(uint16_t dest_mesh_id,
                         const uint8_t *eth_frame, uint16_t eth_len,
                         bool is_broadcast);

// AT+SEND 模式发送
int tpmesh_send_at(uint16_t dest_mesh_id,
                   const uint8_t *data, uint16_t len);

// AT URC 解析
int tpmesh_parse_nnmi(const char *urc,
                      uint16_t *src_mesh_id,
                      uint8_t *data, uint16_t *len);

// 广播限速
bool broadcast_rate_check(void);

// 任务
void top_mesh_rx_task(void *arg);
void ddc_mesh_rx_task(void *arg);
void node_table_maintenance_task(void *arg);

#endif // TPMESH_L2_BRIDGE_H
```

---

## 7. FreeRTOS/LwIP 任务架构

### 7.1 任务设计

```
┌─────────────────────────────────────────────────────────────────┐
│                      FreeRTOS 任务架构                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────┐  高优先级                                  │
│  │  UART RX Task   │  - 从 DMA 缓冲区搬运数据                   │
│  │  (uart_rx_task) │  - 解析帧边界                              │
│  │                 │  - 放入消息队列                            │
│  └────────┬────────┘                                            │
│           │ Queue                                               │
│           ▼                                                     │
│  ┌─────────────────┐  中优先级                                  │
│  │ Mesh Proto Task │  - 从队列取数据                            │
│  │ (mesh_proto_task)│ - 分片重组                                │
│  │                 │  - SCHC 解压                               │
│  │                 │  - 注册/心跳处理                           │
│  │                 │  - 转发到以太网                            │
│  └─────────────────┘                                            │
│                                                                 │
│  ┌─────────────────┐  中优先级                                  │
│  │ ETH Input Hook  │  - 拦截以太网流量                          │
│  │ (ethernetif_    │  - 判断是否需要桥接                        │
│  │  input)         │  - 桥接 or 交给 LwIP                       │
│  └─────────────────┘                                            │
│                                                                 │
│  ┌─────────────────┐  低优先级                                  │
│  │ Maintenance Task│  - 节点表超时检查                          │
│  │ (maint_task)    │  - 统计信息收集                            │
│  └─────────────────┘                                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 7.2 UART RX Task (高优先级)

```c
#define UART_RX_QUEUE_SIZE   10
#define UART_RX_BUF_SIZE     256

typedef struct {
    uint16_t src_mesh_id;   // 发送方 (透传模式下从 payload 解析)
    uint16_t len;
    uint8_t  data[UART_RX_BUF_SIZE];
} uart_rx_msg_t;

static QueueHandle_t g_uart_rx_queue;

/**
 * @brief UART 接收任务 (高优先级)
 *
 * 职责: 快速搬运数据，不做复杂处理
 */
void uart_rx_task(void *arg) {
    uint8_t rx_buf[UART_RX_BUF_SIZE];
    uint16_t rx_len = 0;
    uint32_t last_rx_tick = 0;

    while (1) {
        // 从 DMA 缓冲区读取 (非阻塞)
        int n = uart_read_nonblock(rx_buf + rx_len,
                                   UART_RX_BUF_SIZE - rx_len);
        if (n > 0) {
            rx_len += n;
            last_rx_tick = xTaskGetTickCount();
        }

        // 帧边界检测 (超时断帧: 10ms 无新数据视为一帧结束)
        if (rx_len > 0 &&
            (xTaskGetTickCount() - last_rx_tick) > pdMS_TO_TICKS(10)) {
            // 组装消息
            uart_rx_msg_t msg;
            msg.len = rx_len;
            memcpy(msg.data, rx_buf, rx_len);

            // 解析源 Mesh ID (从隧道头)
            // 透传模式接收不含地址，需从 payload 中 SRC_MESH 字段解析
            if (rx_len >= 4) {
                msg.src_mesh_id = ((uint16_t)rx_buf[2] << 8) | rx_buf[3];
            }

            // 放入队列 (不阻塞)
            xQueueSend(g_uart_rx_queue, &msg, 0);

            rx_len = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1));  // 1ms 轮询
    }
}
```

### 7.3 Mesh Protocol Task (中优先级)

```c
/**
 * @brief Mesh 协议处理任务
 *
 * 职责: 重组、解压、转发
 */
void mesh_proto_task(void *arg) {
    uart_rx_msg_t msg;

    while (1) {
        // 从队列取数据 (阻塞等待)
        if (xQueueReceive(g_uart_rx_queue, &msg, portMAX_DELAY) == pdTRUE) {
            // 检查帧类型
            uint8_t rule_id = msg.data[4];  // L2_HDR + FRAG + SRC_MESH 后

            if (rule_id == SCHC_RULE_REGISTER) {
                // 注册/心跳帧
                top_handle_reg_frame_v2(msg.src_mesh_id,
                                        msg.data + 5, msg.len - 5);
            } else {
                // 数据帧: 重组 + 解压 + 转发
                uint8_t *complete_data;
                uint16_t complete_len;

                int ret = reassemble_l2_packet(msg.src_mesh_id,
                                               msg.data, msg.len,
                                               &complete_data, &complete_len);
                if (ret == 1) {
                    // 重组完成
                    uint8_t eth_frame[1600];
                    uint16_t eth_len;

                    l2_decapsulate_v6(complete_data, complete_len,
                                      eth_frame, &eth_len,
                                      msg.src_mesh_id, MESH_ADDR_TOP_NODE);

                    // 转发到以太网
                    forward_to_eth(eth_frame, eth_len);
                }
            }
        }
    }
}
```

### 7.4 Ethernet Input Hook (驱动层)

```c
/**
 * @brief 以太网输入处理 (桥接 Hook)
 *
 * 调用位置: 在 LwIP ethernet_input 之前
 */
void ethernetif_input(void *arg) {
    struct netif *netif = (struct netif *)arg;
    struct pbuf *p;

    while (1) {
        // 等待接收信号
        if (xSemaphoreTake(g_eth_rx_semaphore, portMAX_DELAY) == pdTRUE) {
            // 从驱动获取 pbuf
            p = low_level_input(netif);
            if (p == NULL) continue;

            // ★ Hook: 检查是否需要桥接 ★
            bridge_action_t action = bridge_check_frame(p);

            switch (action) {
                case BRIDGE_TO_MESH:
                    // 转发到 Mesh
                    bridge_forward_to_mesh(p);
                    pbuf_free(p);  // 已转发，释放
                    break;

                case BRIDGE_PROXY_ARP:
                    // 代理 ARP 回复
                    bridge_send_proxy_arp(p);
                    pbuf_free(p);
                    break;

                case BRIDGE_LOCAL:
                default:
                    // 交给本机 LwIP 处理
                    if (netif->input(p, netif) != ERR_OK) {
                        pbuf_free(p);
                    }
                    break;
            }
        }
    }
}

/**
 * @brief 桥接检查
 */
typedef enum {
    BRIDGE_LOCAL,       // 交给本机 LwIP
    BRIDGE_TO_MESH,     // 转发到 Mesh
    BRIDGE_PROXY_ARP,   // 代理 ARP
    BRIDGE_DROP,        // 丢弃
} bridge_action_t;

bridge_action_t bridge_check_frame(struct pbuf *p) {
    struct eth_hdr *eth = (struct eth_hdr *)p->payload;

    // 检查目标 MAC
    if (is_broadcast_mac(eth->dest.addr)) {
        // 广播帧: 使用过滤器判断
        uint16_t dest_mesh;
        filter_action_t fa = filter_broadcast_frame_v6(
            p->payload, p->tot_len, &dest_mesh);

        if (fa == FILTER_DROP) return BRIDGE_DROP;
        if (fa == FILTER_LOCAL) return BRIDGE_LOCAL;

        // ARP 请求需要检查是否代答
        if (is_arp_request(p)) {
            ip4_addr_t target_ip;
            get_arp_target_ip(p, &target_ip);

            if (node_table_is_ddc_ip(&target_ip)) {
                return BRIDGE_PROXY_ARP;  // ★ 代答 ★
            }
        }

        // 其他广播 (如 BACnet Who-Is): 转发
        return BRIDGE_TO_MESH;
    }

    // 单播: 检查目标 MAC 是否在映射表
    if (node_table_is_ddc_mac(eth->dest.addr)) {
        return BRIDGE_TO_MESH;
    }

    // 目标是本机
    return BRIDGE_LOCAL;
}
```

### 7.5 任务创建

```c
/**
 * @brief 创建所有任务
 */
void create_bridge_tasks(void) {
    // 创建队列
    g_uart_rx_queue = xQueueCreate(UART_RX_QUEUE_SIZE, sizeof(uart_rx_msg_t));

    // UART 接收任务 (高优先级)
    xTaskCreate(uart_rx_task, "uart_rx", 512, NULL,
                configMAX_PRIORITIES - 1, NULL);

    // Mesh 协议任务 (中优先级)
    xTaskCreate(mesh_proto_task, "mesh_proto", 1024, NULL,
                configMAX_PRIORITIES - 2, NULL);

    // 以太网输入任务 (中优先级, 通常由 LwIP netif 驱动)
    xTaskCreate(ethernetif_input, "eth_input", 1024, &g_eth_netif,
                configMAX_PRIORITIES - 2, NULL);

    // 维护任务 (低优先级)
    xTaskCreate(node_table_maintenance_task, "maint", 256, NULL,
                tskIDLE_PRIORITY + 1, NULL);

    // DDC 心跳任务 (仅 DDC 节点)
    if (!g_config.is_top_node) {
        xTaskCreate(ddc_heartbeat_task, "ddc_hb", 512, &g_ddc_config,
                    configMAX_PRIORITIES - 2, NULL);
    }
}
```

---

## 8. 模组流控与 Busy 处理

### 8.1 透传模式发送流控

```c
/**
 * @brief 透传模式发送 (带流控)
 *
 * 透传模式下模组没有返回值，需要靠延时保证不溢出
 */
#define UART_BYTE_TIME_US    87   // 115200 波特率: 1字节约 87us
#define UART_TX_GUARD_MS     5    // 额外保护时间

int tpmesh_send_transparent_safe(uint16_t dest_mesh_id,
                                 const uint8_t *data, uint16_t len) {
    if (len > 198) return -1;

    uint8_t packet[200];
    packet[0] = (dest_mesh_id >> 8) & 0xFF;
    packet[1] = dest_mesh_id & 0xFF;
    memcpy(&packet[2], data, len);

    uint16_t total_len = len + 2;

    // 计算发送时间
    uint32_t tx_time_ms = (total_len * UART_BYTE_TIME_US) / 1000 + UART_TX_GUARD_MS;

    // 获取发送锁 (防止并发)
    if (xSemaphoreTake(g_uart_tx_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return -2;  // 获取锁超时
    }

    // 发送
    int ret = uart_write(packet, total_len);

    // 等待发送完成 + 模组处理时间
    vTaskDelay(pdMS_TO_TICKS(tx_time_ms));

    xSemaphoreGive(g_uart_tx_mutex);

    return ret;
}
```

### 8.2 利用模组状态引脚 (如有)

```c
/**
 * @brief 检查模组是否忙碌 (如果有 BUSY 引脚)
 */
#ifdef TPMESH_HAS_BUSY_PIN

#define BUSY_WAIT_TIMEOUT_MS  500

bool tpmesh_wait_ready(void) {
    uint32_t start = xTaskGetTickCount();

    while (gpio_read(TPMESH_BUSY_PIN) == GPIO_HIGH) {
        if ((xTaskGetTickCount() - start) > pdMS_TO_TICKS(BUSY_WAIT_TIMEOUT_MS)) {
            return false;  // 超时
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return true;
}

int tpmesh_send_transparent_with_busy(uint16_t dest, const uint8_t *data, uint16_t len) {
    // 等待模组就绪
    if (!tpmesh_wait_ready()) {
        return -1;
    }

    return tpmesh_send_transparent_safe(dest, data, len);
}

#endif
```

---

## 9. 注意事项

### 9.1 关键改进总结 (V0.5 → V0.6)

| 问题 | V0.5 | V0.6 解决方案 |
|------|------|---------------|
| 冷启动死锁 | 被动学习 | ★ DDC 主动注册 + 心跳 |
| ARP 延迟 | 转发到 Mesh | ★ Top Node 代答 ARP |
| 带宽浪费 | ASCII 编码 | ★ 透传模式 2 (二进制) |
| 头部开销 | 无压缩 | ★ SCHC 压缩 IP/UDP |
| Mesh 拥塞 | 无控制 | ★ 广播限速 |
| 模组溢出 | 无延时 | ★ 分片发送延时 |
| 节点离线 | 无检测 | ★ 心跳超时检测 |

### 9.2 部署检查清单

**硬件配置:**
- [ ] DDC 的 MAC/IP/Mesh ID 已配置
- [ ] Top Node 的 MAC/IP/Mesh ID 已配置
- [ ] 所有节点 Cell ID 一致
- [ ] UART 连接正确 (115200, 8N1)

**模组配置:**
- [ ] 透传模式 2 已启用 (AT+TRANS=2,0)
- [ ] Top Node: AT+LP=3 (TypeD, 常接收)
- [ ] DDC: AT+LP=2 (TypeC, 低功耗)

**软件任务:**
- [ ] UART RX 任务已启动
- [ ] Mesh Protocol 任务已启动
- [ ] Ethernet Input Hook 已配置
- [ ] DDC 心跳任务已启动
- [ ] 节点表维护任务已启动

### 9.3 首次调试建议

**金标准测试:** BMS Ping DDC

```
1. 确认 DDC 注册成功
   - Top Node 串口日志显示 "DDC registered: IP=10.2, Mesh=0x0002"
   - 发送了 GARP

2. 在 BMS 上执行:
   > arp -d 192.168.10.2          # 清除 ARP 缓存
   > ping 192.168.10.2

3. 预期结果:
   - ARP 响应 < 10ms (代理 ARP)
   - Ping 响应 < 500ms (1跳) / < 2s (4跳)

4. 抓包验证:
   - Wireshark 应看到 ARP Reply 源 MAC 是 DDC 的真实 MAC
   - ICMP 往返正常
```

### 9.4 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|------|----------|----------|
| DDC 无法注册 | 路由未建立 | 检查 +ROUTE:CREATE，增加等待时间 |
| ARP 无响应 | 映射表为空 | 检查 DDC 注册，查看 Top Node 日志 |
| Ping 超时 | Mesh 拥塞 | 减少广播频率，增加分片延时 |
| 数据丢失 | 模组溢出 | 增加发送延时，检查 BUSY 状态 |
| 单向通信 | SCHC 压缩错误 | 检查 IP/端口匹配规则 |

---

## 附录 A: 修订历史

| 版本 | 日期 | 修改内容 |
|------|------|----------|
| V0.1 | - | 初始设计 |
| V0.2 | 2026-02-03 | AT 指令模式 |
| V0.3 | 2026-02-03 | Proxy ARP 架构 |
| V0.4 | 2026-02-03 | L2 隧道桥接 + 广播过滤 |
| V0.5 | 2026-02-03 | MAC 与 Mesh ID 解耦；节点映射表 |
| V0.6 | 2026-02-03 | DDC 主动注册 + 重传状态机；Top Node 代答 ARP + GARP；透传模式2；SCHC 压缩；广播限速；FreeRTOS 任务架构；模组流控 |
| V0.6.1 | 2026-02-03 | 补充 BACnet Who-Is/I-Am 完整数据流程；DDC 广播帧处理；Top Node 转发 DDC 广播帧；Top Node 代答 DDC 的 ARP 请求 |

---

## 10. V0.8 Supplement (UART6 Takeover)

For deployments where another module configures TPMesh via UART6, architecture supplement is documented in:

- `App/x_protocol/UART6_TAKEOVER_MODE.md`

Key points:

- x_protocol can skip module init AT sequence in external-config mode.
- external module can request UART6 handover before configuring TPMesh.
- after external configuration, restart service/device to relaunch x_protocol cleanly.

---

## 11. Build Linker Permission Note (2026-02-07)

- Toolchain `arm-none-eabi-gcc 14.3.1` can emit:
  `ld.exe: ... has a LOAD segment with RWX permissions`.
- Root cause in this project: `.init_array/.fini_array` ended up in the same executable `FLASH` load segment and kept writable flags in ELF metadata.
- Mitigation: in linker script `XC8064/proj/gd32f5xx_flash.ld`, split FLASH load segments with explicit `PHDRS` (`text` vs `ro`) and keep `.preinit_array/.init_array/.fini_array` in the non-executable `ro` segment.
- Scope: this only changes ELF segment permission metadata (remove RWX warning); runtime memory layout and startup flow stay unchanged.

---

## 12. Bridge/ARP Workflow Guardrails (2026-02-08)

- Bridge input must not assume `pbuf` payload is contiguous.
- Ethernet frame parsing and SCHC compression must use a linearized buffer (`pbuf_copy_partial`) before reading headers/body.
- Proxy ARP must only reply for nodes that are both `known` and `online`.
- Unicast forwarding to mesh must only happen for nodes that are both `known` and `online`; offline targets are dropped to avoid stale-map blackholes.
- Implementation note: enforce these checks in `tpmesh_eth_input_hook()` as a single ingress policy layer, then call existing bridge APIs with contiguous frame copies.

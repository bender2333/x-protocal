# x_protocol 统一架构与集成设计文档

> 本文档是 `App/x_protocol` 的唯一架构设计入口（SSOT）。
> 原 `readme.md`、`INTEGRATION.md`、`UART6_TAKEOVER_MODE.md` 已并入本文档。

## 1. 目标与范围

### 1.1 目标

- 在不引入虚拟网卡的前提下，实现 BACnet/IP over TPMesh。
- Top 节点保留物理以太网能力，对 BMS 呈现标准以太网行为。
- Edge/DDC 节点通过 Mesh 与 Top 互通，完成注册、心跳、ARP、UDP 转发闭环。
- 关键路径可观测、可恢复、可验证。

### 1.2 范围

- 主功能目录：`App/x_protocol`。
- 允许的必要集成点：
  - `App/main.c`
  - `Third_Party/lwip/lwip-2.1.2/port/GD32F5xx/FreeRTOS/ethernetif.c`
- 不在本文档范围：无关业务逻辑、第三方库内部重构。

## 2. 角色架构

### 2.1 Top 节点（`TPMESH_MODE_TOP_NODE`）

- 外部面：物理 Ethernet（BMS 所在网段）。
- 无线面：TPMesh（UART6 + AT）。
- 角色职责：
  - ETH 入站分流（本地协议栈 / Mesh 转发 / Proxy ARP / Drop）。
  - DDC 注册管理与心跳 ACK。
  - Mesh 入站解封装并还原为标准以太网帧后从物理网卡发出。

### 2.2 Edge/DDC 节点（`TPMESH_MODE_DDC`）

- 外部面：保留 PHY ETH（管理面/MQTT/本地服务仍走以太网）。
- 内部面：复用本地 LwIP `netif`；`linkoutput` 采用选择性 hook：
  - Mesh 业务流量 -> Mesh。
  - 非 Mesh 业务流量 -> 回落 PHY 原始发送路径。
- 角色职责：
  - 发现 Top 路由后发注册，在线后发心跳。
  - 本地协议栈 TX ->（Mesh 或 PHY）；Mesh RX -> 注入本地协议栈 `netif->input`。

### 2.3 运行状态（Runtime State）

- `RUN`：Top/Edge Mesh 数据面开启。
- `CONFIG`：Mesh 数据面停用并释放 UART6 给配置线程；PHY 本地协议栈保持可用（用于 MQTT 配置链路）。

## 3. 模块分层与职责

| 层 | 文件 | 主要职责 |
|---|---|---|
| 启动与模式装配 | `App/x_protocol/tpmesh_init.c` | Top/Edge 初始化入口、任务创建、以太网 hook 分发、UART6 接管入口 |
| 桥接核心 | `App/x_protocol/tpmesh_bridge.c` | 桥接策略、分片发送/重组、注册心跳状态机、ARP 代理、Mesh 数据处理 |
| AT 协议层 | `App/x_protocol/tpmesh_at.c` | AT 命令收发、URC 解析、并发互斥、回调分发 |
| UART 驱动层 | `App/x_protocol/tpmesh_uart.c` | UART6 硬件初始化、TX、RX 环形缓冲、中断处理 |
| SCHC 层 | `App/x_protocol/tpmesh_schc.c` | SCHC 压缩/解压、规则选择、边界检查 |
| 节点映射表 | `App/x_protocol/node_table.c` | mesh/mac/ip 映射、在线状态、超时维护 |
| 调试输出 | `App/x_protocol/tpmesh_debug.c` | 调试日志输出 |

## 4. 启动与任务生命周期

### 4.1 启动阶段（`main.c`）

1. `EnetInit(false)`。
2. 按 `TPMESH_MODE` 选择：
   - Top：`tpmesh_module_init_top(ethernetif_get_netif())`
   - Edge：`tpmesh_module_init_ddc()`
3. 若已初始化：`tpmesh_create_tasks()`。
4. `vTaskStartScheduler()`。

### 4.2 运行阶段任务

- `tpmesh_at_rx_task`：解析 UART6 输入并分发 `+NNMI/+ROUTE`。
- `tpmesh_bridge_task`：处理 Mesh 队列、协议桥接、节点维护。
- `ddc_heartbeat_task`：仅 Edge 启动，负责注册重试与心跳。

## 5. 核心数据流

### 5.1 Edge 上线到在线闭环（注册 + 心跳）

1. Edge 收到 `+ROUTE: CREATE ... TOP`。
2. 状态进入 `REGISTERING`，发送注册帧（`SCHC_RULE_REGISTER`）。
3. Top 收到后写入 node table，回复 `REGISTER_ACK`。
4. Edge 收到 ACK -> `ONLINE`。
5. Edge 定时发送心跳，Top 回复 `HEARTBEAT_ACK`。
6. Top 对注册/心跳都执行表项刷新，支持 Top 重启后通过心跳恢复映射。

### 5.2 Top 入站（ETH -> Mesh）

入口：`ethernetif_input()` 先本地 `netif->input`，再镜像副本到 `tpmesh_eth_input_hook()`。

策略动作：

- `BRIDGE_LOCAL`：不转发 Mesh（本地栈已先处理）。
- `BRIDGE_TO_MESH`：走 `tpmesh_bridge_forward_to_mesh()`（镜像转发，不阻断本地）。
- `BRIDGE_PROXY_ARP`：走 `tpmesh_bridge_send_proxy_arp()`。
- `BRIDGE_DROP`：仅丢弃镜像方向，不影响本地栈。

注意：Top 入站采用 local-first tap 模式，避免新增本地业务被桥接策略误拦截。

### 5.3 Top 入站（Mesh -> ETH）

1. `tpmesh_at` 收到 `+NNMI`，进入 bridge 消息队列。
2. `tpmesh_bridge_task` 调 `tpmesh_bridge_handle_mesh_data()`。
3. 分片重组 -> SCHC 解压 -> 以太网帧。
4. Top 通过物理网卡 `linkoutput` 发往 BMS 网络。

### 5.4 Edge 出站（本地协议栈 -> Mesh）

1. Edge 初始化时调用 `tpmesh_bridge_attach_ddc_netif(netif_default)`。
2. LwIP TX 经 `netif->linkoutput` 进入选择性 hook。
3. 命中 Mesh 业务规则（如 BACnet UDP / node ARP）时，bridge 封装 tunnel + 分片后通过 `AT+SEND` 发给 Top。
4. 未命中 Mesh 规则时，回落到 PHY 原始 `linkoutput`（保障 MQTT/管理流量）。

### 5.5 Edge 入站（Mesh -> 本地协议栈）

1. 分片重组 + SCHC 解压。
2. 构造 `pbuf`。
3. 调用 `netif_default->input(p, netif_default)` 注入本地栈。

### 5.6 关键业务场景流（补充）

#### 5.6.1 BMS -> DDC 单播 UDP

1. BMS 发送单播以太网帧到 Top（目标 MAC 为 DDC MAC）。
2. Top `tpmesh_bridge_check()` 判定 `BRIDGE_TO_MESH`。
3. Top 进行隧道封装 + SCHC 压缩（可压缩时）并分片发送到目标 DDC mesh_id。
4. DDC 重组并解压后注入本地 `netif->input`。

#### 5.6.2 BMS Who-Is 广播 -> DDC I-Am 广播

1. BMS 发送 BACnet Who-Is 广播帧。
2. Top 本地协议栈先收到 Who-Is，可按需直接响应。
3. Top 判定广播可转发并执行限速。
4. Top 压缩封装后广播到 Mesh。
5. DDC 解压后本地 BACnet 栈收到 Who-Is 并产生 I-Am 广播。
6. DDC 将 I-Am 通过 Mesh 回传 Top，Top 解压并从物理网卡广播发出。

#### 5.6.3 ARP 代理流程（BMS 发现 DDC）

1. BMS 对 DDC IP 发送 ARP Request。
2. Top 查询 node table。
3. 若目标节点 `known + online`，Top 直接发送 Proxy ARP Reply（源 MAC=DDC MAC）。
4. 若未知/离线，Top 不代理并丢弃（等待注册/心跳恢复）。

## 6. 报文与协议策略

### 6.1 隧道头与分片契约（统一格式）

每个分片都使用统一格式：

`[L2_HDR][FRAG_HDR][RULE_ID][payload...]`

- `FRAG_HDR`：bit7=last, bit[6:0]=seq。
- `seq=0`：首片，含完整隧道头 + 首段 payload。
- `seq>0`：后续片仍保留隧道头，重组时仅追加 payload。
- `len > TPMESH_MTU` 的 UART 上报帧直接丢弃，不做静默截断。

### 6.2 SCHC 策略

- 规则：
  - `SCHC_RULE_BACNET_IP` (`0x01`)
  - `SCHC_RULE_IP_ONLY` (`0x02`)
  - `SCHC_RULE_NO_COMPRESS` (`0x00`)
  - `SCHC_RULE_REGISTER` (`0x10`)
- 兼容策略：
  - 非 UDP 或 IPv4 IHL != 20 时回退 `NO_COMPRESS`。
  - 解压时强制容量检查，超界直接失败。

### 6.3 ARP 策略

- Top 对目标为已知且在线 DDC 的 ARP 请求执行 Proxy ARP。
- 未知或离线条目不代理，避免错误响应与黑洞。

### 6.4 AT+SEND / +NNMI 格式

- 发送命令：`AT+SEND=<DEST>,<LEN>,<HEX_DATA>,<TYPE>`
- 接收 URC（优先）：`+NNMI:<SRC>,<DEST>,<RSSI>,<LEN>,<HEX_DATA>`
- 接收 URC（兼容）：`+NNMI:<SRC>,<LEN>,<HEX_DATA>`
- 约束：`LEN <= TPMESH_MTU`，超出直接拒绝或丢弃。

### 6.5 注册/心跳控制帧格式（`SCHC_RULE_REGISTER`）

- 载荷结构：
  - `frame_type:1`
  - `mac:6`
  - `ip:4`（网络序）
  - `mesh_id:2`（当前实现按 MCU 原生字节序，GD32 为小端）
  - `crc16:2`（当前实现按 MCU 原生字节序，GD32 为小端）
- 控制类型：
  - `0x01` Register
  - `0x02` Register ACK
  - `0x03` Heartbeat
  - `0x04` Heartbeat ACK

### 6.6 隧道数据帧格式（按 Rule）

- 通用头：`[L2_HDR:1][FRAG_HDR:1][RULE_ID:1][PAYLOAD:N]`
- `RULE_ID=0x00` (`NO_COMPRESS`)：
  - `PAYLOAD=[SRC_MAC:6][DST_MAC:6][ETHERTYPE+L3+L4+DATA]`
- `RULE_ID=0x01` (`BACNET_IP`)：
  - `PAYLOAD=[SRC_MAC:6][UDP_PAYLOAD]`
  - IPv4/UDP 头在解压时重建。
- `RULE_ID=0x02` (`IP_ONLY`)：
  - `PAYLOAD=[SRC_MAC:6][IP_PAYLOAD]`
  - 仅在满足规则时使用，否则回退 `NO_COMPRESS`。

### 6.7 Mesh 帧格式定义（字节级）

本节定义 `AT+SEND` / `+NNMI` 中 `HEX_DATA` 的字节格式（即 Mesh 隧道帧）。

- `SRC_MESH_ID` 不在 `HEX_DATA` 内，来自 `+NNMI` 头字段。
- `DEST_MESH_ID` 不在 `HEX_DATA` 内，来自 `AT+SEND=<DEST,...>` 的 `DEST`。
- 统一头长度固定为 `3` 字节（`TPMESH_TUNNEL_HDR_LEN`）。

通用格式：

`[L2_HDR:1][FRAG_HDR:1][RULE_ID:1][PAYLOAD:N]`

字段定义（按字节偏移）：

| 偏移 | 字段 | 长度 | 定义 |
|---:|---|---:|---|
| 0 | `L2_HDR` | 1 | bit7=`1` 表示广播，bit[6:0] 保留（当前发送为 0） |
| 1 | `FRAG_HDR` | 1 | bit7=`last`，bit[6:0]=`seq` |
| 2 | `RULE_ID` | 1 | `0x00/0x01/0x02/0x10` |
| 3.. | `PAYLOAD` | N | 由 `RULE_ID` 决定 |

分片约束：

- `seq=0` 为首片；`seq>0` 为后续片。
- 所有分片都保留完整 3 字节头（不是短头格式）。
- 重组要求 `seq` 连续，且后续片 `L2_HDR/RULE_ID` 与首片一致，否则丢弃当前会话。

`RULE_ID=0x00`（`NO_COMPRESS`）载荷：

| 偏移（相对整帧） | 字段 | 长度 |
|---:|---|---:|
| 3 | `SRC_MAC` | 6 |
| 9 | `DST_MAC` | 6 |
| 15 | `ETH_PAYLOAD_FROM_ETHERTYPE` | 可变 |

说明：

- `ETH_PAYLOAD_FROM_ETHERTYPE` 从以太帧 EtherType 开始（即原始帧 `eth[12...]`）。
- 解压后可无损还原原始以太网帧。

`RULE_ID=0x01`（`BACNET_IP`）载荷：

| 偏移（相对整帧） | 字段 | 长度 |
|---:|---|---:|
| 3 | `SRC_MAC` | 6 |
| 9 | `UDP_PAYLOAD` | 可变 |

说明：

- 该规则用于 BACnet/IP（UDP 47808）。
- 解压时重建 Ethernet + IPv4 + UDP 头（UDP 源/目的端口均为 47808）。

`RULE_ID=0x02`（`IP_ONLY`）载荷：

| 偏移（相对整帧） | 字段 | 长度 |
|---:|---|---:|
| 3 | `SRC_MAC` | 6 |
| 9 | `IP_PAYLOAD` | 可变 |

说明：

- 该规则仅用于 UDP 且目的端口非 47808 的 IPv4 报文。
- 解压时重建 Ethernet + IPv4 头，`IP proto` 按当前实现固定为 UDP。

`RULE_ID=0x10`（`REGISTER`）载荷：

`[frame_type:1][mac:6][ip:4][mesh_id:2][crc16:2]`

按整帧偏移：

| 偏移 | 字段 | 长度 |
|---:|---|---:|
| 3 | `frame_type` | 1 |
| 4 | `mac` | 6 |
| 10 | `ip` | 4 |
| 14 | `mesh_id` | 2 |
| 16 | `crc16` | 2 |

常见单片总长度：`3 + 15 = 18` 字节（`FRAG_HDR=0x80`，`seq=0,last=1`）。

### 6.8 报文实例解析（典型帧）

#### 6.8.1 Edge -> Top 心跳（`+NNMI`）

原始报文：

- `+NNMI:0002,FFFE,-71,18,0080100318A788030030C0A80A0B02009F2D`

链路层字段：

- `SRC=0x0002`（来自 `+NNMI` 头，不在 `HEX_DATA`）
- `DEST=0xFFFE`（来自 `+NNMI` 头，不在 `HEX_DATA`）
- `RSSI=-71`
- `LEN=18`

`HEX_DATA` 按字节拆分：

`00 80 10 03 18 A7 88 03 00 30 C0 A8 0A 0B 02 00 9F 2D`

| 偏移 | 字节 | 含义 |
|---:|---|---|
| 0 | `00` | `L2_HDR`，单播 |
| 1 | `80` | `FRAG_HDR`，`last=1`，`seq=0` |
| 2 | `10` | `RULE_ID=SCHC_RULE_REGISTER` |
| 3 | `03` | `frame_type=HEARTBEAT` |
| 4..9 | `18 A7 88 03 00 30` | `mac=18:A7:88:03:00:30` |
| 10..13 | `C0 A8 0A 0B` | `ip=192.168.10.11`（网络序） |
| 14..15 | `02 00` | `mesh_id=0x0002`（当前实现按小端存取） |
| 16..17 | `9F 2D` | `crc16=0x2D9F`（当前实现按小端存取） |

结论：

- 该帧是 Edge(`0x0002`) 发给 Top(`0xFFFE`) 的单片心跳帧。

#### 6.8.2 Top -> Edge 心跳 ACK（`AT+SEND`）

原始命令：

- `AT+SEND=0002,18,00801004006BA0000010C0A80A0AFEFF922B,0`

链路层字段：

- `DEST=0x0002`（来自 `AT+SEND` 目的地址）
- `LEN=18`

`HEX_DATA` 按字节拆分：

`00 80 10 04 00 6B A0 00 00 10 C0 A8 0A 0A FE FF 92 2B`

| 偏移 | 字节 | 含义 |
|---:|---|---|
| 0 | `00` | `L2_HDR`，单播 |
| 1 | `80` | `FRAG_HDR`，`last=1`，`seq=0` |
| 2 | `10` | `RULE_ID=SCHC_RULE_REGISTER` |
| 3 | `04` | `frame_type=HEARTBEAT_ACK` |
| 4..9 | `00 6B A0 00 00 10` | `mac=00:6B:A0:00:00:10`（Top） |
| 10..13 | `C0 A8 0A 0A` | `ip=192.168.10.10`（Top） |
| 14..15 | `FE FF` | `mesh_id=0xFFFE`（当前实现按小端存取） |
| 16..17 | `92 2B` | `crc16=0x2B92`（当前实现按小端存取） |

结论：

- 该帧是 Top 对 Edge 心跳的 ACK，内容与 Top 身份（MAC/IP/MeshID）一致。

#### 6.8.3 Top -> Mesh 广播业务帧（`AT+SEND`, `RULE_ID=0x01`）

原始命令：

- `AT+SEND=0000,27,808001C853092E31D9810B000C0120FFFF00FF1008000000000000,0`

链路层字段：

- `DEST=0x0000`（Mesh 广播地址）
- `LEN=27`

`HEX_DATA` 按字节拆分：

`80 80 01 C8 53 09 2E 31 D9 81 0B 00 0C 01 20 FF FF 00 FF 10 08 00 00 00 00 00 00`

| 偏移 | 字节 | 含义 |
|---:|---|---|
| 0 | `80` | `L2_HDR`，广播（bit7=1） |
| 1 | `80` | `FRAG_HDR`，`last=1`，`seq=0` |
| 2 | `01` | `RULE_ID=SCHC_RULE_BACNET_IP` |
| 3..8 | `C8 53 09 2E 31 D9` | `SRC_MAC=C8:53:09:2E:31:D9` |
| 9..26 | `81 0B 00 0C 01 20 FF FF 00 FF 10 08 00 00 00 00 00 00` | `UDP_PAYLOAD`（18 字节） |

结论：

- 该帧是 Top 侧发往 Mesh 全网的单片广播业务帧。
- 因 `RULE_ID=0x01`，该帧未携带 IP/UDP 头，接收端解压时会按 BACnet/IP 规则重建 IPv4/UDP 头后再入栈。

## 7. UART6 接管模型（最终）

### 7.1 设计决策

采用“单向接管 + 重启边界”模型：

1. 运行时切换到 `CONFIG`（`tpmesh_set_run_state(TPMESH_RUN_STATE_CONFIG)`）。
2. 调用 `tpmesh_request_uart6_takeover()`。
3. 接口在 busy 场景下可阻塞，直到 UART6 安全释放。
4. 外部配置工具接管 UART6 进行配置（PHY/MQTT 通道保持可用）。
5. 终止当前模块/进程并重启。
6. x_protocol 按正常启动路径重新初始化并进入 `RUN`。

### 7.2 明确约束

- 不提供运行中 reclaim 接口。
- 不维护 takeover 持久状态。
- 运行时边界清晰：接管后不再要求原模块继续服务。

## 8. 关键参数与建议值

| 参数 | 默认值 | 位置 | 说明 |
|---|---:|---|---|
| `TPMESH_MODE` | `2` | `tpmesh_init.h` | 0禁用/1Top/2Edge |
| `TPMESH_MTU` | `200` | `tpmesh_bridge.h` | AT+SEND 载荷上限 |
| `TPMESH_HEARTBEAT_MS` | `30000` | `tpmesh_bridge.h` | Edge 心跳周期 |
| `TPMESH_HEARTBEAT_ACK_TIMEOUT_MS` | `10000` | `tpmesh_bridge.h` | ACK 等待超时 |
| `TPMESH_HEARTBEAT_MISS_MAX` | `3` | `tpmesh_bridge.h` | 连续丢失阈值 |
| `TPMESH_REGISTER_RETRY_MS` | `5000` | `tpmesh_bridge.h` | 注册重试间隔 |
| `TPMESH_REGISTER_MAX_RETRIES` | `10` | `tpmesh_bridge.h` | 注册最大重试 |
| `TPMESH_BROADCAST_RATE_MS` | `1000` | `tpmesh_bridge.h` | 广播限速窗口 |
| `TPMESH_BROADCAST_BURST_MAX` | `3` | `tpmesh_bridge.h` | 窗口内突发上限 |

## 9. API 契约（按模块）

### 9.1 初始化层 `tpmesh_init.h`

- `int tpmesh_module_init_top(struct netif *eth_netif)`
- `int tpmesh_module_init_ddc(void)`
- `void tpmesh_create_tasks(void)`
- `bool tpmesh_eth_input_hook(struct netif *netif, struct pbuf *p)`
- `tpmesh_role_t tpmesh_get_role(void)`
- `tpmesh_run_state_t tpmesh_get_run_state(void)`
- `int tpmesh_set_run_state(tpmesh_run_state_t state)`
- `bool tpmesh_data_plane_enabled(void)`
- `bool tpmesh_eth_input_tap_enabled(void)`
- `bool tpmesh_is_initialized(void)`
- `int tpmesh_request_uart6_takeover(void)`
- `void tpmesh_print_status(void)`

### 9.2 桥接层 `tpmesh_bridge.h`

- 初始化：`tpmesh_top_init` / `tpmesh_ddc_init`
- 转发：`tpmesh_bridge_check` / `tpmesh_bridge_forward_to_mesh` / `tpmesh_bridge_send_proxy_arp`
- DDC hook：`tpmesh_bridge_attach_ddc_netif`
- Mesh 入站：`tpmesh_bridge_handle_mesh_data` / `tpmesh_bridge_task`
- 注册心跳：`ddc_send_register` / `ddc_send_heartbeat` / `ddc_heartbeat_task`

### 9.3 AT/UART 层

AT API（`tpmesh_at.h`）：

- `int tpmesh_at_init(void)`
- `int tpmesh_at_release_uart6(void)`
- `bool tpmesh_at_is_uart6_active(void)`
- `at_resp_t tpmesh_at_cmd(...)`
- `at_resp_t tpmesh_at_send_data(...)`
- `void tpmesh_at_rx_task(void *arg)`

UART API（`tpmesh_uart.h`）：

- `int tpmesh_uart6_init(void)`
- `void tpmesh_uart6_deinit(void)`
- `int tpmesh_uart6_send(...)`
- `int tpmesh_uart6_getc(...)`
- `void tpmesh_uart6_irq_handler(void)`

## 10. 代码实现要点（对照）

- 模式装配：`App/main.c` + `App/x_protocol/tpmesh_init.c`
- ETH hook 注入点：`Third_Party/lwip/.../ethernetif.c`
- 分片发送/重组：`App/x_protocol/tpmesh_bridge.c`
- SCHC 安全边界：`App/x_protocol/tpmesh_schc.c`
- 节点在线维护：`App/x_protocol/node_table.c`
- UART6 接管与释放：`App/x_protocol/tpmesh_init.c` + `App/x_protocol/tpmesh_at.c`

## 11. 集成步骤（最小闭环）

1. 确认 `main.c` 调用顺序：`EnetInit` -> `tpmesh_module_init_xxx` -> `tpmesh_create_tasks`。
2. 确认 `ethernetif_input` 为 local-first，并在本地入栈后镜像调用 `tpmesh_eth_input_hook`。
3. 设置 `TPMESH_MODE`（Top/Edge）。
4. 编译并观察启动日志。
5. 进行注册、心跳、ARP、UDP 双向验证。

## 12. 验证清单（可复现）

### 12.1 Top + Edge 联调

1. Edge 上电，等待路由建立。
2. 观察 Top 日志：注册成功、发送 ACK/GARP。
3. 清 BMS ARP 缓存后 ping/UDP 到 Edge。
4. 验证 Top 代理 ARP 响应与 UDP 转发。

### 12.2 大包分片验证

1. 构造 `>TPMESH_MTU` 负载。
2. 验证分片发送连续，重组后长度与内容一致。
3. 验证异常短帧/超长帧被丢弃且不污染后续会话。

### 12.3 UART6 接管验证

1. 调用 `tpmesh_request_uart6_takeover()`。
2. 验证返回成功后外部工具可稳定配置。
3. 终止并重启模块后，验证 x_protocol 恢复正常运行。

## 13. 维护规则

- 架构变更先更新本文档，再改代码。
- 任何新增接口需在对应头文件声明，并在本文档 API 契约补充。
- 旧文档仅保留跳转入口，避免多处事实源漂移。

## 14. 变更历史（合并）

| 版本 | 日期 | 关键变更 |
|---|---|---|
| V0.3 | 2026-02-03 | 引入 Proxy ARP 架构 |
| V0.4 | 2026-02-03 | L2 隧道桥接 + 广播过滤 |
| V0.5 | 2026-02-03 | MAC 与 Mesh ID 解耦，节点映射表 |
| V0.6 | 2026-02-03 | DDC 主动注册 + 重传状态机；Top 代答 ARP + GARP；SCHC；广播限速 |
| V0.6.1 | 2026-02-03 | 补充 Who-Is/I-Am 流程；DDC 广播处理；Top 转发 DDC 广播 |
| V0.7 | 2026-02-07 | AT+SEND/NNMI 对齐；任务与数据链路闭环对齐实现 |
| V0.7.1 | 2026-02-08 | Bridge/ARP 工作流下沉与安全收敛 |
| V0.8 | 2026-02-08 | UART6 takeover 模型纳入主架构 |
| V0.8.1 | 2026-02-08 | SCHC 边界检查、IHL 回退、hook fail-closed |
| V0.8.2 | 2026-02-08 | 分片契约统一、Top/Edge 数据流闭环修复 |
| V0.8.3 | 2026-02-08 | UART6 收敛为单向接管 + 重启模型 |
| V0.9 | 2026-02-12 | 引入 role+run_state 运行时模型；Top 入站改 local-first tap；Edge 出站改选择性 Mesh + PHY fallback |
| V0.9.1 | 2026-02-12 | 补充 Mesh 帧字节级格式定义（通用头/各 Rule 载荷/偏移） |
| V0.9.2 | 2026-02-12 | 补充真实报文样例逐字节解析（NNMI 心跳与 AT+SEND 心跳 ACK） |
| V0.9.3 | 2026-02-12 | 补充广播业务帧样例解析（AT+SEND DEST=0x0000, RULE_ID=0x01） |

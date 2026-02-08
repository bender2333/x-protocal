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

- 外部面：不发送 PHY ETH（业务数据走 Mesh）。
- 内部面：复用本地 LwIP `netif`；通过 hook `netif->linkoutput` 把 TX 导向 Mesh。
- 角色职责：
  - 发现 Top 路由后发注册，在线后发心跳。
  - 本地协议栈 TX -> Mesh；Mesh RX -> 注入本地协议栈 `netif->input`。

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

入口：`ethernetif_input()` -> `tpmesh_eth_input_hook()` -> `tpmesh_bridge_check()`。

策略动作：

- `BRIDGE_LOCAL`：交给 `netif->input`。
- `BRIDGE_TO_MESH`：走 `tpmesh_bridge_forward_to_mesh()`。
- `BRIDGE_PROXY_ARP`：走 `tpmesh_bridge_send_proxy_arp()`。
- `BRIDGE_DROP`：直接消费。

注意：桥接动作采用 fail-closed；一旦判定为 bridge-owned，不回落本地协议栈。

### 5.3 Top 入站（Mesh -> ETH）

1. `tpmesh_at` 收到 `+NNMI`，进入 bridge 消息队列。
2. `tpmesh_bridge_task` 调 `tpmesh_bridge_handle_mesh_data()`。
3. 分片重组 -> SCHC 解压 -> 以太网帧。
4. Top 通过物理网卡 `linkoutput` 发往 BMS 网络。

### 5.4 Edge 出站（本地协议栈 -> Mesh）

1. Edge 初始化时调用 `tpmesh_bridge_attach_ddc_netif(netif_default)`。
2. LwIP TX 经 `netif->linkoutput` 被 hook 到 bridge。
3. bridge 封装 tunnel + 分片后通过 `AT+SEND` 发给 Top。

### 5.5 Edge 入站（Mesh -> 本地协议栈）

1. 分片重组 + SCHC 解压。
2. 构造 `pbuf`。
3. 调用 `netif_default->input(p, netif_default)` 注入本地栈。

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

## 7. UART6 接管模型（最终）

### 7.1 设计决策

采用“单向接管 + 重启边界”模型：

1. 调用 `tpmesh_request_uart6_takeover()`。
2. 接口在 busy 场景下可阻塞，直到 UART6 安全释放。
3. 外部配置工具接管 UART6 进行配置。
4. 终止当前模块/进程并重启。
5. x_protocol 按正常启动路径重新初始化。

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
2. 确认 `ethernetif_input` 调用 `tpmesh_eth_input_hook`。
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

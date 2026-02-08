# TPMesh 架构与集成设计（README）

本文档保留关键设计细节（数据流、帧格式、变更历史）。
完整统一文档请同时参考：`App/x_protocol/ARCHITECTURE.md`。

## 1. 设计目标

- Top 节点保留物理 Ethernet，对 BMS 呈现标准以太网行为。
- Edge/DDC 节点通过 Mesh 与 Top 互通，不新增虚拟网卡。
- 闭环覆盖：注册、心跳、ARP、SCHC、UDP 双向转发。

## 2. 核心数据流（关键场景）

### 2.1 DDC 注册与心跳闭环

1. Edge 收到 `+ROUTE: CREATE ... TOP` 后进入 `REGISTERING`。
2. Edge 发送 `SCHC_RULE_REGISTER` 注册帧。
3. Top 写入 node table 并回复 `REGISTER_ACK`。
4. Edge 切换 `ONLINE`，定时发送心跳。
5. Top 回复 `HEARTBEAT_ACK`，并对注册/心跳统一刷新表项。

### 2.2 Top 入站分流（ETH -> Mesh/Local）

- 入口：`ethernetif_input()` -> `tpmesh_eth_input_hook()` -> `tpmesh_bridge_check()`。
- 动作：
  - `BRIDGE_LOCAL`：交给本地 `netif->input`。
  - `BRIDGE_TO_MESH`：封装/压缩后发 Mesh。
  - `BRIDGE_PROXY_ARP`：Top 代答 ARP。
  - `BRIDGE_DROP`：消费丢弃。

### 2.3 Top 下行（Mesh -> ETH）

1. `tpmesh_at` 解析 `+NNMI`，入 bridge 队列。
2. bridge 任务完成分片重组 + SCHC 解压。
3. Top 使用物理网卡 `linkoutput` 发出标准以太网帧。

### 2.4 Edge 出站（LwIP -> Mesh）

1. Edge 初始化时 hook `netif->linkoutput` 到 bridge。
2. 本地协议栈发出的帧通过 bridge 封装后经 `AT+SEND` 发往 Top。

### 2.5 ARP 代理流程

1. BMS 对 DDC IP 发 ARP Request。
2. Top 查 node table。
3. 仅当目标 `known + online` 时发送 Proxy ARP Reply（源 MAC=DDC MAC）。

## 3. 帧格式与协议细节（关键）

### 3.1 AT 与 URC

- 发送：`AT+SEND=<DEST>,<LEN>,<HEX_DATA>,<TYPE>`
- 接收（优先）：`+NNMI:<SRC>,<DEST>,<RSSI>,<LEN>,<HEX_DATA>`
- 接收（兼容）：`+NNMI:<SRC>,<LEN>,<HEX_DATA>`

### 3.2 隧道统一格式（分片契约）

每个分片都采用同一格式：

`[L2_HDR][FRAG_HDR][RULE_ID][payload...]`

- `FRAG_HDR`: bit7=`last`, bit[6:0]=`seq`
- `seq=0` 首片包含完整隧道头与首段 payload
- `seq>0` 后续片仍保留隧道头，重组时仅追加 payload
- 超过 `TPMESH_MTU` 的上报帧直接丢弃，不做静默截断

### 3.3 注册/心跳控制帧（`SCHC_RULE_REGISTER`）

`[frame_type:1][mac:6][ip:4][mesh_id:2][crc16:2]`

- `0x01` Register
- `0x02` Register ACK
- `0x03` Heartbeat
- `0x04` Heartbeat ACK

### 3.4 SCHC 规则与载荷

- `0x00` `NO_COMPRESS`:
  - `payload=[SRC_MAC:6][DST_MAC:6][ETHERTYPE+L3+L4+DATA]`
- `0x01` `BACNET_IP`:
  - `payload=[SRC_MAC:6][UDP_PAYLOAD]`
- `0x02` `IP_ONLY`:
  - `payload=[SRC_MAC:6][IP_PAYLOAD]`

兼容策略：

- 非 UDP 或 IPv4 IHL != 20 时回退 `NO_COMPRESS`。
- 解压严格做输出容量检查，越界立即失败。

## 4. UART6 接管模型（最终）

采用单向接管 + 重启边界：

1. 调 `tpmesh_request_uart6_takeover()`。
2. 接口可阻塞直到 UART6 安全释放。
3. 外部配置工具接管 UART6。
4. 完成后终止并重启当前模块/进程。
5. x_protocol 从正常初始化路径重新启动。

说明：不提供运行中 reclaim，不维护 takeover 持久状态。

## 5. 变更历史（保留）

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

## 6. 完整文档入口

- `App/x_protocol/ARCHITECTURE.md`

建议：架构改动先更新 `ARCHITECTURE.md`，再同步 README 关键摘要。

# TPMesh 桥接模块集成指南

## 文件结构

```
App/x_protocol/
├── tpmesh_bridge.h     - 主头文件 (API定义)
├── tpmesh_bridge.c     - 桥接主逻辑
├── tpmesh_at.h         - AT命令处理头文件
├── tpmesh_at.c         - UART6 AT命令收发
├── tpmesh_uart.h       - UART6 DMA驱动头文件
├── tpmesh_uart.c       - UART6 DMA驱动实现
├── tpmesh_schc.h       - SCHC压缩头文件
├── tpmesh_schc.c       - SCHC压缩/解压
├── node_table.h        - 节点映射表头文件
├── node_table.c        - 节点映射表管理
├── tpmesh_init.h       - 初始化接口
├── tpmesh_init.c       - 初始化实现
├── tpmesh_debug.h      - 调试输出接口 (printf 重定向)
├── tpmesh_debug.c      - 调试输出实现 (USART2)
├── readme.md           - 设计文档 V0.6.2
└── INTEGRATION.md      - 本文件
```

---

## 集成步骤

### 1. 修改 ethernetif.c ✅ 已完成

**文件位置:** `Third_Party/lwip/lwip-2.1.2/port/GD32F5xx/FreeRTOS/ethernetif.c`

**已添加头文件:**

```c
#include "x_protocol/tpmesh_init.h"
```

**已修改 ethernetif_input 函数:**

```c
void ethernetif_input(void *pvParameters)
{
    struct pbuf *p;
    SYS_ARCH_DECL_PROTECT(sr);

    SYS_ARCH_PROTECT(sr);
    p = low_level_input(low_netif);
    SYS_ARCH_UNPROTECT(sr);

    if (p != NULL) {
        /* TPMesh 桥接钩子: 检查是否需要转发到 Mesh */
        if (tpmesh_eth_input_hook(low_netif, p)) {
            /* 帧已被 TPMesh 处理 (转发到 Mesh 或代理 ARP) */
            pbuf_free(p);
            return;
        }
        
        /* 交给 LwIP 本地处理 */
        if (ERR_OK != low_netif->input(p, low_netif)) {
            pbuf_free(p);
        }
    }
}
```

> Note (2026-02-08): `tpmesh_eth_input_hook()` is fail-closed for bridge-owned actions.
> If policy returns `BRIDGE_TO_MESH` or `BRIDGE_PROXY_ARP`, the frame is considered consumed even when bridge send fails, and will not fall back to local `netif->input`.

**已添加获取 netif 函数:**

```c
struct netif* ethernetif_get_netif(void)
{
    return low_netif;
}
```

### 2. 修改 ethernetif.h ✅ 已完成

**已添加函数声明:**

```c
struct netif* ethernetif_get_netif(void);
```

### 3. 修改 main.c ✅ 已完成

**文件位置:** `App/main.c`

**已添加头文件:**

```c
#include "x_protocol/tpmesh_init.h"
#include "x_protocol/tpmesh_debug.h"
#include "ethernetif.h"
```

**已添加调试输出初始化:**

```c
/* 初始化调试输出 (printf -> USART2) */
tpmesh_debug_init();
```

**已在 EnetInit() 之后添加初始化:**

```c
EnetInit(false);

/* TPMesh 桥接初始化 
 * TPMESH_MODE: 0=禁用, 1=Top Node, 2=DDC
 */
#if (TPMESH_MODE == TPMESH_MODE_TOP_NODE)
{
  struct netif *eth_netif = ethernetif_get_netif();
  if (eth_netif != NULL) {
    tpmesh_module_init_top(eth_netif);
  }
}
#elif (TPMESH_MODE == TPMESH_MODE_DDC)
tpmesh_module_init_ddc();
#else
/* TPMESH_MODE_DISABLED: 正常以太网模式, 不初始化 TPMesh */
#endif
```

**已在 vTaskStartScheduler() 之前创建任务:**

```c
/* TPMesh 桥接任务 */
if (tpmesh_is_initialized()) {
  tpmesh_create_tasks();
}

vTaskStartScheduler();
```

---

## 编译配置

### 1. 添加源文件到项目

将以下文件添加到编译:
- `App/x_protocol/tpmesh_bridge.c`
- `App/x_protocol/tpmesh_at.c`
- `App/x_protocol/tpmesh_schc.c`
- `App/x_protocol/node_table.c`
- `App/x_protocol/tpmesh_init.c`
- `App/x_protocol/tpmesh_debug.c`

### 2. 添加头文件路径

确保 `App/x_protocol` 在头文件搜索路径中。

### 3. 配置 TPMesh 模式

在 `tpmesh_init.h` 或项目配置中:

```c
/* TPMesh 模式配置
 * 0 = 禁用 TPMesh (仅使用 PHY 以太网, 正常模式)
 * 1 = Top Node 模式 (以太网桥接到 Mesh)
 * 2 = DDC 节点模式 (通过 Mesh 连接)
 */
#define TPMESH_MODE    0    /* 默认禁用 */
// #define TPMESH_MODE    1    /* Top Node */
// #define TPMESH_MODE    2    /* DDC */
```

预定义的模式常量:
- `TPMESH_MODE_DISABLED`  = 0
- `TPMESH_MODE_TOP_NODE`  = 1
- `TPMESH_MODE_DDC`       = 2

---

## UART 配置

### UART6 (TPMesh 模组 AT 命令)

确保 UART6 已正确初始化:

1. **GPIO 配置** - 检查 UART6 TX/RX 引脚
2. **时钟使能** - `rcu_periph_clock_enable(RCU_UART6);`
3. **NVIC 配置** - UART6 中断优先级

如果使用其他 UART，修改 `tpmesh_at.h`:

```c
#define TPMESH_UART_NAME    "UART6"  // 修改为实际 UART 名称
```

### USART2 (调试输出 / printf 重定向)

`tpmesh_debug.c` 将 printf 输出重定向到 USART2:

1. **波特率:** 115200 (可在 `tpmesh_debug.c` 中修改 `DEBUG_UART_BAUD`)
2. **配置:** 8N1

如果需要使用其他 UART，修改 `tpmesh_debug.c`:

```c
#define DEBUG_UART_NAME     "USART2"  // 修改为实际名称
```

**使用示例:**

```c
/* 方式 1: 直接使用 printf (自动重定向到 USART2) */
printf("Hello TPMesh: mode=%d\r\n", TPMESH_MODE);

/* 方式 2: 使用 tpmesh_debug_printf */
tpmesh_debug_printf("Node 0x%04X registered\r\n", mesh_id);

/* 方式 3: 发送字符串 */
tpmesh_debug_puts("System ready\r\n");
```

---

## 测试验证

### 1. 基本功能测试

1. 编译并下载固件
2. 串口调试查看输出:
   - `TPMesh Init: Top Node initialized successfully`
   - `TPMesh: Tasks created`

### 2. 模组通信测试

查看 AT 命令响应:
- `TPMesh AT: Initialized on UART6 @ 115200 baud`
- `TPMesh: Module initialized`

### 3. DDC 注册测试 (需要 Top Node + DDC)

1. Top Node 收到 DDC 注册: `TPMesh Top: DDC 0x0002 registered`
2. Top Node 发送 GARP: `TPMesh: GARP sent for 192.168.10.2`
3. DDC 收到 ACK: `TPMesh DDC: Register ACK received`

### 4. BMS Ping DDC 测试

1. BMS (PC) 发送 ARP 请求
2. Top Node 代理应答: `TPMesh: Proxy ARP reply sent for 192.168.10.2`
3. BMS 发送 ICMP Echo
4. Top Node 转发到 Mesh
5. DDC 响应

---

## 调试命令

可以在代码中调用以下函数进行调试:

```c
/* 打印 TPMesh 状态 */
tpmesh_print_status();

/* 打印节点表 */
node_table_dump();
```

---

## 常见问题

### Q: TPMesh 初始化失败

**A:** 检查:
1. UART6 是否正确配置
2. TPMesh 模组是否连接
3. 模组波特率是否为 115200

### Q: DDC 无法注册

**A:** 检查:
1. Mesh 网络是否建立 (查看 +ROUTE 事件)
2. Cell ID 是否一致
3. Top Node Mesh ID 是否为 0xFFFE

### Q: BMS 无法 Ping 通 DDC

**A:** 检查:
1. DDC 是否已注册 (node_table_dump)
2. Top Node 是否代答 ARP
3. 数据是否正确转发

---

## 版本历史

- V0.6.2 (2026-02-04): 初始集成版本


## 变更记录（V0.7 / 2026-02-07）

### 架构更新
- AT 发送格式与 TPMESH_V1-6 对齐：`AT+SEND=<ADDR>,<LEN>,<DATA>,<TYPE>`。
- +NNMI 解析支持标准 5 字段格式（SRC/DEST/RSSI/LEN/DATA），并兼容旧 3 字段格式。
- 模组初始化补全 `AT+TYPE`、`AT+LP`、`AT+NNMI`。
- DDC 侧 Mesh 下行数据打通到 LwIP 输入路径。
- 增加 Mesh 隧道头最小长度校验，避免短帧越界访问。

### 对集成方影响
1. 如果使用自定义串口日志，请关注 `AT+SEND` 日志格式已新增 TYPE 参数。
2. 建议在联调时抓取模组 URC，确认收到 `+NNMI:<SRC>,<DEST>,<RSSI>,<LEN>,<DATA>`。
3. DDC 模式下建议验证 LwIP 输入线程模型（`netif->input` 回调成功返回）。

---

## UART6 Takeover Integration (V0.8)

When an external module needs UART6 to configure TPMesh:

1. Call `tpmesh_request_uart6_takeover()` before external configuration.
2. Function blocks until ownership is safely transferred (or returns error).
3. External module performs UART6 configuration sequence.
4. Terminate/restart current runtime, then relaunch x_protocol.

This model does not use in-process UART6 reclaim/state APIs.

Compile-time policy:

- `TPMESH_MODULE_INIT_POLICY=TPMESH_MODULE_INIT_BY_X_PROTOCOL` (default)
- `TPMESH_MODULE_INIT_POLICY=TPMESH_MODULE_INIT_BY_EXTERNAL`

Details: `App/x_protocol/UART6_TAKEOVER_MODE.md`

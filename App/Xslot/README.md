# DDC MQTT AT 透传模块

## 概述

本模块实现 DDC 与 PC 之间通过 MQTT 进行 AT 指令透传通信。

## 架构

```
┌─────────────────────────────────────────────────────────────────────────┐
│                              PC 端                                       │
│  ┌──────────────┐     ┌──────────────┐     ┌──────────────┐            │
│  │  pc_mqtt_    │────▶│ MQTT Broker  │     │ mDNS Service │            │
│  │  tool.py     │     │  (Mosquitto) │     │ (zeroconf)   │            │
│  └──────────────┘     └──────────────┘     └──────────────┘            │
│         │                    ▲                    │                     │
│         │                    │                    │ 注册服务:           │
│         │                    │                    │ _ddc-mqtt._tcp.local│
└─────────│────────────────────│────────────────────│─────────────────────┘
          │                    │                    │
          │ Publish            │ Subscribe          │ mDNS 广播
          │ ddc/{id}/at/cmd    │ ddc/{id}/at/resp   │
          ▼                    │                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                              DDC 端                                      │
│  ┌──────────────┐     ┌──────────────┐     ┌──────────────┐            │
│  │   mDNS       │────▶│ MQTT Client  │────▶│   UART2      │──▶ AT 模组 │
│  │  Discovery   │     │  (lwIP)      │◀────│  透传        │◀── 响应    │
│  └──────────────┘     └──────────────┘     └──────────────┘            │
└─────────────────────────────────────────────────────────────────────────┘
```

## 文件结构

```
App/Xslot/
├── communication.h         # 统一头文件
├── communication_task.c    # 通信主任务（状态机）
├── mdns_discovery.c        # mDNS Broker 发现
├── mqtt_app.c              # MQTT 客户端
├── at_passthrough.c        # AT 指令透传
└── README.md               # 本文档

tools/
└── pc_mqtt_tool.py         # PC 端工具
```

## 工作流程

### 1. DDC 启动
```
DDC 上电
    │
    ▼
初始化 mDNS 监听 (端口 5353)
    │
    ▼
初始化 UART2 (115200 8N1)
    │
    ▼
等待 mDNS 服务发现...
```

### 2. PC 启动配置工具
```
运行 pc_mqtt_tool.py
    │
    ▼
注册 mDNS 服务: _ddc-mqtt._tcp.local
    │
    ▼
连接 MQTT Broker
    │
    ▼
等待设备上线...
```

### 3. DDC 发现并连接
```
DDC 收到 mDNS 广播
    │
    ▼
解析 Broker IP:Port
    │
    ▼
连接 MQTT Broker
    │
    ▼
订阅 ddc/{device_id}/at/cmd
    │
    ▼
发布上线消息到 ddc/status
```

### 4. AT 指令透传
```
PC 发送 AT 命令
    │
    ├─▶ MQTT Publish: ddc/{id}/at/cmd
    │
    ▼
DDC 收到命令
    │
    ├─▶ UART2 发送 AT 命令
    │
    ▼
AT 模组响应
    │
    ├─▶ UART2 接收响应
    │
    ▼
DDC 发布响应
    │
    ├─▶ MQTT Publish: ddc/{id}/at/resp
    │
    ▼
PC 收到响应
```

## 使用方法

### PC 端

1. 安装依赖：
```bash
pip install zeroconf paho-mqtt
```

2. 启动 MQTT Broker (如 Mosquitto)：
```bash
mosquitto -v
```

3. 运行工具：
```bash
python tools/pc_mqtt_tool.py --broker 127.0.0.1 --port 1883
```

4. 命令示例：
```
[DDC_0011223344]> AT+GMR
  发送: 'AT+GMR\r\n' -> ddc/DDC_0011223344/at/cmd
  等待响应...

[AT 响应] AT version:1.7.4.0
OK
```

### DDC 端

在 `main.c` 中添加任务：

```c
#include "Xslot/communication.h"

// 在 main() 中创建任务
xTaskCreate(communication_task, "comm_task", 0x800, NULL, 
            tskIDLE_PRIORITY + 1, NULL);
```

## Topic 定义

| Topic | 方向 | 说明 |
|-------|------|------|
| `ddc/status` | DDC → PC | 设备状态（上线/离线）|
| `ddc/{id}/at/cmd` | PC → DDC | AT 命令 |
| `ddc/{id}/at/resp` | DDC → PC | AT 响应 |

## 配置

### UART 配置
- 端口：UART2
- 波特率：115200
- 数据位：8
- 停止位：1
- 校验：无

### mDNS 配置
- 服务类型：`_ddc-mqtt._tcp.local`
- 端口：5353 (标准 mDNS 端口)

### MQTT 配置
- 默认端口：1883
- QoS：0
- Keep-alive：60 秒

## 注意事项

1. DDC 只有在发现 mDNS 服务后才会连接 MQTT
2. 当 PC 端工具退出时，mDNS 服务注销，DDC 会自动断开
3. AT 响应超时时间为 500ms
4. 设备 ID 基于 MAC 地址生成

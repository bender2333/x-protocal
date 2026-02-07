/**
 * @file tpmesh_at.c
 * @brief TPMesh AT 命令模块实现 (响应队列 + TX互斥锁)
 *
 * 单模式设计: 所有 AT 命令均在 FreeRTOS Task 中执行
 *
 * 数据流:
 *   发送者 Task                   RX Task
 *   ┌─────────────┐             ┌─────────────┐
 *   │ take mutex   │             │ parse_rx_bytes│
 *   │ queue reset  │             │   ↓          │
 *   │ uart6_send   │ ──UART──→  │ dispatch_line│
 *   │ queue receive│ ←──queue── │ queue overwrite│
 *   │ give mutex   │             │              │
 *   └─────────────┘             └─────────────┘
 *
 * 线程安全:
 * - s_line_buf: 仅由 RX Task (parse_rx_bytes) 访问, 单消费者
 * - s_resp_queue: RX Task 写, 发送者 Task 读, FreeRTOS 队列内部锁
 * - UART TX: s_tx_mutex 互斥锁保护
 *
 * @version 1.2.0
 */

#include "tpmesh_at.h"
#include "tpmesh_debug.h"
#include "tpmesh_uart.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * RTOS 对象
 * ============================================================================
 */

/**
 * 响应队列 (深度 1, 传递 at_resp_t)
 * - 写入端: dispatch_line() (在 RX Task 上下文)
 * - 读取端: send_and_wait() (在命令发送者 Task 上下文)
 */
static QueueHandle_t s_resp_queue = NULL;

/**
 * TX 互斥锁
 * - 保证同一时刻只有一个 Task 发送 AT 命令
 * - 防止 AT+SEND 和 heartbeat 等同时发送导致串口数据混乱
 */
static SemaphoreHandle_t s_tx_mutex = NULL;

/* ============================================================================
 * 行解析缓冲 (仅 RX Task 访问)
 * ============================================================================
 */

static char s_line_buf[TPMESH_URC_LINE_SIZE];
static uint16_t s_line_idx = 0;

/* ============================================================================
 * 回调 + 状态
 * ============================================================================
 */

static tpmesh_data_cb_t s_data_cb = NULL;
static tpmesh_route_cb_t s_route_cb = NULL;
static bool s_initialized = false;

/* ============================================================================
 * 私有函数声明
 * ============================================================================
 */

static void dispatch_line(const char *line);
static int parse_rx_bytes(void);
static at_resp_t send_and_wait(const uint8_t *raw, uint16_t raw_len,
                               uint32_t timeout_ms);
static int hex_to_bytes(const char *hex, uint8_t *out, uint16_t max_len);
static void bytes_to_hex(const uint8_t *data, uint16_t len, char *hex);

/* ============================================================================
 * 公共函数 - 初始化
 * ============================================================================
 */

int tpmesh_at_init(void) {
  if (s_initialized) {
    return 0;
  }

  /* 1. UART6 初始化 (GPIO + 外设 + 中断, 无 RTOS 依赖) */
  if (tpmesh_uart6_init() != 0) {
    tpmesh_debug_printf("AT: UART6 init failed\n");
    return -1;
  }

  /*
   * 2. 创建 RTOS 对象
   *
   * ★ 安全性说明 ★
   * xQueueCreate / xSemaphoreCreateMutex 内部仅调用 pvPortMalloc()
   * pvPortMalloc 从静态堆 ucHeap[] 分配, 不依赖调度器
   * 因此在 main() 中 vTaskStartScheduler() 之前调用是安全的
   *
   * 注意: 这些对象的 take/receive 操作仅在 Task 中使用
   */
  s_resp_queue = xQueueCreate(1, sizeof(at_resp_t));
  s_tx_mutex = xSemaphoreCreateMutex();

  if (s_resp_queue == NULL || s_tx_mutex == NULL) {
    tpmesh_debug_printf("AT: RTOS obj create failed (heap?)\n");
    if (s_resp_queue) {
      vQueueDelete(s_resp_queue);
      s_resp_queue = NULL;
    }
    if (s_tx_mutex) {
      vSemaphoreDelete(s_tx_mutex);
      s_tx_mutex = NULL;
    }
    return -2;
  }

  /* 3. 重置内部状态 */
  s_line_idx = 0;
  s_line_buf[0] = '\0';
  s_data_cb = NULL;
  s_route_cb = NULL;

  s_initialized = true;
  tpmesh_debug_printf("AT: Initialized (queue + mutex)\n");
  return 0;
}

void tpmesh_at_deinit(void) {
  if (!s_initialized) {
    return;
  }
  tpmesh_uart6_deinit();
  if (s_resp_queue) {
    vQueueDelete(s_resp_queue);
    s_resp_queue = NULL;
  }
  if (s_tx_mutex) {
    vSemaphoreDelete(s_tx_mutex);
    s_tx_mutex = NULL;
  }
  s_initialized = false;
}

/* ============================================================================
 * 内部核心: 发送原始数据 + 等待响应
 * ============================================================================
 */

/**
 * @brief 发送原始字节并等待 OK/ERROR 响应
 *
 * 前提: 必须在 FreeRTOS Task 中调用 (调度器已启动)
 *
 * 流程:
 *   1. xSemaphoreTake(s_tx_mutex) — 防止并发发送
 *   2. xQueueReset(s_resp_queue)  — 清空旧响应
 *   3. tpmesh_uart6_send()        — 发送命令
 *   4. xQueueReceive(s_resp_queue, timeout) — 等待 RX Task 写入响应
 *   5. xSemaphoreGive(s_tx_mutex) — 释放锁
 *
 * @param raw       发送数据 (含 \r\n)
 * @param raw_len   长度
 * @param timeout_ms 超时 (ms)
 * @return AT_RESP_OK / AT_RESP_ERROR / AT_RESP_TIMEOUT
 */
static at_resp_t send_and_wait(const uint8_t *raw, uint16_t raw_len,
                               uint32_t timeout_ms) {
  /* 1. 获取 TX 互斥锁 (最长等 5 秒) */
  if (xSemaphoreTake(s_tx_mutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    tpmesh_debug_printf("AT: TX mutex timeout\n");
    return AT_RESP_TIMEOUT;
  }

  /* 2. 清空响应队列中的旧/残留响应 */
  xQueueReset(s_resp_queue);

  /* 3. 发送 */
  if (tpmesh_uart6_send(raw, raw_len) != 0) {
    xSemaphoreGive(s_tx_mutex);
    return AT_RESP_ERROR;
  }

  /* 4. 等待 RX Task 将响应写入队列 */
  at_resp_t resp;
  if (xQueueReceive(s_resp_queue, &resp, pdMS_TO_TICKS(timeout_ms)) == pdTRUE) {
    /* 收到 OK 或 ERROR */
  } else {
    resp = AT_RESP_TIMEOUT;
  }

  /* 5. 释放互斥锁 */
  xSemaphoreGive(s_tx_mutex);
  return resp;
}

/* ============================================================================
 * 公共函数 - 命令发送
 * ============================================================================
 */

at_resp_t tpmesh_at_cmd(const char *cmd, uint32_t timeout_ms) {
  if (!s_initialized || cmd == NULL) {
    return AT_RESP_ERROR;
  }

  char buf[256];
  int len = snprintf(buf, sizeof(buf), "%s\r\n", cmd);

  at_resp_t r = send_and_wait((uint8_t *)buf, (uint16_t)len, timeout_ms);

  tpmesh_debug_printf("AT: [%s] -> %s\n", cmd,
                      r == AT_RESP_OK      ? "OK"
                      : r == AT_RESP_ERROR ? "ERROR"
                                           : "TIMEOUT");

  return r;
}

int tpmesh_at_cmd_no_wait(const char *cmd) {
  if (!s_initialized || cmd == NULL) {
    return -1;
  }

  char buf[256];
  int len = snprintf(buf, sizeof(buf), "%s\r\n", cmd);

  /* 用互斥锁保护, 防止与其他发送交错 */
  if (xSemaphoreTake(s_tx_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
    return -2;
  }
  int ret = tpmesh_uart6_send((uint8_t *)buf, (uint16_t)len);
  xSemaphoreGive(s_tx_mutex);

  tpmesh_debug_printf("AT: [%s] (no wait)\n", cmd);
  return ret;
}

at_resp_t tpmesh_at_send_data(uint16_t dest_mesh_id, const uint8_t *data,
                              uint16_t len) {
  if (!s_initialized || data == NULL || len == 0) {
    return AT_RESP_ERROR;
  }
  if (len > TPMESH_MTU) {
    return AT_RESP_ERROR;
  }

  /* 构建 "AT+SEND=XXXX,LEN,HEXDATA,TYPE\r\n" */
  char cmd[TPMESH_AT_CMD_MAX_LEN];
  char hex_data[TPMESH_MTU * 2 + 1];
  bytes_to_hex(data, len, hex_data);

  int cmd_len = snprintf(cmd, sizeof(cmd), "AT+SEND=%04X,%u,%s,%u\r\n",
                         dest_mesh_id, (unsigned)len, hex_data,
                         (unsigned)TPMESH_AT_SEND_TYPE_DEFAULT);
  if (cmd_len <= 0 || cmd_len >= (int)sizeof(cmd)) {
    return AT_RESP_ERROR;
  }

  return send_and_wait((uint8_t *)cmd, (uint16_t)cmd_len, TPMESH_AT_TIMEOUT_MS);
}

/* ============================================================================
 * 兼容函数 (供 tpmesh_bridge.c 调用)
 * ============================================================================
 */

int tpmesh_at_send(uint16_t dest_mesh_id, const uint8_t *data, uint16_t len) {
  at_resp_t r = tpmesh_at_send_data(dest_mesh_id, data, len);
  return (r == AT_RESP_OK) ? 0 : -1;
}

/* ============================================================================
 * 公共函数 - 接收处理
 * ============================================================================
 */

int tpmesh_at_poll(void) { return parse_rx_bytes(); }

void tpmesh_at_set_data_cb(tpmesh_data_cb_t cb) { s_data_cb = cb; }

void tpmesh_at_set_route_cb(tpmesh_route_cb_t cb) { s_route_cb = cb; }

/* ============================================================================
 * 公共函数 - 模组初始化
 * ============================================================================
 */

int tpmesh_module_init(uint16_t mesh_id, bool is_top_node) {
  at_resp_t r;

  tpmesh_debug_printf("Module: Init (ID=0x%04X, %s)\n", mesh_id,
                      is_top_node ? "CENTER" : "ORDINARY");

  /* AT 基本测试 */
  r = tpmesh_at_cmd("AT", 1000);
  if (r != AT_RESP_OK) {
    tpmesh_debug_printf("Module: AT test failed (%d)\n", r);
    return -1;
  }

  /* 设置地址 */
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "AT+ADDR=%04X", mesh_id);
  r = tpmesh_at_cmd(cmd, TPMESH_AT_TIMEOUT_MS);
  if (r != AT_RESP_OK) {
    tpmesh_debug_printf("Module: ADDR failed\n");
    return -2;
  }

  /* 设置 Cell ID */
  r = tpmesh_at_cmd("AT+CELL=0", TPMESH_AT_TIMEOUT_MS);
  if (r != AT_RESP_OK) {
    tpmesh_debug_printf("Module: CELL failed\n");
    return -3;
  }

  /* 设置功耗模式 (Top=TypeD 常接收, DDC=TypeC 低功耗) */
  {
    uint8_t lp = is_top_node ? TPMESH_AT_LP_TOP_DEFAULT : TPMESH_AT_LP_DDC_DEFAULT;
    snprintf(cmd, sizeof(cmd), "AT+LP=%u", (unsigned)lp);
  }
  r = tpmesh_at_cmd(cmd, TPMESH_AT_TIMEOUT_MS);
  if (r != AT_RESP_OK) {
    tpmesh_debug_printf("Module: LP failed\n");
    return -4;
  }

  tpmesh_debug_printf("Module: Init OK\n");
  return 0;
}

void tpmesh_module_reset(void) {
  tpmesh_at_cmd_no_wait("AT+REBOOT");
  /* 等待模组重启 - 调用者需自行延时 */
}

/* ============================================================================
 * 公共函数 - 任务
 * ============================================================================
 */

void tpmesh_at_rx_task(void *arg) {
  (void)arg;
  tpmesh_debug_printf("AT RX Task: started\n");

  while (1) {
    parse_rx_bytes();
    vTaskDelay(pdMS_TO_TICKS(1)); /* 1ms 轮询周期 */
  }
}

int tpmesh_at_parse_nnmi(const char *urc, uint16_t *src_mesh_id, uint8_t *data,
                         uint16_t *len) {
  /* 优先支持: +NNMI:<SRC>,<DEST>,<RSSI>,<LEN>,<DATA>
   * 兼容旧格式: +NNMI:<SRC>,<LEN>,<DATA>
   */
  if (urc == NULL || src_mesh_id == NULL || data == NULL || len == NULL) {
    return -10;
  }
  if (strncmp(urc, "+NNMI:", 6) != 0) {
    return -1;
  }

  const char *p = urc + 6;
  while (*p == ' ') p++;

  char *endptr = NULL;
  unsigned long src = strtoul(p, &endptr, 16);
  if (endptr == p || *endptr != ',') {
    return -2;
  }
  *src_mesh_id = (uint16_t)src;
  p = endptr + 1;
  while (*p == ' ') p++;

  /* 尝试新格式: DEST,RSSI,LEN,DATA */
  unsigned long dest = strtoul(p, &endptr, 16);
  (void)dest;
  if (endptr != p && *endptr == ',') {
    p = endptr + 1;
    while (*p == ' ') p++;

    (void)strtol(p, &endptr, 10); /* RSSI */
    if (endptr != p && *endptr == ',') {
      p = endptr + 1;
      while (*p == ' ') p++;

      unsigned long declared_len = strtoul(p, &endptr, 10);
      if (endptr == p || *endptr != ',') {
        return -3;
      }
      p = endptr + 1;
      while (*p == ' ') p++;

      if (declared_len > TPMESH_MTU || declared_len > *len) {
        return -4;
      }

      int bytes = hex_to_bytes(p, data, (uint16_t)declared_len);
      if (bytes != (int)declared_len) {
        return -5;
      }
      *len = (uint16_t)declared_len;
      return 0;
    }
  }

  /* 回退旧格式: LEN,DATA */
  unsigned long declared_len = strtoul(p, &endptr, 10);
  if (endptr == p || *endptr != ',') {
    return -6;
  }
  p = endptr + 1;
  while (*p == ' ') p++;

  if (declared_len > TPMESH_MTU || declared_len > *len) {
    return -7;
  }
  int bytes = hex_to_bytes(p, data, (uint16_t)declared_len);
  if (bytes != (int)declared_len) {
    return -8;
  }

  *len = (uint16_t)declared_len;
  return 0;
}

/* ============================================================================
 * 私有函数 - 行分发
 * ============================================================================
 */

/**
 * @brief 处理一行完整的响应/URC (在 RX Task 上下文中执行)
 *
 * - "OK" / "+CMD:OK" / "ERROR" / "+CMD:ERROR" → xQueueOverwrite(s_resp_queue)
 * - "+NNMI:<data>" → s_data_cb()
 * - "+ROUTE:<event>" → s_route_cb()
 * - 其他 → 忽略 (命令回显等)
 *
 * 注: TPMesh 模组回复格式为 "+CMD:OK\r\n" (如 +AT:OK, +ADDR:OK)
 */
static void dispatch_line(const char *line) {
  if (line == NULL || line[0] == '\0') {
    return;
  }

  /* ---- OK: "OK" 或 "+CMD:OK" (如 +AT:OK, +ADDR:OK, +NNMI:OK) ---- */
  {
    bool is_ok = false;

    if (strcmp(line, "OK") == 0) {
      is_ok = true;
    } else if (line[0] == '+') {
      /* 匹配 "+xxx:OK" 模式: 找到 ":OK" 且后面是字符串结尾 */
      const char *p = strstr(line, ":OK");
      if (p != NULL && p[3] == '\0') {
        is_ok = true;
      }
    }

    if (is_ok) {
      if (s_resp_queue) {
        at_resp_t r = AT_RESP_OK;
        xQueueOverwrite(s_resp_queue, &r);
      }
      return;
    }
  }

  /* ---- ERROR: "ERROR", "ERROR,n", 或 "+CMD:ERROR" ---- */
  {
    bool is_err = false;

    if (strncmp(line, "ERROR", 5) == 0) {
      is_err = true;
    } else if (line[0] == '+') {
      /* 匹配 "+xxx:ERROR" 模式 */
      if (strstr(line, ":ERROR") != NULL) {
        is_err = true;
      }
    }

    if (is_err) {
      if (s_resp_queue) {
        at_resp_t r = AT_RESP_ERROR;
        xQueueOverwrite(s_resp_queue, &r);
      }
      return;
    }
  }

  /* ---- +NNMI: 数据 URC (注意: +NNMI:OK 已在上面被捕获) ---- */
  if (strncmp(line, "+NNMI:", 6) == 0) {
    uint16_t src_id = 0;
    uint8_t buf[TPMESH_MTU];
    uint16_t urc_len = TPMESH_MTU;

    if (tpmesh_at_parse_nnmi(line, &src_id, buf, &urc_len) == 0) {
      if (s_data_cb) {
        s_data_cb(src_id, buf, urc_len);
      }
    }
    return;
  }

  /* ---- +ROUTE: 路由 URC ---- */
  if (strncmp(line, "+ROUTE:", 7) == 0) {
    const char *p = line + 7; /* 指向 "CREATE ADDR[0xFFFE]" 的开头 */
    while (*p == ' ') p++;    /* 兼容 "+ROUTE: CREATE ..." */

    /* 1. 增大缓冲区，防止越界 (25字节对于包含地址的完整长串可能比较紧凑) */
    char event[64] = {0};
    uint16_t addr = 0;

    /* 2. 直接复制整行内容到 event，直到遇到字符串结束符 \0 */
    /* 使用 snprintf 是最安全的做法，自动处理边界和 \0 */
    snprintf(event, sizeof(event), "%s", p);

    /* 3. 解析地址: 此时 event 内容已经是 "CREATE ADDR[0xFFFE]" */
    /* 我们直接在 event 字符串内部查找 "ADDR[" */
    char *addr_start = strstr(event, "ADDR[");
    if (addr_start != NULL) {
      const char *q = addr_start + 5; /* 跳过 "ADDR[" */
      char hex_buf[9];
      uint8_t hex_len = 0;

      while (*q != '\0' && *q != ']' && hex_len < sizeof(hex_buf) - 1) {
        if (isxdigit((unsigned char)*q)) {
          hex_buf[hex_len++] = *q;
        }
        q++;
      }

      if (hex_len > 0) {
        hex_buf[hex_len] = '\0';
        addr = (uint16_t)strtoul(hex_buf, NULL, 16);
      }
    }

    /* 4. 回调: 此时 event 是全长字符串，addr 是解析出的整数 */
    if (s_route_cb) {
      s_route_cb(event, addr);
    }
    return;
  }

  /* 其他行: 忽略 (命令回显等) */
}

/* ============================================================================
 * 私有函数 - 字节读取 + 行解析
 * ============================================================================
 */

/**
 * @brief 从 UART ring buffer 读取字节, 按行解析, 分发
 *
 * 仅由 tpmesh_at_rx_task() 调用 (单一消费者, 无需锁)
 *
 * @return 处理的字节数
 */
static int parse_rx_bytes(void) {
  int count = 0;
  uint8_t ch;

  while (tpmesh_uart6_getc(&ch) == 0) {
    count++;

    /* 添加到行缓冲区 */
    if (s_line_idx < TPMESH_URC_LINE_SIZE - 1) {
      s_line_buf[s_line_idx++] = (char)ch;
      s_line_buf[s_line_idx] = '\0';
    } else {
      /* 缓冲区溢出, 丢弃并重置 */
      s_line_idx = 0;
      s_line_buf[0] = '\0';
      continue;
    }

    /* 检测行结束 (\r\n) */
    if (s_line_idx >= 2 && s_line_buf[s_line_idx - 2] == '\r' &&
        s_line_buf[s_line_idx - 1] == '\n') {

      /* 去掉 \r\n */
      s_line_buf[s_line_idx - 2] = '\0';

      /* 处理非空行 */
      if (s_line_idx > 2) {
        dispatch_line(s_line_buf);
      }

      /* 重置行缓冲 */
      s_line_idx = 0;
      s_line_buf[0] = '\0';
    }
  }

  return count;
}

/* ============================================================================
 * 私有函数 - Hex 转换
 * ============================================================================
 */

static int hex_to_bytes(const char *hex, uint8_t *out, uint16_t max_len) {
  uint16_t i = 0;
  while (hex[0] && hex[1] && i < max_len) {
    char h[3] = {hex[0], hex[1], '\0'};
    out[i++] = (uint8_t)strtol(h, NULL, 16);
    hex += 2;
  }
  return i;
}

static void bytes_to_hex(const uint8_t *data, uint16_t len, char *hex) {
  static const char hc[] = "0123456789ABCDEF";
  for (uint16_t i = 0; i < len; i++) {
    *hex++ = hc[(data[i] >> 4) & 0x0F];
    *hex++ = hc[data[i] & 0x0F];
  }
  *hex = '\0';
}

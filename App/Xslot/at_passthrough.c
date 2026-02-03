/**
 * @file at_passthrough.c
 * @brief AT 指令透传模块
 *
 * 功能：
 * 1. 接收 MQTT 消息中的 AT 命令，发送到 UART
 * 2. 接收 UART 响应，通过 MQTT 发送回 PC
 */

#include "communication.h"
#include "AppConfig.h"
#include "EKStdLib.h"
#include "EKUart.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * 私有变量
 * ============================================================================ */

/* UART 句柄 */
static HANDLE s_uart_handle = -1;

/* 接收缓冲区 */
static uint8_t s_rx_buffer[AT_RX_BUFFER_SIZE];
static uint16_t s_rx_index = 0;

/* 发送缓冲区 */
static uint8_t s_tx_buffer[AT_TX_BUFFER_SIZE];

/* 设备 ID */
static char s_device_id[32] = {0};

/* Topic 名称 */
static char s_topic_cmd[64] = {0};
static char s_topic_resp[64] = {0};

/* UART 文件结构 */
static BYTE s_uart_in_buf[AT_RX_BUFFER_SIZE];
static BYTE s_uart_out_buf[AT_TX_BUFFER_SIZE];
static EKFILE s_uart_file = {{s_uart_in_buf, 0, 0, AT_RX_BUFFER_SIZE},
                             {s_uart_out_buf, 0, 0, AT_TX_BUFFER_SIZE},
                             NULL};

/* 统计信息 */
static comm_stats_t s_stats = {0};

/* 响应超时计时器 */
static uint32_t s_response_timeout = 0;
static bool s_waiting_response = false;

/* ============================================================================
 * 私有函数声明
 * ============================================================================ */

static void uart_init(void);
static void generate_device_id(void);
static void process_uart_rx(void);

/* ============================================================================
 * 公共函数实现
 * ============================================================================ */

void at_passthrough_init(void) {
  printf("AT Passthrough: Initializing...\n");

  /* 生成设备 ID */
  generate_device_id();

  /* 构造 Topic 名称 */
  snprintf(s_topic_cmd, sizeof(s_topic_cmd), "%s/%s/at/cmd", MQTT_TOPIC_PREFIX,
           s_device_id);
  snprintf(s_topic_resp, sizeof(s_topic_resp), "%s/%s/at/resp",
           MQTT_TOPIC_PREFIX, s_device_id);

  printf("AT Passthrough: Device ID = %s\n", s_device_id);
  printf("AT Passthrough: CMD Topic = %s\n", s_topic_cmd);
  printf("AT Passthrough: RESP Topic = %s\n", s_topic_resp);

  /* 初始化 UART */
  uart_init();

  /* 清空缓冲区 */
  memset(s_rx_buffer, 0, sizeof(s_rx_buffer));
  s_rx_index = 0;
  memset(&s_stats, 0, sizeof(s_stats));

  printf("AT Passthrough: Initialized\n");
}

void at_passthrough_process(void) {
  if (s_uart_handle < 0) {
    return;
  }

  /* 处理 UART 空闲事件 */
  UART_OnIdle(s_uart_handle);

  /* 读取 UART 接收数据 */
  process_uart_rx();

  /* 检查响应超时 */
  if (s_waiting_response) {
    s_response_timeout++;
    /* 超时时间约 500ms (假设 process 每 10ms 调用一次) */
    if (s_response_timeout > 50) {
      /* 发送当前缓冲区内容（即使不完整） */
      if (s_rx_index > 0 && mqtt_client_is_connected()) {
        mqtt_client_publish(s_topic_resp, s_rx_buffer, s_rx_index);
        s_stats.mqtt_tx_count++;
        s_rx_index = 0;
      }
      s_waiting_response = false;
      s_response_timeout = 0;
    }
  }
}

void at_passthrough_on_mqtt_cmd(const uint8_t *data, uint16_t len) {
  if (s_uart_handle < 0 || data == NULL || len == 0) {
    return;
  }

  printf("AT Passthrough: Received CMD (%d bytes): ", len);
  for (uint16_t i = 0; i < len && i < 32; i++) {
    printf("%c", data[i] >= 32 ? data[i] : '.');
  }
  printf("\n");

  /* 发送到 UART */
  s32_t bytes_written = 0;
  s16_t result =
      UART_puts(s_uart_handle, (LPCSTR)data, len, &bytes_written, 1000);

  if (result == S_OK) {
    s_stats.uart_tx_bytes += bytes_written;
    /* 开始等待响应 */
    s_waiting_response = true;
    s_response_timeout = 0;
    s_rx_index = 0; /* 清空接收缓冲区准备接收响应 */
  } else {
    printf("AT Passthrough: UART write failed, result=%d\n", result);
    s_stats.error_count++;
  }
}

void at_passthrough_get_device_id(char *id, uint16_t len) {
  if (id != NULL && len > 0) {
    strncpy(id, s_device_id, len - 1);
    id[len - 1] = '\0';
  }
}

const char *at_passthrough_get_cmd_topic(void) { return s_topic_cmd; }

const char *at_passthrough_get_resp_topic(void) { return s_topic_resp; }

void communication_get_stats(comm_stats_t *stats) {
  if (stats != NULL) {
    memcpy(stats, &s_stats, sizeof(comm_stats_t));
  }
}

/* ============================================================================
 * 私有函数实现
 * ============================================================================ */

/**
 * @brief 初始化 UART
 */
static void uart_init(void) {
  UART_CONFIG cfg;

  /* 默认配置：19200 8N1 半双工 */
  cfg.nBaud = 115200;
  cfg.nControl = HALFDUPLEX | EIGHTBIT | PARITY_NONE | ONESTOPBIT;

  /* 打开 UART2 */
  s_uart_handle = EKfopen("UART2", &s_uart_file, &cfg);

  if (s_uart_handle >= 0) {
    printf("AT Passthrough: UART2 opened successfully\n");
  } else {
    printf("AT Passthrough: Failed to open UART2\n");
  }
}

/**
 * @brief 生成设备 ID（基于 MAC 地址）
 */
static void generate_device_id(void) {
  u32_t mac_high = 0, mac_low = 0;

  /* 从配置模块获取 MAC 地址 */
  ConfigGetMAC(&mac_high, &mac_low);

  /* 生成设备 ID: DDC_XXXXXXXXXXXX (12位十六进制) */
  snprintf(s_device_id, sizeof(s_device_id), "DDC_%04X%08X",
           (uint16_t)(mac_high & 0xFFFF), (uint32_t)mac_low);
}

/**
 * @brief 处理 UART 接收数据
 */
static void process_uart_rx(void) {
  s16_t ch;

  /* 读取所有可用数据 */
  while ((ch = UART_getc(s_uart_handle)) >= 0) {
    if (s_rx_index < AT_RX_BUFFER_SIZE - 1) {
      s_rx_buffer[s_rx_index++] = (uint8_t)ch;
      s_stats.uart_rx_bytes++;

      /* 重置超时计时器 */
      s_response_timeout = 0;
    }

    /* 检查是否收到完整响应 */
    /* AT 响应通常以 \r\n 结尾，或者以 OK/ERROR 结尾 */
    if (s_rx_index >= 2) {
      /* 检查 \r\n 结尾 */
      if (s_rx_buffer[s_rx_index - 2] == '\r' &&
          s_rx_buffer[s_rx_index - 1] == '\n') {
        /* 检查是否是完整响应（OK 或 ERROR） */
        if (s_rx_index >= 4) {
          /* 检查 OK\r\n */
          if (memcmp(s_rx_buffer + s_rx_index - 4, "OK\r\n", 4) == 0) {
            /* 响应完成，发送 */
            if (mqtt_client_is_connected()) {
              mqtt_client_publish(s_topic_resp, s_rx_buffer, s_rx_index);
              s_stats.mqtt_tx_count++;
            }
            s_rx_index = 0;
            s_waiting_response = false;
            continue;
          }
        }
        if (s_rx_index >= 7) {
          /* 检查 ERROR\r\n */
          if (memcmp(s_rx_buffer + s_rx_index - 7, "ERROR\r\n", 7) == 0) {
            /* 响应完成，发送 */
            if (mqtt_client_is_connected()) {
              mqtt_client_publish(s_topic_resp, s_rx_buffer, s_rx_index);
              s_stats.mqtt_tx_count++;
            }
            s_rx_index = 0;
            s_waiting_response = false;
            continue;
          }
        }
      }
    }
  }
}

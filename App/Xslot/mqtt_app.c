/**
 * @file mqtt_app.c
 * @brief MQTT 客户端模块
 *
 * 功能：
 * 1. 动态连接 Broker（地址由 mDNS 发现）
 * 2. 订阅 AT 命令 Topic
 * 3. 发布 AT 响应
 */

#include "communication.h"
#include "lwip/apps/mqtt.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/tcpip.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * 私有变量
 * ============================================================================ */

/* MQTT 客户端控制块 */
static mqtt_client_t *s_mqtt_client = NULL;

/* Broker 信息 */
static char s_broker_ip[16] = {0};
static uint16_t s_broker_port = 1883;

/* 连接状态 */
static bool s_connected = false;

/* 设备 ID 和 Topic */
static char s_device_id[32] = {0};
static char s_topic_cmd[64] = {0};
static char s_topic_resp[64] = {0};

/* 统计计数器 */
static uint32_t s_rx_count = 0;

/* ============================================================================
 * 外部函数声明（来自 at_passthrough.c）
 * ============================================================================ */

extern const char *at_passthrough_get_cmd_topic(void);
extern const char *at_passthrough_get_resp_topic(void);

/* ============================================================================
 * 私有函数声明
 * ============================================================================ */

static void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                               mqtt_connection_status_t status);
static void mqtt_incoming_publish_cb(void *arg, const char *topic,
                                     u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len,
                                  u8_t flags);
static void mqtt_sub_request_cb(void *arg, err_t result);
static void mqtt_pub_request_cb(void *arg, err_t result);

/* ============================================================================
 * 公共函数实现
 * ============================================================================ */

void ddc_mqtt_init(void) {
  printf("MQTT Client: Initializing...\n");

  s_mqtt_client = NULL;
  s_connected = false;
  memset(s_broker_ip, 0, sizeof(s_broker_ip));
  s_broker_port = 1883;

  /* 获取设备 ID 和 Topic */
  at_passthrough_get_device_id(s_device_id, sizeof(s_device_id));
  strncpy(s_topic_cmd, at_passthrough_get_cmd_topic(), sizeof(s_topic_cmd) - 1);
  strncpy(s_topic_resp, at_passthrough_get_resp_topic(),
          sizeof(s_topic_resp) - 1);

  printf("MQTT Client: Device ID = %s\n", s_device_id);
}

void ddc_mqtt_connect(const char *ip, uint16_t port) {
  ip_addr_t broker_ip;
  err_t err;

  if (ip == NULL || strlen(ip) == 0) {
    printf("MQTT Client: Invalid broker IP\n");
    return;
  }

  /* 保存 Broker 信息 */
  strncpy(s_broker_ip, ip, sizeof(s_broker_ip) - 1);
  s_broker_port = port > 0 ? port : 1883;

  printf("MQTT Client: Connecting to %s:%d\n", s_broker_ip, s_broker_port);

  /* 如果已有连接，先断开 */
  if (s_mqtt_client != NULL) {
    ddc_mqtt_disconnect();
  }

  /* 创建客户端实例 */
  s_mqtt_client = mqtt_client_new();
  if (s_mqtt_client == NULL) {
    printf("MQTT Client: Failed to allocate client\n");
    return;
  }

  /* 解析 Broker IP */
  if (!ipaddr_aton(s_broker_ip, &broker_ip)) {
    printf("MQTT Client: Invalid IP address format\n");
    return;
  }

  /* 填充连接信息 */
  struct mqtt_connect_client_info_t ci;
  memset(&ci, 0, sizeof(ci));
  ci.client_id = s_device_id;
  ci.client_user = NULL;
  ci.client_pass = NULL;
  ci.keep_alive = 60;
  ci.will_topic = NULL;
  ci.will_msg = NULL;
  ci.will_qos = 0;
  ci.will_retain = 0;

  /* 设置接收回调 */
  mqtt_set_inpub_callback(s_mqtt_client, mqtt_incoming_publish_cb,
                          mqtt_incoming_data_cb, NULL);

  /* 发起连接 (调用 lwIP 的 mqtt_client_connect) */
  err = mqtt_client_connect(s_mqtt_client, &broker_ip, s_broker_port,
                            mqtt_connection_cb, NULL, &ci);

  if (err != ERR_OK) {
    printf("MQTT Client: Connect request failed, err=%d\n", err);
  }
}

void ddc_mqtt_disconnect(void) {
  if (s_mqtt_client != NULL) {
    printf("MQTT Client: Disconnecting...\n");
    mqtt_disconnect(s_mqtt_client);
    /* 注意：mqtt_client_free 需要在断开后调用 */
    /* mqtt_client_free(s_mqtt_client); */
    s_mqtt_client = NULL;
  }
  s_connected = false;
}

bool ddc_mqtt_is_connected(void) {
  if (s_mqtt_client == NULL) {
    return false;
  }
  /* 调用 lwIP 的 mqtt_client_is_connected */
  return mqtt_client_is_connected(s_mqtt_client) != 0;
}

void ddc_mqtt_publish(const char *topic, const uint8_t *payload,
                      uint16_t len) {
  if (!ddc_mqtt_is_connected() || topic == NULL) {
    return;
  }

  err_t err = mqtt_publish(s_mqtt_client, topic, payload, len, 0, /* QoS 0 */
                           0,                                     /* retain */
                           mqtt_pub_request_cb, NULL);

  if (err != ERR_OK) {
    printf("MQTT Client: Publish failed, err=%d\n", err);
  }
}

void ddc_mqtt_subscribe(const char *topic) {
  if (!ddc_mqtt_is_connected() || topic == NULL) {
    return;
  }

  err_t err = mqtt_sub_unsub(s_mqtt_client, topic, 0, mqtt_sub_request_cb, NULL,
                             1 /* subscribe */);

  if (err != ERR_OK) {
    printf("MQTT Client: Subscribe failed, err=%d\n", err);
  } else {
    printf("MQTT Client: Subscribing to %s\n", topic);
  }
}

/* ============================================================================
 * 私有函数实现 - 回调
 * ============================================================================ */

/**
 * @brief 连接状态回调
 */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                               mqtt_connection_status_t status) {
  (void)arg;
  (void)client;

  if (status == MQTT_CONNECT_ACCEPTED) {
    printf("MQTT Client: Connected to broker!\n");
    s_connected = true;

    /* 订阅 AT 命令 Topic */
    ddc_mqtt_subscribe(s_topic_cmd);

    /* 发布上线消息 */
    char online_msg[128];
    snprintf(online_msg, sizeof(online_msg),
             "{\"device\":\"%s\",\"status\":\"online\"}", s_device_id);
    ddc_mqtt_publish("ddc/status", (uint8_t *)online_msg, strlen(online_msg));

  } else {
    printf("MQTT Client: Connection failed/disconnected, reason=%d\n", status);
    s_connected = false;
  }
}

/**
 * @brief 接收 Publish 开始回调（Topic）
 */
static void mqtt_incoming_publish_cb(void *arg, const char *topic,
                                     u32_t tot_len) {
  (void)arg;
  printf("MQTT Client: Incoming message on topic '%s', len=%u\n", topic,
         (unsigned int)tot_len);
}

/* 临时接收缓冲区 */
static uint8_t s_incoming_data[MQTT_MSG_BUFFER_SIZE];
static uint16_t s_incoming_len = 0;

/**
 * @brief 接收数据回调（Payload）
 */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len,
                                  u8_t flags) {
  (void)arg;

  /* 累积数据 */
  if (s_incoming_len + len < MQTT_MSG_BUFFER_SIZE) {
    memcpy(s_incoming_data + s_incoming_len, data, len);
    s_incoming_len += len;
  }

  /* 检查是否是最后一块数据 */
  if (flags & MQTT_DATA_FLAG_LAST) {
    s_rx_count++;

    printf("MQTT Client: Received complete message (%d bytes)\n", s_incoming_len);

    /* 将数据转发到 AT 透传模块 */
    at_passthrough_on_mqtt_cmd(s_incoming_data, s_incoming_len);

    /* 重置缓冲区 */
    s_incoming_len = 0;
  }
}

/**
 * @brief 订阅请求回调
 */
static void mqtt_sub_request_cb(void *arg, err_t result) {
  (void)arg;
  if (result == ERR_OK) {
    printf("MQTT Client: Subscribe successful\n");
  } else {
    printf("MQTT Client: Subscribe failed, err=%d\n", result);
  }
}

/**
 * @brief 发布请求回调
 */
static void mqtt_pub_request_cb(void *arg, err_t result) {
  (void)arg;
  if (result != ERR_OK) {
    printf("MQTT Client: Publish failed, err=%d\n", result);
  }
}

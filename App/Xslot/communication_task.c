/**
 * @file communication_task.c
 * @brief 通信主任务
 *
 * 整合 mDNS 发现、MQTT 客户端、AT 透传
 */

#include "communication.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/* ============================================================================
 * 状态机
 * ============================================================================ */

typedef enum {
    COMM_STATE_INIT,           /* 初始化 */
    COMM_STATE_WAITING,        /* 等待发现 Broker */
    COMM_STATE_CONNECTING,     /* 连接 Broker */
    COMM_STATE_CONNECTED,      /* 已连接，透传中 */
    COMM_STATE_DISCONNECTED    /* 断开，等待重连 */
} comm_state_t;

static comm_state_t s_state = COMM_STATE_INIT;
static uint32_t s_retry_count = 0;

/* ============================================================================
 * 主任务
 * ============================================================================ */

void communication_task(void *pvParameters) {
    (void)pvParameters;
    
    broker_info_t broker;
    uint32_t tick_count = 0;
    uint32_t system_tick_ms = 0;
    
    printf("\n========================================\n");
    printf("DDC Communication Task Starting...\n");
    printf("========================================\n\n");
    
    /* 等待网络初始化完成 */
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    s_state = COMM_STATE_INIT;
    
    for (;;) {
        /* 更新系统时间 (用于退避算法) */
        system_tick_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        switch (s_state) {
            
        case COMM_STATE_INIT:
            printf("COMM: Initializing modules...\n");
            
            /* 初始化各模块 */
            mdns_discovery_init();
            at_passthrough_init();
            mqtt_client_init();
            
            s_state = COMM_STATE_WAITING;
            s_retry_count = 0;
            printf("COMM: Waiting for broker discovery...\n");
            printf("COMM: Will send mDNS queries with exponential backoff (1s -> 5min max)\n");
            break;
            
        case COMM_STATE_WAITING:
            /* 检查是否发现了 Broker (被动监听) */
            if (mdns_discovery_get_broker(&broker)) {
                printf("COMM: Broker discovered at %s:%d\n", 
                       broker.ip, broker.port);
                s_state = COMM_STATE_CONNECTING;
            } else {
                /* 主动查询 (带退避算法) */
                mdns_discovery_query_with_backoff(system_tick_ms);
                
                /* 每 30 秒打印等待状态 */
                if ((tick_count % 3000) == 0) {
                    printf("COMM: Still waiting... (queries sent: %lu, next in: %lu ms)\n",
                           (unsigned long)mdns_discovery_get_query_count(),
                           (unsigned long)mdns_discovery_get_backoff_interval());
                }
            }
            break;
            
        case COMM_STATE_CONNECTING:
            printf("COMM: Connecting to broker...\n");
            ddc_mqtt_connect(broker.ip, broker.port);
            
            /* 等待连接结果 */
            for (int i = 0; i < 100; i++) {  /* 最多等待 10 秒 */
                vTaskDelay(pdMS_TO_TICKS(100));
                if (ddc_mqtt_is_connected()) {
                    printf("COMM: Connection established!\n");
                    s_state = COMM_STATE_CONNECTED;
                    s_retry_count = 0;
                    break;
                }
            }
            
            if (s_state != COMM_STATE_CONNECTED) {
                s_retry_count++;
                printf("COMM: Connection failed, retry %d\n", (int)s_retry_count);
                
                if (s_retry_count >= 5) {
                    /* 重试次数过多，回到等待状态 */
                    printf("COMM: Too many retries, back to waiting\n");
                    s_state = COMM_STATE_WAITING;
                    s_retry_count = 0;
                    /* 重置退避算法，因为 Broker 可能已经不可用 */
                    mdns_discovery_reset_backoff();
                    mdns_discovery_service_removed();
                } else {
                    /* 延迟后重试 */
                    vTaskDelay(pdMS_TO_TICKS(2000));
                }
            } else {
                /* 连接成功，重置退避算法 */
                mdns_discovery_reset_backoff();
            }
            break;
            
        case COMM_STATE_CONNECTED:
            /* 检查连接状态 */
            if (!ddc_mqtt_is_connected()) {
                printf("COMM: Connection lost\n");
                s_state = COMM_STATE_DISCONNECTED;
                break;
            }
            
            /* 处理 AT 透传 */
            at_passthrough_process();
            
            /* 检查 mDNS 服务是否还存在 */
            if (mdns_discovery_get_state() == BROKER_STATE_IDLE) {
                printf("COMM: Broker service disappeared\n");
                ddc_mqtt_disconnect();
                s_state = COMM_STATE_WAITING;
            }
            break;
            
        case COMM_STATE_DISCONNECTED:
            printf("COMM: Disconnected, attempting reconnect...\n");
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            /* 尝试重连 */
            if (mdns_discovery_get_broker(&broker)) {
                s_state = COMM_STATE_CONNECTING;
            } else {
                s_state = COMM_STATE_WAITING;
            }
            break;
            
        default:
            s_state = COMM_STATE_INIT;
            break;
        }
        
        tick_count++;
        vTaskDelay(pdMS_TO_TICKS(10));  /* 10ms 循环周期 */
    }
}

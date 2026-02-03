#ifndef __MEKINTP_H
#define __MEKINTP_H

#include "stdio.h"
// NTP时间戳结构
typedef struct
{
    uint32_t seconds;
    uint32_t fraction;
} ntp_timestamp_t;

typedef struct
{
    uint8_t li_vn_mode;                  // 跳跃指示器、版本号和模式
    uint8_t stratum;                     // 层级
    uint8_t poll;                        // 轮询间隔
    uint8_t precision;                   // 精度
    uint32_t root_delay;                 // 根延迟
    uint32_t root_dispersion;            // 根分散
    uint32_t reference_id;               // 参考标识符
    ntp_timestamp_t reference_timestamp; // 参考时间戳
    ntp_timestamp_t originate_timestamp; // 起始时间戳
    ntp_timestamp_t receive_timestamp;   // 接收时间戳
    ntp_timestamp_t transmit_timestamp;  // 发送时间戳
} ntp_packet_t;

typedef struct
{
    uint16_t year;   // 年份，如2023
    uint8_t month;   // 月份，1-12
    uint8_t day;     // 日，1-31
    uint8_t hour;    // 时，0-23
    uint8_t minute;  // 分，0-59
    uint8_t second;  // 秒，0-59
    uint8_t weekday; // 星期几，0-6（0表示周日）
} datetime_t;

void ntp_task(void *pvParameters);
#endif

/*!
    \file    ymodem.h
    \brief   ymodem header file for GD32 SDK

    \version 2024-06-30, V1.0.0, demo for GD32
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#ifndef YMODEM_H
#define YMODEM_H

#include "Platform/gd32xx.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define YMODEM_PACKET_OVERHEAD 5    /* 3 bytes header + 2 bits crc */

/* ymodem frame type enum */
typedef enum
{
    YMODEM_SOH  = 0x01,                                            /*!< ymodem frame data length is 128 */
    YMODEM_STX  = 0x02,                                            /*!< ymodem frame data length is 1024 */
    YMODEM_EOT  = 0x04,                                            /*!< ymodem end of transmission */
    YMODEM_ACK  = 0x06,                                            /*!< ymodem ack */
    YMODEM_NACK = 0x15,                                            /*!< ymodem no ack */
    YMODEM_CA   = 0x18,                                            /*!< ymodem transmission termination */
    YMODEM_C    = 0x43,                                            /*!< ymodem request packet */
}ymodem_frame_type_enum;

/* ymodem packet size */
typedef enum
{
    YMODEM_PACKET_SIZE     = 128,                                   /*!< ymodem packet size is 128 */
    YMODEM_PACKET_1K_SIZE  = 1024,                                  /*!< ymodem packet size is 1024 */
}ymodem_packet_size_enum;

/* ymodem flag enum */
typedef enum
{
    YMODEM_FLAG_GET   = 0x00000000,                                  /*!< ymodem frame packet get flag */
    YMODEM_FLAG_FPT   = 0x00000001,                                  /*!< ymodem frist packet flag */
    YMODEM_FLAG_STR   = 0x00000002,                                  /*!< ymodem start flag */
    YMODEM_FLAG_EOT   = 0x00000004,                                  /*!< ymodem EOT frame flag */
    YMODEM_FLAG_ACT   = 0x00000008,                                  /*!< ymodem activate flag */
    YMODEM_FLAG_NUME  = 0x00000010,                                  /*!< ymodem frame number error flag */
    YMODEM_FLAG_CRCE  = 0x00000020,                                  /*!< ymodem frame CRC error flag */
    
    YMODEM_FLAG_STP   = 0x00000080,                                  /*!< ymodem stop transmission flag */
    YMODEM_FLAG_CAG   = 0x00000100,                                  /*!< ymodem get CA flag */
    YMODEM_FLAG_HDRE  = 0x00000200,                                  /*!< ymodem header error flag */
    YMODEM_FLAG_EMAX  = 0x00000400,                                  /*!< ymodem error numbers max flag */
    YMODEM_FLAG_INTGET   = 0x00000800,                               /*!< ymodem frame packet interrupt get flag */
}ymodem_flag_enum;

#define MAX_PACKET_ERROR_NUMBER             40                       /*!< max numbers of error*/
#if defined(PLATFORM_GD32F5XX)
#define RECEIVE_TIMEOUT_C                   10000000
#define RECEIVE_TIMEOUT                     1000000
#elif defined(PLATFORM_GD32H7XX)
#define RECEIVE_TIMEOUT_C                   40000000
#define RECEIVE_TIMEOUT                     1000000
#else
#define RECEIVE_TIMEOUT_C                   10000000
#define RECEIVE_TIMEOUT                     1000000
#endif

typedef void (*ymodem_send_fun)(uint8_t data);
typedef uint8_t (*ymodem_rx_fun)(void *, uint16_t len, uint32_t timeout);
typedef void (*ymodem_packet_deal_fun)(void *);
typedef void (*delay_ms_fun)(uint32_t data);
typedef int (*ymodem_printf_fun)(const char *format, ...);
typedef int (*ymodem_flash_erase_fun)(uint32_t offset, int len);
typedef int (*ymodem_flash_write_fun)(uint32_t offset, const void *data, int len);
typedef unsigned long int (*ymodem_strtoul_fun)(const char *, char **, int);
#ifndef SECURITY_PROTECT_DISABLE
typedef void (*ymodem_fwdg_reload_fun)(void);
#endif /* SECURITY_PROTECT_DISABLE */

/* ymodem packet struct */
typedef struct
{
    unsigned char packet_flag;
    unsigned char packet_num;                                       /*!< ymodem frame data length is 128 or 1024 */
    unsigned char packet_inum;                                      /*!< ymodem frame number */
    unsigned char packet_data[YMODEM_PACKET_1K_SIZE];               /*!< ymodem frame data */
    unsigned char packet_crc[2];                                    /*!< ymodem frame crc data */
    unsigned int packet_recnum;                                     /*!< ymodem frist packet have get falg */
    unsigned int ymodem_flag;                                       /*!< ymodem frist packet have get falg */
    unsigned int file_size;                                         /*!< ymodem transform file size */
    unsigned int get_size;                                          /*!< ymodem transform file have get size */
    unsigned int down_off;                                          /*!< code download address offset */
    unsigned int down_size;                                         /*!< code download size */
    unsigned int wait_second;                                      /*!< code download size */
    ymodem_send_fun         ymodem_send;                            /*!< ymodem send a data function */
    ymodem_rx_fun           ymodem_rx;
    ymodem_packet_deal_fun  ymodem_packet_deal;                     /*!< ymodem pack deal function */
    ymodem_printf_fun       ymodem_printf;
    ymodem_flash_erase_fun  ymodem_flash_erase;
    ymodem_flash_write_fun  ymodem_flash_write;
    ymodem_strtoul_fun      ymodem_strtoul;
#ifndef SECURITY_PROTECT_DISABLE
    ymodem_fwdg_reload_fun  ymodem_fwdg_reload;
#endif /* SECURITY_PROTECT_DISABLE */
}ymodem_packet_str;

ymodem_flag_enum ymodem_receive_packet(ymodem_packet_str *packet_str);
void ymodem_image_erase(uint32_t offset, int len);
void ymodem_download_check(ymodem_packet_str *packet_str, delay_ms_fun delay_ms);
void ymodem_packet_deal(ymodem_packet_str *packet_str);

void uart_putc(uint8_t c);
uint8_t uart_rx_to(ymodem_packet_str *ymodem_packet, uint16_t len, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif    /* YMODEM_H */

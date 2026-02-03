/*!
    \file    ymodem.c
    \brief   ymodem configure for for GD32 SDK

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

#include "ymodem.h"

static uint32_t ymodem_file_size_get(ymodem_packet_str *packet_str);

/*!
    \brief    ymodem crc16 calculation function,for YMODEM it is CRC16-CCIT
    \param[in]  crc_init: the initial value of CRC16, for CRC16-CCIT it is 0x0000U
    \param[in]  addr: the address of CRC16 input data
    \param[in]  len: the length input data of crc16 which need to be calculated
    \param[out] none
    \retval     the result of crc16 calculation(0-0xffff)
*/
static uint16_t crc16_ccitt(uint16_t crc_init, uint8_t *addr, uint32_t len)
{
    uint8_t data;
    uint16_t crc = crc_init;
    int i;
    for (; len > 0; len--) {
        data = *addr++;

        crc = crc ^ (data << 8);
        for (i = 0; i < 8; i++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }

    crc = crc^0x0000;
    return crc; 
}

ymodem_flag_enum ymodem_receive_packet(ymodem_packet_str *packet_str)
{
    uint32_t packet_len = 0;
    ymodem_flag_enum status = YMODEM_FLAG_GET;
    uint16_t crc_result = 0;

    switch (packet_str->packet_flag) {
        /* data length is 128 */
        case YMODEM_SOH:
            packet_str->packet_crc[0] = packet_str->packet_data[YMODEM_PACKET_SIZE];
            packet_str->packet_crc[1] = packet_str->packet_data[YMODEM_PACKET_SIZE+1];
            packet_len = YMODEM_PACKET_SIZE;
        break;
        /* data length is 1024 */
        case YMODEM_STX:
            packet_len = YMODEM_PACKET_1K_SIZE;
            break;
        case YMODEM_EOT:
                status |= YMODEM_FLAG_EOT;
                return status;
            break;
        case YMODEM_CA:
            /* Abort by sender */
            if(YMODEM_CA == packet_str->packet_num) {
                packet_len = 2;
            } else {
                status |= YMODEM_FLAG_CAG;
            }
            return status;
            break;
        default:
            status |= YMODEM_FLAG_HDRE;
            return status;
            break;
    }

    /* last packet do not care */
    if((packet_str->ymodem_flag & YMODEM_FLAG_EOT) != 0) {
        return status;
    }

    if ((packet_str->packet_num & 0xff) != (0xff & packet_str->packet_recnum)) {
        status |= YMODEM_FLAG_NUME;
        return status;
    } else {
        packet_str->packet_recnum += 1;
    }

    /* crc check do now */
    crc_result = crc16_ccitt(0x0000, ((uint8_t *)packet_str+3), packet_len+2);
    if(crc_result != 0) {
        status |= YMODEM_FLAG_CRCE;
        return status;
    }

    /* first packet include file name and size */
    if(0 == packet_str->packet_num && 0 == (packet_str->ymodem_flag&YMODEM_FLAG_FPT)) {
        packet_str->file_size = ymodem_file_size_get(packet_str);
        status |= YMODEM_FLAG_FPT;
    }

    return status;
}

void ymodem_packet_deal(ymodem_packet_str *packet_str)
{
    int packet_length = 0;

    if (YMODEM_SOH == packet_str->packet_flag) {
        packet_length = YMODEM_PACKET_SIZE;
    } else {
        packet_length = YMODEM_PACKET_1K_SIZE;
    }

    /* last packet is get no need save this packet data */
    if(packet_str->get_size >= packet_str->file_size) {
        return;
    }

    if(packet_length < packet_str->file_size - packet_str->get_size) {
        packet_str->ymodem_flash_write(packet_str->down_off + packet_str->get_size, (int8_t *)(packet_str->packet_data), packet_length);
    } else {    /* last packet */
        packet_str->ymodem_flash_write(packet_str->down_off + packet_str->get_size, (int8_t *)(packet_str->packet_data), packet_str->file_size - packet_str->get_size); 
    }

    packet_str->get_size += packet_length;
}

/*!
    \brief      if get any data form uart in 5 seconds, will start download
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ymodem_download_check(ymodem_packet_str *packet_str, delay_ms_fun delay_ms)
{
    uint32_t i = 0, errors = 0;
    static int eot_state = 0;
    ymodem_flag_enum packet_status  = 0;
    packet_str->ymodem_flag &= ~YMODEM_FLAG_ACT;

    /* step 1 wait for activate ymodem down */
    /* wait 5S for activate ymodem down */
    packet_str->ymodem_printf("wait for ymodem download \033[1;40;31m%dS\033[0m", packet_str->wait_second);
    while(packet_str->ymodem_rx(packet_str, 1, 10000)==0 && i<packet_str->wait_second){
        i++;
        delay_ms(1000);
#ifndef SECURITY_PROTECT_DISABLE
        packet_str->ymodem_fwdg_reload();
#endif /* SECURITY_PROTECT_DISABLE */
        packet_str->ymodem_printf("\b\b\033[1;40;31m%dS\033[0m", packet_str->wait_second-i); 
    }
    packet_str->ymodem_printf("\r\n");

    /* step 2 if not activate return */
    if (i>=packet_str->wait_second) {
        return;
    }

    /* step 2 erase erea move after get file size */
    packet_str->ymodem_printf("ymodem start download.\r\n" );

    /* step 3 send YMODEM_C until start transmission */
    do {
        packet_str->ymodem_send(YMODEM_C);
#ifndef SECURITY_PROTECT_DISABLE
        packet_str->ymodem_fwdg_reload();
#endif /* SECURITY_PROTECT_DISABLE */
    } while(packet_str->ymodem_rx(packet_str, YMODEM_PACKET_1K_SIZE+YMODEM_PACKET_OVERHEAD, RECEIVE_TIMEOUT_C)==0); 

    /* step 4 deal packet */
    while (1) {
        packet_status = ymodem_receive_packet(packet_str);
#ifndef SECURITY_PROTECT_DISABLE
        packet_str->ymodem_fwdg_reload();
#endif /* SECURITY_PROTECT_DISABLE */
        switch (packet_status) {
            /* frist packet */
            case YMODEM_FLAG_FPT:
                if(packet_str->file_size > packet_str->down_size) {
                    packet_str->ymodem_send(YMODEM_CA);
                    packet_str->ymodem_send(YMODEM_CA);
                    return ;
                } else {
                    /* erase erea here after get file size */
                     packet_str->ymodem_flash_erase(packet_str->down_off, packet_str->file_size);
                    /* ymodem start transmission */
                    packet_str->ymodem_flag |= YMODEM_FLAG_STR;
                    packet_str->ymodem_flag |= YMODEM_FLAG_FPT;
                }
                packet_str->ymodem_send(YMODEM_ACK);
                packet_str->ymodem_send(YMODEM_C);
                break;
            /* normal packet */
            case YMODEM_FLAG_GET:
                ymodem_packet_deal(packet_str);
                /* if is last packet, stop transmission */
                if((packet_str->ymodem_flag & YMODEM_FLAG_EOT) != 0) {
                    packet_str->ymodem_flag |= YMODEM_FLAG_STP; 
                }
                packet_str->ymodem_send(YMODEM_ACK);
                break;
            /* end */
            case YMODEM_FLAG_EOT:
                if (eot_state == 0) {
                    eot_state += 1;
                    packet_str->ymodem_send(YMODEM_NACK);
                } else {
                    packet_str->ymodem_send(YMODEM_ACK);
                    packet_str->ymodem_send(YMODEM_C);
                    packet_str->ymodem_flag |= YMODEM_FLAG_EOT;
                    eot_state = 0;
                }
                break;
            case YMODEM_FLAG_CAG:
                packet_str->ymodem_flag |= YMODEM_FLAG_CAG;
                packet_str->ymodem_flag |= YMODEM_FLAG_STP;
                break;
            case YMODEM_FLAG_NUME:
            case YMODEM_FLAG_CRCE:
            case YMODEM_FLAG_HDRE:
                errors++;
                if(errors > MAX_PACKET_ERROR_NUMBER) {
                    packet_str->ymodem_send(YMODEM_CA);
                    packet_str->ymodem_send(YMODEM_CA);
                    packet_str->ymodem_flag |= YMODEM_FLAG_EMAX;
                    packet_str->ymodem_flag |= YMODEM_FLAG_STP;
                } else {
                    /* transmission again */
                    packet_str->ymodem_send(YMODEM_NACK);
                }
                break;
            default:
                packet_str->ymodem_send(YMODEM_NACK);
                break;
        }
        if((packet_str->ymodem_flag & YMODEM_FLAG_STP) != 0) {
              delay_ms(500);
            break;
        }
        while(packet_str->ymodem_rx(packet_str, YMODEM_PACKET_1K_SIZE+YMODEM_PACKET_OVERHEAD, 1000000)==0);
    }

}

/*!
    \brief      get the file size from frist packet
    \param[in]  struct of ymodem packet
    \param[out] none
    \retval     size of file
*/
static uint32_t ymodem_file_size_get(ymodem_packet_str *packet_str)
{
    int size_start = 0;
    uint32_t file_size;
    for(size_start=0; size_start<YMODEM_PACKET_SIZE; size_start++) {
        if (packet_str->packet_data[size_start] == '\0') {
            break;
        }
    }

    file_size = packet_str->ymodem_strtoul((const char*)(packet_str->packet_data + size_start+1), 0, 10);
    return file_size;
}

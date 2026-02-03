/*!
    \file    enet_adapter.c
    \brief   enet adapter

    \version 2024-12-27, V1.2.0, demo for GD32F5xx
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

#include "netconf.h"
#include "main.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"

#include "Source/APP_Source/Common/sys_init.h"

#define SYSTEMTICK_PERIOD_MS  1
#define CLOCK_SWITCH_TIMEOUT       ((uint32_t)1000uL)

__IO uint32_t g_localtime = 0; /* for creating a time reference incremented by 10ms */
uint32_t g_timedelay;

/*!
    \brief      after the netif is fully configured, it will be called to initialize the function of telnet, client and udp
    \param[in]  netif: the struct used for lwIP network interface
    \param[out] none
    \retval     none
*/
void lwip_netif_status_callback(struct netif *netif)
{
    if(((netif->flags & NETIF_FLAG_UP) != 0) && (0 != netif->ip_addr.addr)) {
        /* initilaize the tcp server: telnet 8000 */
//        hello_gigadevice_init();
        /* initilaize the tcp client: echo 10260 */
//        tcp_client_init();
        /* initilaize the udp: echo 1025 */
//        udp_echo_init();
    }
}

/*!
    \brief      insert a delay time
    \param[in]  ncount: number of 10ms periods to wait for
    \param[out] none
    \retval     none
*/
void delay_10ms(uint32_t ncount)
{
    /* capture the current local time */
    g_timedelay = g_localtime + ncount*10;

    /* wait until the desired delay finish */
    while(g_timedelay > g_localtime) {
    }
}

/*!
    \brief      updates the system local time
    \param[in]  none
    \param[out] none
    \retval     none
*/
void time_update(void)
{
    g_localtime += SYSTEMTICK_PERIOD_MS;
}

void _sys_exit(int x)
{
    x = x;
}

void enet_sys_init(void)
{
    /* setup ethernet system(GPIOs, clocks, MAC, DMA, systick) */
    enet_system_setup();

    /* initilaize the LwIP stack */
    lwip_stack_init();

#ifdef USE_IAP_TFTP
    /* Initialize the TFTP server */
//    iap_tftp_init();
#endif
}

SYS_INIT(enet_sys_init)
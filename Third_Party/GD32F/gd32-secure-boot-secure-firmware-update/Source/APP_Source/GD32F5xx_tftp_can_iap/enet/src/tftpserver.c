/*!
    \file  tftpserver.c
    \brief IAP demo 
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-08-15, V1.0.0, demo for GD32F4xx
*/

#include "tftpserver.h"
#include <string.h>
#include <stdio.h>
#include "main.h"

#include "Source/IBL_Source/ibl_export.h"
#include "Source/IBL_Source/ibl_def.h"
#include "Utilities/Third_Party/mcuboot-main/boot/gigadevice/flash_hal/flash_layout.h"

#ifdef USE_IAP_TFTP

static uint32_t s_flash_write_addr;
static struct udp_pcb *udppcb;
int16_t last_packet_falg = 0;

static void tftp_fill_pkt_opcode(char *buf, tftp_opcode op);
static void tftp_fill_pkt_block(char* buf, uint16_t block);
static tftp_opcode tftp_pick_pkt_opcode(char *buf);
static uint16_t tftp_pick_pkt_block(char *buf);

static void app_flash_write(__IO uint32_t *addr, uint32_t* data ,uint16_t len);
static int8_t app_flash_erase(void);

static void tftp_request_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port);
static void tftp_write_request_ack(struct udp_pcb *pcb, ip_addr_t *addr, int port);
static void tftp_data_to_write_recv(void *args, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port);
static err_t tftp_ackpkt_send(struct udp_pcb *pcb, ip_addr_t *addr, int port, int block);

static void tftp_reset_for_request(struct udp_pcb *upcb, int *args);

/*!
    \brief      pick out the tftp opcode
    \param[in]  buf: pointer to the tftp packet
    \param[out] none 
    \retval     the tftp opcode
*/
static tftp_opcode tftp_pick_pkt_opcode(char *buf)
{
    return (tftp_opcode)(buf[1]);
}

/*!
    \brief      pick out the tftp block num
    \param[in]  buf: pointer to the tftp packet
    \param[out] none  
    \retval     the tftp block num
*/
static uint16_t tftp_pick_pkt_block(char *buf)
{
    uint16_t *p = (uint16_t *)buf;
    return ntohs(p[1]);
}

/*!
    \brief      fill in the tftp opcode
    \param[in]  buf: pointer to the tftp packet
    \param[in]  op: the tftp opcode
    \param[out] none
    \retval     none
*/
static void tftp_fill_pkt_opcode(char *buf, tftp_opcode op)
{
    buf[0] = 0;
    buf[1] = (uint8_t)op;
}

/*!
    \brief      fill in the tftp block num
    \param[in]  buf: pointer to the tftp packet
    \param[in]  block: the tftp block num
    \param[out] none
    \retval     none
*/
static void tftp_fill_pkt_block(char* buf, uint16_t block)
{
    uint16_t *p = (uint16_t *)buf;
    p[1] = htons(block);
}

/*!
    \brief      erase the defined app used flash area 
    \param[in]  none
    \param[out] none
    \retval     0 if success, -1 if error
*/
static int8_t app_flash_erase(void)
{
    uint32_t page_start = RE_FLASH_BASE-FLASH_BASE+RE_IMG_1_APP_OFFSET;
    int len = FLASH_PARTITION_SIZE;

    ibl_flash_erase(page_start, len);

    return 0;
}

/*!
    \brief      write data to the defined app used flash area in words 
    \param[in]  addr: start address for writing data buffer
    \param[in]  data: pointer to data buffer to write
    \param[in]  len: length of data in words
    \param[out] none
    \retval     none
*/
static void app_flash_write(__IO uint32_t *addr, uint32_t* data , uint16_t len)
{
    ibl_flash_write(*addr-FLASH_BASE, data, len);
}

/*!
    \brief      called when a tftp request is received on port 69
    \param[in]  args: the user argument
    \param[in]  pcb: the udp_pcb that has received the data
    \param[in]  p: the packet buffer
    \param[in]  addr: the IP address to send packet to
    \param[in]  port: the port number to send packet to
    \param[out] none
    \retval     none
*/
static void tftp_request_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
    tftp_opcode opcode;
    struct udp_pcb *pcb_data;
    err_t err;

    /* create a new UDP control block */
    pcb_data = udp_new();
    if (!pcb_data){
        return;
    }
    /* tftp file data transmission is working on another port, here works on port 1025 */
    err = udp_bind(pcb_data, IP_ADDR_ANY, 1025);
    if (err != ERR_OK){
        return;
    }

    /* get the operation code from the tftp packet */
    opcode = tftp_pick_pkt_opcode(p->payload);    
    switch(opcode){
    case TFTP_WRITE:
        /* if packet is tftp write request, return ack packet and start to receive data */
        tftp_write_request_ack(pcb_data, addr, port);
        break;
    
    case TFTP_READ:

    default:
        udp_remove(pcb_data);
        break;
    }

    pbuf_free(p);
}

/*!
    \brief      process tftp write request
    \param[in]  pcb: the udp_pcb that has received the data
    \param[in]  addr: the IP address to send packet to
    \param[in]  port: the port number to send packet to
    \param[out] none
    \retval     none
*/
static void tftp_write_request_ack(struct udp_pcb *pcb, ip_addr_t *addr, int port)
{
    /* save for parameter: packet block num */
    int *args = NULL;

    args = mem_malloc(sizeof *args);
    if (!args){
        tftp_reset_for_request(pcb, args);
        return;
    }

    /* the block num for ack to write request is 0 */
    *args = 0;

    ibl_trace(IBL_ALWAYS, "start erase\r\n");
    /* erase user app used flash area */
    app_flash_erase();
    ibl_trace(IBL_ALWAYS, "end erase\r\n");
 
    s_flash_write_addr = RE_FLASH_BASE + RE_IMG_1_APP_OFFSET;  
    
    /* set callback for reception of the tftp file data */
    udp_recv(pcb, (udp_recv_fn)tftp_data_to_write_recv, args);
 
    /* ack to the write request packet */
    tftp_ackpkt_send(pcb, addr, port, *args);
}

/*!
    \brief      close the data transmit connection to 1025 port
    \param[in]  pcb: the udp_pcb
    \param[in]  args: the user argument
    \param[out] none
    \retval     none
*/
static void tftp_reset_for_request(struct udp_pcb *pcb, int *args)
{
    mem_free(args);

    udp_disconnect(pcb);
    udp_remove(pcb);
  
    /* reset to wait for request packet reception */
    udp_recv(udppcb, (udp_recv_fn)tftp_request_recv, NULL);
}


/*!
    \brief      called when a tftp file data(after a write request) is received on port 1025
    \param[in]  args: the user argument
    \param[in]  pcb: the udp_pcb that has received the data
    \param[in]  p: the packet buffer
    \param[in]  addr: the IP address to send packet to
    \param[in]  port: the port number to send packet to
    \param[out] none
    \retval     none
*/
static void tftp_data_to_write_recv(void *args, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
    int *block = (int *)args;
    uint16_t  wr_words = 0;
    uint16_t data_len;

    if (p->len != p->tot_len){
        return;
    }

    /* when the received packet block num is what we want */
    if (tftp_pick_pkt_block(p->payload) == (*block + 1)){
        /* update to the latest received block number */
        (*block)++;
    
        /* if the received data packet has data to write */
        if (p->len > TFTP_DATA_PKT_HDR){ 

            data_len = p->len - TFTP_DATA_PKT_HDR;
            /* write received file data to flash */
            app_flash_write(&s_flash_write_addr, p->payload+TFTP_DATA_PKT_HDR, data_len);
            s_flash_write_addr += data_len;

        }
    }
 
    /* send the ack packet */
    tftp_ackpkt_send(pcb, addr, port, *block);   

    /* judge if the latest written data is the last data packet or not(tftp works in this way) */
    if (p->len < TFTP_DATA_PKT_MAX){
        tftp_reset_for_request(pcb, args);
        last_packet_falg = 1;
    }

    pbuf_free(p);
}

/*!
    \brief      generate and send the tftp ack packet
    \param[in]  pcb: the udp_pcb that has received the data
    \param[in]  addr: the IP address to send packet to
    \param[in]  port: the port number to send packet to
    \param[in]  block: the block num to fill in the ack packet
    \param[out] none
    \retval     err_t: error value
*/
static err_t tftp_ackpkt_send(struct udp_pcb *pcb, ip_addr_t *addr, int port, int block)
{
    err_t err;
    struct pbuf *p;
    char pkt[TFTP_ACK_PKT];

    /* fill in the option code to the packet */
    tftp_fill_pkt_opcode(pkt, TFTP_ACK);
    /* fill in the block num to the packet: for write request, block num -- 0; for data packet,
       block num -- received data packet block num */
    tftp_fill_pkt_block(pkt, block);

    /* copy the ack packet generated to the allocated pbuf */
    p = pbuf_alloc(PBUF_TRANSPORT, TFTP_ACK_PKT, PBUF_POOL);
    if (!p){
        return ERR_MEM;
    }
    memcpy(p->payload, pkt, TFTP_ACK_PKT);

    /* send ack packet */
    err = udp_sendto(pcb, p, addr, port);

    pbuf_free(p);
    return err;
}

/*!
    \brief      initialize the tftp application
    \param[in]  none
    \param[out] none
    \retval     none
*/
void iap_tftp_init(void)
{
    err_t err;

    /* create a new UDP control block */
    udppcb = udp_new();
    if (!udppcb){
        return ;
    }

    /* bind this pcb to port 69, port 69 is used for tftp request decode */
    err = udp_bind(udppcb, IP_ADDR_ANY, 69);
    if (ERR_OK == err){
        udp_recv(udppcb, (udp_recv_fn)tftp_request_recv, NULL);
    } 
}

#endif


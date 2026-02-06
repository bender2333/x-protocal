#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);
void ethernetif_input(void *pvParameters);

/* 获取以太网 netif 指针 (供 TPMesh 模块使用) */
struct netif *ethernetif_get_netif(void);

#endif

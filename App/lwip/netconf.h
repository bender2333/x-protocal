#ifndef __NETCONF_H__
#define __NETCONF_H__
#include "gd32f5xx.h"

void lwip_stack_init(void);
void lwip_frame_recv(void);
void lwip_timeouts_check(__IO uint32_t curtime);
#endif
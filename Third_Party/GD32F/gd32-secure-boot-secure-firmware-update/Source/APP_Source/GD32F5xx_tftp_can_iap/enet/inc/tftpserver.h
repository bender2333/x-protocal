/*!
    \file  tftpserver.h
    \brief the header file of tftpserver 
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-08-15, V1.0.0, demo for GD32F4xx
*/

#ifndef TFTPSERVER_H_
#define TFTPSERVER_H_

#include "lwip/mem.h"
#include "lwip/udp.h"


#define TFTP_OPCODE_LEN         2
#define TFTP_BLKNUM_LEN         2
#define TFTP_DATA_LEN_MAX       512
#define TFTP_DATA_PKT_HDR      (TFTP_OPCODE_LEN + TFTP_BLKNUM_LEN)
#define TFTP_ACK_PKT           (TFTP_OPCODE_LEN + TFTP_BLKNUM_LEN)
#define TFTP_DATA_PKT_MAX      (TFTP_DATA_PKT_HDR + TFTP_DATA_LEN_MAX)

/* tftp opcodes as specified in rfc1350 */
typedef enum {
  TFTP_READ  = 1,
  TFTP_WRITE = 2,
  TFTP_DATA  = 3,
  TFTP_ACK   = 4,
  TFTP_ERROR = 5
} tftp_opcode;


void iap_tftp_init(void);

#endif /* TFTPSERVER_H_ */

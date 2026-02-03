/**
  ******************************************************************************
  * @file    can_bootloader.h
  * $Author: wdluo $
  * $Revision: 17 $
  * $Date:: 2012-07-06 11:16:48 +0800 #$
  * @brief   基于CAN总线的Bootloader程序.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, ViewTool</center>
  *<center><a href="http:\\www.viewtool.com">http://www.viewtool.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_update_H
#define __CAN_update_H
/* Includes ------------------------------------------------------------------*/
#include "gd32f5xx.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

typedef  void (*pfunction)(void);

#define APPLICATIONADDRESS         ((uint32_t)(0x08000000+RE_IMG_1_APP_OFFSET))  /* User start code space */

//固件类型值定义
#define FW_TYPE_BOOT                0x55
#define FW_TYPE_APP                 0xAA
//定义当前固件类型为BOOT
#define FW_TYPE         FW_TYPE_BOOT
//定义数据收发的帧ID，必须跟上位机软件配置一致，否则无法正常工作。
//对于CAN总线，数据收发ID可以定义为一个ID，也可以定义为不同的ID
#define MSG_RECEIVE_ID              0x3C
#define MSG_SEND_ID                 0x3D
//定义数据收发帧ID类型,0-标准帧，1-扩展帧
#define MSG_ID_TYPE                 0
//定义节点广播地址
#define NAD_BROADCAST               0x7F
//BOOT命令定义
#define CMD_GET_FW_INFO             0x80
#define CMD_ENTER_BOOT              0xC1
#define CMD_ERASE_APP               0x42
#define CMD_SET_ADDR_OFFSET         0x03
#define CMD_TRAN_DATA               0xC4
#define CMD_WRITE_DATA              0x85
#define CMD_ENTER_APP               0x06
//BOOT错误定义
#define CAN_BOOT_ERR_SUCCESS        0   //没有错误
#define CAN_BOOT_ERR_ERASE          1   //固件擦除出错
#define CAN_BOOT_ERR_ERASE_IN_APP   2   //当前模式为APP，不能擦除固件
#define CAN_BOOT_ERR_WRITE_OUTRANGE 3   //当前地址超出了正常的地址范围
#define CAN_BOOT_ERR_WRITE_IN_APP   4   //当前模式不能写入固件数据
#define CAN_BOOT_ERR_WRITE          5   //数据写入程序存储器出错
#define CAN_BOOT_ERR_WRITE_OUT_ADDR 6   //数据长度超出了程序存储器范围 
#define CAN_BOOT_ERR_TRAN_CRC       7   //数据传输CRC校验出错
#define CAN_BOOT_ERR_WRITE_CRC      8   //数据写入芯片CRC校验出错


/* base address of the FMC sectors */
#define ADDR_FMC_SECTOR_0       ((uint32_t)0x08000000) /*!< base address of sector 0,  16 kbytes */
#define ADDR_FMC_SECTOR_1       ((uint32_t)0x08004000) /*!< base address of sector 1,  16 kbytes */
#define ADDR_FMC_SECTOR_2       ((uint32_t)0x08008000) /*!< base address of sector 2,  16 kbytes */
#define ADDR_FMC_SECTOR_3       ((uint32_t)0x0800C000) /*!< base address of sector 3,  16 kbytes */
#define ADDR_FMC_SECTOR_4       ((uint32_t)0x08010000) /*!< base address of sector 4,  64 kbytes */
#define ADDR_FMC_SECTOR_5       ((uint32_t)0x08020000) /*!< base address of sector 5, 128 kbytes */
#define ADDR_FMC_SECTOR_6       ((uint32_t)0x08040000) /*!< base address of sector 6, 128 kbytes */
#define ADDR_FMC_SECTOR_7       ((uint32_t)0x08060000) /*!< base address of sector 7, 128 kbytes */
#define ADDR_FMC_SECTOR_8       ((uint32_t)0x08080000) /*!< base address of sector 8, 128 kbytes */
#define ADDR_FMC_SECTOR_9       ((uint32_t)0x080A0000) /*!< base address of sector 9, 128 kbytes */
#define ADDR_FMC_SECTOR_10      ((uint32_t)0x080C0000) /*!< base address of sector 10, 128 kbytes */
#define ADDR_FMC_SECTOR_11      ((uint32_t)0x080E0000) /*!< base address of sector 11, 128 kbytes */
#define ADDR_FMC_SECTOR_12      ((uint32_t)0x08100000) /*!< base address of sector 12, 128 kbytes */
#define ADDR_FMC_SECTOR_13      ((uint32_t)0x08120000) /*!< base address of sector 13, 128 kbytes */
#define ADDR_FMC_SECTOR_14      ((uint32_t)0x08140000) /*!< base address of sector 14, 128 kbytes */
#define ADDR_FMC_SECTOR_15      ((uint32_t)0x08160000) /*!< base address of sector 15, 128 kbytes */
#define ADDR_FMC_SECTOR_16      ((uint32_t)0x08180000) /*!< base address of sector 16, 128 kbytes */
#define ADDR_FMC_SECTOR_17      ((uint32_t)0x081A0000) /*!< base address of sector 17, 128 kbytes */
#define ADDR_FMC_SECTOR_18      ((uint32_t)0x081C0000) /*!< base address of sector 18, 128 kbytes */
#define ADDR_FMC_SECTOR_19      ((uint32_t)0x081E0000) /*!< base address of sector 19, 128 kbytes */

#define ADDR_FMC_SECTOR_20      ((uint32_t)0x08200000) /*!< base address of sector 20, 16 kbytes */
#define ADDR_FMC_SECTOR_21      ((uint32_t)0x08400000) /*!< base address of sector 21, 16 kbytes */
#define ADDR_FMC_SECTOR_22      ((uint32_t)0x08200000) /*!< base address of sector 22, 16 kbytes */
#define ADDR_FMC_SECTOR_23      ((uint32_t)0x08200000) /*!< base address of sector 23, 16 kbytes */
#define ADDR_FMC_SECTOR_24      ((uint32_t)0x08210000) /*!< base address of sector 24, 64 kbytes */
#define ADDR_FMC_SECTOR_25      ((uint32_t)0x08220000) /*!< base address of sector 25, 128 kbytes */
#define ADDR_FMC_SECTOR_26      ((uint32_t)0x08240000) /*!< base address of sector 26, 128 kbytes */
#define ADDR_FMC_SECTOR_27      ((uint32_t)0x08260000) /*!< base address of sector 27, 128 kbytes */
#define ADDR_FMC_SECTOR_28      ((uint32_t)0x08280000) /*!< base address of sector 28, 128 kbytes */
#define ADDR_FMC_SECTOR_29      ((uint32_t)0x082A0000) /*!< base address of sector 29, 128 kbytes */
#define ADDR_FMC_SECTOR_30      ((uint32_t)0x082C0000) /*!< base address of sector 30, 128 kbytes */
#define ADDR_FMC_SECTOR_31      ((uint32_t)0x082E0000) /*!< base address of sector 31, 128 kbytes */
#define ADDR_FMC_SECTOR_32      ((uint32_t)0x08300000) /*!< base address of sector 32, 128 kbytes */
#define ADDR_FMC_SECTOR_33      ((uint32_t)0x08320000) /*!< base address of sector 33, 128 kbytes */
#define ADDR_FMC_SECTOR_34      ((uint32_t)0x08340000) /*!< base address of sector 34, 128 kbytes */
#define ADDR_FMC_SECTOR_35      ((uint32_t)0x08360000) /*!< base address of sector 35, 128 kbytes */
#define ADDR_FMC_SECTOR_36      ((uint32_t)0x08380000) /*!< base address of sector 36, 128 kbytes */
#define ADDR_FMC_SECTOR_37      ((uint32_t)0x083A0000) /*!< base address of sector 37, 128 kbytes */
#define ADDR_FMC_SECTOR_38      ((uint32_t)0x083C0000) /*!< base address of sector 38, 128 kbytes */
#define ADDR_FMC_SECTOR_39      ((uint32_t)0x083E0000) /*!< base address of sector 39, 128 kbytes */
#define ADDR_FMC_SECTOR_40      ((uint32_t)0x08400000) /*!< base address of sector 40, 256 kbytes */
#define ADDR_FMC_SECTOR_41      ((uint32_t)0x08440000) /*!< base address of sector 41, 256 kbytes */
#define ADDR_FMC_SECTOR_42      ((uint32_t)0x08480000) /*!< base address of sector 42, 256 kbytes */
#define ADDR_FMC_SECTOR_43      ((uint32_t)0x084C0000) /*!< base address of sector 43, 256 kbytes */
#define ADDR_FMC_SECTOR_44      ((uint32_t)0x08500000) /*!< base address of sector 44, 256 kbytes */
#define ADDR_FMC_SECTOR_45      ((uint32_t)0x08540000) /*!< base address of sector 45, 256 kbytes */
#define ADDR_FMC_SECTOR_46      ((uint32_t)0x08580000) /*!< base address of sector 46, 256 kbytes */
#define ADDR_FMC_SECTOR_47      ((uint32_t)0x085C0000) /*!< base address of sector 47, 256 kbytes */
#define ADDR_FMC_SECTOR_48      ((uint32_t)0x08600000) /*!< base address of sector 48, 256 kbytes */
#define ADDR_FMC_SECTOR_49      ((uint32_t)0x08640000) /*!< base address of sector 49, 256 kbytes */
#define ADDR_FMC_SECTOR_50      ((uint32_t)0x08680000) /*!< base address of sector 50, 256 kbytes */
#define ADDR_FMC_SECTOR_51      ((uint32_t)0x086C0000) /*!< base address of sector 51, 256 kbytes */
#define ADDR_FMC_SECTOR_52      ((uint32_t)0x08700000) /*!< base address of sector 52, 256 kbytes */
#define ADDR_FMC_SECTOR_53      ((uint32_t)0x08740000) /*!< base address of sector 53, 256 kbytes */

static const unsigned short crc16tab[256]= {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};
  
extern unsigned short crc16_ccitt(const unsigned char *buf, unsigned int len);

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
uint32_t GetSector(uint32_t Address);
fmc_state_enum CAN_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr);
uint16_t CAN_BOOT_GetAddrData(void);
void CAN_BOOT_ExecutiveCommand(can_receive_message_struct *pRxMessage);
void CAN_BOOT_JumpToApplication(__IO uint32_t Addr);
uint8_t GetNAD(void);
uint32_t fmc_sector_get(uint32_t addr);

#endif
/*********************************END OF FILE**********************************/

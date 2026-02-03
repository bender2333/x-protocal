/*!
    \file    ymodem_update.c
    \brief   ymodem update

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

#include "Platform/gd32xx.h"
#include "Source/APP_Source/systick.h"
#include "Utilities/Third_Party/letter-shell-shell2.x/shell_ext.h"
#include "Source/IBL_Source/ibl_export.h"
#include "Source/IBL_Source/ibl_def.h"
#include "Utilities/Third_Party/mcuboot-main/boot/gigadevice/flash_hal/flash_layout.h"
#include "Utilities/Third_Party/mcuboot-main/boot/bootutil/src/bootutil_priv.h"

#include "task.h"
#include "can_update.h"

#ifdef USE_IAP_CAN

FlagStatus can0_receive_flag;
can_receive_message_struct receive_message;

can_parameter_struct can_init_parameter;
can_filter_parameter_struct can_filter_parameter;
int16_t can_jump_falg = 0;

/*!
    \brief      initialize CAN and filter
    \param[in]  can_parameter
      \arg        can_parameter_struct
    \param[in]  can_filter
      \arg        can_filter_parameter_struct
    \param[out] none
    \retval     none
*/
void can_config(can_parameter_struct can_parameter, can_filter_parameter_struct can_filter)
{
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_INIT_STRUCT, &can_filter);
    /* initialize CAN register */
    can_deinit(CAN0);
    can_deinit(CAN1);
    
    /* initialize CAN parameters */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = ENABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_5TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_4TQ;
    
    /* 1MBps */
    can_parameter.prescaler = 5;

    /* initialize CAN */
    can_init(CAN0, &can_parameter);
    can_init(CAN1, &can_parameter);
    
    /* initialize filter */ 
    can_filter.filter_number=0;
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    
    can_filter_init(&can_filter);
    
    /* CAN1 filter number */
    can_filter.filter_number = 15;
    can_filter_init(&can_filter);
    
        can_interrupt_enable(CAN0, CAN_INT_RFNE0);
//        can_interrupt_enable(CAN1, CAN_INT_RFNE0);
}

/*!
    \brief      configure the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nvic_config(void)
{
    /* configure CAN0 NVIC */
    nvic_irq_enable(CAN0_RX0_IRQn,0,0);

    /* configure CAN1 NVIC */
    nvic_irq_enable(CAN1_RX0_IRQn,1,1);
}

/*!
    \brief      configure GPIO
    \param[in]  none
    \param[out] none
    \retval     none
*/
void can_gpio_config(void)
{
    /* enable CAN clock */
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOI);
    rcu_periph_clock_enable(RCU_GPIOH);

    /* configure CAN0 GPIO */
    gpio_output_options_set(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_mode_set(GPIOI, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_af_set(GPIOI, GPIO_AF_9, GPIO_PIN_9);

    gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_af_set(GPIOH, GPIO_AF_9, GPIO_PIN_13);
}

void can_update(void)
{
    const struct flash_area *fap;
    
    ibl_trace(IBL_ALWAYS, "APP can update start\r\n");

    /* configure NVIC */
    nvic_config();

    /* configure GPIO */
    can_gpio_config();
    /* initialize CAN and filter */
    can_config(can_init_parameter, can_filter_parameter);
    
    while(can_jump_falg == 0){
        if(can0_receive_flag){
            can0_receive_flag = RESET;
            CAN_BOOT_ExecutiveCommand(&receive_message);
        }
    }
    flash_area_open(FLASH_AREA_1_ID, &fap);
    boot_write_magic(fap);
    ibl_trace(IBL_ALWAYS, "can update success, system reboot.\r\n");
    delay_1ms(100);
    /* generate a system reset */
    NVIC_SystemReset();
}

void can_update_set(void)
{
    /* now still in interrupt service function, so we just set flag */
    shell_task_flag |= TASK_FLAG_CAN_UPDATE;
}
SHELL_EXPORT_CMD(can_update, can_update_set, can update image);

/**
  * @brief  发送一帧CAN数据
  * @param  CANx CAN通道号
  * @param  TxMessage CAN消息指针
  * @retval None
  */
uint8_t CAN_WriteData(can_trasnmit_message_struct *TxMessage)
{
    uint8_t TransmitMailbox;   
    uint32_t TimeOut=0;
    TransmitMailbox = can_message_transmit(CAN0,TxMessage);
    while(can_transmit_states(CAN0,TransmitMailbox)!=CAN_TRANSMIT_OK){
        TimeOut++;
        if(TimeOut > 100){
            return 1;
        }
        delay_1ms(1);
    }
    return 0;
}

/**
  * @brief  将数据烧写到指定地址的Flash中 。
  * @param  Address Flash起始地址。
  * @param  Data 数据存储区起始地址。
  * @param  DataNum 数据字节数。
  * @retval 数据烧写状态。
  */
fmc_state_enum CAN_BOOT_ProgramDatatoFlash(uint32_t StartAddress,uint8_t *pData,uint32_t DataNum) 
{
    fmc_state_enum FLASHStatus=FMC_READY;
    
    ibl_flash_write(StartAddress-FLASH_BASE, pData, DataNum);
    
    return    FLASHStatus;
}
/**
  * @brief  擦出指定扇区区间的Flash数据 。
  * @param  StartPage 起始扇区地址
  * @param  EndPage 结束扇区地址
  * @retval 扇区擦出状态  
  */
fmc_state_enum CAN_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr)
{
    fmc_state_enum FLASHStatus=FMC_READY;
    
    ibl_flash_erase(RE_IMG_1_APP_OFFSET, FLASH_PARTITION_SIZE);
    
    return FLASHStatus;
}

/**
  * @brief  控制程序跳转到指定位置开始执行 。
  * @param  Addr 程序执行地址。
  * @retval 程序跳转状态。
  */
void CAN_BOOT_JumpToApplication(uint32_t Addr)
{
    static pfunction Jump_To_Application;
    __IO uint32_t JumpAddress; 
    /* Test if user code is programmed starting from address "ApplicationAddress" */
    if (((*(__IO uint32_t*)Addr) & 0x2FFE0000 ) == 0x20000000)
    { 
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t*) (Addr + 4);
        Jump_To_Application = (pfunction) JumpAddress;
//        __set_PRIMASK(1);//关闭所有中断
        rcu_periph_clock_disable(RCU_CAN0);
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*)Addr);
        Jump_To_Application();
    }
}
/**
  * @brief  获取CAN节点地址，该函数根据自己的实际情况进行修改
  * @param  None
  * @retval None
  */
uint8_t GetNAD(void)
{
    uint8_t NAD=0x01;
    //根据芯片唯一序号来合成一个NAD
//    uint32_t sn = *(__IO uint32_t*)(0x1FFFF7E8)+*(__IO uint32_t*)(0x1FFFF7EC)+*(__IO uint32_t*)(0x1FFFF7F0);
//    NAD = (sn>>24)+(sn>>16)+(sn>>8)+sn;

    return NAD&0x7F;
}

/**
  * @brief  执行主机下发的命令
  * @param  pRxMessage CAN总线消息
  * @retval 无
  */
void CAN_BOOT_ExecutiveCommand(can_receive_message_struct *pRxMessage)
{
    can_trasnmit_message_struct TxMessage;
    uint8_t ret,i;
    uint8_t *pData;
    uint8_t PCI = pRxMessage->rx_data[1];
    uint8_t SID=0xFF;
    uint8_t CMD=0xFF;
    static uint8_t OLD_CMD = 0xFF;
    uint16_t crc_data;
    static uint8_t CF_Index=1;
    static uint32_t start_addr = APPLICATIONADDRESS;
    static uint32_t data_size=0;
    static uint32_t data_index=0;
    static uint32_t addr_offset=0;
    static uint8_t data_temp[1024];
    
    can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &TxMessage);

    //根据消息获取SID和CMD
    if((PCI&0xF0)==0x00){
        SID = pRxMessage->rx_data[2];
        CMD = pRxMessage->rx_data[3];
    }else if((PCI&0xF0)==0x10){
        SID = pRxMessage->rx_data[3];
        CMD = pRxMessage->rx_data[4];
    }
    //准备发送的数据
    TxMessage.tx_dlen = 8;
    if(MSG_ID_TYPE == CAN_FF_STANDARD){
        TxMessage.tx_sfid = MSG_SEND_ID;
        TxMessage.tx_ff = CAN_FF_STANDARD;
    }else{
        TxMessage.tx_efid = MSG_SEND_ID;
        TxMessage.tx_ff = CAN_FF_EXTENDED;
    }
    TxMessage.tx_ft = CAN_FT_DATA;
    TxMessage.tx_data[0] = GetNAD();//填充NAD
    //解析命令
    switch(CMD)
    {
        case CMD_GET_FW_INFO:
            OLD_CMD = CMD_GET_FW_INFO;
            TxMessage.tx_data[1] = 0x06;
            TxMessage.tx_data[2] = SID+0x40;
            TxMessage.tx_data[3] = FW_TYPE;     // Boot
            TxMessage.tx_data[4] = 0;//固件版本号 Major
            TxMessage.tx_data[5] = 0;//固件版本号 Minor
            TxMessage.tx_data[6] = 0;//固件版本号 Revision
            TxMessage.tx_data[7] = 1;//固件版本号 Build
            CAN_WriteData(&TxMessage);
            break;
        case CMD_ENTER_BOOT:
            OLD_CMD = CMD_ENTER_BOOT;
            if(FW_TYPE == FW_TYPE_APP){
                // Do nothing;
            }
            break;
        case CMD_ERASE_APP:
            OLD_CMD = CMD_ERASE_APP;
            if(FW_TYPE == FW_TYPE_APP){
                TxMessage.tx_data[1] = 0x06;
                TxMessage.tx_data[2] = SID+0x40;
                TxMessage.tx_data[3] = CAN_BOOT_ERR_ERASE_IN_APP;
            }else{
                uint32_t EraseSize = (pRxMessage->rx_data[4]<<24)|(pRxMessage->rx_data[5]<<16)|(pRxMessage->rx_data[6]<<8)|(pRxMessage->rx_data[7]<<0);
                TxMessage.tx_data[1] = 0x06;
                TxMessage.tx_data[2] = SID+0x40;
                __set_PRIMASK(1);
                fmc_unlock();
                ret = CAN_BOOT_ErasePage(APPLICATIONADDRESS,APPLICATIONADDRESS+EraseSize);
                fmc_lock();
                __set_PRIMASK(0);
                if(ret==FMC_READY){
                    TxMessage.tx_data[3] = CAN_BOOT_ERR_SUCCESS;
                }else{
                    TxMessage.tx_data[3] = CAN_BOOT_ERR_ERASE;
                }
            }
            TxMessage.tx_data[4] = 0xFF;TxMessage.tx_data[5] = 0xFF;TxMessage.tx_data[6] = 0xFF;TxMessage.tx_data[7] = 0xFF;
            CAN_WriteData(&TxMessage);
            break;
        case CMD_SET_ADDR_OFFSET:
            OLD_CMD = CMD_SET_ADDR_OFFSET;
            TxMessage.tx_data[1] = 0x06;
            TxMessage.tx_data[2] = SID+0x40;
            if(FW_TYPE == FW_TYPE_APP){
                TxMessage.tx_data[3] = CAN_BOOT_ERR_WRITE_IN_APP;
                TxMessage.tx_data[4] = 0xFF;TxMessage.tx_data[5] = 0xFF;TxMessage.tx_data[6] = 0xFF;TxMessage.tx_data[7] = 0xFF;
            }else{
                data_size = 0;
                data_index = 0;
                addr_offset = (pRxMessage->rx_data[4]<<24)|(pRxMessage->rx_data[5]<<16)|(pRxMessage->rx_data[6]<<8)|(pRxMessage->rx_data[7]<<0);
                start_addr = addr_offset+APPLICATIONADDRESS;
                if(start_addr<APPLICATIONADDRESS){
                    TxMessage.tx_data[3] = CAN_BOOT_ERR_WRITE_OUTRANGE;
                }else{
                    uint16_t buffer_len=sizeof(data_temp);
                    TxMessage.tx_data[3] = CAN_BOOT_ERR_SUCCESS;
                    TxMessage.tx_data[4] = buffer_len>>8;
                    TxMessage.tx_data[5] = buffer_len&0xFF;
                    TxMessage.tx_data[6] = 0xFF;TxMessage.tx_data[7] = 0xFF;
                }
            }
            CAN_WriteData(&TxMessage);
            break;
        case CMD_TRAN_DATA:
            OLD_CMD = CMD_TRAN_DATA;
            if((OLD_CMD == CMD_TRAN_DATA)||(OLD_CMD == CMD_SET_ADDR_OFFSET)){
                if((PCI&0xF0)==0x00){                   // 数据小于等于4字节
                    data_size = (PCI&0xF)-2;
                    pData = &pRxMessage->rx_data[4];
                    while(data_index<data_size){
                        data_temp[data_index++] = *pData++;
                    }
                }else if((PCI&0xF0)==0x10){             // 数据大于4字节时的第一包数据
                    CF_Index = 1;
                    data_size = (((PCI&0xF)<<8)|pRxMessage->rx_data[2])-2;
                    pData = &pRxMessage->rx_data[5];
                    for(i=0;i<3;i++){
                        data_temp[data_index++] = *pData++;
                    }
                }
            }
            break;
        case CMD_WRITE_DATA:
            OLD_CMD = CMD_WRITE_DATA;
            TxMessage.tx_data[1] = 0x06;
            TxMessage.tx_data[2] = SID+0x40;
            if(FW_TYPE == FW_TYPE_APP){
                TxMessage.tx_data[3] = CAN_BOOT_ERR_WRITE_IN_APP;
                TxMessage.tx_data[4] = 0xFF;TxMessage.tx_data[5] = 0xFF;TxMessage.tx_data[6] = 0xFF;TxMessage.tx_data[7] = 0xFF;
            }else{
                crc_data = (pRxMessage->rx_data[4]<<8)|(pRxMessage->rx_data[5]<<0);
                if((data_size != data_index)||(crc_data != crc16_ccitt(data_temp,data_index))){
                    TxMessage.tx_data[3] = CAN_BOOT_ERR_TRAN_CRC;
                }else{
                    __set_PRIMASK(1);
                    fmc_unlock();
                    ret = CAN_BOOT_ProgramDatatoFlash(start_addr,data_temp,data_size);
                    fmc_lock();	
                    __set_PRIMASK(0);
                    if(ret==FMC_READY){
                        crc_data = crc16_ccitt((const unsigned char*)(data_temp),data_size);
                        //再次对写入Flash中的数据进行CRC校验，确保写入Flash的数据无误
                        if(crc_data!=crc16_ccitt((const unsigned char*)(start_addr),data_size)){
                            TxMessage.tx_data[3] = CAN_BOOT_ERR_WRITE_CRC;
                        }else{
                            TxMessage.tx_data[3] = CAN_BOOT_ERR_SUCCESS;
                        }
                    }else{
                        TxMessage.tx_data[3] = CAN_BOOT_ERR_WRITE;
                    }
                }
            }
            TxMessage.tx_data[4] = 0xFF;TxMessage.tx_data[5] = 0xFF;TxMessage.tx_data[6] = 0xFF;TxMessage.tx_data[7] = 0xFF;
            CAN_WriteData(&TxMessage);
            break;
        case CMD_ENTER_APP:
            OLD_CMD = CMD_ENTER_APP;
            if(FW_TYPE == FW_TYPE_BOOT){
                if((*((uint32_t *)APPLICATIONADDRESS)!=0xFFFFFFFF)){
//                    CAN_BOOT_JumpToApplication(APPLICATIONADDRESS);
                    can_jump_falg = 1;
                }
            }
            break;
        case 0xFF:                                                          // 默认值为0xFF
            if(OLD_CMD == CMD_TRAN_DATA){
                if(((PCI&0xF0)==0x20)&&((CF_Index&0xF)==(PCI&0xF))){        // 数据大于4字节时的后续数据包
                    CF_Index++;
                    pData = &pRxMessage->rx_data[2];
                    if((data_index+6)<=data_size){
                        for(i=0;i<6;i++){
                        data_temp[data_index++] = *pData++;
                        }
                    }else{
                        while(data_index<data_size){
                        data_temp[data_index++] = *pData++;
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
}

/*!
    \brief      gets the sector of a given address
    \param[in]  address: a given address(0x08000000~0x0877FFFF)
    \param[out] none
    \retval     the sector of a given address
*/
uint32_t fmc_sector_get(uint32_t addr)
{
    uint32_t sector = 0;

    if ((addr < ADDR_FMC_SECTOR_1) && (addr >= ADDR_FMC_SECTOR_0)) {
        sector = CTL_SECTOR_NUMBER_0;
    } else if ((addr < ADDR_FMC_SECTOR_2) && (addr >= ADDR_FMC_SECTOR_1)) {
        sector = CTL_SECTOR_NUMBER_1;
    } else if ((addr < ADDR_FMC_SECTOR_3) && (addr >= ADDR_FMC_SECTOR_2)) {
        sector = CTL_SECTOR_NUMBER_2;
    } else if ((addr < ADDR_FMC_SECTOR_4) && (addr >= ADDR_FMC_SECTOR_3)) {
        sector = CTL_SECTOR_NUMBER_3;
    } else if ((addr < ADDR_FMC_SECTOR_5) && (addr >= ADDR_FMC_SECTOR_4)) {
        sector = CTL_SECTOR_NUMBER_4;
    } else if ((addr < ADDR_FMC_SECTOR_6) && (addr >= ADDR_FMC_SECTOR_5)) {
        sector = CTL_SECTOR_NUMBER_5;
    } else if ((addr < ADDR_FMC_SECTOR_7) && (addr >= ADDR_FMC_SECTOR_6)) {
        sector = CTL_SECTOR_NUMBER_6;
    } else if ((addr < ADDR_FMC_SECTOR_8) && (addr >= ADDR_FMC_SECTOR_7)) {
        sector = CTL_SECTOR_NUMBER_7;
    } else if ((addr < ADDR_FMC_SECTOR_9) && (addr >= ADDR_FMC_SECTOR_8)) {
        sector = CTL_SECTOR_NUMBER_8;
    } else if ((addr < ADDR_FMC_SECTOR_10) && (addr >= ADDR_FMC_SECTOR_9)) {
        sector = CTL_SECTOR_NUMBER_9;
    } else if ((addr < ADDR_FMC_SECTOR_11) && (addr >= ADDR_FMC_SECTOR_10)) {
        sector = CTL_SECTOR_NUMBER_10;
    } else if ((addr < ADDR_FMC_SECTOR_12) && (addr >= ADDR_FMC_SECTOR_11)) {
        sector = CTL_SECTOR_NUMBER_11;
    } else if ((addr < ADDR_FMC_SECTOR_13) && (addr >= ADDR_FMC_SECTOR_12)) {
        sector = CTL_SECTOR_NUMBER_12;
    } else if ((addr < ADDR_FMC_SECTOR_14) && (addr >= ADDR_FMC_SECTOR_13)) {
        sector = CTL_SECTOR_NUMBER_13;
    } else if ((addr < ADDR_FMC_SECTOR_15) && (addr >= ADDR_FMC_SECTOR_14)) {
        sector = CTL_SECTOR_NUMBER_14;  
    } else if ((addr < ADDR_FMC_SECTOR_16) && (addr >= ADDR_FMC_SECTOR_15)) {
        sector = CTL_SECTOR_NUMBER_15;
    } else if ((addr < ADDR_FMC_SECTOR_17) && (addr >= ADDR_FMC_SECTOR_16)) {
        sector = CTL_SECTOR_NUMBER_16;
    } else if ((addr < ADDR_FMC_SECTOR_18) && (addr >= ADDR_FMC_SECTOR_17)) {
        sector = CTL_SECTOR_NUMBER_17;
    } else if ((addr < ADDR_FMC_SECTOR_19) && (addr >= ADDR_FMC_SECTOR_18)) {
        sector = CTL_SECTOR_NUMBER_18;
    } else if ((addr < ADDR_FMC_SECTOR_20) && (addr >= ADDR_FMC_SECTOR_19)) {
        sector = CTL_SECTOR_NUMBER_19;
    } else if ((addr < ADDR_FMC_SECTOR_21) && (addr >= ADDR_FMC_SECTOR_20)) {
        sector = CTL_SECTOR_NUMBER_20;
    } else if ((addr < ADDR_FMC_SECTOR_22) && (addr >= ADDR_FMC_SECTOR_21)) {
        sector = CTL_SECTOR_NUMBER_21;
    } else if ((addr < ADDR_FMC_SECTOR_23) && (addr >= ADDR_FMC_SECTOR_22)) {
        sector = CTL_SECTOR_NUMBER_22;
    } else if ((addr < ADDR_FMC_SECTOR_24) && (addr >= ADDR_FMC_SECTOR_23)) {
        sector = CTL_SECTOR_NUMBER_23;
    } else if ((addr < ADDR_FMC_SECTOR_25) && (addr >= ADDR_FMC_SECTOR_24)) {
        sector = CTL_SECTOR_NUMBER_24;
    } else if ((addr < ADDR_FMC_SECTOR_26) && (addr >= ADDR_FMC_SECTOR_25)) {
        sector = CTL_SECTOR_NUMBER_25;
    } else if ((addr < ADDR_FMC_SECTOR_27) && (addr >= ADDR_FMC_SECTOR_26)) {
        sector = CTL_SECTOR_NUMBER_26;
    } else if ((addr < ADDR_FMC_SECTOR_28) && (addr >= ADDR_FMC_SECTOR_27)) {
        sector = CTL_SECTOR_NUMBER_27;
    } else if ((addr < ADDR_FMC_SECTOR_29) && (addr >= ADDR_FMC_SECTOR_28)) {
        sector = CTL_SECTOR_NUMBER_28;
    } else if ((addr < ADDR_FMC_SECTOR_30) && (addr >= ADDR_FMC_SECTOR_29)) {
        sector = CTL_SECTOR_NUMBER_29;
    } else if ((addr < ADDR_FMC_SECTOR_31) && (addr >= ADDR_FMC_SECTOR_30)) {
        sector = CTL_SECTOR_NUMBER_30;
    } else if ((addr < ADDR_FMC_SECTOR_32) && (addr >= ADDR_FMC_SECTOR_31)) {
        sector = CTL_SECTOR_NUMBER_31;
    } else if ((addr < ADDR_FMC_SECTOR_33) && (addr >= ADDR_FMC_SECTOR_32)) {
        sector = CTL_SECTOR_NUMBER_32;
    } else if ((addr < ADDR_FMC_SECTOR_34) && (addr >= ADDR_FMC_SECTOR_33)) {
        sector = CTL_SECTOR_NUMBER_33;
    } else if ((addr < ADDR_FMC_SECTOR_35) && (addr >= ADDR_FMC_SECTOR_34)) {
        sector = CTL_SECTOR_NUMBER_34;
    } else if ((addr < ADDR_FMC_SECTOR_36) && (addr >= ADDR_FMC_SECTOR_35)) {
        sector = CTL_SECTOR_NUMBER_35;
    } else if ((addr < ADDR_FMC_SECTOR_37) && (addr >= ADDR_FMC_SECTOR_36)) {
        sector = CTL_SECTOR_NUMBER_36;
    } else if ((addr < ADDR_FMC_SECTOR_38) && (addr >= ADDR_FMC_SECTOR_37)) {
        sector = CTL_SECTOR_NUMBER_37;
    } else if ((addr < ADDR_FMC_SECTOR_39) && (addr >= ADDR_FMC_SECTOR_38)) {
        sector = CTL_SECTOR_NUMBER_38;  
    } else if ((addr < ADDR_FMC_SECTOR_40) && (addr >= ADDR_FMC_SECTOR_39)) {
        sector = CTL_SECTOR_NUMBER_39;
    } else if ((addr < ADDR_FMC_SECTOR_41) && (addr >= ADDR_FMC_SECTOR_40)) {
        sector = CTL_SECTOR_NUMBER_40;
    } else if ((addr < ADDR_FMC_SECTOR_42) && (addr >= ADDR_FMC_SECTOR_41)) {
        sector = CTL_SECTOR_NUMBER_41;
    } else if ((addr < ADDR_FMC_SECTOR_43) && (addr >= ADDR_FMC_SECTOR_42)) {
        sector = CTL_SECTOR_NUMBER_42;
    } else if ((addr < ADDR_FMC_SECTOR_44) && (addr >= ADDR_FMC_SECTOR_43)) {
        sector = CTL_SECTOR_NUMBER_43;
    } else if ((addr < ADDR_FMC_SECTOR_45) && (addr >= ADDR_FMC_SECTOR_44)) {
        sector = CTL_SECTOR_NUMBER_44;
    } else if ((addr < ADDR_FMC_SECTOR_46) && (addr >= ADDR_FMC_SECTOR_45)) {
        sector = CTL_SECTOR_NUMBER_45;
    } else if ((addr < ADDR_FMC_SECTOR_47) && (addr >= ADDR_FMC_SECTOR_46)) {
        sector = CTL_SECTOR_NUMBER_46;
    } else if ((addr < ADDR_FMC_SECTOR_48) && (addr >= ADDR_FMC_SECTOR_47)) {
        sector = CTL_SECTOR_NUMBER_47;
    } else if ((addr < ADDR_FMC_SECTOR_49) && (addr >= ADDR_FMC_SECTOR_48)) {
        sector = CTL_SECTOR_NUMBER_48;
    } else if ((addr < ADDR_FMC_SECTOR_50) && (addr >= ADDR_FMC_SECTOR_49)) {
        sector = CTL_SECTOR_NUMBER_49;
    } else if ((addr < ADDR_FMC_SECTOR_51) && (addr >= ADDR_FMC_SECTOR_50)) {
        sector = CTL_SECTOR_NUMBER_50;
    } else if ((addr < ADDR_FMC_SECTOR_52) && (addr >= ADDR_FMC_SECTOR_51)) {
        sector = CTL_SECTOR_NUMBER_51;
    } else if ((addr < ADDR_FMC_SECTOR_53) && (addr >= ADDR_FMC_SECTOR_52)) {
        sector = CTL_SECTOR_NUMBER_52;
    } else if (addr >= ADDR_FMC_SECTOR_53) {
        sector = CTL_SECTOR_NUMBER_53;
    }

    return sector;
}

unsigned short crc16_ccitt(const unsigned char *buf, unsigned int len)
{
    register unsigned int counter;
    register unsigned short crc = 0;
    for( counter = 0; counter < len; counter++)
        crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *(unsigned char *)buf++)&0x00FF];
    return crc;
}

#endif

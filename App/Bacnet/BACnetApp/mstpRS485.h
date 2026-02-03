#ifndef __MSTPRS485_H__
#define __MSTPRS485_H__

#include "device.h"
#include "bacdef.h"
#include "BacnetMSTPServer.h"

#define MAX_RECEIVE_PDU_BUFFER	2
#define	MAX_TRANSMIT_PDU_BUFFER	6

extern unsigned Tframe_abort[TOTAL_BACNET_SERIALPORT];

/**
 * @brief   Get MS/TP port structure.
 *
 * @param[in] ch MS/TP channel number.
 *
 * @return  Pointer to MS/TP port structure.
 */
volatile struct mstp_port_struct_t* getMstpPort(uint8_t ch);

/**
 * @brief   MS/TP RS485 communication initialization.
 *
 * @param[in] ch MS/TP channel number.
 *
 * @return  None.
 */
void mstpRS485_Init(uint8_t ch);

/**
 * @brief   MS/TP RS485 object execution loop.
 *
 * @param[in] ch MS/TP channel number.
 *
 * @return  None.
 */
void mstpRS485_Obj(uint8_t ch);

/**
 * @brief   1 millisecond timer callback.
 *
 * @param   None.
 *
 * @return  None.
 */
void mstp1MSTimer(void);

/**
 * @brief   UART transmit/receive interrupt callback routine for MS/TP communication.
 *
 * @param[in] bStatus UART interrupt status (receive, transmit, or error).
 * @param[in] ch      Received/transmitted data byte.
 *
 * @return  None.
 */
void mstpOnTxRx(/*void *ptr, */uint8_t bStatus, uint8_t ch);

/**
 * @brief   UART send/transmit completed callback routine for MS/TP communication.
 *
 * @param   None.
 *
 * @return  None.
 */
void mstpSendComplete();

/**
 * @brief   Get next Expecting-Reply / Non-Expecting-Reply MPDU from transmit queue and create the MS/TP frame to be sent.
 *
 * @param[in] mstp_port Pointer to MS/TP port structure.
 * @param[in] timeout   Timeout value (currently unused).
 *
 * @return  Frame length if frame available, 0 if no frame to send.
 */
uint16_t MSTP_Get_Send(volatile struct mstp_port_struct_t * mstp_port, unsigned timeout);

/**
 * @brief   Get netx ACK / reply MPDU from transmit queue and create the MS/TP frame to be sent.
 *
 * @param[in] mstp_port Pointer to MS/TP port structure.
 * @param[in] timeout   Timeout value (currently unused).
 *
 * @return  Frame length if matching reply found, 0 if not found, -1 if error.
 */
uint16_t MSTP_Get_Reply(volatile struct mstp_port_struct_t * mstp_port, unsigned timeout);

/**
 * @brief   Put received frame into receive queue to be processed.
 *
 * @param[in] mstp_port Pointer to MS/TP port structure.
 *
 * @return  Length of received data, or 0 if failed.
 */
uint16_t MSTP_Put_Receive(volatile struct mstp_port_struct_t *mstp_port);

/**
 * @brief   Peek the PDU packet in receive queue head without removing it.
 *
 * @param[in] ch Serial channel number.
 *
 * @return  Pointer to PDU packet if available, NULL otherwise.
 */
BACNETMSTPFRAME* MSTP_PduPacketPeek(uint8_t ch);

/**
 * @brief   process PDU packet from receive queue head, and remove it.
 *
 * @param[in] ch Serial channel number.
 *
 * @return  None.
 */
void MSTP_PduPacketPop(uint8_t ch);

/**
 * @brief   Crate MPDU and put it into transmit queue.
 *
 * @param[in] ch        Serial channel number.
 * @param[in] src       Source station address.
 * @param[in] frametype MS/TP frame type.
 * @param[in] buffer    Pointer to data buffer.
 * @param[in] size      Size of data buffer.
 *
 * @return  True if packet queued successfully, false otherwise.
 */
bool MSTP_SendPduPacket(uint8_t ch, uint8_t src, uint8_t frametype, uint8_t *buffer, uint16_t size);

/**
 * @brief   Send MS/TP frame via RS-485 interface.
 *
 * @param[in] mstp_port Pointer to MS/TP port structure.
 * @param[in] buffer    Pointer to frame data buffer.
 * @param[in] nbytes    Number of bytes to send.
 *
 * @return  None.
 */
void RS485_Send_Frame(volatile struct mstp_port_struct_t *mstp_port, uint8_t * buffer, uint16_t nbytes);

/**
 * @brief   Set MS/TP max info frames 1~4.
 *
 * @param[in] ch MS/TP channel number.
 * @param[in] max_info_frames    max info frames.
 *
 * @return  None.
 */
void MSTP_Set_MaxInfoFrame(uint8_t ch, uint8_t max_info_frames);

/**
 * @brief   Set MS/TP max masters 1~127.
 *
 * @param[in] ch MS/TP channel number.
 * @param[in] max_info_frames    max masters.
 *
 * @return  None.
 */
void MSTP_Set_MaxMaster(uint8_t ch, uint8_t max_master);

#endif	//end of __MSTPRS485_H__
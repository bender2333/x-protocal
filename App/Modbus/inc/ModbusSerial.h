#ifndef __ModbusSerial_H_68A29DBF_4828_41c5_962A_EC635C021E41
#define __ModbusSerial_H_68A29DBF_4828_41c5_962A_EC635C021E41

#define MODBUS_RXBUFFSIZE   600
#define MODBUS_TXBUFFSIZE   260

/**
 * @brief   Modbus serial communication initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ModbusSerialInit(void);

/**
 * @brief   Modbus serial communication object loop.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ModbusSerialObj(void);

/**
 * @brief   Validate Modbus request and prepare to reply.
 *
 * @param[in]  data     Pointer to the Modbus request buffer.
 * @param[in]  nLength  Buffer length.
 * @param[out] none
 * @return     Returns 0 if invalid request, otherwise returns ccs+1 (16 bits Modbus Response packet data length).
 */
u16_t ModbusSerialRTURequest(u8_t *data, u16_t nLength);

/**
 * @brief   Compute and add Modbus CRC bytes into response packet.
 *
 * @param[in]  data     Pointer to the Modbus response buffer.
 * @param[in]  nLength  Response length.
 * @param[out] none
 * @return     none
 */
void ModbusSerialRTUReply(u8_t *data, u16_t nLength);

/**
 * @brief   Reset Modbus serial settings to default values.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ModbusSerialDefault(void);

#endif	/* end of __ModbusSerial_H_68A29DBF_4828_41c5_962A_EC635C021E41 */




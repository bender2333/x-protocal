#ifndef __SerialPort_H_E7A2489C_8142_49f8_B331_251302FD2ACC
#define __SerialPort_H_E7A2489C_8142_49f8_B331_251302FD2ACC

#include "App.h"

#define SERIALPORT_MODBUS		0x00
#define SERIALPORT_BACNET		0x01
#define SERIALPORT_MODBUSMASTER	        0x04


extern u16_t SerialID;
extern u16_t SerialPortSet;
extern BOOL ResetSerial;
extern u16_t SerialID2;
extern u16_t SerialPortSet2;
extern BOOL ResetSerial2;
extern u16_t m_COM1ModbusTestEn;

/**
 * @brief   Serial port initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void SerialPortInit(void);

/**
 * @brief   Serial port object loop.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void SerialPortObj(void);

/**
 * @brief   Serial port communication reset/re-initialize.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void SerialPortResetCom(void);

/**
 * @brief   Restore serial port settings to default.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void SerialPortDefault(void);
#endif	/* end of __SerialPort_H_E7A2489C_8142_49f8_B331_251302FD2ACC */

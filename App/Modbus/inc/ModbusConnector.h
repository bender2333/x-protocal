#ifndef __MODBUSCONNECTOR_H_
#define __MODBUSCONNECTOR_H_

#include "App.h"
#include "ModbusClient.h"
#include "AppConfig.h"
#include "modbus.h"

///define maximum communication loss retries
#define MAX_RETRIES 3


/**
 * @brief   Modbus master-slave connector initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ModbusConnectorInit();

/**
 * @brief   Get next Modbus master request from queue to send.
 *
 * @param[in]  ch   Modbus connector channel number (0-based).
 * @param[in]  pBuf   Pointer to buffer for frame data.
 * @param[out] none
 * @return     Frame length in bytes.
 */
uint16_t ModbusConnectorGetNextFrame(u16_t ch, u8_t *pBuf);

/**
 * @brief   Handle Modbus connector device failure.
 *
 * @param[in]  ch   Modbus connector channel number (0-based).
 * @param[out] none
 * @return     none
 */
void ModbusConnectorDeviceFail(u16_t ch);

/**
 * @brief   Process uart received Modbus slave reply.
 *
 * @param[in]  ch   Modbus connector number (0-based).
 * @param[in]  pBuf   Pointer to the received buffer.
 * @param[in]  len    Frame length in bytes.
 * @param[out] none
 * @return     TRUE if frame processed successfully, FALSE otherwise.
 */
BOOL ModbusConnectorReceiveFrame(u16_t ch, BYTE *pBuf, u16_t len);

#endif  // __MODBUSCONNECTOR_H_870DBDF4_5D53_4CC2_A757_494A60030B88
#ifndef __MODBUSSERVER_H_82005224_C32F_46d6_A825_2272DFF3EB78
#define __MODBUSSERVER_H_82005224_C32F_46d6_A825_2272DFF3EB78

#include "App.h"
#include "ModbusOpts.h"
#include "ModbusOptsCal.h"

typedef struct
{
    u16_t regNo;
    u8_t bitPos;
}BITREGS;

#ifndef REG8
  #define REG8    0
#endif
#ifndef REG16
  #define REG16   1
#endif
#ifndef REG16
  #define REG32   2
#endif

typedef struct
{
    u8_t Name[65];
    u8_t DeviceID[65];
    u8_t Version[65];
}MODBUSDEVICEINFO;

extern BOOL ModbusLastWriteOK;

/**
 * @brief   Modbus Server initialization      
 *
 * @param[in]  None.
 *
 * @return  None.
 */
void ModbusServerInit(void);

/**
 * @brief   Identify, validate and process Modbus request.
 *
 * @param[in,out] pData   Pointer to the data buffer.
 * @param[in]     dataLen Data length.
 * @param[in]     bDirect Direct write flag.
 *
 * @return  Response packet length on success, or exception code on error.
 */
u16_t ModbusServerIndication(BYTE *pData, u16_t dataLen, BOOL bDirect);

/**
 * @brief   Read an output coil register.
 *
 * @param[in]  Addr    Coil register address.
 * @param[out] retVal  Pointer to the return value.
 *
 * @return  TRUE if valid address, FALSE otherwise.
 */
BOOL ModbusServerReadCoilRegister(u16_t Addr, BOOL *retVal);

/**
 * @brief   Read a discrete input register.
 *
 * @param[in]  Addr    Discrete input register address.
 * @param[out] retVal  Pointer to the return value.
 *
 * @return  TRUE if valid address, FALSE otherwise.
 */
BOOL ModbusServerReadDiscreteRegister(u16_t Addr, BOOL *retVal);

/**
 * @brief   Write a holding register.
 *
 * @param[in]  Addr    Holding register address.
 * @param[in]  writeVal Write value.
 *
 * @return  TRUE if valid address, FALSE otherwise.
 */
BOOL ModbusServerWriteHoldingRegisterUInt16(u16_t Addr, u16_t writeVal);

/**
 * @brief   Write UINT32 to two continuous holding registers.
 *
 * @param[in] Addr      First register address.
 * @param[in] writeVal  Write value.
 *
 * @return  TRUE if successfully saved, FALSE otherwise.
 */
BOOL ModbusServerWriteHoldingRegisterUInt32(u16_t Addr, u32_t writeVal);

/**
 * @brief   Write float to two continuous holding registers.
 *
 * @param[in] Addr      First register address.
 * @param[in] writeVal  Write value.
 *
 * @return  TRUE if successfully saved, FALSE otherwise.
 */
BOOL ModbusServerWriteHoldingRegisterFloat(u16_t Addr, float writeVal);

/**
 * @brief   Write a output coil register.
 *
 * @param[in]  Addr    Coil register address.
 * @param[in]  writeVal Write value.
 *
 * @return  TRUE if successfully saved, FALSE otherwise.
 */
BOOL ModbusServerWriteCoilRegister(u16_t Addr, BOOL writeVal);

/// below TBD ///
#define PASSTHROUGHSTATUSIDLE               0x00
#define PASSTHROUGHSTATUSONSEND             0x01
#define PASSTHROUGHSTATUSWAITREPLY          0x02
#define PASSTHROUGHSTATUSBUSY               0x03
#define PASSTHROUGHSTATUSERROR              0xFE
#define PASSTHROUGHSTATUSFAILED             0xFF

#define MBPT_BUFNUM   2
#define MBPT_BUFSIZE  240
typedef struct
{
    u8_t slaveAddr;
    BOOL bWrite;
    u8_t curSendBufIndex;
    u8_t retry;
    u8_t status[MBPT_BUFNUM];
    u8_t Txbuf[MBPT_BUFNUM][MBPT_BUFSIZE];
    u8_t Rxbuf[MBPT_BUFNUM][MBPT_BUFSIZE];
    u8_t txLength[MBPT_BUFNUM];
    u8_t rxLength[MBPT_BUFNUM];
}MODBUSPASSTHROUGHDATA;

void ModbusServerPTUpdateTimer();
BOOL ModbusServerGetPassThrough();
void ModbusServerPTOnFail();
u16_t ModbusServerPTOnSend(BYTE* pBuf);
void ModbusServerPTOnRcv(BYTE* pBuf, BYTE length);
#endif	// end of __MODBUSSERVER_H_82005224_C32F_46d6_A825_2272DFF3EB78

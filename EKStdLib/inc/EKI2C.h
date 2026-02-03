#ifndef __I2C_H_FC5AA2F8_9074_4947_97CD_8B436B1BF90E
#define __I2C_H_FC5AA2F8_9074_4947_97CD_8B436B1BF90E

#include "EKDataType.h"

//I2C control byte
typedef struct
{
    u8_t    nPhyAddr;   //I2C physical address
    u8_t    nAddrMode;  //I2C Address mode
    u8_t    nPageSize;
    u16_t  nMemorySize;
}I2C_CONFIG;

#define I2C_CFG_READ    0
#define I2C_CFG_WRITE   1

//I2C Address Mode Definition
#define I2C_NORMAL      0x00
#define I2C_EXTENDED    0x01

/**
 * @brief  I2C stream initialization
 * @param[in] pName Device name
 * @param[in] pFile File stream pointer
 * @param[in] pVoid Configuration buffer pointer, I2C_CONFIG
 * @return Device handle on success, -1 if not valid
 */
HANDLE I2C_Init(LPCSTR pName, EKFILE *pFile, void* pVoid);

/**
 * @brief  I2C stream read/write config
 * @param[in] hHandle Device handle
 * @param[in] nFunc 0 - Read, 1 - Write
 * @param[in] pVoid Configuration buffer pointer, I2C_CONFIG struct
 * @return S_OK on success
 */
s16_t I2C_SetControl(HANDLE hHandle, u8_t nFunc, void *pVoid);

/**
 * @brief Set the I2C current address 
 * @param[in] hHandle Device handle
 * @param[in] nAddress New address
 * @return S_OK on success
 */
s16_t I2C_Seek(HANDLE hHandle, u32_t nAddress);

/**
 * @brief Check for I2C write cycle busy state
 * @param[in] hHandle Device handle
 * @return TRUE if device is in write cycle, FALSE if device is ready
 */
BOOL I2C_isWriteBusy(HANDLE hHandle);

/**
 * @brief Read one byte from I2C 
 * @param[in] hHandle Device handle
 * @return Data byte on success, error code on failure
 */
s16_t I2C_getc(HANDLE hHandle);

/**
 * @brief Write one byte to I2C 
 * @param[in] hHandle Device handle
 * @param[in] ch Data byte
 * @return Data byte on success, error code on failure
 */
s16_t I2C_putc(HANDLE hHandle, u8_t ch);

/**
 * @brief Write data stream to I2C device
 * @param[in] hHandle Device handle
 * @param[in] pBuf Data buffer
 * @param[in] nLen Data length, -1 if NULL terminated
 * @param[out] pByteWritten Number of bytes written, can be null
 * @param[in] nTimeout Timeout interval in milliseconds
 * @return S_OK on success, error code on failure
 */
s16_t I2C_puts(HANDLE hHandle, LPCSTR pBuf, s32_t nLen, s32_t *pByteWritten, s16_t nTimeout);

/**
 * @brief Read data stream from I2C device
 * @param[in] hHandle Device handle
 * @param[out] pBuffer Data buffer
 * @param[in] nLen Data length to read
 * @param[out] pByteRead Number of bytes read, can be null
 * @param[in] nTimeout Timeout interval in milliseconds
 * @return S_OK on success, error code on failure
 */
s16_t I2C_gets(HANDLE hHandle, LPSTR pBuf, s32_t nLen, s32_t *pByteRead, s16_t nTimeout);

#endif	// end of __I2C_H_FC5AA2F8_9074_4947_97CD_8B436B1BF90E

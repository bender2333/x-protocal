#ifndef __DEV_SOFTI2C_H_41454766_000E_44f1_87F8_58385FBAA615
#define __DEV_SOFTI2C_H_41454766_000E_44f1_87F8_58385FBAA615

#include "MyDevice.h"
#include "EKStdLib.h"

//*****************************************************************************
//
// Macro allowing us to pack the fields of a structure.
//
//*****************************************************************************
#if defined(ccs) ||             \
    defined(codered) ||         \
    defined(gcc) ||             \
    defined(rvmdk) ||           \
    defined(__ARMCC_VERSION) || \
    defined(sourcerygxx)
#define PACKEDSTRUCT __attribute__ ((packed))
#elif defined(ewarm)
#define PACKEDSTRUCT
#else
#error Unrecognized COMPILER!
#endif

#ifdef ewarm
#pragma pack(1)
#endif
typedef struct
{
    tGPIOInfo   i2cSCLInfo;
    tGPIOInfo   i2cSDAInfo;
    tGPIOInfo   i2cWPInfo;
}
PACKEDSTRUCT tI2CInfo;
#ifdef ewarm
#pragma pack()
#endif

#define DEV_I2C_MAX 1

#define DEV_I2C0    0

/** 
 * @brief I2C peripheral and GPIO pins initialization, can be called by stream EKfopen
 * 
 * @param[in] hHandle I2C handle number
 * 
 * @return none
 */
void dev_i2c_init(HANDLE hHandle);

/**
 * @brief Send I2C start bit
 * 
 * @param[in] hHandle I2C handle number
 * 
 * @return none
 */
void dev_i2c_StartBit(HANDLE hHandle);

/**
 * @brief Re-Send I2C start bit
 * 
 * @param[in] hHandle I2C handle number
 * 
 * @return none
 */
void dev_i2c_ReStartBit(HANDLE hHandle);

/**
 * @brief Send I2C stop bit
 * 
 * @param[in] hHandle I2C handle number
 * 
 * @return none
 */
void dev_i2c_StopBit(HANDLE hHandle);

/**
 * @brief Send one bit out to I2C
 * 
 * @param[in] hHandle I2C handle number
 * @param[in] bHigh 0: logic low, 1: logic high
 * 
 * @return none
 */
void dev_i2c_BitOut(HANDLE hHandle, BOOL bHigh);

/**
 * @brief Read one bit in from I2C
 * 
 * @param[in] hHandle I2C handle number
 * 
 * @return BOOL TRUE: logic high, FALSE: logic low
 */
BOOL dev_i2c_BitIn(HANDLE hHandle);

/**
 * @brief Transmit data to the I2C device
 * 
 * @param[in] hHandle I2C handle number
 * @param[in] ch Data to be written
 * @param[in] bCtrl Control parameter (unused)
 * 
 * @return BOOL TRUE: operation succeeds, FALSE: No acknowledge from I2C device
 */
BOOL dev_i2c_ByteOut(HANDLE hHandle, u8_t ch, BOOL bCtrl);

/**
 * @brief Read one byte of data from the device
 * 
 * @param[in] hHandle I2C handle number
 * @param[in] bLast TRUE if last byte to read, FALSE otherwise
 * 
 * @return u8_t Data read from device
 */
u8_t dev_i2c_ByteIn(HANDLE hHandle, BOOL bLast);

/**
 * @brief General call read address function
 * 
 * @param[in] hHandle I2C handle number
 * @param[in] pLDAC Pointer to LDAC GPIO info
 * @param[in] pcmd Command array
 * @param[out] pAddr Address to read
 * 
 * @return BOOL TRUE if successful, FALSE otherwise
 */
BOOL dev_i2c_GeneralCall_ReadAddr(HANDLE hHandle, tGPIOInfo * pLDAC, u8_t * pcmd, u8_t * pAddr);

#endif	// end of __DEVICEI2C_H_41454766_000E_44f1_87F8_58385FBAA615

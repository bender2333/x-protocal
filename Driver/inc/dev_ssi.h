#ifndef __DEV_SSI_H_
#define __DEV_SSI_H_

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

#define DEV_SPI_MAX 3
#define DEV_SPI_FSS_MAX 3
     
#ifdef ewarm
#pragma pack(1)
#endif
typedef struct
{
    uint32_t            ulBase;
    rcu_periph_enum     ucPortClk;
    tGPIOInfo   ssiFSSInfo[DEV_SPI_FSS_MAX];
    tGPIOInfo   ssiSCKInfo;
    tGPIOInfo   ssiTXInfo;
    tGPIOInfo   ssiRXInfo;
}
PACKEDSTRUCT tSSIInfo;
#ifdef ewarm
#pragma pack()
#endif

typedef enum
{
  SPI1_PGA_CS = 0,
  SPI1_DAC_CS,
  SPI1_IO_CS
}SPI1_CS;

#define DEV_SPI0    0

extern const tSSIInfo ssiInfo[DEV_SPI_MAX];

/**
 * @brief SPI peripheral and GPIO initialization
 * 
 * @param[in] hHandle SPI handle number
 * 
 * @return none
 */
void dev_ssi_init(HANDLE hHandle);

/**
 * @brief Control SPI chip select signal
 * 
 * @param[in] hSSI SPI handle number
 * @param[in] bCS TRUE to select chip, FALSE to deselect
 * @param[in] csChannel Chip select channel number
 * 
 * @return none
 */
void dev_ssi_ChipSelect(HANDLE hSSI, BOOL bCS, BYTE csChannel);

/**
 * @brief Send data to SPI
 * 
 * @param[in] hSSI SPI handle number
 * @param[in] csChannel Chip select channel number
 * @param[in] autocs TRUE to automatically control chip select, FALSE otherwise
 * @param[in] data Pointer to data buffer, BYTE array
 * @param[in] size Size of data buffer
 * 
 * @return none
 */
void dev_ssi_PutData(HANDLE hSSI, BYTE csChannel, BOOL autocs, u8_t* data, u8_t size);

/**
 * @brief Receive data from SPI 
 * 
 * @param[in] hSSI SPI handle number
 * @param[in] csChannel Chip select channel number
 * @param[out] pData Pointer to receive data buffer, BYTE array
 * 
 * @return none
 */
void dev_ssi_GetData(HANDLE hSSI, BYTE csChannel, u8_t * pData);

/**
 * @brief Send 16-bit data to SPI 
 * 
 * @param[in] hSSI SPI handle number
 * @param[in] csChannel Chip select channel number
 * @param[in] data 16-bit data to send
 * 
 * @return none
 */
void dev_ssi_PutData16(HANDLE hSSI, BYTE csChannel, u16_t data);

/**
 * @brief Receive 16-bit data from SPI 
 * 
 * @param[in] hSSI SPI handle number
 * @param[in] csChannel Chip select channel number
 * @param[out] pData Pointer to receive 16-bit data buffer
 * 
 * @return none
 */
void dev_ssi_GetData16(HANDLE hSSI, BYTE csChannel, u16_t * pData);

#endif	// end of __DEVICEI2C_H_41454766_000E_44f1_87F8_58385FBAA615

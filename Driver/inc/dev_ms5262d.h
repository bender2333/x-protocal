
#ifndef __DEV_MS5262D_H_
#define __DEV_MS5262D_H_

#include "MyDevice.h"
#include "EKStdLib.h"

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

#define	DEVAO_12BIT	0x0FFF
#define	DEVAO_13BIT	0x1FFF
#define	DEVAO_14BIT	0x3FFF
#define	DEVAO_15BIT	0x7FFF
#define	DEVAO_16BIT	0xFFFF

#define	DEVAORESOLUTION	DEVAO_12BIT

#define DEV_EXTDAC_AO_TOTAL    4

/**
 * @brief MS5262D DAC peripheral and GPIO pins initialization
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_ms5262d_Init(void);

/**
 * @brief Write data to MS5262D DAC channel
 * 
 * @param[in] ch Channel number
 * @param[in] data 12-bit DAC data value
 * 
 * @return none
 */
void dev_ms5262d_Write(u8_t ch, u16_t data);

#endif	

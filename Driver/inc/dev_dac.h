#ifndef __DEV_DAC_H_
#define __DEV_DAC_H_

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

#define DEV_DAC_TOTAL    2

/**
 * @brief DAC peripheral and GPIO pins initialization
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_dac_Init(void);

/**
 * @brief Write data to two DAC channels concurrently
 * 
 * @param[in] data1 Data value for DAC channel 0
 * @param[in] data2 Data value for DAC channel 1
 * 
 * @return none
 */
void dev_dac_concurrent_Write(WORD data1, WORD data2);

/**
 * @brief Write data to specified DAC channel
 * 
 * @param[in] ch DAC channel number
 * @param[in] data Data value to write
 * 
 * @return none
 */
void dev_dac_Write(BYTE ch, WORD data);

#endif

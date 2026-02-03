#ifndef __DEV_SN74595_H__
#define __DEV_SN74595_H__

#include <stdint.h>
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

#define TOTAL_SHIFTREG  8
// SN74595 config
  typedef enum { 
    LED_P1, 
    LED_P8,
    LED_P7,
    LED_P6,
    LED_P5,
    LED_P4,
    LED_P3,
    LED_P2,
    UI_SEL_RD5,  // 8
    UI_SEL_I5,  // 9
    UI_SEL_V8, // 10
    UI_SEL_V7, // 11 
    UI_SEL_RD8,
    UI_SEL_I8,
    UI_SEL_RD7,
    UI_SEL_I7,
    LED_P9,     // 16
    LED_P10,
    LED_P11,
    LED_P12,
    UO_SEL_V10,
    UO_SEL_I10,
    UO_SEL_I9,
    UO_SEL_V9,
    UI_SEL_RD4,  // 24
    UI_SEL_I4,
    UI_SEL_RD3, 
    UI_SEL_I3, 
    UI_SEL_V6,
    UI_SEL_V5,
    UI_SEL_RD6,
    UI_SEL_I6,
    UO_SEL_V8,  // 32
    UO_SEL_I8,
    UO_SEL_I7, 
    UO_SEL_V7, 
    UO_SEL_V6,
    UO_SEL_I6,
    UO_SEL_I5,
    UO_SEL_V5,  
    UI_SEL_V2, // 40
    UI_SEL_V1,
    UI_SEL_RD2, 
    UI_SEL_I2, 
    UI_SEL_RD1,
    UI_SEL_I1,
    UI_SEL_V4,
    UI_SEL_V3,
    UO_SEL_V3, // 48
    UI_SEL_V11,
    UI_SEL_V12, 
    UI_SEL_I11, 
    UI_SEL_I12,
    UI_SEL_RD12,
    UO_SEL_V4,
    UI_SEL_RD11,
    UI_SEL_V9, // 56
    UO_SEL_V1,
    UI_SEL_RD9, 
    UO_SEL_V2, 
    UI_SEL_RD10,
    UI_SEL_I10,
    UI_SEL_I9,
    UI_SEL_V10,
    IOSEL_MAX  //64   //UI_UI1-8, UIO_UI9-12, UIO_UO1-4, UO_UO5-10
} IO_SEL_BIT;

typedef struct _SN74595PERIF
{
    uint8_t portIndex;        //SPI Slave Index
    uint8_t pPolarity[TOTAL_SHIFTREG];      //Shift Register outpit Pins polarity
    uint8_t pBuf[TOTAL_SHIFTREG];                 //Shift Register Buffer
    uint8_t pBufPrev[TOTAL_SHIFTREG];             //Shift Register Buffer Previous
} SN74595PERIF;

/**
 * @brief SN74595 shift all registers one-round immediately
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_sn74595_shiftAll();

/**
 * @brief Set individual bit in SN74595 shift register
 * 
 * @param[in] bit Bit position to set
 * @param[in] bOnOff TRUE to set bit high, FALSE to set bit low
 * @param[in] bImmediate TRUE to shift data immediately, FALSE to delay
 * 
 * @return none
 */
void dev_sn74595_SetBit(uint8_t bit, BOOL bOnOff, BOOL bImmediate);

/**
 * @brief Get individual bit status from SN74595 shift register
 * 
 * @param[in] bit Bit position to read
 * 
 * @return BOOL TRUE if bit is set, FALSE if bit is clear
 */
BOOL dev_sn74595_GetBit(uint8_t bit);

/**
 * @brief Clear all bits in SN74595 shift register
 * 
 * @param[in] nCount Number of times to clear and shift
 * 
 * @return none
 */
void dev_sn74595_clrAll(uint8_t nCount);


/**
 * @brief SN74595 shift register peripheral and GPIO initialization
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_sn74595_init();

#endif // end of __DEV_MCP6S28_H__
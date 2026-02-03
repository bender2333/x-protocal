#ifndef __DEV_DO_H_EFD22F7F_743C_4cee_BAEA_03C4B469625F
#define __DEV_DO_H_EFD22F7F_743C_4cee_BAEA_03C4B469625F

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

#define DEV_DO_MAX	10

/**
 * @brief DO peripheral and GPIO pins initialization 
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_do_init(void);

/**
 * @brief Set DO channel state
 * 
 * @param[in] ch Channel number
 * @param[in] bOnOff TRUE to turn ON, FALSE to turn OFF
 * 
 * @return none
 */
void dev_do_set(u8_t ch, BOOL bOnOff);

/**
 * @brief Get device total DO channel count
 * 
 * @param[in] none
 * 
 * @return BYTE Total number of DO channels (DEV_DO_MAX)
 */
BYTE dev_do_total(void);

#endif	// end of __MLN7400DO_H_EFD22F7F_743C_4cee_BAEA_03C4B469625F

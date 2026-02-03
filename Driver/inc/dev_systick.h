
#ifndef __DEV_SYSTICK_H
#define __DEV_SYSTICK_H

#include "EKDataType.h"

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

#define SYSTICKHZ   10000    // 0.1ms
#define MSTICKCOUNT 10

/**
 * @brief system tick initialization
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_systick_init(void);

/**
 * @brief System tick interrupt handler
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_systick_IntHandler(void);

/**
 * @brief Get system tick value/count
 * 
 * @param[in] none
 * 
 * @return u32_t Current system tick value
 */
u32_t dev_systick_ValueGet(void);

/**
 * @brief Check if a timeout has occurred
 * 
 * @param[in] nexttick Expected tick value for timeout
 * 
 * @return BOOL TRUE if timeout occurred, FALSE otherwise
 */
BOOL dev_systick_checkTimeout(u32_t nexttick);

#endif /* __MLN7400_H */

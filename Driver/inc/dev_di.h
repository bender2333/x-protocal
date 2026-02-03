#ifndef __DEV_DI_H_1194753E_AB21_4275_ABFA_A9A2E8D5763A
#define __DEV_DI_H_1194753E_AB21_4275_ABFA_A9A2E8D5763A

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

#define DEV_DI_MAX      12
#define DEV_DI_HARDWARE 0

/**
 * @brief DI peripheral and GPIO pins initialization, register ADC convert callback for DI status update
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_di_init(void);

/**
 * @brief Get total digital input count
 * 
 * @param[in] none
 * 
 * @return BYTE total digital input count
 */
BYTE dev_di_total(void);

/**
 * @brief Get hardware digital input count
 * 
 * @param[in] none
 * 
 * @return BYTE hardware digital input count
 */
BYTE dev_get_hardware_di_cnt(void);

/**
 * @brief Get all digital input (hardware di + ADI) status 
 * 
 * @param[in] ch Channel number
 * 
 * @return BOOL TRUE if input is ON, FALSE if OFF or invalid channel
 */
BOOL dev_get_all_di(u8_t ch);
#endif

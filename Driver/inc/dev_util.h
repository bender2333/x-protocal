#ifndef __DEV_UTIL_A756FE38_6F97_42d0_81BD_B5CFC341B1A0
#define __DEV_UTIL_A756FE38_6F97_42d0_81BD_B5CFC341B1A0

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

/**
 * @brief   Toggle the watchdog GPIO pin state.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void WatchDogToggle(void);

/**
 * @brief   Watchdog peripheral and GPIO pin initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void WatchDogInit(void);

/**
 * @brief   Watchdog peripheral and GPIO pin de-initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void WatchDogDeInit(void);
#endif     // __MLN7400_UTIL

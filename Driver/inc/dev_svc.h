#ifndef __DEV_SVC_H_
#define __DEV_SVC_H_

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
 * @brief Service pin peripheral and GPIO pins initialization
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_svc_init(void);

/**
 * @brief Get service gpio pin status 
 * 
 * @param[in] none
 * 
 * @return BOOL TRUE if pin is pressed (low), FALSE if released (high)
 */
BOOL dev_svc_gpio_get(void); 


/**
 * @brief Get stablized service pin state 
 * 
 * @param[in] none
 * 
 * @return BOOL TRUE if pin state changed from low to high, FALSE otherwise
 */
BOOL dev_svc_state_get(void); 

/**
 * @brief Service pin object function, timer elapsed to update service pin state
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_svc_Obj(void);

/**
 * @brief Get service pin pressed duration in seconds
 * 
 * @param[in] none
 * 
 * @return WORD Duration in seconds since pin was pressed
 */
WORD dev_svc_get_pressDuration(void);

/**
 * @brief Get service pin continously pressed-released status, given desired continous count
 * 
 * @param pressCount desired continous pressed-released count
 * @return BOOL TRUE if pin is pressed-released for given count, FALSE otherwise
 */
BOOL dev_svc_get_contPress_state(u16_t pressCount);
#endif

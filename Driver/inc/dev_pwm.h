#ifndef __DEV_PWM_H_
#define __DEV_PWM_H_

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

typedef struct
{
    rcu_periph_enum timClk;
    uint32_t timBase;
    uint16_t timCh;
    tGPIOInfo gpioInfo;
}PWMConfInfo;

//*****************************************************************************
//
// Macro allowing us to pack the fields of a structure.
//
//*****************************************************************************
#define DEV_PWM_TOTAL    4

/**
 * @brief  PWM timer peripheral and GPIO pins initialization
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_pwm_Init(void);

/**
 * @brief Write PWM duty cycle value to specified channel
 * 
 * @param[in] ch PWM channel number
 * @param[in] data PWM duty cycle value
 * 
 * @return none
 */
void dev_pwm_Write(u8_t ch, float data);

#endif

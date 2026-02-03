#ifndef __DEV_GPIO_H_
#define __DEV_GPIO_H_

#include "MyDevice.h"
#include "EKStdLib.h"
#include "gd32f5xx.h"

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

//*****************************************************************************
//
// Structure used to map a GPIO port and pin on the target
// part (lm3s9b96 in this case) to LED.  Field ucPortIndex is the index into
// the g_pulGPIOBase array containing the base address of the port.
//
//*****************************************************************************
#ifdef ewarm
#pragma pack(1)
#endif
typedef struct
{
    uint32_t ucPortBase;
    uint32_t ulPinNo;
    uint32_t ulPinSpeed;
    uint32_t ulPinMode;
    uint32_t ulPinPup;
    uint32_t ulPinType;
    uint32_t ulPinAfNum;
}
PACKEDSTRUCT GPIO_InitTypeDef;
      
typedef struct
{
    rcu_periph_enum ucPortClk;
    GPIO_InitTypeDef ulPinConfig;
}
PACKEDSTRUCT tGPIOInfo;
#ifdef ewarm
#pragma pack()
#endif

#endif

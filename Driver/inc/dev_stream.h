#ifndef __DEV_STREAM_H_BE9E09E5_2951_4e1d_89A7_AD1F3CB21EB5
#define __DEV_STREAM_H_BE9E09E5_2951_4e1d_89A7_AD1F3CB21EB5

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

extern const devsw Device[];

/**
 * @brief Get number of registered stream devices, used in EKStdLib (uart, i2c, usb)
 * 
 * @param[in] none
 * 
 * @return s16_t Number of registered stream devices
 */
s16_t GetTotalDevice(void);

#endif  // end of __STREAMLPC22xx_H_BE9E09E5_2951_4e1d_89A7_AD1F3CB21EB5

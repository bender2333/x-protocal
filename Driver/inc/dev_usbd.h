#ifndef __DEV_USBD_A756FE38_6F97_42d0_81BD_B5CFC341B1A0
#define __DEV_USBD_A756FE38_6F97_42d0_81BD_B5CFC341B1A0

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
 * @brief   Check if USB CDC is ready to receive a packet.
 *
 * @param[in]  itf     Interface number.
 * @param[out] none
 * @return     TRUE if ready to receive, FALSE otherwise.
 */
BOOL dev_usbcdc_readyReceive(BYTE itf);

/**
 * @brief   USB CDC interface and GPIO pins initialization and connect the interface.
 *
 * @param[in]  pCallback   Pointer to USB callback function.
 * @param[out] none
 * @return     none
 */
void dev_usbcdc_connect(const USBCALLBACK *pCallback);

#endif     // __MLN7400_UTIL

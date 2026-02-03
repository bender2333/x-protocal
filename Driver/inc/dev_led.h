////////////////////////////////////////////
//
// sbStdLib - Soft Device Standard Library
//
// Filename : LEDMsp430.h
// Description : LED definition for MSP430
//
// (c) Copyright 2003-2004, Soft Device Sdn Bhd
// All Rights Reserved
//
// Rev :
// April-2004 :  First draft
//
////////////////////////////////////////////
#ifndef __DEV_LED_H_1B3AC002_FFFB_4266_B27B_1CE5DC60028C
#define __DEV_LED_H_1B3AC002_FFFB_4266_B27B_1CE5DC60028C

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
      
#ifdef BOOTLOADER
#define DEV_LED_MAX     2
#else
#define DEV_LED_MAX     5
#endif
      

extern LEDDATA LEDs[];

/**
 * @brief LED peripheral and GPIO pins initialization
 * 
 * @param[in] none
 * 
 * @return none
 */
void dev_led_init(void);

/**
 * @brief LED status set/update for all LEDs
 * 
 * @param[in] Status 32 bits LED status, 1 : ON, 0 : OFF
 * 
 * @return none
 */
void dev_led(u32_t Status); 

/**
 * @brief Set individual LED status directly
 * 
 * @param[in] nIndex LED index (0 to DEV_LED_MAX-1)
 * @param[in] bStatus TRUE to turn ON, FALSE to turn OFF
 * 
 * @return none
 */
void dev_led_set_direct(u8_t nIndex, BOOL bStatus);

#endif

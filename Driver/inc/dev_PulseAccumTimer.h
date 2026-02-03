
/*===========================================================================*/
/*                                                                           */
/*  This confidential and proprietary software may be used only              */
/*  as authorized by a licensing agreement from MCSLOGIC Inc.                */
/*  In the event of publication, the following notice is applicable:         */
/*                                                                           */
/*  (C) COPYRIGHT 2003 MCSLOGIC INC. ALL RIGHTS RESERVED                     */
/*                                                                           */
/*  The entire notice above must be reproduced on all authorized copies.     */
/*                                                                           */
/*  File name : mln7400.h	                                                 */
/*  Author    : MCS Logic Inc.                                               */
/*  Abstract  : Definition of System Register.                               */
/*  Creation  : 28/10/2003                                                   */
/*                                                                           */
/*===========================================================================*/
#ifndef __DEV_PULSEACCUMTIMER_H__
#define __DEV_PULSEACCUMTIMER_H__

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

typedef void (DEVPULSEACCUMINTCALLBACK)(void);

/**
 * @brief Pulse accumulation MCU timer initialization
 * 
 * @param[in] none
 * 
 * @return none
 */
void Dev_PulseAccumTimerInit(void);

/**
 * @brief Set pulse accumulation timer interrupt callback function
 * 
 * @param[in] cb Pointer to callback function
 * 
 * @return none
 */
void Dev_PulseAccumTimer_SetIntCallback(DEVPULSEACCUMINTCALLBACK* cb);

/**
 * @brief Enable pulse accumulation MCU timer interrupt
 * 
 * @param[in] en TRUE to enable interrupt (always enabled when called)
 * 
 * @return none
 */
void Dev_PulseAccumTimer_SetEnable(BOOL en);

#endif /* __MLN7400_H */

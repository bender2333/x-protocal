#ifndef __COMMMON_H_BF40535E_B01E_4d73_B5A2_6DB9B7E55C28
#define __COMMMON_H_BF40535E_B01E_4d73_B5A2_6DB9B7E55C28

#include "AutomationOpts.h"
#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Communitacion Monitor Object Initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void CommMonInit(void);

/**
 * @brief   Communication monitor object processing Loop.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void  CommMonObj(void);

/* Reset Communitacion Monitor status */
/**
 * @brief   Reset communication monitor status.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void CommMonReset(void);

/**
 * @brief   Check if communication is lost.
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if communication is lost, FALSE otherwise.
 */
BOOL IsCommLost(void);

/* Communitacion lost Timeout */
extern u16_t CommMonTimeOut;

#ifdef __cplusplus
}
#endif
#endif  //end of __COMMMON_H_BF40535E_B01E_4d73_B5A2_6DB9B7E55C28

#ifndef __EKSYSTICK_H_54A78F98_33BE_4c1a_B2FB_BAF653B7AE71
#define __EKSYSTICK_H_54A78F98_33BE_4c1a_B2FB_BAF653B7AE71

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

// Timer data structure
#ifdef ewarm
#pragma pack(1)
#endif

typedef struct _TIMER
{
    u16_t uTimer;           //Timer counter
    void * lpReserved;      //Reserved
}
PACKEDSTRUCT TIMER;

typedef void (TIMERPROC_01MS)(void);                      //0.1ms callback procedure definition
typedef void (TIMERPROC_1MS)(void);                       //1ms callback procedure definition

typedef struct _TIMER01MSCALLBACK
{
	TIMERPROC_01MS	*pfncCallback;
	LPVOID			lpNext;
}TIMER01MSCALLBACK;

extern volatile u32_t gMinuteTick;

/**
 * @brief Timer system initi
 * @param[in] none
 * @return none
 */
void TimerInit(void);

/**
 * @brief Register 1ms countdown/countup timer into timer list
 * @param[in] pNewTimer 1ms timer pointer
 * @param[in] bIncrement TRUE for countup, FALSE for countdown
 * @return none
 */
void Reg1msTimer(TIMER *pNewTimer, BOOL bIncrement);

/**
 * @brief Register 1s countdown/countup timer into timer list
 * @param[in] pNewTimer 1ms timer pointer
 * @param[in] bIncrement TRUE for countup, FALSE for countdown
 * @return none
 */
void Reg1sTimer(TIMER *pNewTimer, BOOL bIncrement);

/**
 * @brief 0.1ms timer service routine
 * @param[in] none
 * @return none
 */
void TimerProc01ms(void);

/**
 * @brief Assign 0.1ms callback procedure
 * @param[in] pfnProc 0.1ms callback procedure address
 * @return none
 */
void Set01msProc(TIMERPROC_01MS *pfnProc);

/**
 * @brief Assign 0.1ms callback procedure
 * @param[in] pfnProc 0.1ms callback procedure address
 * @return none
 */
void Set01msProc2(TIMERPROC_01MS *pfnProc);

/**
 * @brief Assign 0.1ms callback procedure
 * @param[in] pfnProc 0.1ms callback procedure address
 * @return none
 */
void Set01msProc3(TIMERPROC_01MS *pfnProc);

/**
 * @brief Assign 1ms callback procedure
 * @param[in] pfnProc 0.1ms callback procedure address
 * @return none
 */
void Set1msProc(TIMERPROC_1MS *pfnProc);

/**
 * @brief Timer operation object loop
 * @param[in] none
 * @return none
 */
void TimerObj(void);

/**
 * @brief Get timer tick count
 * @param[in] none
 * @return Current timer tick count
 */
u32_t GetTimerTickCount(void);

DWORD TimerStart(void);
BOOL  TimerIsExpired(DWORD timer, DWORD millisecs);
DWORD TimerGetTickElapsed(DWORD timer);
#define _1S		1000
#endif

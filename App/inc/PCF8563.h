#ifndef __RTC_H_1452AFE2_8815_41d7_9C33_889A82F47C96
#define __RTC_H_1452AFE2_8815_41d7_9C33_889A82F47C96

#include "EKStdLib.h"

////define RTC State
#define RTC_OK				0x01
#define RTC_RELIABLE		0x02
#define RTC_CHANGE			0x04
#define RTC_UTCCHANGE		0x08

extern u8_t RTCState;
extern u16_t	RtcSecond;
extern u16_t	RtcMin;
extern u16_t	RtcHour;
extern u16_t	RtcDay;
extern u16_t	RtcMonth;
extern u16_t	RtcYear;
extern u16_t  RtcWeekday;
extern u8_t DateTime[48];
extern u16_t	RtcUTCSecond;
extern u16_t	RtcUTCMin;
extern u16_t	RtcUTCHour;
extern u16_t	RtcUTCDay;
extern u16_t	RtcUTCMonth;
extern u16_t	RtcUTCYear;
extern u16_t  RtcUTCWeekday;
extern s16_t  RtcUTCOffset;
extern WORD  ObBootTimerMinute;

/**
 * @brief   RTC system and PCF8563 Initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void RTCInit(void);

/**
 * @brief   RTC object loop.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void RTCObj(void);

/**
 * @brief   Trigger a RTC write to PCF8563 chip.
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if write successful, FALSE otherwise.
 */
BOOL RTCWrite(void);

#endif	/* end of __RTC_H_1452AFE2_8815_41d7_9C33_889A82F47C96 */

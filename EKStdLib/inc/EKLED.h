#ifndef __EKLED_H_37B436FA_5D6D_4119_A376_33B07E56A877
#define __EKLED_H_37B436FA_5D6D_4119_A376_33B07E56A877

#include "EKDataType.h"

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

#ifdef ewarm
#pragma pack(1)
#endif

typedef enum {
    LED_POWER_FAULT,
    LED_POWER_NORMAL,
    LED_POWER_INSUFFICIENT,
    LED_STATUS_NORMAL,
    LED_STATUS_UNINIT_PROG_LOST,
    LED_STATUS_DOWNLOADING,
    LED_STATUS_DOWNLOAD_FAILED_SETTING_LOST,
    LED_STATUS_BUTTON_POSITIONING,
    LED_STATUS_COMMAND_POSITIONING,
    LED_RS485_1_IDLE,
    LED_RS485_1_COMMUNICATING,
    LED_RS485_2_IDLE,
    LED_RS485_2_COMMUNICATING,
    LED_MODE_MAX
} LEDMODE;


//LED data structure
typedef struct _LEDDATA
{
    u16_t   OnTime;         //LED turn on time, in 10ms resulotion
    u16_t   OffTime;        //LED turn off time, in 10ms resulotion
    u8_t   LEDStatus;      //LED status
    u16_t   Reserved;       //Reserved
}
PACKEDSTRUCT LEDDATA;

#ifdef ewarm
#pragma pack()
#endif

typedef enum _LED
{
    POWER_GREEN   = 0x00,
    STATUS_GREEN    = 0x01,
    STATUS_RED    = 0x02,
    RS485_GREEN1    = 0x03,
    RS485_GREEN2    = 0x04,
    LEDPMAX 
}LED;

typedef struct {
    u8_t nIndex;
    u8_t bOnOff;
    u32_t taskperiodms;
} OnOff_t;

typedef struct {
    u8_t nIndex;
    u16_t ontime;
    u16_t offtime;
    u32_t BlinkCntPerPeriod;
    u32_t slpPeriodms;
    u32_t taskperiodms;
} Blink_t;

void LEDInit(void);                           //LED initialization routine
void LEDSet(LEDMODE mode);
#endif	// end of __LED_H_37B436FA_5D6D_4119_A376_33B07E56A877

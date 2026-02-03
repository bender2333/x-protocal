#ifndef __EKSTDLIB_H_BC9DBC5B_1BA3_4341_B7CA_483194E0F9DA
#define __EKSTDLIB_H_BC9DBC5B_1BA3_4341_B7CA_483194E0F9DA

#include "EKDataType.h"
#include "EKStdDefs.h"
#include "EKStdErr.h"
#include "EKTimer.h"
#include "EKLED.h"
#include "EKUtil.h"
#include "EKFifo.h"
#include "EKStream.h"
#include "EKUart.h"
#include "EKI2C.h"
#include "EKUsbCDC.h"

typedef struct
{
    u16_t year;
    u8_t month;
    u8_t day;
    u8_t wday;
}EKDATE;


typedef struct
{
    u8_t hour;
    u8_t min;
    u8_t sec;
}EKTIME;

#endif	// end of __EKSTDLIB_H_BC9DBC5B_1BA3_4341_B7CA_483194E0F9DA

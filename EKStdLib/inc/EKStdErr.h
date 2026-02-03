#ifndef __EKERROR_H_FA82F994_B67E_43c1_8595_24F7B6156BE7
#define __EKERROR_H_FA82F994_B67E_43c1_8595_24F7B6156BE7

#include "EKDataType.h"

////////////////////////////////////
#define S_OK            0x0000
#define S_BUSY          0x0001

////////////////////////////////////

// 0x8000 - 0x9FFF  reserve for system common error definition
#define E_FAILED        -1
#define E_SYSTEM        -1      //System general error flag.
#define E_NOTFOUND      -2      //Not found.
#define E_NOTSUPPORT    -3      //Not support.
#define E_INVHANDLE     -4      //Invalid Parameter.
#define E_TIMEOUT       -5      //Timeout error.
#define E_INIT          -6      //Initialization error.
#define E_PARAM         -7      //parameter Error
#define E_NAMETOOLONG   -8      //filename exceed limit
#define E_EXIST         -9      //filename exist
#define E_FULL          -10     //system resource ful
#define E_OPEN          -11     //open file fail
#define E_GETINFO       -12     //get info fail
#define E_CLOSED        -13     //file already closed

//I2C error flags
#define E_I2C_FAIL                  0x9000
#define E_I2C_PAGESIZE              0x9001
#define E_I2C_BUSY                  0x9002
#define E_I2C_TIMEOUT               0x9003
// Device error message         0xA000 - 0xBFFF
// User define error message    0xC000 - 0xFFFF

#endif      //end of __ERROR_H_FA82F994_B67E_43c1_8595_24F7B6156BE7

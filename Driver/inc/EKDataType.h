#ifndef __EKDATATYPE_H_2947806F_4E90_41a9_BC7C_486E0F86D924
#define __EKDATATYPE_H_2947806F_4E90_41a9_BC7C_486E0F86D924

#include <stddef.h>
#include "arch.h"

typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef unsigned   short   WORD;
typedef signed     short   s16_t;
typedef unsigned   long    DWORD;
typedef unsigned long long  u64_t;
typedef signed long long    s64_t;

typedef s32_t           HANDLE;     //device/file handler, signed 16 bits
typedef u8_t    		BOOL;       //boolean, unsigned 8 bits
typedef unsigned   char    BYTE;
typedef const char *     LPCSTR;     //constant string pointer
typedef char *           LPSTR;      //string  pointer
typedef void*           LPVOID;     //void pointer
typedef void (pVoidFunc)(void);

#ifndef OFF
#define OFF     0
#endif
#ifndef ON
#define ON      1
#endif

#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

#ifndef DISABLED
#define DISABLED   0
#endif
#ifndef ENABLED
#define ENABLED    1
#endif

#ifndef BOOLSTATUS_OFF
#define BOOLSTATUS_OFF   0
#endif
#ifndef BOOLSTATUS_ON
#define BOOLSTATUS_ON    1
#endif
#ifndef BOOLSTATUS_NULL
#define BOOLSTATUS_NULL  2
#endif


#ifndef ACTIONDIRECT
#define ACTIONDIRECT        0
#endif
#ifndef ACTIONREVERSE
#define ACTIONREVERSE      1
#endif
#endif	// end of __EKDATATYPE_H_2947806F_4E90_41a9_BC7C_486E0F86D924

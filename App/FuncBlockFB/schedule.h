#ifndef __SCHEDULE_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __SCHEDULE_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int		meta;
	char strName[16];

	BYTE startHour1;
	BYTE startMinute1;
	BYTE stopHour1;
	BYTE stopMinute1;

	BYTE startHour2;
	BYTE startMinute2;
	BYTE stopHour2;
	BYTE stopMinute2;

	BYTE out;
	BYTE val1;
	BYTE val2;
	BYTE def;

	u32_t m_timer;
}DAILYSCHBOOLDATA;

void DailySchBoolInit(void *pData, BOOL bDef);
void DailySchBoolExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DailySchBoolLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DailySchBoolDataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct
{
	int		meta;
	char strName[16];

	BYTE startHour1;
	BYTE startMinute1;
	BYTE stopHour1;
	BYTE stopMinute1;

	BYTE startHour2;
	BYTE startMinute2;
	BYTE stopHour2;
	BYTE stopMinute2;

	u32_t m_timer;
	float out;
	float val1;
	float val2;
	float def;
}DAILYSCHFLOATDATA;

void DailySchFloatInit(void *pData, BOOL bDef);
void DailySchFloatExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DailySchFloatLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DailySchFloatDataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct
{
	int		meta;
	char strName[16];

	float year;
	float month;
	float day;
	float wday;
	float hour;
	float min;
	float sec;
}DATETIMEDATA;


void DateTimeInit(void *pData, BOOL bDef);
void DateTimeExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DateTimeLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DateTimeDataSize(void);

////////////////////////////////// P0 /////////////////////////////////////

typedef struct
{
	int		meta;
	char strName[16];

	bool enable1;
	bool enable2;
	bool enable3;
	bool enable4;

	BYTE startHour1;
	BYTE startMinute1;
	BYTE startHour2;
	BYTE startMinute2;
	BYTE startHour3;
	BYTE startMinute3;
	BYTE startHour4;
	BYTE startMinute4;

	u32_t m_timer;
	float out;
	float val1;
	float val2;
	float val3;
	float val4;
	float def;
}DAILYSCHFLOAT4DATA;

void DailySchFloat4Init(void *pData, BOOL bDef);
void DailySchFloat4Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DailySchFloat4LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DailySchFloat4DataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct
{
	int		meta;
	char strName[16];

	BYTE 	weekday;
	float 	in1;//mon
	float 	in2;
	float 	in3;
	float 	in4;
	float 	in5;
	float 	in6;
	float 	in7;//sun

	float 	out;
	u32_t 	m_timer;
}WEEKLYSCHFLOATDATA;

void WeeklySchFloatInit(void *pData, BOOL bDef);
void WeeklySchFloatExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void WeeklySchFloatLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int WeeklySchFloatDataSize(void);

///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif      //__TIMING_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
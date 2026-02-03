#ifndef __FUTIL3_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __FUTIL3_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////
typedef struct //vincent 20230303 new fb
{
	int		meta;
	char strName[16];

	float min;
    float max;
    float avg;
	float in1;
	float in2;
	float in3;
	float in4;
}MINMAXAVG4DATA;

void MinMaxAvg4Init(void *pData, BOOL bDef);
void MinMaxAvg4Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void MinMaxAvg4LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int MinMaxAvg4DataSize(void);
///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

    BOOL bSel;
	float out;
	float in1;
	float in2;
	float in3;
	float in4;
}MINMAXSEL4DATA;

void MinMaxSel4Init(void *pData, BOOL bDef);
void MinMaxSel4Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void MinMaxSel4LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int MinMaxSel4DataSize(void);
///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	float   out;    
	float   in;
	float	step[10];
    float	stepValue[10];
   
}STEPPEDCONTROLDATA;


void SteppedControlInit(void *pData, BOOL bDef);
void SteppedControlExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void SteppedControlLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int SteppedControlDataSize(void);
///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BOOL 	in;
	BOOL    resetCount;
	BOOL    resetTime;
	BOOL    m_prevIn;
	DWORD	onCount;
	DWORD	offCount;
	DWORD 	onTimer;
	DWORD	offTimer;
	DWORD	m_timer;
   
}DISCTOTALIZERDATA;


void DiscTotalizerInit(void *pData, BOOL bDef);
void DiscTotalizerExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DiscTotalizerLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DiscTotalizerDataSize(void);
///////////////////////////////////////////////////////////////////////
	
typedef struct
{
	int meta;
	char strName[16];
	BOOL enable;
	DWORD m_timer;
	float out;
	float in;
	float startVal;
	float upRate;
	float downRate;
}RATELIMITDATA;

void RateLimitInit(void *pData, BOOL bDef);
void RateLimitExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void RateLimitLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int RateLimitDataSize(void);
///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif      //__FUTIL3_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
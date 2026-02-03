#ifndef __TIMING_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __TIMING_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE in;
	BYTE m_lastIn;
	u32_t hold;
	u32_t m_timer;
	float delayTime;
}DLYOFFDATA;



void DlyOffInit(void *pData, BOOL bDef);
void DlyOffExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DlyOffLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DlyOffDataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE in;
	BYTE m_lastIn;
	u32_t hold;
	u32_t m_timer;
	float delayTime;
}DLYONDATA;



void DlyOnInit(void *pData, BOOL bDef);
void DlyOnExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DlyOnLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DlyOnDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE in;
	BYTE canRetrig;
	BYTE m_lastIn;
	u32_t m_timer;
	float pulseWidth;
}ONESHOTDATA;



void OneShotInit(void *pData, BOOL bDef);
void OneShotExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void OneShotLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int OneShotDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE run;
	BYTE m_lastRun;
	u32_t time;
	u32_t left;
	u32_t m_timer;
}TIMERDATA;

void fbTimerInit(void *pData, BOOL bDef);
void fbTimerExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void fbTimerLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int fbTimerDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE trigger;
	BYTE m_LastTrigger;
	u16_t remainTime;
	u16_t overrideTime;

	u32_t m_timer;
}OVERRIDEDATA;



void OverrideInit(void *pData, BOOL bDef);
void OverrideExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void OverrideLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int OverrideDataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct //vincent 20230303 new fb
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE outNot;
	BYTE in;
	BYTE m_lastIn;
	u32_t onhold;
	u32_t offhold;
	u32_t delayOnTime;
	u32_t delayOffTime;
	u32_t m_timer;
}DLYDATA;



void DlyInit(void *pData, BOOL bDef);
void DlyExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DlyLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DlyDataSize(void);

///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif      //__TIMING_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
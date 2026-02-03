#ifndef __LATCHES_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __LATCHES_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct //vincent 20230303 new fb
{
	int		meta;
	char strName[16];

	BOOL out;
	BOOL in;
	BOOL clock;
	BOOL m_lastClock;

}BOOLEANLATCHDATA;

void BooleanLatchInit(void *pData, BOOL bDef);
void BooleanLatchExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void BooleanLatchLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int BooleanLatchDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BOOL clock;
	BOOL m_lastClock;

	int out;
	int in;
}INTLATCHDATA;

void IntLatchInit(void *pData, BOOL bDef);
void IntLatchExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void IntLatchLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int IntLatchDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BOOL clock;
	BOOL m_lastClock;

	float out;
	float in;
}FLOATLATCHDATA;

void FloatLatchInit(void *pData, BOOL bDef);
void FloatLatchExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FloatLatchLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int FloatLatchDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif      //__LATCHES_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
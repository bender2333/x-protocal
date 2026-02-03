#ifndef __HOSTCOMM_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __HOSTCOMM_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int		meta;
	char strName[16];

	BYTE down;
	u16_t timeOut;
}HOSTCOMMDATA;


void HostCommInit(void *pData, BOOL bDef);
void HostCommExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void HostCommLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int HostCommDataSize(void);
///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif      //__TIMING_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
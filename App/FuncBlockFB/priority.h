#ifndef __PRIORITY_H_F93567A6_11C6_476A_A9F8_A518F0C0C41D
#define __PRIORITY_H_F93567A6_11C6_476A_A9F8_A518F0C0C41D

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
	BYTE enable;
}PRIORITYBINARY;


void PriorityBinaryInit(void *pData, BOOL bDef);
void PriorityBinaryExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void PriorityBinaryLinkWrite(void *pFB, BYTE sourceType, void* pSrcData, BYTE toSlot);
int PriorityBinaryDataSize(void);
//////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE enable;
	float out;
	float in;
}PRIORITYFLOAT;


void PriorityFloatInit(void *pData, BOOL bDef);
void PriorityFloatExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void PriorityFloatLinkWrite(void *pFB, BYTE sourceType, void* pSrcData, BYTE toSlot);
int PriorityFloatDataSize(void);


#ifdef __cplusplus
}
#endif
#endif      //__FMATH_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
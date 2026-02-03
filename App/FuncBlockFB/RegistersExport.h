#ifndef __REGISTERSEXPORT_H_C2BC4AF3_3144_4255_9767_AC5B6550F205
#define __REGISTERSEXPORT_H_C2BC4AF3_3144_4255_9767_AC5B6550F205

#include "App.h"

#define TOTALEXPORTDISCRETE			48
#define TOTALEXPORTCOIL				48
#define TOTALEXPORTINPUTINT16		48
#define TOTALEXPORTINPUTFLOAT		48
#define TOTALEXPORTHOLDINGINT16		48
#define TOTALEXPORTHOLDINGFLOAT		48

typedef struct
{
	int meta;
	char strName[16];

	BYTE in;
	short addr;
}BOOLDATA;

typedef struct
{
	int meta;
	char strName[16];

	BYTE out;
	BYTE in;
	BYTE m_prevIn;
	short addr;
}BOOLWRDATA;

typedef struct
{
	int meta;
	char strName[16];

	short addr;
	short in;
}SHORTDATA;

typedef struct
{
	int meta;
	char strName[16];

	short addr;
	short out;
	short in;
	short m_prevIn;
}SHORTWRDATA;

typedef struct
{
	int meta;
	char strName[16];

	short addr;
	int in;
}LONGDATA;

typedef struct
{
	int meta;
	char strName[16];

	short addr;
	int out;
	int in;
	int m_prevIn;
}LONGWRDATA;


typedef struct
{
	int meta;
	char strName[16];

	short addr;
	float in;
}FLOATDATA;

typedef struct
{
	int meta;
	char strName[16];

	short addr;
	float out;
	float in;
	float m_prevIn;
}FLOATWRDATA;

#ifdef __cplusplus
extern "C" {
#endif

extern BYTE ExportDiscreteInput[(TOTALEXPORTDISCRETE + 7)/8];
extern BYTE ExportCoilOutput[(TOTALEXPORTCOIL + 7)/8];

extern u16_t ExportInputRegisterInt16[TOTALEXPORTINPUTINT16];
extern float ExportInputRegisterFloat[TOTALEXPORTINPUTFLOAT];

extern u16_t ExportHoldingRegisterInt16[TOTALEXPORTHOLDINGINT16];
extern float ExportHoldingRegisterFloat[TOTALEXPORTHOLDINGFLOAT];

void RegistersExportInit(void);
void RegistersExportObj(void);

void FBRegistersExportBoolInit(void *pData, BOOL bDef);
void FBRegistersExportBoolExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FBRegistersExportBoolWrInit(void *pData, BOOL bDef);
void FBRegistersExportBoolWrExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FBRegistersExportShortInit(void *pData, BOOL bDef);
void FBRegistersExportShortExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FBRegistersExportShortWrInit(void *pData, BOOL bDef);
void FBRegistersExportShortWrExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FBRegistersExportLongInit(void *pData, BOOL bDef);
void FBRegistersExportLongExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FBRegistersExportLongWrInit(void *pData, BOOL bDef);
void FBRegistersExportLongWrExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FBRegistersExportFloatInit(void *pData, BOOL bDef);
void FBRegistersExportFloatExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FBRegistersExportFloatWrInit(void *pData, BOOL bDef);
void FBRegistersExportFloatWrExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
int RegistersExportBoolDataSize(void);
int RegistersExportShortDataSize(void);
int RegistersExportLongDataSize(void);
int RegistersExportFloatDataSize(void);
int RegistersExportBoolWrDataSize(void);
int RegistersExportShortWrDataSize(void);
int RegistersExportLongWrDataSize(void);
int RegistersExportFloatWrDataSize(void);
void RegisterBoolLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
void RegisterBoolWrLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
void RegisterShortLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
void RegisterShortWrLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
void RegisterLongLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
void RegisterLongWrLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
void RegisterFloatLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
void RegisterFloatWrLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
void RegistersExportWrDefault(BOOL save);
#ifdef __cplusplus
}
#endif
#endif	/* end of __REGISTERSEXPORT_H_C2BC4AF3_3144_4255_9767_AC5B6550F205 */


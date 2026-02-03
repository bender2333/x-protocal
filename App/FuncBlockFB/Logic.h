#ifndef __LOGIC_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __LOGIC_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

	
typedef struct
{
	int		meta;
	char strName[16];

	BYTE  sel;
	float out1;
	float out2;
	float in;
}ADEMUX2DATA;

void ADemux2Init(void *pData, BOOL bDef);
void ADemux2Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ADemux2LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ADemux2DataSize(void);
///////////////////////////////////////////////////////////////////////


typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE in1;
	BYTE in2;
	BYTE in3;
	BYTE in4;
}AND4DATA;


void And4Init(void *pData, BOOL bDef);
void And4Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void And4LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int And4DataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE in1;
	BYTE in2;
	BYTE in3;
	BYTE in4;
}OR4DATA;


void Or4Init(void *pData, BOOL bDef);
void Or4Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void Or4LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int Or4DataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE in1;
	BYTE in2;
}XORDATA;


void XorInit(void *pData, BOOL bDef);
void XorExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void XorLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int XorDataSize(void);


///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE in;
}NOTDATA;


void NotInit(void *pData, BOOL bDef);
void NotExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void NotLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int NotDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	int		meta;
	char strName[16];

	BYTE s1;
	float out;
	float in1;
	float in2;
}ASWDATA;


void ASWInit(void *pData, BOOL bDef);
void ASWExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ASWLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ASWDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE s1;
	BYTE out;
	BYTE in1;
	BYTE in2;
}BSWDATA;


void BSWInit(void *pData, BOOL bDef);
void BSWExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void BSWLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int BSWDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE s1;
	s32_t out;
	s32_t in1;
	s32_t in2;
}ISWDATA;


void ISWInit(void *pData, BOOL bDef);
void ISWExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ISWLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ISWDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE in;
	BYTE m_lastIn;
}B2PDATA;


void B2PInit(void *pData, BOOL bDef);
void B2PExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void B2PLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int B2PDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	int		meta;
	char strName[16];

	BYTE out1;
	BYTE out2;
	BYTE out3;
	BYTE out4;
	s32_t	in;
	s32_t startsAt;
}DEMUXI2B4DATA;


void DemuxI2B4Init(void *pData, BOOL bDef);
void DemuxI2B4Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DemuxI2B4LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DemuxI2B4DataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE s1;
	float out;
	float in1;
	float in2;
	float in3;
	float in4;
}ASEL4DATA;

void ASEL4Init(void *pData, BOOL bDef);
void ASEL4Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ASEL4LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ASEL4DataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE s1;
	BYTE out;
	BYTE in1;
	BYTE in2;
	BYTE in3;
	BYTE in4;
}BSEL4DATA;


void BSEL4Init(void *pData, BOOL bDef);
void BSEL4Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void BSEL4LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int BSEL4DataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE s1;
	s32_t out;
	s32_t in1;
	s32_t in2;
	s32_t in3;
	s32_t in4;
}ISEL4DATA;


void ISEL4Init(void *pData, BOOL bDef);
void ISEL4Execute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ISEL4LinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ISEL4DataSize(void);

#ifdef __cplusplus
}
#endif
#endif      //__LOGIC_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
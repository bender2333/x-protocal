#ifndef __TYPES_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __TYPES_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	int		meta;
	char strName[16];

	BYTE in1;
	BYTE in2;
	BYTE in3;
	BYTE in4;
	BYTE in5;
	BYTE in6;
	BYTE in7;
	BYTE in8;
	BYTE in9;
	BYTE in10;
	BYTE in11;
	BYTE in12;
	BYTE in13;
	BYTE in14;
	BYTE in15;
	BYTE in16;
	float out;
	float count;
}B2FDATA;



void B2FInit(void *pData, BOOL bDef);
void B2FExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void B2FLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int B2FDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
}CONSTBOOLDATA;

void ConstBoolInit(void *pData, BOOL bDef);
void ConstBoolExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ConstBoolLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ConstBoolDataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct
{
	int		meta;
	char strName[16];

	float out;
}CONSTFLOATDATA;

void ConstFloatInit(void *pData, BOOL bDef);
void ConstFloatExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ConstFloatLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ConstFloatDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	u32_t out;
}CONSTINTDATA;

void ConstIntInit(void *pData, BOOL bDef);
void ConstIntExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ConstIntLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ConstIntDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out1;
	BYTE out2;
	BYTE out3;
	BYTE out4;
	BYTE out5;
	BYTE out6;
	BYTE out7;
	BYTE out8;
	BYTE out9;
	BYTE out10;
	BYTE out11;
	BYTE out12;
	BYTE out13;
	BYTE out14;
	BYTE out15;
	BYTE out16;

	BYTE ovrf;
	float in;
}F2BDATA;

void F2BInit(void *pData, BOOL bDef);
void F2BExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void F2BLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int F2BDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	s32_t out;
	float in;
}F2IDATA;

void F2IInit(void *pData, BOOL bDef);
void F2IExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void F2ILinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int F2IDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	s32_t in;
	float out;
}I2FDATA;

void I2FInit(void *pData, BOOL bDef);
void I2FExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void I2FLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int I2FDataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct //vincent 20230303 new fb
{
	int		meta;
	char strName[16];

	BYTE in1;
	BYTE in2;
	BYTE in3;
	BYTE in4;
	BYTE in5;
	BYTE in6;
	BYTE in7;
	BYTE in8;
	BYTE in9;
	BYTE in10;
	BYTE in11;
	BYTE in12;
	BYTE in13;
	BYTE in14;
	BYTE in15;
	BYTE in16;
	u32_t out;
	u32_t count;
}B2IDATA;



void B2IInit(void *pData, BOOL bDef);
void B2IExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void B2ILinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int B2IDataSize(void);

////////////////////////////////// P0 /////////////////////////////////////

typedef struct //yfy 20250513 new fb
{
	int	 meta;
	char strName[16];

	BYTE in1;
	BYTE in2;
	u16_t out;
}PACKSHORTDATA;

void PackShortInit(void *pData, BOOL bDef);
void PackShortExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void PackShortLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int PackShortDataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct //yfy 20250513 new fb
{
	int	 meta;
	char strName[16];

	BYTE in1;
	BYTE in2;
	BYTE in3;
	BYTE in4;
	u32_t out;
}PACKLONGDATA;

void PackLongInit(void *pData, BOOL bDef);
void PackLongExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void PackLongLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int PackLongDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct //yfy 20250513 new fb
{
	int	 meta;
	char strName[16];

	BYTE out1;
	BYTE out2;
	u16_t in;
}UNPACKSHORTDATA;

void UnpackShortInit(void *pData, BOOL bDef);
void UnpackShortExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void UnpackShortLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int UnpackShortDataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct //yfy 20250513 new fb
{
	int	 meta;
	char strName[16];

	BYTE out1;
	BYTE out2;
	BYTE out3;
	BYTE out4;
	u32_t in;
}UNPACKLONGDATA;

void UnpackLongInit(void *pData, BOOL bDef);
void UnpackLongExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void UnpackLongLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int UnpackLongDataSize(void);

///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif      //__TYPES_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
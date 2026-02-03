#ifndef __FMATH_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __FMATH_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in1;
  float in2;
} ADD2DATA;

void Add2Init(void *pData, BOOL bDef);
void Add2Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Add2LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Add2DataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  BYTE m_firstTen;
  BYTE m_index;

  u16_t maxTime;
  u32_t m_timer;

  float in;
  float out;
  float m_lastValue;

  float m_samples[10];
} AVG10DATA;

void Avg10Init(void *pData, BOOL bDef);
void Avg10Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Avg10LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Avg10DataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  BYTE m_first;

  u16_t dampingFactor;
  u16_t m_prevDampingFactor;

  float in;
  float out;
  float m_total;
  float m_avg;
} AVGNDATA;

void AvgNInit(void *pData, BOOL bDef);
void AvgNExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void AvgNLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int AvgNDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  BYTE div0;

  float out;
  float dividend;
  float divisor;
} DIV2DATA;

void Div2Init(void *pData, BOOL bDef);
void Div2Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Div2LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Div2DataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
  float offset;
} FLOATOFFSETDATA;

void FloatOffsetInit(void *pData, BOOL bDef);
void FloatOffsetExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void FloatOffsetLinkWrite(void *pFBData, BYTE sourceType, void *pData,
                          BYTE slot);
int FloatOffsetDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in1;
  float in2;
} MAXDATA;

void MaxInit(void *pData, BOOL bDef);
void MaxExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void MaxLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int MaxDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in1;
  float in2;
} MINDATA;

void MinInit(void *pData, BOOL bDef);
void MinExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void MinLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int MinDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  BYTE reset;
  BYTE m_first;
  float minOut;
  float maxOut;
  float in;
} MINMAXDATA;

void MinMaxInit(void *pData, BOOL bDef);
void MinMaxExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void MinMaxLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int MinMaxDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in1;
  float in2;
} MUL2DATA;

void Mul2Init(void *pData, BOOL bDef);
void Mul2Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Mul2LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Mul2DataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
} NEGDATA;

void NegInit(void *pData, BOOL bDef);
void NegExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void NegLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int NegDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  s32_t decimalPlaces;
  float out;
  float in;
} ROUNDDATA;

void RoundInit(void *pData, BOOL bDef);
void RoundExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void RoundLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int RoundDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float minuend;
  float subtrahend;
} SUB2DATA;

void Sub2Init(void *pData, BOOL bDef);
void Sub2Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Sub2LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Sub2DataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  BYTE m_first;
  u16_t time;
  u16_t scanPeriod;
  u16_t m_execCount;

  u32_t m_timer;

  float m_sum;
  float out;
  float in;
} TIMEAVGDATA;

void TimeAvgInit(void *pData, BOOL bDef);
void TimeAvgExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void TimeAvgLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int TimeAvgDataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
} SQUAREROOTDATA;

void SquareRootInit(void *pData, BOOL bDef);
void SquareRootExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void SquareRootLinkWrite(void *pFBData, BYTE sourceType, void *pData,
                         BYTE slot);
int SquareRootDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
} INVERSEDATA;

void InverseInit(void *pData, BOOL bDef);
void InverseExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void InverseLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int InverseDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
} CEILINGDATA;

void CeilingInit(void *pData, BOOL bDef);
void CeilingExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void CeilingLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int CeilingDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
} FLOORDATA;

void FloorInit(void *pData, BOOL bDef);
void FloorExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void FloorLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int FloorDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
} FABSDATA;

void FabsInit(void *pData, BOOL bDef);
void FabsExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void FabsLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int FabsDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
} EXPDATA;

void ExpInit(void *pData, BOOL bDef);
void ExpExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void ExpLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int ExpDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float x;
  float y;
} POWDATA;

void PowInit(void *pData, BOOL bDef);
void PowExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void PowLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int PowDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
} LOGDATA;

void LogInit(void *pData, BOOL bDef);
void LogExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void LogLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int LogDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in;
} LOG10DATA;

void Log10Init(void *pData, BOOL bDef);
void Log10Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Log10LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Log10DataSize(void);

///////////////////////////////////////////////////////////////////////

typedef struct // vincent 20230303 new fb
{
  int meta;
  char strName[16];

  float out;
  float in1;
  float in2;
  float in3;
  float in4;
} ADD4DATA;

void Add4Init(void *pData, BOOL bDef);
void Add4Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Add4LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Add4DataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in1;
  float in2;
  float in3;
  float in4;
} MAX4DATA;

void Max4Init(void *pData, BOOL bDef);
void Max4Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Max4LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Max4DataSize(void);
///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in1;
  float in2;
  float in3;
  float in4;
} MIN4DATA;

void Min4Init(void *pData, BOOL bDef);
void Min4Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Min4LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Min4DataSize(void);
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  float out;
  float in1;
  float in2;
  float in3;
  float in4;
} AVG4DATA;

void Avg4Init(void *pData, BOOL bDef);
void Avg4Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Avg4LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int Avg4DataSize(void);
///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  u32_t out;
  u32_t in1;
  u32_t in2;
} MULINT2DATA;

void MulInt2Init(void *pData, BOOL bDef);
void MulInt2Execute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void MulInt2LinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int MulInt2DataSize(void);

//////////////////////////////// P1 ///////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  BYTE div0;

  float out;
  float in;
  float mod;
} MODDATA;

void MODInit(void *pData, BOOL bDef);
void MODExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void MODLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);
int MODDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  BYTE enable; // 使能 0/1
  // 输出
  float Out; // 随机输出值

  // 输入
  float HighLimit; // 上限
  float LowLimit;  // 下限
  u32_t Period;    // 更新周期（秒）

  // 内部状态
  u32_t m_timer; // 定时器时间戳
} RANDGENDATA;

void RandGenInit(void *pData, BOOL bDef);
void RandGenExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void RandGenLinkWrite(void *pFB, BYTE sourceType, void *pSrcData, BYTE toSlot);
int RandGenDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct {
  int meta;
  char strName[16];

  // 输出
  float Y; // 计算结果

  // 输入
  float x; // 自变量
  float a0;
  float a1;
  float a2;
  float a3;
  float a4;

} POLYNOMIALCALDATA;

void PolynomialCalInit(void *pData, BOOL bDef);
void PolynomialCalExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void PolynomialCalLinkWrite(void *pFB, BYTE sourceType, void *pSrcData,
                            BYTE toSlot);
int PolynomialCalDataSize(void);

///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif //__FMATH_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
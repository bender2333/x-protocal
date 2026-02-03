#ifndef __FUNC_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __FUNC_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int		meta;
	char strName[16];

	BYTE xgy;
	BYTE xey;
	BYTE xly;
	float x;
	float y;
}CMPRDATA;


void CmprInit(void *pData, BOOL bDef);
void CmprExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void CmprLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int CmprDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE in;
	BYTE dir;
	BYTE enable;
	BYTE reset;
	BYTE m_lastIn;
	s32_t out;
	s32_t preset;
}COUNTDATA;

void CountInit(void *pData, BOOL bDef);
void CountExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void CountLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int CountDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	int		meta;
	char strName[16];

	BYTE	in;
	BYTE	m_completeCycle;
	BYTE	m_lastIn;
	u32_t	m_lastEdge;
	float	pps;
	float	ppm;
}FREQDATA;

void FreqInit(void *pData, BOOL bDef);
void FreqExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FreqLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int FreqDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	float in;
	float risingEdge;
	float fallingEdge;
}HYSTERESISDATA;

void HysteresisInit(void *pData, BOOL bDef);
void HysteresisExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void HysteresisLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int HysteresisDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE m_firstHalf;
	s32_t out;
	s32_t min;
	s32_t max;
	s32_t delta;
	s32_t secs;
	u32_t m_lastUpdate;
}IRAMPDATA;

void IRampInit(void *pData, BOOL bDef);
void IRampExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void IRampLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int IRampDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	float out;
	float in;
	float lowLmt;
	float highLmt;
}LIMITERDATA;

void LimiterInit(void *pData, BOOL bDef);
void LimiterExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void LimiterLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int LimiterDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	float out;
	float in;
	float x0;
	float y0;
	float x1;
	float y1;
	float x2;
	float y2;
	float x3;
	float y3;
	float x4;
	float y4;
	float x5;
	float y5;
	float x6;
	float y6;
	float x7;
	float y7;
	float x8;
	float y8;
	float x9;
	float y9;
}LINEARIZEDATA;

void LinearizeInit(void *pData, BOOL bDef);
void LinearizeExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void LinearizeLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int LinearizeDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE rampType;
	BYTE m_firstHalf;
	u16_t period;
	u16_t scanPeriod;

	u32_t m_timer;

	float out;
	float min;
	float max;
	float m_delta;

}RAMPDATA;

void RampInit(void *pData, BOOL bDef);
void RampExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void RampLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int RampDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE set;
	BYTE reset;
	BYTE m_lastSet;
	BYTE m_lastReset;

}SRLATCHDATA;

void SRLatchInit(void *pData, BOOL bDef);
void SRLatchExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void SRLatchLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int SRLatchDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE ticksPerSec;

	u32_t m_timer;

}TICKTOCKDATA;

void TickTockInit(void *pData, BOOL bDef);
void TickTockExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void TickTockLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int TickTockDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE in;
	BYTE ovr;
	BYTE rst;
	BYTE cDwn;
	BYTE holdAtLimit;
	BYTE m_lastIn;
	float out;
	float limit;

}UPDNDATA;

void UpDnInit(void *pData, BOOL bDef);
void UpDnExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void UpDnLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int UpDnDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct  //vincent 20230303 new fb
	{
		int		meta;
		char strName[16];

		BYTE in;
		BYTE dir;
		BYTE enable;
		BYTE reset;
		BYTE m_lastIn;

		s32_t out;
		s32_t preset;
		s32_t upLimit;
		s32_t downLimit;

	}COUNTLIMITDATA;

	void CountLimitInit(void *pData, BOOL bDef);
	void CountLimitExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
	void CountLimitLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
	int CountLimitDataSize(void);

///////////////////////////////////////// P0 //////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE in;
	BYTE m_lastIn;
	BYTE edge; //rising or falling
	BYTE hold;
	BYTE reset;

}EDGETRIGDATA;

void EdgeTrigInit(void *pData, BOOL bDef);
void EdgeTrigExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void EdgeTrigLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int EdgeTrigDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE out;
	BYTE hold;
	BYTE reset;

	float in;
	float m_lastIn;
	float cov;
	
}COVTRIGDATA;

void COVTrigInit(void *pData, BOOL bDef);
void COVTrigExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void COVTrigLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int COVTrigDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char 	strName[16];

	BYTE 	out;

	u8_t 	DutyCycle;
	u32_t 	Period;

	u32_t 	m_timer;

}PWMDATA;

void PWMInit(void *pData, BOOL bDef);
void PWMExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void PWMLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int PWMDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char 	strName[16];

	bool	first;

	float 	in;
	float 	out;
	u32_t	sampleCount;
	u32_t 	scanPeriod;

	u32_t 	m_timer;

}COVSTABILIZERDATA;

void COVStabilizerInit(void *pData, BOOL bDef);
void COVStabilizerExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void COVStabilizerLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int COVStabilizerDataSize(void);

///////////////////////////////////////// P1 //////////////////////////////////////////////
typedef struct
{
    int		meta;
	char strName[16];

	BOOL out;
	BOOL in;
    BOOL mode;
    BYTE OnCount;
    BYTE samplingCount;
    BYTE remainCount;
    BOOL intermediade;
    u32_t period;
    u32_t m_timer;
} DIFILTERDATA;

void DIFilterInit(void *pData, BOOL bDef);
void DIFilterExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DIFilterLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DIFilterDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    int		meta;
	char strName[16];

    BOOL mode;
	float out;
	float in;
    float smoothFactor;
    float prevOut;
    float input_buffer[10];
    uint32_t index;
} AIFILTERDATA;

void AIFilterInit(void *pData, BOOL bDef);
void AIFilterExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void AIFilterLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int AIFilterDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    int		meta;
	char strName[16];

	float out;
	float in;
    float prevIn;
    u32_t period;
    u32_t m_timer;
} AICHNGRATECALDATA;

void AIChngRateCalInit(void *pData, BOOL bDef);
void AIChngRateCalExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void AIChngRateCalLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int AIChngRateCalDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
	
    BYTE enable;
    float out;
    float in;
    float lowlimit;
    float highlimit;
    u32_t period;
    u32_t m_timer;
} AUTOINCDECDATA;

void AutoIncDecInit(void *pData, BOOL bDef);
void AutoIncDecExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void AutoIncDecLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int AutoIncDecDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

    BOOL enable;          // 使能
    BOOL devOn[8];        // 设备开机
    BOOL avail_Dev[8];    // 各设备可用状态
    BOOL status_Dev[8];   // 各设备开机状态
    BYTE devQty;         // 设备总数量
    BYTE reqQty;         // 需求数量

	BOOL bSort;

    u16_t chngHours;      // 定周期切换时间
    u16_t runtime_Dev[8]; // 各设备总累计运行时间
    u16_t cntnrt_Dev[8];  // 本设备连续运行时间

} PRIO8DEVDATA;
void Prio8DevInit(void *pData, BOOL bDef);
void Prio8DevExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Prio8DevLinkWrite(void *pFB, BYTE sourceType, void *pSrcData, BYTE toSlot);
int Prio8DevDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

    BOOL enable;          // 使能
    BOOL devOn[32];        // 设备开机
    BOOL avail_Dev[32];    // 各设备可用状态
    BOOL status_Dev[32];   // 各设备开机状态
    BYTE devQty;         // 设备总数量
    BYTE reqQty;         // 需求数量

	BOOL bSort;

    u16_t chngHours;      // 定周期切换时间
    u16_t runtime_Dev[32]; // 各设备总累计运行时间
    u16_t cntnrt_Dev[32];  // 本设备连续运行时间

} PRIO32DEVDATA;
void Prio32DevInit(void *pData, BOOL bDef);
void Prio32DevExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void Prio32DevLinkWrite(void *pFB, BYTE sourceType, void *pSrcData, BYTE toSlot);
int Prio32DevDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	// 输入
	BOOL enable;		// 使能 0/1
	BOOL Burst;		// 快速加载 0/1
	BYTE OpMode;	// 0=Cooling, 1=Heating

	BYTE ReqQty; // 需求台数
	BYTE TotalQty; // 设备总台数

	float SP;		// 设定值
	float PV;		// 过程值
	float DZ;		// 死区
	u32_t Period;	// 周期（秒）
		
	// 内部状态
	u32_t m_Timer; // 当前方向开始计时刻

} STEPCTRLDATA;

void StepCtrlInit(void *pData, BOOL bDef);
void StepCtrlExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void StepCtrlLinkWrite(void *pFB, BYTE sourceType, void *pSrcData, BYTE toSlot);
int StepCtrlDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	BYTE ReqQty;	// 负荷需求台数
	BYTE TotalQty; 	// 设备总台数（0..16）
	BYTE activeStages;		 // 当前已启用的台数
	BYTE Stage[16]; // Stage1..Stage16 启停命令

	float ReqLoad;	// 需求百分比 0..100
	float DZ;		// 死区百分比 0..100
	u32_t Interval; // 启停间隔（秒）

	// 内部状态
	u32_t m_timer;		 // 上次启/停变化时刻

} LOADASSIGNDATA;

void LoadAssignInit(void *pData, BOOL bDef);
void LoadAssignExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void LoadAssignLinkWrite(void *pFB, BYTE sourceType, void *pSrcData, BYTE toSlot);
int LoadAssignDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif      //__FUNC_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
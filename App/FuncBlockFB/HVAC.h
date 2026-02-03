#ifndef __HAVC_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
#define __HAVC_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0

#include "App.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////
#define PSYCHROMETRICSCALE_CELCIUS		0
#define PSYCHROMETRICSCALE_FAHRENHEIT	1
#define PSYCHROMETRICSCALE_LAST			2

typedef struct
{
	int		meta;
	char	strName[16];

	u16_t	tempScale;
	u32_t	m_Timer;
	float   ambientTemp;
	float	relHumidity;
	float	dewPoint;
	float   vaporPressure;
	float   satPressure;
	float	enthalpy;
	float	wetBulb;
}PSYCHROMETRICDATA;

void PsychrometricInit(void *pData, BOOL bDef);
void PsychrometricExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void PsychrometricLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int PsychrometricDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

	BYTE	fb;
	BYTE	open;
	BYTE	close;
	BYTE	m_DriveStatus;
	BYTE	m_DriveCheck;

	u16_t	travelTime;
	u16_t	pos;
	u16_t	m_DrivePos;
	u16_t	m_DriveCurrentPos;
	u16_t	m_DriveTrueEndPos;
	u32_t	m_Timer;

	float	in;
	float	feedback;
	float	highScale;
	float	lowScale;
	float	hystr;
	float	m_DrivePrevValue;
	
}DRIVEDATA;

void DriveInit(void *pData, BOOL bDef);
void DriveExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void DriveLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int DriveDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
    BLOOPACTION_DIRECT = 0,
    BLOOPACTION_REVERSE = 1,
}BLOOPACTION;

typedef enum
{
	BDISABLEACTION_ZERO = 0,
    BDISABLEACTION_MAX_VALUE = 1,
    BDISABLEACTION_MIN_VALUE = 2,
    BDISABLEACTION_HOLD = 3,
    
}BDISABLEACTION;

#define LOOP_MAX_EXECUTE_TIME		60000
#define LOOP_MIN_EXECUTE_TIME		100
#define LOOP_DEFAULT_EXECUTE_TIME	500

typedef struct
{
	int		meta;
	char strName[16];

    /* exposed parameters */
    BYTE	enable;
	BYTE	loopAction;
	BYTE	disableAction;
	BYTE	resetIntegral;
	BYTE	m_prevEnable;

	u16_t	rampTime;		//seconds
	u16_t	cycleTime;	//milliseconds

	u32_t	m_lastExecuteTime;
	u32_t	m_timer;
	u32_t	m_rampEndTicks;

	float	out;
	float	setPoint;
	float	controlledVariable;
	float	proportionalConstant;
	float	integralConstant;
	float	derivativeConstant;
	float	bias;
	float	maxOutput;
	float	minOutput;

	float   m_kProportionalCurrent;
	float   m_kIntegralCurrent;    
	
	double  m_errorSum;
	double  m_lastError;

} LOOPDATA;

void LoopInit(void *pData, BOOL bDef);
void LoopExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void LoopLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int LoopDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char strName[16];

    BYTE    state;
	BYTE    prevState;
	u32_t	time;
	u32_t	timer;
}RUNTIMEDATA;


void RunTimeInit(void *pData, BOOL bDef);
void RunTimeExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void RunTimeLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int RunTimeDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	BYTE out;
	BYTE enable;
	BYTE mode;
	float cv;
	float sp;
	float cutIn;
	float cutOut;
}THERMOSTATDATA;

void ThermostatInit(void *pData, BOOL bDef);
void ThermostatExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ThermostatLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ThermostatDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	BYTE dOn;
	BYTE ovfl;
	BYTE numOuts;

	float in;
	float inMin;
	float inMax;
	float delta;

	BYTE out[16];
}LSEQDATA;

void LSeqInit(void *pData, BOOL bDef);
void LSeqExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void LSeqLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int LSeqDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	float out;
	float in;
	float inMin;
	float inMax;
	float outMin;
	float outMax;
}RESETDATA;

void ReSetInit(void *pData, BOOL bDef);
void ReSetExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ReSetLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ReSetDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	BYTE enable;
	BYTE dOn;

	float in;
	float hysteresis;
	BYTE out[4];
	float threshold[4];
}REHEATSEQDATA;

void ReheatSeqInit(void *pData, BOOL bDef);
void ReheatSeqExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ReheatSeqLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ReheatSeqDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////

//YIN
//typedef struct
//{
//	int meta;
//	char strName[16];
//
//	BYTE In;
//	BYTE NumberOutputs;
//	BYTE Feedback;
//	BOOL bClearAlarm;
//	BOOL Out[10];
//	BOOL isAlarmOn[10];
//
//	DWORD maxRuntime;
//	DWORD m_maxRuntimeTimer;
//	
//	DWORD feedbackDelay;
//	DWORD m_feedbackDelayTimer;
//	DWORD alarmClear;
//	DWORD m_alarmClearTimer;
//	
//	int CycleCount[10];
//	
//}LEADLAGCYCLEDATA;
//
//void LeadLagCycleInit(void *pData, BOOL bDef);
//void LeadLagCycleExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
//void LeadLagCycleLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
//int LeadLagCycleDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	int meta;
	char strName[16];
	float In;
	int MaxStages;
	WORD MinOntime;
//	DWORD m_MinOntimer;
	WORD MinOfftime;
//	DWORD m_MinOfftimer;
	WORD InterStgOntime;
	DWORD m_CyclerLoopStaticOntimer;
	WORD InterStgOfftime;
	DWORD m_CyclerLoopStaticOfftimer;
	BOOL bManualOverrideOff;
	int ActiveStages;
//	DWORD m_Ontimer;
//	DWORD m_Offtimer;
	BOOL Direction;
	float hyst;
}STAGERDATA;

void StagerInit(void *pData, BOOL bDef);
void StagerExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void StagerLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int StagerDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
	float In;
	int MaxStages;
	WORD MinOntime;
//	DWORD m_MinOntimer;
	WORD MinOfftime;
//	DWORD m_MinOfftimer;
	WORD InterStgOntime;
	DWORD m_CyclerLoopStaticOntimer;
	WORD InterStgOfftime;
	DWORD m_CyclerLoopStaticOfftimer;
	BOOL bManualOverrideOff;
	int ActiveStages;
//	DWORD m_Ontimer;
//	DWORD m_Offtimer;
	BOOL Direction;
	float hyst;
	BYTE cph;
	float antAuth;
	float CyclerLoopStaticAntic;
}CYCLERDATA;

void CyclerInit(void *pData, BOOL bDef);
void CyclerExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void CyclerLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int CyclerDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
	float cmdFlowPercent;
	float sensorflowvol;
	float minFlowSetpt;
	float maxFlowSetpt;
	BYTE ManualOverride;
	float ManualOverrideValue;
	float effectiveFlowSetpt;
	float damperPosition;
}FLOWCONTROLDATA;

void FlowcontrolInit(void *pData, BOOL bDef);
void FlowcontrolExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void FlowcontrolLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int FlowcontrolDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
//YIN
typedef struct
{
	int meta;
	char strName[16];

	BOOL revAct;
	
	int deriveGain;

	float sensor;
	float setPt;
	float tr;
	float minAOchange;
	float maxAOchange;
	float Db;
	float propErr;
	float AiaLoopStaticolderr;
	float Out;
	
}AIADATA;

void AIAInit(void *pData, BOOL bDef);
void AIAExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void AIALinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int AIADataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
	BYTE effOccuCurrentState;
	float resetInput;
	float reset0Pct;
	float reset100Pct;
	float resetAmount;
	float occupiedSP;
	float standbySP;
	float unoccupiedSP;
	float effSP;
}GENSPCALDATA;

void GenSPCalInit(void *pData, BOOL bDef);
void GenSPCalExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void GenSPCalLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int GenSPCalDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
	BYTE SchCrrState;
	BYTE WMOverride;
	BYTE PrevWMOverride;
	BYTE NetworkManOcc;
	BYTE PrevNetworkManOcc;
	BYTE OccSensorState;
	BYTE NetLastinWin;
	BYTE OccSensorOperation;
	BYTE EffOccCrrState;
	BYTE ManOverrideState;
}OCCARBDATA;

void OccArbInit(void *pData, BOOL bDef);
void OccArbExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void OccArbLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int OccArbDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
	BYTE sysSwitch;
	BYTE cmdMode;
	float supplyTemp;
	float spaceTemp;
	float effHeatSP;
	float effCoolSP;
	BOOL allowAutoChange;
	float effSP;
	BYTE effTempMode;
	BYTE controlType;
	float effModeArb;
	float prevModeArb;
	float prevTempMode;
}SETTEMPMODEDATA;

void SetTempModeInit(void *pData, BOOL bDef);
void SetTempModeExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void SetTempModeLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int SetTempModeDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
	BYTE EffOccCrrState;
	BYTE SchNextState;
	WORD SchTUNCOS;
	float SP;
	float HeatRampRate;
	float CoolRampRate;
	BYTE ManOverrideState;
	float EffHeatSP;
	float EffCoolSP;
	float OccupiedCool;
	float StandbyCool;
	float UnOccupiedCool;
	float OccupiedHeat;
	float StandbyHeat;
	float UnOccupiedHeat;
}TEMPSPCALDATA;

void TempSPCalInit(void *pData, BOOL bDef);
void TempSPCalExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void TempSPCalLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int TempSPCalDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
	BYTE inStagesActive;
	BYTE stagesActive;
	BYTE prevStagesActive;
	BYTE startStage;
	BYTE endStage;
	BOOL isInit;
	BYTE runtimeReset;
	BOOL Stage[16];
	WORD stgStsOut;
	WORD offset;
	BYTE maxStage;
	BYTE prevMaxStage;
	BYTE leadLag;
	DWORD runTimeForOffStgs[16];
	DWORD runTimeForOnStgs[16];
}STAGEDRIVERDATA;

void StageDriverInit(void *pData, BOOL bDef);
void StageDriverExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void StageDriverLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int StageDriverDataSize(void);
	
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	BYTE out[4];
	BYTE enable;
	BYTE alarm;
	WORD OutputDelay;

	DWORD m_delayTimer;
	float powerSeg[4];
	float in;
	float m_previn;
	float offset;
	float hysteresis;
	float totalOutput;
}REHEATCONTROLDATA;

void ReheatControlInit(void *pData, BOOL bDef);
void ReheatControlExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void ReheatControlLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int ReheatControlDataSize(void);
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	BYTE In;
	BYTE NumberOutputs;
	BYTE Feedback;
	BOOL enableFeedback;
	BOOL bClearAlarm;
	BOOL m_bWasOn;
	BOOL Out[10];
	BOOL isAlarmOn[10];

	DWORD maxRuntime;
	DWORD m_maxRuntimeTimer;
	
	DWORD feedbackDelay;
	DWORD m_feedbackDelayTimer;
	DWORD alarmClear;
	DWORD m_alarmClearTimer;
	
	int CycleCount[10];
	
}LEADLAGCYCLEDATA;

void LeadLagCycleInit(void *pData, BOOL bDef);
void LeadLagCycleExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void LeadLagCycleLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int LeadLagCycleDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	BYTE In;
	BYTE NumberOutputs;
	BYTE Feedback;
	BOOL enableFeedback;
	BOOL bClearAlarm;
	BOOL m_bWasOn;
	BOOL Out[10];
	BOOL isAlarmOn[10];

	DWORD maxRuntime;
	DWORD m_maxRuntimeTimer;
	
	DWORD feedbackDelay;
	DWORD m_feedbackDelayTimer;
	DWORD alarmClear;
	DWORD m_alarmClearTimer;
	
	int RunTime[10];
	
}LEADLAGRUNTIMEDATA;

void LeadLagRuntimeInit(void *pData, BOOL bDef);
void LeadLagRuntimeExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void LeadLagRuntimeLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int LeadLagRuntimeDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
    
	BOOL        on;
	BOOL        schedule;
	BOOL        loadFailed;
	BOOL        out[10];
	BOOL        alarm[10];
	BOOL        feedback[10];
	BYTE        numberOutputs;
	BYTE        requiredLoads;
	BYTE        rotateMode;
	BYTE        nextLoad;
	u32_t       rotateInterval;
	u32_t       runTime[10];
	u32_t       sortedRunTime[10];
	u32_t       runTimeRemainingUntilShift;
	u32_t       leadRunTimeAtStart;
	u32_t       feedBackDelay;
	u32_t       m_feedBackTimer;
}RUNTIMEBALANCERDATA;

void RunTimeBalancerInit(void *pData, BOOL bDef);
void RunTimeBalancerExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void RunTimeBalancerLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int RunTimeBalancerDataSize(void);

///////////////////////////////////// P0 //////////////////////////////////////////////////////
typedef struct
{
	int		meta;
	char 	strName[16];

	bool	b_first;
	float	temp;
	float	res;
	float	c1;
	float	c2;
	float	c3;
	float	xid;
	float	last_res;

}NTCRT2NTCDATA;

void NtcRT2NTCInit(void *pData, BOOL bDef);
void NtcRT2NTCExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void NtcRT2NTCLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int NtcRT2NTCDataSize(void);

///////////////////////////////////////////////////////////////////////
	
typedef struct
{
	int meta;
	char strName[16];
	
	//open, close, hold position
	BYTE in;
	BOOL open;
	BOOL bclose;
	BOOL emergencyHold;

	float fb;
	float expectation;

	float minDuration;
	float fullDuration;
	uint32_t m_Timer;	
}POS3CMDDATA;

void Pos3CmdInit(void *pData, BOOL bDef);
void Pos3CmdExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void Pos3CmdLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int Pos3CmdDataSize(void);

///////////////////////////////////////////////////////////////////////
	
typedef struct
{
	int meta;
	char strName[16];
	
	//backup or rotation mode
	BYTE mode;
	BOOL enable;
	BOOL primaryPump;
	BOOL backupPump;
	BOOL errorFb;

	uint32_t rotationPeriod; //mins
	uint32_t m_Timer;	
}TWINPUMPDATA;

void TwinPumpInit(void *pData, BOOL bDef);
void TwinPumpExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void TwinPumpLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int TwinPumpDataSize(void);

///////////////////////////////////////////////////////////////////////
	
typedef struct
{
	int meta;
	char strName[16];

	 /* exposed parameters */
    bool	enable;
	bool    error;

	uint32_t	m_lastExecuteTime;

	float	setPoint;
	float	presentValue;
	float	kp; //proportional constant
	float	ti; //integral constant
	float   opd; //operation point difference

	float   m_kIntegralCurrent;

	float 	spvc;
	float 	spvh;

}CASLEACTRLDATA;

void CasLeaCtrlInit(void *pData, BOOL bDef);
void CasLeaCtrlExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void CasLeaCtrlLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int CasLeaCtrlDataSize(void);

////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

    float Diameter;
    float PressureDiff;
    float Length;
    float Viscosity;
    float FlowRate;
}PRES2FLOWDATA;

void Pres2FlowInit(void *pData, BOOL bDef);
void Pres2FlowExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void Pres2FlowLinkWrite(void *pFB, BYTE sourceType, void* pSrcData, BYTE toSlot);
int Pres2FlowDataSize(void);

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];
	
	BYTE mode;
	BOOL bit1;
	BOOL bit2;
	BOOL bit3;

	float in;
	float lowLimit;
	float highLimit;
	
}BIT3EHTRDATA;

void Bit3EHtrInit(void *pData, BOOL bDef);
void Bit3EHtrExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void Bit3EHtrLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int Bit3EHtrDataSize(void);

///////////////////////////////// P1 //////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

	// 输出
	float AO;  // PID 输出 0..100
	float Inc; // 增量输出

	// 输入
	BYTE EN;			// 使能
	BYTE INV;			// 0=正比例(输出随 PV-SP 增大而增大)，1=反比例
	BYTE ERPRV;			// 主传感器故障
	BYTE DisableAction; // 失能时输出：Min/Max/Hold/Zero
	BYTE ResetIntegral; // 积分清零(上升沿)
	BYTE m_prevEN;		 // 上一帧 EN
	BYTE m_prevReset;	 // 上一帧 ResetIntegral
	BYTE m_firstRun;	 // 首帧
	float PV;			// 过程值
	float SP;			// 设定值
	float DZ;			// 死区

	float P;			// Kp
	float I;			// Ki
	float D;			// Kd
	float Max;			// 最大输出
	float Min;			// 最小输出
	float Period;		// 周期(ms)
	float AO_UNA;		// 异常触发 0/1
	float AFBV;			// 异常时输出

	float m_e1;			 // e(k-1)
	float m_e2;			 // e(k-2)
	float m_lastOutput;	 // 上一次输出
	u32_t RampTime;		// 斜坡时间(s)，0=不限制
	u32_t m_timer; // 上次执行时间戳
} PIDINCDATA;

void PidIncInit(void *pData, BOOL bDef);
void PidIncExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);
void PidIncLinkWrite(void *pFB, BYTE sourceType, void *pSrcData, BYTE toSlot);
int PidIncDataSize(void);

///////////////////////////////////////////////////////////////////////
typedef struct
{
	int meta;
	char strName[16];

    bool coolHeat;      // 冷/热
    float supplyT;      // 供水温度
    float returnT;      // 回水温度
    float waterFlow;    // 水流量
    float plantPower;   // 总电功率
    bool plantOn;       // 本站开机
    bool reset;         // 复位清零
    float actCop;       // 实时COP
    float actCoolPower; // 实时制冷功率
    float actHeatPower; // 实时制热功率
    float accumCool;    // 总积累冷量
    float accumHeat;    // 总积累热量
    float accumEnergy;  // 总电能
    float accumCoolDay; // 上一日累积冷量
    float accumHeatDay; // 上一日累积热量
    float accumCopDay;  // 上一日COP
    float accumCop;     // 总COP

    u32_t m_timer;
} COOLHEATCALCDATA;

void CoolHeatCalcInit(void *pData, BOOL bDef);
void CoolHeatCalcExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
void CoolHeatCalcLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);
int CoolHeatCalcDataSize(void);

///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif      //__HAVC_H_A8F103CA_3792_4a5d_8E6D_EFD6D96C0CA0
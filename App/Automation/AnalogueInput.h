#ifndef __ANALOGUEINPUT_H_E2B3F8CF_1D1D_4a52_A0DD_5A6563ABF0F3
#define __ANALOGUEINPUT_H_E2B3F8CF_1D1D_4a52_A0DD_5A6563ABF0F3

#include "App.h"

#define AITEMPDAMPINGFACTOR 128 //This is for Sensor Mode, decrease it based on speed requirement

typedef enum
{
    AITYPE_VOLTAGE,
    AITYPE_CURRENT,
    AITYPE_RESISTANCE,
    AITYPE_THERMISTOR,
    AITYPE_DI,
} AI_TYPE;

typedef enum
{
    AI_NO_FAULT,
    AI_OPEN,
    AI_SHORT,
    AI_OVERRANGE,
    AI_UNDERRANGE,
    AI_NOSENSOR,
}AI_RELIABILITY;

////define Alarm Status
#define ALARMOFF 0
#define ALARMHIGH 0x01
#define ALARMLOW 0x02
#define ALARMONHIGH 0x10
#define ALARMONLOW 0x20
#define ALARMON 0x30 

/// @brief UI channel Information ///
typedef struct
{
    int meta;
    char strName[16];
    BYTE channel;
} UIDATASHORT;

/// @brief UI channel data ///
typedef struct
{
    BYTE index;
    BYTE alarm;
    BYTE alarmType;

    BYTE alarmReset;
    BYTE alarmResetType;
    BYTE highAlarmEnable;
    BYTE lowAlarmEnable;
    BYTE linearization;
    BYTE lowCutoffEnable;
    BYTE outOfService;
    BYTE resetMinValue;
    BYTE resetMaxValue;

    short reliability;
    short alarmDelay;
    short type;
    short decimalPoint;
    short temperatureTable; // 4 bytes packed

    float maxValue;
    float minValue;
    float rawValue;

    float value;

    float alarmDeadband;

    float alarmHighLimit;
    float alarmLowLimit;

    float digitalOffLevel;
    float digitalOnLevel;
    float lowCutoffValue;
    float offset;
    float scaleHighValue;
    float scaleLowValue;
    float userSetValue;
    int SCALE;
} UIDATA;

/// @brief UI Object///
typedef struct
{
    ////// Data ////////////////////////////////////////////////////
    UIDATA uidata;

    ////// Operations //////////////////////////////////////////////
    u16_t AnalogueInputTypePre; //+ 2018-02-09
    double AIAverageValue;
    BYTE AIAverageCount;
    float m_AIDecimal[5];
    u8_t m_AIAlarmStatus;
    u16_t m_AIAlarmTime;
    u8_t m_AIDigital;
    u8_t m_AIGain;
} ANALOGINPUTOBJECT;

extern ANALOGINPUTOBJECT AIObj[TOTALANALOGUEINPUTS + MAX_MASTER_AI];
extern u16_t CalibrationGainSet; //Production used

/**
 * @brief Reset AI data to default settings for all channels
 * @param[in] none
 * @return none
 */
void AnalogueInputDefault(void);

/**
 * @brief  AI Object initialization
 * @param[in] none
 * @return none
 */
void AnalogueInputInit(void);

/**
 * @brief  AI Object execution Loop
 * @param[in] none
 * @return none
 */
void AnalogueInputObj(void);

/**
 * @brief Get AI digital state for specified channel
 * @param[in] channel The analog input channel number
 * @return TRUE if ADI is ON, FALSE if ADI is OFF
 */
BOOL AnalogueInputGetADI(BYTE channel);

/**
 * @brief AI Function Block initialization
 * @param[in] pData Pointer to function block data
 * @param[in] bDef Default/First time initialization flag
 * @return none
 */
void FBAnalogueInputInit(void *pData, BOOL bDef);

/**
 * @brief AI function block execution
 * @param[in] pData Pointer to function block data
 * @param[in] nLinks Number of links
 * @param[in] pLinks Pointer to links list
 * @return none
 */
void FBAnalogueInputExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);

/**
 * @brief Get AI function block data size (Info + Data)
 * @param[in] none
 * @return Size of analog input data structure
 */
int AnalogueInputDataSize(void);

/**
 * @brief Write link data to AI function block
 * @param[in] pFB Pointer to the AI function block data
 * @param[in] sourceType Source data type
 * @param[in] pSrcData Pointer to source data
 * @param[in] toSlot Target slot number
 * @return none
 */
void AnalogueInputLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);

/**
 * @brief   Direct set AI function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The AI channel number
 * @param[in] pBuf Buffer containing the aodata to set
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int AnalogueInputSetIOData(void *pCompData, int channel, void *pBuf, int dataSize);

/**
 * @brief Get AI function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The AI channel number
 * @param[out] pBuf Buffer to store the data
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int AnalogueInputGetIOData(void *pCompData, int channel, void *pBuf, int dataSize);
#endif /* end of __ANALOGUEINPUT_H_E2B3F8CF_1D1D_4a52_A0DD_5A6563ABF0F3 */

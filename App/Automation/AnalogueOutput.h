#ifndef __ANALOGUEOUTPUT_H_48C7536F_1E91_4cb6_8942_A8AA103CFB11
#define __ANALOGUEOUTPUT_H_48C7536F_1E91_4cb6_8942_A8AA103CFB11

#include "AppConfig.h"
#include "AutomationOpts.h"

typedef enum
{
    AOTYPE_VOLTAGE,
    AOTYPE_CUR020,
	AOTYPE_DO,
} AO_TYPE;

/// @brief AO channel Information ///
typedef struct
{
	int meta;
	char strName[16];
	BYTE channel;
}AODATASHORT;

/// @brief AO channel data ///
typedef struct
{
	BYTE index;
	BYTE clampingHighEnable;
	BYTE clampingLowEnable;
	BYTE outOfService;
	BYTE resetMinValue;
	BYTE resetMaxValue;
	BYTE reverseOutput;
	BYTE squarerootOutput;

	short type; // 4 bytes packed

	float value;
	float rawValue;
	float minValue;
	float maxValue;

	float clampingHigh;
	float clampingLow;
	float scaleHigh;
	float scaleLow;
	float setValue;
	float in1;
	float in2;
	float in3;
	float in4;
	float in5;
	float in6;
	float in7;
	float in8;
	float in9;
	float in10;
	float in11;
	float in12;
	float in13;
	float in14;
	float in15;
	float in16;
}AODATAV2;

/// @brief AO Object///
typedef struct
{
    /////////////////Data/////////////////////////////
    AODATAV2 aodata;
    /////////////////Operations///////////////////////
    float AnalogueOutputEmergency;/////////////Emergency
    float AnalogueOutputManualOverride;/////////////Manual Override
    u16_t AnalogueOutputTypePre;
    float AOPriorityArray[16];
}ANALOGOUTPUTOBJECT;

extern ANALOGOUTPUTOBJECT AOObj[TOTALANALOGUEOUTPUTS + MAX_MASTER_AO];

/**
 * @brief Reset AO data to default settings for all channels
 * @param[in] none
 * @return none
 */
void AnalogueOutputDefault(void);

/**
 * @brief  AO Object initialization
 * @param[in] none
 * @return none
 */
void AnalogueOutputInit(void);

/**
 * @brief  AO Object execution Loop
 * @param[in] none
 * @return none
 */
void AnalogueOutputObj(void);

/**
 * @brief  Force all AO channels to output 0.0
 * @param[in] none
 * @return none
 */
void AnalogueOutputForceOff(void);

/**
 * @brief AO Function Block initialization
 * @param[in] pData Pointer to function block data
 * @param[in] bDef Default/First time initialization flag
 * @return none
 */
void FBAnalogueOutputInit(void *pData, BOOL bDef);

/**
 * @brief AO function block execution
 * @param[in] pData Pointer to function block data
 * @param[in] nLinks Number of links
 * @param[in] pLinks Pointer to links list
 * @return none
 */
void FBAnalogueOutputExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);

/**
 * @brief Get AO function block data size (Info + Data)
 * @param[in] none
 * @return Size of AO data structure
 */
int AnalogueOutputDataSize(void);

/**
 * @brief Write link data to AO function block
 * @param[in] pFB Pointer to the AO function block data
 * @param[in] sourceType Source data type
 * @param[in] pSrcData Pointer to source data
 * @param[in] toSlot Target slot number
 * @return none
 */
void AnalogueOutputLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);

/**
 * @brief   Direct set AO function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The AO channel number
 * @param[in] pBuf Buffer containing the uidata to set
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int AnalogueOutputSetIOData(void *pCompData, int channel, void *pBuf, int dataSize);

/**
 * @brief Get AO function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The AO channel number
 * @param[out] pBuf Buffer to store the data
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int AnalogueOutputGetIOData(void *pCompData, int channel, void *pBuf, int dataSize);

#endif	/* end of __ANALOGUEOUTPUT_H_48C7536F_1E91_4cb6_8942_A8AA103CFB11 */


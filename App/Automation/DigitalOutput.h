#ifndef __DIGITALOUTPUT_H_F5C8FBE4_249F_4b3e_B1EB_5DDBB072F945
#define __DIGITALOUTPUT_H_F5C8FBE4_249F_4b3e_B1EB_5DDBB072F945

#include "AutomationOpts.h"

/// @brief DO channel Information ///
typedef struct
{
	int meta;
	char strName[16];
	BYTE channel;
}DODATASHORT;

/// @brief DO channel data ///
typedef struct
{
    BYTE index;
	BYTE out;

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
	BYTE setState;

	BYTE outOfService;
	BYTE polarity;
	BYTE minOffOnStart;

	BYTE resetOffCounter;
	BYTE resetOnCounter;
	BYTE resetOffTimer;
	BYTE resetOnTimer;
	
	short interOutputDelay;
	short minOffTime;
	short minOnTime;

	short offCounter;
	short onCounter; // 4 bytes packed

	u32_t offTimer;
	u32_t onTimer;
}DODATAV2;

/// @brief DO Object///
typedef struct
{
	//////Data///////////////////////
	DODATAV2 dodata;
	//////Operations/////////////////
	u8_t  DO_ManualOverrideOff;
	u8_t  DO_ManualOverrideOn;
	u8_t  DO_ManualEmergencyOff;
	u8_t  DO_ManualEmergencyOn;
	u8_t DO_PriorityArray[16];
}DIGITALOUTPUTOBJECT;

extern DIGITALOUTPUTOBJECT DOObj[TOTALDIGITALOUTPUTS + MAX_MASTER_DO];

/**
 * @brief Reset DO data to default settings for all channels
 * @param[in] none
 * @return none
 */
void DigitalOutputDefault(void);

/**
 * @brief  DO Object initialization
 * @param[in] none
 * @return none
 */
void DigitalOutputInit(void);

/**
 * @brief  DO Object execution Loop
 * @param[in] none
 * @return none
 */
void DigitalOutputObj(void);

/**
 * @brief  Force all DO channels OFF
 * @param[in] none
 * @return none
 */
void DigitalOutputForceOff(void);

/**
 * @brief DO Function Block initialization
 * @param[in] pData Pointer to function block data
 * @param[in] bDef Default/First time initialization flag
 * @return none
 */
void FBDigitalOutputInit(void *pData, BOOL bDef);

/**
 * @brief DO function block execution
 * @param[in] pData Pointer to function block data
 * @param[in] nLinks Number of links
 * @param[in] pLinks Pointer to links list
 * @return none
 */
void FBDigitalOutputExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);

/**
 * @brief Get DO function block data size (Info + Data)
 * @param[in] none
 * @return Size of DO data structure
 */
int DigitalOutputDataSize(void);

/**
 * @brief Write link data to DO function block
 * @param[in] pFB Pointer to the DO function block data
 * @param[in] sourceType Source data type
 * @param[in] pSrcData Pointer to source data
 * @param[in] toSlot Target slot number
 * @return none
 */
void DigitalOutputLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);

/**
 * @brief   Direct set DO function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The DO channel number
 * @param[in] pBuf Buffer containing the dodata to set
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int DigitalOutputSetIOData(void *pCompData, int channel, void *pBuf, int dataSize);

/**
 * @brief Get DO function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The DO channel number
 * @param[out] pBuf Buffer to store the data
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int DigitalOutputGetIOData(void *pCompData, int channel, void *pBuf, int dataSize);

#endif	/* end of __DIGITALOUTPUT_H_F5C8FBE4_249F_4b3e_B1EB_5DDBB072F945 */

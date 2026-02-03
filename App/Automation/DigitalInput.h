#ifndef __DIGITALINPUT_H_0484A059_08BE_4ea5_9168_773E218B16C1
#define __DIGITALINPUT_H_0484A059_08BE_4ea5_9168_773E218B16C1

#include "App.h"

/// @brief DI channel Information ///
typedef struct
{
	int meta;
	char strName[16];
	BYTE channel;
}DIDATASHORT;

/// @brief DI channel data ///
typedef struct
{
  	BYTE index;
	BYTE state;
	BYTE alarm;
	BYTE offLatch;
	BYTE onLatch;

	BYTE alarmEnable;
	BYTE alarmMonitorState;
	BYTE alarmResetType;
	BYTE clearOffLatch;
	BYTE clearOnLatch;
	BYTE outOfService;
	BYTE polarity;
	BYTE resetAlarm;
	BYTE resetOffCounter;
	BYTE resetOnCounter;
	BYTE resetOffTimer;
	BYTE resetOnTimer;
	BYTE userSetState;

	short offCounter;
	short onCounter;
	short alarmDelayTime; // 4 bytes packed

	u32_t offTimer;
	u32_t onTimer;

}DIDATA;

/// @brief DI Object///
typedef struct
{
  ////////Data//////////////
  DIDATA didata;
  ////////Operations////////
  s8_t m_DIReadCounter;
}DIGITALINPUTOBJECT;

extern DIGITALINPUTOBJECT DIObj[TOTALDIGITALINPUTS + MAX_MASTER_DI];

/**
 * @brief Reset DI data to default settings for all channels
 * @param[in] none
 * @return none
 */
void DigitalInputDefault(void);

/**
 * @brief  DI Object initialization
 * @param[in] none
 * @return none
 */
void DigitalInputInit(void);

/**
 * @brief  DI Object execution Loop
 * @param[in] none
 * @return none
 */
void DigitalInputObj(void);

/**
 * @brief DI Function Block initialization
 * @param[in] pData Pointer to function block data
 * @param[in] bDef Default/First time initialization flag
 * @return none
 */
void FBDigitalInputInit(void *pData, BOOL bDef);

/**
 * @brief DI function block execution
 * @param[in] pData Pointer to function block data
 * @param[in] nLinks Number of links
 * @param[in] pLinks Pointer to links list
 * @return none
 */
void FBDigitalInputExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);

/**
 * @brief Get DI function block data size (Info + Data)
 * @param[in] none
 * @return Size of DI data structure
 */
int DigitalInputDataSize(void);

/**
 * @brief Write link data to DI function block
 * @param[in] pFB Pointer to the DI function block data
 * @param[in] sourceType Source data type
 * @param[in] pSrcData Pointer to source data
 * @param[in] toSlot Target slot number
 * @return none
 */
void DigitalInputLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);

/**
 * @brief   Direct set DI function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The DI channel number
 * @param[in] pBuf Buffer containing the didata to set
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int DigitalInputSetIOData(void *pCompData, int channel, void *pBuf, int dataSize);

/**
 * @brief Get DI function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The DI channel number
 * @param[out] pBuf Buffer to store the data
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int DigitalInputGetIOData(void *pCompData, int channel, void *pBuf, int dataSize);

#endif	/* end of __DIGITALINPUT_H_0484A059_08BE_4ea5_9168_773E218B16C1 */


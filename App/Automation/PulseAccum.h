#ifndef __PULSEACCUM_H_49DADDF4_D867_454e_A7C9_84C9B294C6C8
#define __PULSEACCUM_H_49DADDF4_D867_454e_A7C9_84C9B294C6C8

#include "App.h"
#include "AutomationOpts.h"

/// @brief Pulse Accum channel Information ///
typedef struct
{
	int meta;
	char strName[16];
	BYTE channel;
}PULSEACCUMDATASHORT;

/// @brief Pulse Accum channel data ///
typedef struct
{
	BYTE resetCount;
	BYTE enable;

	u32_t count;
	float unitPerPulse;
	float costPerUnit;
	float totalUnit;
	float totalCost;
}PULSEACCUMDATA;

extern BYTE PulseAccumEnable[(TOTALPULSEACCUM + 7) / 8];
extern BYTE PulseAccumResetCount[(TOTALPULSEACCUM + 7) / 8];
extern float PulseAccumUnitPerPulse[TOTALPULSEACCUM];
extern float PulseAccumCostPerUnit[TOTALPULSEACCUM];
extern volatile u32_t PulseAccumCount[TOTALPULSEACCUM];
extern float PulseAccumTotalUnit[TOTALPULSEACCUM];
extern float PulseAccumTotalCost[TOTALPULSEACCUM];

/**
 * @brief Reset Pulse Accum data to default settings for all channels
 * @param[in] none
 * @return none
 */
void PulseAccumDefault(void);

/**
 * @brief  Pulse Accum Object initialization
 * @param[in] none
 * @return none
 */
void PulseAccumInit(void);

/**
 * @brief  Pulse Accum Object execution Loop
 * @param[in] none
 * @return none
 */
void PulseAccumObj(void);

/**
 * @brief  Pulse Accum Timer Interrupt routine callback
 * @param[in] none
 * @return none
 */
void PulseAccumTimerInterrupt(void);

/**
 * @brief Pulse Accum Function Block initialization
 * @param[in] pData Pointer to function block data
 * @param[in] bDef Default/First time initialization flag
 * @return none
 */
void FBPulseAccumInit(void *pData, BOOL bDef);

/**
 * @brief Get Pulse Accum function block data size (Info + Data)
 * @param[in] none
 * @return Size of Pulse Accum data structure
 */
void FBPulseAccumExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);

/**
 * @brief Get Pulse Accum function block data size (Info + Data)
 * @param[in] none
 * @return Size of Pulse Accum data structure
 */
int PulseAccumDataSize(void);

/**
 * @brief Write link data to Pulse Accum function block
 * @param[in] pFB Pointer to the Pulse Accum function block data
 * @param[in] sourceType Source data type
 * @param[in] pSrcData Pointer to source data
 * @param[in] toSlot Target slot number
 * @return none
 */
void PulseAccumLinkWrite(void *pFBData, BYTE sourceType, void* pData, BYTE slot);

/**
 * @brief   Direct set Pulse Accum function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The Pulse Accum channel number
 * @param[in] pBuf Buffer containing the pulse Accum data to set
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int PulseAccumSetIOData(void *pCompData, int channel, void *pBuf, int dataSize);

/**
 * @brief Get Pulse Accum function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The Pulse Accum channel number
 * @param[out] pBuf Buffer to store the data
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int PulseAccumGetIOData(void *pCompData, int channel, void *pBuf, int dataSize);
#endif	/* end of __PULSEACCUM_H_49DADDF4_D867_454e_A7C9_84C9B294C6C8 */


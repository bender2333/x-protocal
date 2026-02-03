#ifndef __UIO_H_E2B3F8CF_1D1D_4a52_A0DD_5A6663ABF0F4
#define __UIO_H_E2B3F8CF_1D1D_4a52_A0DD_5A6663ABF0F4

#include "App.h"

typedef enum {
  UIO_AITYPE_VOLTAGE,
  UIO_AITYPE_CURRENT,
  UIO_AITYPE_RESISTANCE,
  UIO_AITYPE_THERMISTOR,
  UIO_AITYPE_DI,
  UIO_AOTYPE_VOLTAGE,
  UIO_AOTYPE_DO,
} UIO_TYPE;

/// @brief UIO channel Information ///
typedef struct {
  int meta;
  char strName[16];
  BYTE channel;
  BYTE Type;
} UIODATASHORT;

/// @brief UIO Object///
typedef struct {
  ///// Operations /////
  WORD UIOType;
  WORD UIOPreType;

  ///// Data /////
  union UIO {
    ANALOGINPUTOBJECT AIObj;
    DIGITALINPUTOBJECT DIObj;
    ANALOGOUTPUTOBJECT AOObj;
    DIGITALOUTPUTOBJECT DOObj;
  };

} UIOOBJECT;

extern UIOOBJECT UIOObj[TOTALUIO + MAX_MASTER_UIO];

/**
 * @brief Reset UIO data to default settings for all channels
 * @param[in] none
 * @return none
 */
void UniversalIODefault();

/**
 * @brief  UIO Object initialization
 * @param[in] none
 * @return none
 */
void UniversalIOInit(void);

/**
 * @brief  UIO Object execution Loop
 * @param[in] none
 * @return none
 */
void UniversalIOObj(void);

/**
 * @brief Get UIO ADI for specified channel
 * @param[in] channel The analog input channel number
 * @return TRUE if ADI is ON, FALSE if ADI is OFF
 */
BOOL UIOAnalogueInputGetADI(BYTE channel);

/**
 * @brief UIO Function Block initialization
 * @param[in] pData Pointer to function block data
 * @param[in] bDef Default/First time initialization flag
 * @return none
 */
void FBUIOInit(void *pData, BOOL bDef);

/**
 * @brief UIO function block execution
 * @param[in] pData Pointer to function block data
 * @param[in] nLinks Number of links
 * @param[in] pLinks Pointer to links list
 * @return none
 */
void FBUIOExecute(void *pData, int nLinks, INTERNALLINKS *pLinks);

/**
 * @brief Get UIO function block data size (Info + Data)
 * @param[in] none
 * @return Size of UIO data structure
 */
int UIODataSize(void);

/**
 * @brief Write link data to UIO function block
 * @param[in] pFB Pointer to the UIO function block data
 * @param[in] sourceType Source data type
 * @param[in] pSrcData Pointer to source data
 * @param[in] toSlot Target slot number
 * @return none
 */
void UIOLinkWrite(void *pFBData, BYTE sourceType, void *pData, BYTE slot);

/**
 * @brief   Direct set UIO function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The UIO channel number
 * @param[in] pBuf Buffer containing the UIO uidata/aodata/didata/dodata to set
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int UIOSetIOData(void *pCompData, int channel, void *pBuf, int dataSize);

/**
 * @brief Get UIO function block data for specific channel
 * @param[in] pCompData Pointer to component data
 * @param[in] channel The UIO channel number
 * @param[out] pBuf Buffer to store the data
 * @param[in] dataSize Size of the buffer
 * @return 0 on success, 2 if data size mismatch, 3 if invalid channel
 */
int UIOGetIOData(void *pCompData, int channel, void *pBuf, int dataSize);
#endif /* end of __UIO_H_E2B3F8CF_1D1D_4a52_A0DD_5A6663ABF0F4 */

#ifndef __FUNCBLOCK_H_E2B3F8CF_1D1D_4a52_A0DD_5A6563ABF0F3
#define __FUNCBLOCK_H_E2B3F8CF_1D1D_4a52_A0DD_5A6563ABF0F3

typedef enum _FUNCTIONENUM {
  fUI = 0,
  fDI,
  fDO,
  fAO,
  // fLCD,

  // Registers
  fBool,
  fBoolWr,
  fShort,
  fShortWr,
  fLong,
  fLongWr,
  fFloat,
  fFloatWr,

  // HVAC
  fThermostat,
  fRunTime,
  fDrive,
  fPsychrometric,
  fLoop,
  fLSeq,
  fReSet,
  fReheatSeq,

  // Math
  fAdd2,
  fAvg10,
  fAvgN,
  fDiv2,
  fFloatOffset,
  fMax,
  fMin,
  fMinMax,
  fMul2,
  fNeg,
  fRound,
  fSub2,
  fTimeAvg,

  // Logic
  fADemux2,
  fAnd4,
  fOr4,
  fXor,
  fNot,
  fASW,
  fBSW,
  fISW,
  fB2P,
  fDemuxI2B4,

  // func
  fCmpr,
  fCount,
  fFreq,
  fHysteresis,
  fIRamp,
  fLimiter,
  fLinearize,
  fRamp,
  fSRLatch,
  fTickTock,
  fUpDn,

  // Timing
  fDlyOff,
  fDlyOn,
  fOneShot,
  fTimer,

  // Types
  fB2F,
  fConstBool,
  fConstFloat,
  fConstInt,
  fF2B,
  fF2I,
  fI2F,

  // 1.1.00
  fPulseAccum,
  fDailySchBool = 67,
  fDailySchFloat,

  // 2.0.00
  fPriorityBinary,
  fPriorityFloat,

  fSquareRoot,
  fInverse,
  fCeiling,
  fFloor,
  fFabs,
  fExp,
  fPow,
  fLog,
  fLog10,

  fDateTime,
  fOverride,
  fHostComm,

  fUIO = 118,

  /// P0 ///
  fEdgeTrigger, // Func
  fCOVTrigger,
  fPWM,
  fCOVStablizer,

  fNtcRT2NTC, // HVAC
  fPos3Cmd,
  fTwinPump,
  fCasLeaCtrlr,
  fPres2Flow,
  fBit3EHtr,

  fDailySchFloat4, // Schedule
  fWeekSchFloat,

  fPackShort, // Types
  fPackLong,
  fUnpackShort,
  fUnpackLong,

  /// P1 ///
  fMod, // Math
  fRandGen,
  fPolynomialCal,

  fDiFilter, // Func
  fAiFilter,
  fAiChngRateCal,
  fAutoIncDec,
  fPrio8Dev,
  fPrio32Dev,
  fStepCtrl,
  fLoadAssign,

  fPidInc, // HVAC
  fCoolHeatCalc,

  fLast,
} FUNCTIONENUM;

typedef enum _FBSLOTTYPE {
  SLOT_VOID = 0,
  SLOT_BOOL = 1,
  SLOT_BYTE = 2,
  SLOT_SHORT = 3,
  SLOT_INT = 4,
  SLOT_LONG = 5,
  SLOT_FLOAT = 6,
  SLOT_DOUBLE = 7
} FBSLOTTYPE;

typedef struct {
  BYTE fromSlot;
  BYTE toSlot;
  WORD toCompIndex;
} INTERNALLINKS;

typedef struct {
  u16_t Check1;
  u16_t Check2;
  u16_t Total;
  u16_t LastAddr;
} FBINFO;

typedef struct {
  int offset;
  BYTE slotType;
} FBSLOTINFO;

typedef struct {
  void *pData;
  BYTE slotType;
} FBIOSLOTINFO;

typedef struct {
  int meta;
  char strName[16];
} BASICBLOCK;

#define FBBOOLTYPE 0
#define FBSHORTTYPE 1
#define FBLONGTYPE 2
#define FBFLOATTYPE 3

/* Function Block callbacks registration */
typedef void(FUNCBLOCKINIT)(void *pData, BOOL bDef);
typedef void(FUNCBLOCKEXEC)(void *pData, int nLinks, INTERNALLINKS *pLinks);
typedef void(FUNCBLOCKLINKWRITE)(void *pFB, BYTE sourceType, void *pSrcData,
                                 BYTE toSlot);
typedef int(FUNCBLOCKDATASIZE)(void);
typedef int(FUNCBLOCKSETIODATA)(void *pCompData, int channel, void *pBuf,
                                int dataSize);
typedef int(FUNCBLOCKGETIODATA)(void *pCompData, int channel, void *pBuf,
                                int dataSize);

/**
 * @brief   Function block Initialization.
 *
 * @param[in]  init   TRUE for the very first initialization, FALSE for
 * re-initialize.
 * @param[out] none
 * @return     none
 */
void FunctionBlockInit(bool init);

/**
 * @brief   Execute function blocks.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void FunctionBlockExec(void);

/**
 * @brief   Load all function blocks from EEPROM.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void FunctionBlockLoad(void);

/**
 * @brief   Re-load/Re-initialize function blocks.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void FunctionBlockReload(void);

/**
 * @brief   Get slot data pointer and type from specific function block.
 *
 * @param[in]  pSlotInfo   Pointer to the slot structure.
 * @param[in]  slot        Slot index.
 * @param[in]  pFB         Pointer to function block data.
 * @param[out] pSlotType   Pointer to slot type.
 * @return     Pointer to slot data, or NULL if invalid slot.
 */
void *FBGetSlot(const FBSLOTINFO *pSlotInfo, BYTE slot, BYTE *pFB,
                BYTE *pSlotType);

/**
 * @brief   Write slot data from link.
 *
 * @param[in]  pSlotInfo    Pointer to the slot structure.
 * @param[in]  pSrcSlotData Pointer to the link source slot data.
 * @param[in]  srcType      Source data type.
 * @param[in]  destSlot     Destination slot.
 * @param[out] pDestData    Pointer to the link destination data.
 * @return     none
 */
void FBWriteSlot(const FBSLOTINFO *pSlotInfo, BYTE *pSrcSlotData, BYTE srcType,
                 BYTE destSlot, BYTE *pDestData);
/**
 * @brief   Propagate data to specified slot.
 *
 * @param[in]  pSlotInfo   Pointer to the slot structure.
 * @param[in]  nLinks      Number of links.
 * @param[in]  pLinks      Pointer to links list.
 * @param[in]  pSrc        Pointer to source data info.
 * @param[out] none
 * @return     none
 */
void FBPropagateTo(const FBSLOTINFO *pSlotInfo, BYTE nLinks,
                   INTERNALLINKS *pLinks, BYTE *pSrc);

/**
 * @brief   Set destination data to source.
 *
 * @param[in]  sourceType   Source data type.
 * @param[in]  pSrc         Pointer to source data.
 * @param[in]  destType     Destination data type.
 * @param[out] pDest        Pointer to destination data.
 * @return     none
 */
void FBSetData(BYTE sourceType, void *pSrc, BYTE destType, void *pDest);

/**
 * @brief   Write I/O slot data (deprecated from IOs, kept for pulseAccum only).
 *
 * @param[in]  channel      Channel number.
 * @param[in]  pIOSlotInfo  Pointer to I/O slot information.
 * @param[in]  sourceType   Source data type.
 * @param[in]  pSrcData     Pointer to source data.
 * @param[in]  toSlot       Target slot.
 * @param[out] none
 * @return     none
 */
void FBWriteIOSlot(int channel, const FBIOSLOTINFO *pIOSlotInfo,
                   BYTE sourceType, void *pSrcData, BYTE toSlot);

/**
 * @brief   Propagate data to specified slot (deprecated from IOs, kept for
 * pulseAccum only).
 *
 * @param[in]  channel      Channel number.
 * @param[in]  pIOSlotInfo  Pointer to I/O slot information.
 * @param[in]  nLinks       Number of links.
 * @param[in]  pLinks       Pointer to links array.
 * @param[out] none
 * @return     none
 * @TODO: 确认下功能
 */
void FBIOPropagateTo(int channel, const FBIOSLOTINFO *pIOSlotInfo, int nLinks,
                     INTERNALLINKS *pLinks);

/**
 * @brief   Search for export register name by address and type.
 *
 * @param[in]  addr    Register address.
 * @param[in]  type    Register type.
 * @param[out] pBuf    Buffer to store register name.
 * @return     TRUE if found, FALSE otherwise.
 * @TODO: 确认功能，找什么？返回值为啥是bool
 */
BOOL FBSearchExpRegName(int addr, BYTE type, BYTE *pBuf);

/**
 * @brief   Search for I/O name by channel and type (deprecated from IOs, kept
 * for pulseAccum only).
 *
 * @param[in]  channel   Channel number.
 * @param[in]  type      I/O type.
 * @param[out] pBuf      Buffer to store I/O name.
 * @return     TRUE if found, FALSE otherwise.
 */
BOOL FBSearchIOName(int channel, BYTE type, BYTE *pBuf);

/**
 * @brief   Check if export holding register is long type.
 *
 * @param[in]  addr   Register address.
 * @param[in]  type   Register type.
 * @param[out] none
 * @return     TRUE if long type, FALSE otherwise.
 */
BOOL FBExpHoldingIsLong(int addr, BYTE type);

/**
 * @brief   Get total size of function blocks.
 *
 * @param[in]  none
 * @param[out] none
 * @return     Total size in bytes.
 */
int FunctionBlockGetSize(void);

/**
 * @brief   Get function block data by block number.
 *
 * @param[in]  block    Block number.
 * @param[out] pData    Pointer to store block data.
 * @return     Size of data read.
 */
int FunctionBlockGetBlock(int block, BYTE *pData);

/**
 * @brief   Link a slot to another component slot.
 *
 * @param[in]  toComp      Target component.
 * @param[in]  toSlot      Target slot.
 * @param[in]  sourceType  Source data type.
 * @param[in]  pSrcData    Pointer to source data.
 * @param[out] none
 * @return     none
 */
void FBLinkToComp(WORD toComp, BYTE toSlot, BYTE sourceType, void *pSrcData);

/**
 * @brief   Direct set function block data.
 *
 * @param[in]  FBNo      Function block number.
 * @param[in]  hHandle   Component handle.
 * @param[in]  Channel   Channel number.
 * @param[in]  dataSize  Size of data.
 * @param[in]  pData     Pointer to data.
 * @param[out] none
 * @return     Status code.
 */
int FBSetFBData(int FBNo, int hHandle, int Channel, int dataSize, BYTE *pData);

/**
 * @brief   Get function block data.
 *
 * @param[in]  FBNo      Function block number.
 * @param[in]  hHandle   Component handle.
 * @param[in]  Channel   Channel number.
 * @param[in]  dataSize  Size of data buffer.
 * @param[out] pData     Pointer to store data.
 * @return     Status code.
 */
int FBGetFBData(int FBNo, int hHandle, int Channel, int dataSize, BYTE *pData);

#endif /* end of __FUNCBLOCK_H_E2B3F8CF_1D1D_4a52_A0DD_5A6563ABF0F3 */

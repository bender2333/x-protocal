#ifndef __MODBUSSLAVE_H__
#define __MODBUSSLAVE_H__

union MCSData
{
	bool Boolean;
	uint16_t Integer16;
	uint32_t Long32;
	float Float;
};

typedef struct 
{
    int meta;
	char strName[16];
	BYTE customDevAddr; 	// ReadWrite, 3rd party device Address 	
    BYTE status;     		// Read Only in APM, 0 = offline, 1 = online, 2 = wrong model, 3 = invalid
    BYTE Type;		 		// Read Only in APM, 1 = outputCoil, 2 = inputcoil , 3 = outputreg 4 = inputreg(Requested to be configured from Webpage setting)
    BYTE dType; 	 		// Read Only in APM, 0 = Bool, 1 = Int16 , 2 = Int32 3 = Float(Requested to be Configure from Webpage setting)
	BYTE bWordSwap;			// 4Bytes WORD swap
    BYTE ReadOnly; 			// internal use
	BYTE Port;				// RS485 Port, 1-2
	WORD ptIndex;	 		// ReadWrite, point index 0-5, 6 points per device		
	WORD Addr; 				// Read Only in APM, Modbus reg addr(Requested to be configure from Webpage setting)
    union MCSData Out; 		// union data
    union MCSData In; 		// union data
    union MCSData PrevIn; 	// internal use
	/* data */
}MsPointData;

extern void ModbusCustomSlaveInit(void *pData, BOOL bDef);
extern void ModbusCustomSlaveExecute(void *pData, int nLinks, INTERNALLINKS* pLinks);
extern void ModbusCustomSlaveLinkWrite(void *pFB, BYTE sourceType, void* pSrcData, BYTE toSlot);
extern int ModbusCustomSlaveDataSize(void);
extern int ModbusCustomSlaveSetData(void *pCompData, int channel, void *pBuf, int dataSize);
#endif
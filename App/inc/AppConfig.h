#ifndef __APPCONFIG_H_2EFBD03C_6E33_40e0_8B81_8D455705A105
#define __APPCONFIG_H_2EFBD03C_6E33_40e0_8B81_8D455705A105

#include "MyDevice.h"
#include "EKStdLib.h"
#include "bacenum.h"
#include "FuncBlock.h"
#include "AutomationOpts.h"

////define application
#define MODBUSAPPLICATION	1
#define BACNETAPPLICATION	1

////define network
#define TOTAL_SERIALPORT        2
#define TOTAL_BACNET_SERIALPORT        1
#define TOTAL_BACNET_NETWORK TOTAL_BACNET_SERIALPORT + 1

////define model
#define MODELCODE	              0x0501 

////define version
#define APPLICATIONVERSION		405
#define APPLICATIONVERSIONSTR	        "0.4.05"
#define REGISTERVERSION			1001

#pragma pack(1)
typedef struct
{
	u8_t        ModelName[16];      //Model name
	u16_t	    Version;            //Application firmware Version
	u8_t        DeviceName[32];     //Device name
	u8_t	    DeviceLocation[32]; //Device location
    uint8_t     IPAddress[4];       //IP address
    uint8_t     Subnet[4];          //Subnet Mask
    uint8_t     Gateway[4];         //Gateway
}HARDWARECONFIG;                    //Device configuration
#pragma pack()

typedef struct
{
    HARDWARECONFIG	HardwareConfig;                 //Device configuration
    SERIALCONFIG	SerialConfig;                   //Serial Port Config
    SERIALCONFIG	BacnetSerialConfig;             //Serial Port Config for BACnet MS/TP
    u8_t		    UserLogin[8];                   //Webpage Login User
    u8_t		    ConfigPassword[8];              //Webpage Login / BACnet reinitialization Password
    u16_t           ModbusRTUMode;                  //Modbus Serial Mode, 0 = RTU slave, 1 = Modbus Master
    u32_t	        AutoBaudrate;                   //must be last variable
    u16_t	        ModbusTurnaroundTime;           //must be last variable
    uint16_t        Port[2];                        //BacnetIP / modbusTCP port
    uint16_t        Network[TOTAL_BACNET_NETWORK];         //BACnet network number
    u8_t            NetworkQuality[TOTAL_BACNET_NETWORK];  //BACnet network quality
    u8_t			MMDeviceAddr[MAX_SLAVE_DEVICE];                //Slave address in Modbus Master mode 
    u8_t			MMDeviceIndex[MAX_SLAVE_DEVICE];               //Slave index in Modbus Master mode 
    BOOL			MCS32BitWordSwap[MAX_SLAVE_DEVICE];            //16bit word swap in Modbus Master mode 
    u16_t			PingFrequency;                  //Ping frequency in Modbus Master mode 
}DEVICESETTING;

////define calibration
#define AOTABLEOFFSET  0
#define AICTABLEOFFSET (sizeof(AnalogueOutputCalibrateTableROM))
#define AIVTABLEOFFSET (sizeof(AnalogueOutputCalibrateTableROM) + sizeof(AnalogueInputCurrentCalibrateTableROM))
#define AIRTABLEOFFSET (sizeof(AnalogueOutputCalibrateTableROM) + sizeof(AnalogueInputCurrentCalibrateTableROM) + sizeof(AnalogueInputVoltageCalibrateTableROM))

#define TOTALCALIBRATIONSIZE ((sizeof(AnalogueOutputCalibrateTableROM) + sizeof(AnalogueInputCurrentCalibrateTableROM) \
                                + sizeof(AnalogueInputVoltageCalibrateTableROM)) + sizeof(AnalogueInputResCalibrateTableROM) \
                                + sizeof(UIOUIResCalibrateTableROM) + sizeof(UIOUICurrentCalibrateTableROM) \
                                + sizeof(UIOUIVoltageCalibrateTableROM) + sizeof(UIOAOCalibrateTableROM))

////define config state
#define CONFIGSTATE_EEPROM		    0x01
#define CONFIGSTATE_STATUS		    0x02
#define CONFIGSTATE_WRITE		    0x04
#define CONFIGSTATE_WRITEOK		    0x08
#define CONFIGSTATE_RESETREGS	            0x10
#define CONFIGSTATE_RESETREGSALL	        0x11
#define CONFIGSTATE_RESETTEMPTALBE	    0x20

////define EEPROM address
#define SETTING_START_ADDR	        0x80
#define MODBUSCOIL_DATAADDR	        0x300
#define MODBUSHOLDING_DATAADDR	        0x780
#define FB_SADDR	                0x2000
#define EXPREGSHDR_SADDR        0x7040
#define EXPREGSCOIL_SADDR       0x7050
#define EXPREGSHOLDING_SADDR    0x7080
#define PULSE_SADDR             0x71A0
#define CALIBRATION_SADDR	0x7400
#define BACNETOBJ_SADDR         0x10000
#define FBINFO_SADDR            (FB_SADDR + 16)
#define FBPASSWD_SADDR          (FBINFO_SADDR + 16)
#define FBDATA_SADDR            (FBPASSWD_SADDR + 32)

/**
 * @brief Configuration/setting Initialization
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ConfigInit(void);

/**
 * @brief Configuration/setting Object loop
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ConfigObj(void);

/**
 * @brief Check if config is empty(restore default or nothing saved) 
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if config is empty
 */
BOOL isConfigEmpty(void);

/**
 * @brief Get BACnet IP and Modbus TCP port
 * 
 * @param[in]  none
 * @param[out] none
 * @return     BACnet IP / Modbus TCP port number.
 */
uint16_t ConfigGetBIPPort();
uint16_t ConfigGetMTCPPort();

/**
 * @brief Get BACnet network number
 * 
 * @param[in]  portID   1 = IP, 2 = MS/TP.
 * @param[out] none
 * @return     Network number for the specified port.
 */
uint16_t ConfigGetNetwork(uint8_t portID);

/**
 * @brief Set BACnet network number
 * 
 * @param[in]  portID   1 = IP, 2 = MS/TP.
 * @param[in]  network  BACnet network number
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSetNetwork(uint8_t portID, uint16_t network);

/**
 * @brief   Get Modbus turnaround time
 *
 * @param[out] MBTurnaround   Pointer to Modbus turnaround time.
 * @return     none
 */
void ConfigGetModbusTurnaround(u16_t* MBTurnaround);

/**
 * @brief   Write and save the Modbus turnaround time to EEPROM
 *
 * @param[in]  MBTurnaround   Pointer to Modbus turnaround time.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSetModbusTurnaround(u16_t* MBTurnaround);

/**
 * @brief   Load device settings from EEPROM.
 *
 * @param[out] pCfg   Pointer to store loaded device settings.
 * @return     none
 */
void ConfigLoadDeviceSetting(DEVICESETTING *pCfg);

/**
 * @brief   Write device settings and optionally to be saved.
 *
 * @param[in]  pCfg         Pointer to device settings to write.
 * @param[in]  bWriteEEPROM TRUE to write to EEPROM, FALSE otherwise.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigWriteDeviceSetting(DEVICESETTING *pCfg, BOOL bWriteEEPROM);

/**
 * @brief   Get current/loaded device settings.
 *
 * @param[in]  none
 * @param[out] none
 * @return     Pointer to loaded device settings.
 */
DEVICESETTING * ConfigGetDeviceSetting(void);

/**
 * @brief   Get Hardware ID, i.e. Serial Number.
 *
 * @param[out] HighID   Pointer to high 32 bits of hardware ID.
 * @param[out] LowID    Pointer to low 32 bits of hardware ID.
 * @return     none
 */
void ConfigGetHardwareID(u32_t *HighID, u32_t *LowID);

/**
 * @brief   Get device MAC address.
 *
 * @param[out] HighMAC   Pointer to high 32 bits of MAC address.
 * @param[out] LowMAC    Pointer to low 32 bits of MAC address.
 * @return     none
 */
void ConfigGetMAC(u32_t *HighMAC, u32_t *LowMAC);

/**
 * @brief   Get serial port settings.
 *
 * @param[out] pCfg   Pointer to store serial port settings.
 * @return     none
 */
void ConfigGetSerialSetting(SERIALCONFIG* pCfg);

/**
 * @brief   Set and save serial port settings.
 *
 * @param[in]  pCfg   Pointer to serial port settings to write.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSetSerialSetting(SERIALCONFIG* pCfg);

/**
 * @brief   Get BACnet MS/TP serial port settings.
 *
 * @param[out] pCfg   Pointer to store BACnet MS/TP serial port settings.
 * @return     none
 */
void ConfigGetBacnetSerialSetting(SERIALCONFIG* pCfg);

/**
 * @brief   Set and save BACnet MS/TP serial port settings.
 *
 * @param[in]  pCfg   Pointer to BACnet MS/TP serial port settings to write.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigWriteBacnetSerialSetting(SERIALCONFIG* pCfg);

/**
 * @brief   Get hardware info settings.
 *
 * @param[out] pCfg   Pointer to store hardware info.
 * @return     none
 */
void ConfigGetHardwareSetting(HARDWARECONFIG *pCfg);

/**
 * @brief   Save specific calibration data.
 *
 * @param[in]  addr   Starting address of the calibration data.
 * @param[in]  cnt    Number of calibration entries to save.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSaveCalibration(u16_t addr, u16_t cnt);

/**
 * @brief   Write and optionally save all calibration data.
 *
 * @param[in]  bResetCalibrate   TRUE to save if calibration data is restored/empty , FALSE to not save.
 * @param[out] none
 * @return     none
 */
void ConfigWriteAllCalibrate(BOOL bResetCalibrate);

/**
 * @brief   Save specific Modbus holding registers.
 *
 * @param[in]  addr   Starting address of holding registers.
 * @param[in]  cnt    Number of holding registers to save.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSaveDataHoldingRegs(u16_t addr, u16_t cnt);

/**
 * @brief   Save specific single Modbus coil registers.
 *
 * @param[in]  addr   address of the coil register.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSaveCoilDataReg(u16_t addr);

/**
 * @brief   Save multiple Modbus coil registers.
 *
 * @param[in]  saddr   Starting address of coil registers.
 * @param[in]  cnt    Number of coil registers to save.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSaveMultipleCoilDataReg(u16_t saddr, u16_t cnt);

/**
 * @brief Write and save specific BACnet Object to EEPROM
 * 
 * @param[in] objType BACnet object type.
 * @param[in] channel Channel number for the object.
 * @param[out] none
 * @return TRUE if successful, FALSE otherwise. 
 */
BOOL ConfigWriteBacnetObject(BACNET_OBJECT_TYPE objType, BYTE channel);

/**
 * @brief Write and save BACnet Device Configuration, default refer BACNETDEVICESET structure to EEPROM
 * 
 * @param[in] pDev Pointer to the BACnet device configuration.
 * @param[out] none
 * @return TRUE if successful, FALSE otherwise. 
 */
BOOL ConfigWriteBacnetDevice(void * pDev);

/**
 * @brief   Load all Modbus registers and BACnet objects.
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigLoadRegsSetting(void);

/**
 * @brief   Write and save all Modbus registers to EEPROM
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ConfigWriteAllRegs(void);

/**
 * @brief   Write and save all Settings(included config, modbus Regs, bacnet points) to EEPROM
 *
 * @param[in]  bResetCalibrate   TRUE to save calibration if calibration data is restored/empty, FALSE to not save calibration but others.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigWriteAllSetting(BOOL bResetCalibrate);

/**
 * @brief   Get user login name from configuration.
 *
 * @param[out] pCfg   Pointer to stored user login name.
 * @return     none
 */
void ConfigGetUser(char *pCfg);

/**
 * @brief   Get Password from configuration.
 *
 * @param[out] pCfg   Pointer to stored Password.
 * @return     none
 */
void ConfigGetPassword(char *pCfg);

/**
 * @brief   Reset and save default function block.
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigResetFB(void);

/**
 * @brief   Load and Get function block information.
 *
 * @param[out] pInfo   Pointer to stored function block information.
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigFBGetInfo(FBINFO *pInfo);

/**
 * @brief   Load and Get specific function block's size.
 *
 * @param[in]  addr      Address of function block in EEPROM.
 * @param[out] pFB       Pointer to the function block fnum.
 * @param[out] dataSize  Pointer to the data size.
 * @param[out] linkSize  Pointer to the link size.
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigFBGetBlockSize(int addr, BYTE *pFB, BYTE *dataSize, BYTE *linkSize);

/**
 * @brief  Load and Get specific function block's data.
 *
 * @param[in]  addr     Address of function block in EEPROM.
 * @param[out] pData    Pointer to function block data.
 * @param[out] pLinks   Pointer to function block links.
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigFBGetBlock(int addr, BYTE *pData, BYTE *pLinks);

/**
 * @brief   Write and Save function blocks, called once for each function block to EEPROM
 *
 * @param[in]  totalFB   Total number of function blocks to be written.
 * @param[in]  curFB     Current function block number which is being written.
 * @param[in]  pData     Pointer to Current function block data.
 * @param[in]  size      Size of Current function block data.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigWriteFB(int totalFB, int curFB, BYTE *pData, int size);

/**
 * @brief   Load all export registers.
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigLoadExpRegisters(void);

/**
 * @brief   Write and save all export registers to EEPROM
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigWriteExpRegisters(void);

/**
 * @brief   Save export holding registers to EEPROM during modbus protocol write.
 *
 * @param[in]  addr   Starting modbus address for export holding registers.
 * @param[in]  cnt    Number of export holding registers to save.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSaveExpRegsHolding(u16_t addr, u16_t cnt);

/**
 * @brief   Save export coil registers to EEPROM during modbus protocol write.
 *
 * @param[in]  saddr   Starting modbus address for export coil registers.
 * @param[in]  cnt    Number of export coil registers to save.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSaveExpRegsMultipleCoil(u16_t saddr, u16_t cnt);

/**
 * @brief   Save single export coil register to EEPROM during modbus protocol write.
 *
 * @param[in]  addr   Address of export coil register to save.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigSaveExpRegsCoil(u16_t addr);

/**
 * @brief   Reset function block password to default (no password).
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigResetFBPW(void);

/**
 * @brief   Write and save function block password to EEPROM.
 *
 * @param[in]  pData   Pointer to password data.
 * @param[in]  size    Size of password data.
 * @param[out] none
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigWriteFBPW(BYTE *pData, int size);

/**
 * @brief   Load function block password from EEPROM.
 *
 * @param[out] pData   Pointer to password data.
 * @param[in]  size    Size of password data buffer.
 * @return     TRUE if successful, FALSE otherwise.
 */
BOOL ConfigLoadFBPW(BYTE *pData, int size);

/**
 * @brief   BACnet device warm start callback(reboot).
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ConfigDeviceWarmStartCB(void);

/**
 * @brief   BACnet device cold start callback(clear BACnet settings and reboot).
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ConfigDeviceColdStartCB(void);

/**
 * @brief   Start timer count and Reboot after delay ms.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ConfigRebootTimerInit(u32_t delay);

/* refer #define config state */
extern u8_t ConfigState;
extern volatile u32_t m_SerialNumberHigh;
extern volatile u32_t m_SerialNumberLow;

extern volatile u32_t m_MACAddressHigh;
extern volatile u32_t m_MACAddressLow;

extern const u16_t ApplicationVersion;
extern const u16_t RegisterVersion;
extern const u16_t ModelCode;
extern const u16_t ModbusVersion;
extern const u16_t BacnetVersion;

extern BOOL WDOGDisable;
/* LED set status, Reserved for future use */
extern BYTE LEDSetStatus; 


#endif	/* end of __APPCONFIG_H_2EFBD03C_6E33_40e0_8B81_8D455705A105 */

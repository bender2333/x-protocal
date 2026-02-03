#ifndef __MODBUSCLIENT_H_
#define __MODBUSCLIENT_H_

#include "App.h"
#include "AppConfig.h"

typedef enum
{
    MPUNCONFIGURE,
    MPCOIL,
    MPDISCRETE,
    MPHOLDINGREG,
    MPINREG
} MODBUSCLIENT_REGTYPE;

typedef enum
{
    MPD_INT16,
    MPD_UINT16,
    MPD_INT32,
    MPD_FLOAT,
} MODBUSCLIENT_DATATYPE;

////define Modbus Slave device status
#define MD_IDLE					0xFF
#define MD_OFFLINE              0x00
#define MD_ONLINE				0x01
#define MD_WRONG_MODEL			0x02
#define MD_INVALID				0x03

///define 4 bytes WORD swap
#define MD_LONGSWAP             0x01
#define MD_FLOATSWAP            0x02

///define support write multiple
#define MD_F15                  0x04
#define MD_F16                  0x08

///define Modbus Client slave device and Registers Config
#define TOTALDEVICE                 7
#define DISCRETEINPUTPERDEV			16
#define COILOUTPUTPERDEV			36
#define INPUTREGSPERDEV				56
#define HOLDINGREGSPERDEV			112
#define TOTALREGISTERS          TOTALDEVICE*(DISCRETEINPUTPERDEV+COILOUTPUTPERDEV+INPUTREGSPERDEV+HOLDINGREGSPERDEV)
#define TOTALMASTERDISCRETEINPUT      (DISCRETEINPUTPERDEV * TOTALDEVICE)
#define TOTALMASTERCOILOUTPUT         (COILOUTPUTPERDEV * TOTALDEVICE)
#define TOTALMASTERINPUTREGISTERS     (INPUTREGSPERDEV * TOTALDEVICE)
#define TOTALMASTERHOLDINGREGISTERS   (HOLDINGREGSPERDEV * TOTALDEVICE)

/// @brief Modbus Point Setting
typedef struct
{
    // char  Name[33];
    u16_t Addr;
    BYTE  Type;
    BYTE  dType;
} MODBUSPTSET;

/// @brief Modbus Point Object
typedef struct _MODBUSPOINT
{
    MODBUSPTSET Setting;        //setting
    BOOL  bLoaded;              //loaded or not (by reading from modbus)         
    uint16_t Index;             //Point Index
    void* Data;                 //Point Current/Output Data
    void* In;                   //Point Input Data
    struct _MODBUSPOINT *pNext; // Pointer to next point in device
    void* pDev;                 // Pointer to device
}MODBUSPOINT;

/// @brief Modbus Device Setting
typedef struct
{
    char Name[33];
    BYTE ID;
    BYTE Setting;
    // BYTE AddrType;
} MODBUSDEVSET;

/// @brief Modbus Device Object
typedef struct _MODBUSDEVICE
{
    MODBUSDEVSET Setting;       //setting
    u16_t * pStatus;            //pointer to the device status, refer Modbus Slave device status
	u16_t Code;                 //Device Model Code
    u16_t OffLineCount;         //Offline Counter
    u16_t Retries;              //Communication fail retries
	u8_t Index;                 //device index
    MODBUSPOINT *pPt;           // Pointer to device current point
    struct _MODBUSDEVICE *pNext;// Pointer to next device
    void *pNet;                 // Pointer to network (reserved)
    void *lpReserved;           // reserved
}MODBUSDEVICE;

/// @brief Modbus Network Setting
typedef struct
{
	char Name[33];
	BYTE Com;
} MODBUSNETSET;

/// @brief Modbus Network Object
typedef struct
{
    MODBUSNETSET Setting;       //setting
    MODBUSDEVICE* pDevice;      //Network Current Device
    MODBUSPOINT*  pcurPoint;    //Network Current Point
}MODBUSNET;

extern WORD DeviceStatus[TOTALDEVICE];
extern WORD DeviceSlaveCode[TOTALDEVICE];
extern MODBUSDEVICE * DevicePtr[TOTALDEVICE];

/**
 * @brief   Get Modbus client network by communication port number.
 *
 * @param[in]  com    Communication port number (1-based).
 * @param[out] none
 * @return     Pointer to Modbus network, or NULL if not found.
 */
MODBUSNET* ModbusClientGetNet(u16_t com);

/**
 * @brief   Add a Modbus device to the specified network.
 *
 * @param[in]  pdev           Pointer to device settings, if not NULL, add it into the network, else create a new one using other parameters.
 * @param[in]  com            Communication port number (1 if only one port supported).
 * @param[in]  devID          Device ID (slave address).
 * @param[in]  name           Device name string.
 * @param[in]  longByteOrder  TRUE for long byte order word swap.
 * @param[in]  floatByteOrder TRUE for float byte order word swap.
 * @param[in]  bSupport15     TRUE if device supports function 15.
 * @param[in]  bSupport16     TRUE if device supports function 16.
 * @param[out] none
 * @return     Pointer to added device, or NULL if failed.
 */
MODBUSDEVICE* ModbusClientAddDevice(MODBUSDEVSET* pdev, int com, BYTE devAddr, char* name, BOOL longByteOrder, BOOL floatByteOrder, BOOL bSupport15, BOOL bSupport16);

/**
 * @brief   Get Modbus device by communication port and device ID.
 *
 * @param[in]  com    Communication port number (1 if only one port supported).
 * @param[in]  devID  Device ID (slave address).
 * @param[out] none
 * @return     Pointer to Modbus device, or NULL if not found.
 */
MODBUSDEVICE* ModbusClientGetDevice(int com, BYTE devID);

/**
 * @brief   Remove a Modbus device from the network.
 *
 * @param[in]  port   Port number  (1 if only one port supported).
 * @param[in]  devAddr  Device Address to remove.
 * @param[out] none
 * @return     TRUE if device removed successfully, FALSE otherwise.
 */
BOOL ModbusClientRemoveDevice(int port, BYTE devAddr);

/**
 * @brief   Add a Modbus register point to a device.
 *
 * @param[in]  ppt      Pointer to point settings, if not NULL, add it into the device, else create a new one using other parameters.
 * @param[in]  com      Communication port number.
 * @param[in]  devAddr    Device ID.
 * @param[in]  Index  Point index.
 * @param[in]  name     Point name string.
 * @param[in]  type     Point type (coil, discrete, input register, holding register).
 * @param[in]  regAddr  Register address.
 * @param[in]  dType    Data type (16-bit, 32-bit, float).
 * @param[in]  mapData  Pointer to mapped IO's data (refer to properties in uidata/aodata/didata/dodata)
 * @param[out] none
 * @return     Pointer to added register point, or NULL if failed.
 */
MODBUSPOINT* ModbusClientAddRegister(MODBUSPTSET* ppt, int com, BYTE devAddr, uint16_t Index, char * name, BYTE type, int regAddr, BYTE dType, void* mapData);

/**
 * @brief   Get Modbus register point by device, type and address.
 *
 * @param[in]  pDev   Pointer to Modbus device.
 * @param[in]  type   Register type.
 * @param[in]  addr   Register address.
 * @param[out] none
 * @return     Pointer to register point, or NULL if not found.
 */
MODBUSPOINT * ModbusClientGetRegByAddr(MODBUSDEVICE* pDev, BYTE type, u16_t addr);

/**
 * @brief   Get Modbus register point by device, type and address, and flag it with the point's occupying modbus register count.
 *
 * @param[in]  pDev   Pointer to Modbus device.
 * @param[in]  type   Register type.
 * @param[in]  addr   Register address.
 * @param[out] fCnt   Pointer to store the point's occupying modbus register count.
 * @return     Pointer to register point, or NULL if not found.
 */
MODBUSPOINT * ModbusClientGetAndFlagRegByAddr(MODBUSDEVICE* pDev, BYTE type, u16_t addr, u8_t *fCnt);

/**
 * @brief   Clear all Modbus client connections and reset to default state.
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if cleared successfully.
 */
BOOL ModbusClientClearAllConnection(void);

/**
 * @brief   Callback routine to handle Modbus client offline events.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ModbusClientOfflineCallback();

/**
 * @brief   Modbus client initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ModbusClientInit();

/**
 * @brief   Re-initialize Modbus client device.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ModbusClientDeviceReInit(void);
#endif
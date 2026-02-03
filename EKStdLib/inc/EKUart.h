#ifndef __EKUART_H_7243F3FD_7A71_48f1_90BF_5BE4D0312010
#define __EKUART_H_7243F3FD_7A71_48f1_90BF_5BE4D0312010

#include "EKStdLib.h"

//*****************************************************************************
//
// Macro allowing us to pack the fields of a structure.
//
//*****************************************************************************
#if defined(ccs) ||             \
    defined(codered) ||         \
    defined(gcc) ||             \
    defined(rvmdk) ||           \
    defined(__ARMCC_VERSION) || \
    defined(sourcerygxx)
#define PACKEDSTRUCT __attribute__ ((packed))
#elif defined(ewarm)
#define PACKEDSTRUCT
#else
#error Unrecognized COMPILER!
#endif

//Control Config
#define FULLDUPLEX  0x0000
#define HALFDUPLEX  0x0001

#define SEVENBIT    0x0000
#define EIGHTBIT    0x0002
#define NINEBIT     0x0004


#define ONESTOPBIT  0x0000
#define TWOSTOPBIT  0x0008

#define PARITY_NONE 0x0000
#define PARITY_EVEN 0x0010
#define PARITY_ODD  0x0020

#define RTS_CTS     0x0100
#define DTR_DSR     0x0200

#define NOFIFO      0x0000
#define AUTOSWITCH  0x1000
#define USEFIFO     0x2000

// Device error message         0xA000 - 0xBFFF
#define E_UART_TXMAXLEN     0xA000
#define E_UART_ASYNBUSY     0xA001

// Device error message         0xA000 - 0xBFFF
#define E_UART_TXMAXLEN     0xA000
#define E_UART_ASYNBUSY     0xA001
#define E_UART_BUFFFULL		0XA002


// Device Control Function
#define UART_CFG_SET    0x00
#define UART_CFG_GET    0x01
#define UART_RTS        0x02
#define UART_CTS        0x04
#define UART_DTR        0x08
#define UART_DSR        0x10
#define UART_USE_INT_RX	0x40
#define UART_USE_INT_TX	0x80

#ifdef ewarm
#pragma pack(1)
#endif
typedef struct
{
	u32_t  nBaud;
    u16_t  nControl;

    //TODO  : add RTS/CTS control,
    //      : add XON/XOFF control.
    //      : add modem support.
}
PACKEDSTRUCT UART_CONFIG;
#ifdef ewarm
#pragma pack()
#endif

typedef enum _SerialParity
{
	ParityNone = 0,
	ParityOdd,
	ParityEven,
	ParityMark,
	ParitySpace
}
SerialParity;

#ifdef ewarm
#pragma pack(1)
#endif
typedef struct
{
	u8_t	Interface;
	u8_t	FlowControl;
	u32_t	Baudrate;
	u8_t	Databit;
	u8_t	Parity;
	u8_t	Stopbit;
	u8_t	Option;
	u16_t  	ResvTimeout;
	u8_t   	SerialID;//mac
	u8_t   	MaxInfoFrame;
	u8_t   	MaxMaster;
	u32_t	AutoBaudrate;
}
PACKEDSTRUCT SERIALCONFIG;
#ifdef ewarm
#pragma pack()
#endif

typedef enum _SerialInterface
{
	RS232 = 0x01,
	RS485 = 0x02,
	RS422 = 0x04,
	CRS232 = 0x10,
	CRS485 = 0x20,
	CRS422 = 0x40,
}SerialInterface;

typedef enum _SerialFlowControl
{
	RTSCTS	= 0x01,
	DTRDSR	= 0x02,
	XONOFF	= 0x04,
	CRTSCTS	= 0x10,
	CDTRDSR = 0x20,
	CXONOFF	= 0x40
}SerialFlowControl;

typedef enum _SerialOption
{
	SettingOverwrite	= 0x01,
	DataPortOverwrite	= 0x02,
	HasFIFO             = 0x04,
	AutoSwitch			= 0x08,
	BACnetDev          = 0x10,    //+ 2014-09-17
}SerialConfigBit;

#define UARTCBSTATUS_RECEIVE	0x01
#define UARTCBSTATUS_TRANSMIT	0x02
#define UARTCBSTATUS_CTS		0x04
#define UARTCBSTATUS_ORE		0x08
#define UARTCBSTATUS_PE			0x10
#define UARTCBSTATUS_FE			0x20
#define UARTCBSTATUS_NE			0x40
#define	UARTCBSTATUS_ERROR		0x80

/// @brief UART callback function
typedef void (ONUARTSENDCOMPLETECALLBACK)(void);
typedef void (UARTCALLBACK)(u8_t bStatus, u8_t ch);

/**
 * @brief Set UART send complete callback
 * @param[in] hHandle Interface handle
 * @param[in] pCallback Callback function pointer
 * @return none
 */
void SetOnUARTSendCompleteCallBack(HANDLE hHandle, ONUARTSENDCOMPLETECALLBACK *pCallback);

/**
 * @brief UART interface initialization
 * @param[in] pName Interface name
 * @param[in] pFile File stream pointer
 * @param[in] pVoid Configuration buffer pointer, UART_CONFIG
 * @return Device handle on success, -1 if not valid
 */
HANDLE UART_Init(LPCSTR pName, EKFILE *pFile, void* pVoid);

/**
 * @brief  store received UART data into UART FIFO
 * @param[in] hHandle Device handle
 * @param[in] ch Data byte
 * @return 0 on success, -1 if buffer full
 */
s16_t UART_Receive(HANDLE hHandle, u8_t ch);

/**
 * @brief Get one byte data from UART FIFO
 * @param[in] hHandle Device handle
 * @return < 0 if data not available, >= 0 for data byte
 */
s16_t UART_getc(HANDLE hHandle);

/**
 * @brief Send one byte to UART
 * @param[in] hHandle Device handle
 * @param[in] ch Data byte to send
 * @return Always OK
 */
s16_t UART_putc(HANDLE hHandle, u8_t ch);

/**
 * @brief Write data stream to UART
 * @param[in] hHandle Device handle
 * @param[in] pBuf Data buffer
 * @param[in] nLen Data length, -1 if NULL terminated
 * @param[out] pByteWritten Number of bytes written, can be null
 * @param[in] nTimeout Timeout interval in milliseconds (0=immediate, -1=no timeout)
 * @return E_TIMEOUT on timeout error, S_OK on success
 */
s16_t UART_puts(HANDLE hHandle, LPCSTR pBuf,
                        s32_t nLen, s32_t *pByteWritten, s16_t nTimeout);

/**
 * @brief Read data stream from UART
 * @param[in] hHandle Device handle
 * @param[out] pBuffer Data buffer
 * @param[in] nLen Data length to read, -1 to read all
 * @param[out] pByteRead Number of bytes read, can be null
 * @param[in] nTimeout Timeout interval in milliseconds (0=immediate, -1=no timeout)
 * @return S_OK on success, E_TIMEOUT on timeout error
 */				
s16_t UART_gets(HANDLE hHandle, LPSTR pBuffer, s32_t nLen,
                        s32_t *pByteRead, s16_t nTimeout);

/**
 * @brief UART start asynchronous send data 
 * @param[in] hHandle Device handle
 * @return none
 */						
void UART_AsynSend(HANDLE hHandle);

/**
 * @brief Check whether asynchronous sending is in progress
 * @param[in] hHandle Device handle
 * @return TRUE if sending is in progress, FALSE otherwise
 */
BOOL UART_IsWriteBusy(HANDLE hHandle);

/**
 * @brief UART read/write configuration 
 * @param[in] hHandle Device handle
 * @param[in] nFunc Function code, 0=read, 1=write
 * @param[in] pVoid Data pointer
 * @return 0 on success
 */
s16_t UART_Control(HANDLE hHandle, u8_t nFunc, void *pVoid);

/**
 * @brief UART idle time processing
 * @param[in] hHandle Device handle
 * @return none
 */
void UART_OnIdle(HANDLE idle);

/**
 * @brief Set UART callback function
 * @param[in] hHandle Device handle
 * @param[in] pCallback Callback function pointer
 * @return none
 */
void SetCommCallBack(HANDLE hHandle, UARTCALLBACK *pCallback);

/**
 * @brief Open UART port and do initialization
 * @param[in] port Port number
 * @param[in] pFile File stream pointer
 * @param[in] pCfg Serial configuration pointer
 * @return Device handle on success, -1 on failure
 */
HANDLE OpenCom(u8_t port, EKFILE* pFile, SERIALCONFIG *pCfg);

/**
 * @brief Set UART receive enable (RS-485 needed)
 * @param[in] hHandle Device handle
 * @return none
 */
void SetRxEnable(HANDLE hHandle);

/**
 * @brief Get UART status
 * @param[in] none
 * @return UART status
 */
BOOL UART_Status(void);
#endif	// end of __UART_H_7243F3FD_7A71_48f1_90BF_5BE4D0312010

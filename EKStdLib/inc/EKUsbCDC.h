#ifndef __EKUSBCDC_H__
#define __EKUSBCDC_H__

#include "EKStdLib.h"
#include "usbd_core.h"

#define CDC_SEND_ENCAPSULATED_COMMAND               0x00
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01
#define CDC_SET_COMM_FEATURE                        0x02
#define CDC_GET_COMM_FEATURE                        0x03
#define CDC_CLEAR_COMM_FEATURE                      0x04
#define CDC_SET_LINE_CODING                         0x20
#define CDC_GET_LINE_CODING                         0x21
#define CDC_SET_CONTROL_LINE_STATE                  0x22
#define CDC_SEND_BREAK_ON                           0x23
#define CDC_SEND_BREAK_OFF                          0x24
#define CDC_SEND_CUSTOM_SETTING                     0x30

/// @brief USB CDC Line Coding
typedef struct
{
  u32_t bitrate;
  BYTE  stopbit;
  BYTE  parity;
  BYTE  databit;
}USB_CDC_LINECODING;

/// @brief USB CDC Callback
typedef void (ONUSBSENDCOMPLETECALLBACK)(void);
typedef void (ONUSBRECEIVECALLBACK)(BYTE *pBuf, u16_t len);
typedef BOOL USBCDCONCONTROL(usb_req *pReq, BYTE* pbuf, BYTE nItf);
typedef BOOL USBCDCONRECEIVE(BYTE* pBuf, u16_t len, BYTE nItf);
typedef BOOL USBCDCONTRANSMIT(BYTE nItf);
typedef BOOL USBCDCCONCMDEPTRANSMIT(BYTE nItf); //Vincent 20160128 0.0.3
typedef void (ONUSBSETTING)(BYTE type, void *pBuf, int size, int itf);

typedef struct
{
	USBCDCONCONTROL *pOnControl;
	USBCDCONRECEIVE *pOnReceive;
	USBCDCONTRANSMIT *pOnTransmit;
  USBCDCCONCMDEPTRANSMIT *pOnInterruptEpTransmit;//Vincent 20160128 0.0.3

}USBCALLBACK;

/// @brief USB CDC Connection interface
typedef struct _USBDEVICES
{
    LPCSTR				pDeviceName;
    EKFILE				*pFile;
    u8_t				Status;
	  volatile BOOL		bTxBusy;
    volatile BOOL		bCmdTxBusy; //Vincent 20160128 0.0.3
    volatile BOOL		bPacketReceived;
	  USB_CDC_LINECODING	lineCoding;
	  ONUSBSENDCOMPLETECALLBACK	*m_pOnSendCompleteCallback;
	  ONUSBRECEIVECALLBACK		*m_pOnReceiveCallback;
	  ONUSBSETTING				*m_pOnSettingCallback;
    TIMER				TxTimer;
    TIMER                               CmdTxTimer;
    TIMER				RxTimer;
}USBCDCCONNECTIONS;

/**
 * @brief Set USB CDC send complete callback
 * @param[in] hHandle Device handle
 * @param[in] pCallback Callback function pointer
 * @return none
 */
void SetOnUSBSendCompleteCallBack(HANDLE hHandle, ONUSBSENDCOMPLETECALLBACK *pCallback);

/**
 * @brief Set USB CDC receive callback
 * @param[in] hHandle Device handle
 * @param[in] pCallback Callback function pointer
 * @return none
 */
void SetOnUSBReceiveCallBack(HANDLE hHandle, ONUSBRECEIVECALLBACK *pCallback);

/**
 * @brief Set USB CDC on-setting callback
 * @param[in] hHandle Device handle
 * @param[in] pCallback Callback function pointer
 * @return none
 */
void SetOnUSBSettingCallBack(HANDLE hHandle, ONUSBSETTING *pCallback);

/**
 * @brief Connect USB CDC device
 * @param[in] none
 * @return TRUE on success, FALSE on failure
 */
BOOL UsbCDC_Connect(void);

/**
 * @brief Disconnect USB CDC device
 * @param[in] none
 * @return TRUE on success, FALSE on failure
 */
BOOL UsbCDC_Disconnect(void);

/**
 * @brief USB CDC device initialization
 * @param[in] pName Device name
 * @param[in] pFile File stream pointer
 * @param[in] pVoid Initialization buffer pointer
 * @return Device handle on success, -1 if not valid
 */
HANDLE UsbCDC_Init(LPCSTR pName, EKFILE *pFile, void* pVoid);

/**
 * @brief Check if USB CDC write is busy
 * @param[in] hHandle Device handle
 * @return TRUE if write is busy, FALSE otherwise
 */
BOOL UsbCDC_IsWriteBusy(HANDLE hHandle);

/**
 * @brief Get one byte from USB CDC interface
 * @param[in] hHandle Device handle
 * @return Data byte on success, -1 if no data available
 */
s16_t UsbCDC_getc(HANDLE hHandle);

/**
 * @brief Send one byte to USB CDC interface
 * @param[in] hHandle Device handle
 * @param[in] ch Data byte to send
 * @return S_OK on success
 */
s16_t UsbCDC_putc(HANDLE hHandle, u8_t ch);

/**
 * @brief Read/write USB CDC interface configuration
 * @param[in] hHandle Device handle
 * @param[in] nFunc Control function, 0 for read, 1 for write
 * @param[in] pVoid Control data pointer
 * @return S_OK on success, error code on failure
 */
s16_t UsbCDC_Control(HANDLE hHandle, u8_t nFunc, void *pVoid);

/**
 * @brief Read data from USB CDC interface
 * @param[in] hHandle Device handle
 * @param[out] pBuffer Data buffer
 * @param[in] nLen Data length to read, -1 to read all
 * @param[out] pByteRead Number of bytes read, can be null
 * @param[in] nTimeout Timeout interval in milliseconds
 * @return S_OK on success, E_TIMEOUT on timeout error
 */
s16_t UsbCDC_gets(HANDLE hHandle, LPSTR pBuffer, s32_t nLen, s32_t *pByteRead, s16_t nTimeout);

/**
 * @brief Write data Stream to USB CDC interface
 * @param[in] hHandle Device handle
 * @param[in] pBuf Data buffer
 * @param[in] nLen Data length, -1 if NULL terminated
 * @param[out] pByteWritten Number of bytes written, can be null
 * @param[in] nTimeout Timeout interval in milliseconds
 * @return E_TIMEOUT on timeout error, S_OK on success
 */
s16_t UsbCDC_puts(HANDLE hHandle, LPCSTR pBuf, s32_t nLen, s32_t *pByteWritten, s16_t nTimeout);

/**
 * @brief Flush USB CDC buffers
 * @param[in] hHandle Device handle
 * @return none
 */
void UsbCDC_Flush(HANDLE hHandle);

/**
 * @brief Get USB CDC receive buffer count
 * @param[in] hHandle Device handle
 * @return Number of bytes in receive buffer
 */
s16_t UsbCDC_GetRcvBufferCount(HANDLE hHandle);

/**
 * @brief Send FIFO data stream to USB CDC interface
 * @param[in] hHandle Device handle
 * @return none
 */
void USBCDC_SendFIFO(HANDLE hHandle);

#endif	//end of __EKUSBCDC_H__
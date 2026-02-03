#ifndef __EKSTREAM_H_755E2250_0BEB_49bf_87E8_D7D59E265448
#define __EKSTREAM_H_755E2250_0BEB_49bf_87E8_D7D59E265448

#include "EKStdLib.h"

typedef struct
{
    FIFO    inStream;		//incoming stream buffer
    FIFO    outStream;		//outgoing stream buffer
//    u8_t    Status;         //Stream Status
    LPVOID  pDevData;       //device data
}EKFILE;



// Device declaration helper macro function
// name - instance name of device
// inSize - incoming stream buffer size
// outSize - outgoing stream buffer size
#define IMPLEMENT_FILE(name, inSize, outSize)\
	/*stream buffer*/\
    u8_t inbuf##name[inSize];\
    u8_t outbuf##name[outSize];\
	/*file stream*/\
    EKFILE file##name = { \
                        {&(inbuf##name[0]), 0, 0, inSize}, \
                        {&(outbuf##name[0]), 0, 0, outSize}, \
                        0};\
	/*device handle*/\
    HANDLE h##name;\

// Device declaration helper macro function without file io buffer
// name - instance name of device
// inSize - incoming stream buffer size
// outSize - outgoing stream buffer size
#define IMPLEMENT_FILE_NOBUF(name)\
	/*file stream*/\
    EKFILE file##name = { \
                        {NULL, 0, 0, 0}, \
                        {NULL, 0, 0, 0}, \
                        NULL};\
	/*device handle*/\
    HANDLE h##name;\

/* Device table declarations */
typedef struct		        /* device table entry */
{
	HANDLE  (*dvOpen)(LPCSTR, EKFILE*, void*);					    //Opens device & do initialization
	s16_t	(*dvRead)(HANDLE, LPSTR, s32_t, s32_t* , s16_t);	    //Reads data from stream
	s16_t	(*dvWrite)(HANDLE, LPCSTR, s32_t, s32_t*, s16_t);	    //Writes data to stream
    s16_t	(*dvEdit)(HANDLE, LPCSTR, s32_t, u8_t);	                //Edit stream data
	s16_t	(*dvClose)(HANDLE);	                                    //close stream
	s16_t	(*dvSeek)(HANDLE, u32_t);							    //Moves the file pointer to a specified location
    s16_t	(*dvTell)(HANDLE, u32_t*, u32_t*);			            //Get file size & current pointer
	s16_t	(*dvgetc)(HANDLE);								        //Reads one byte data from stream
	s16_t	(*dvputc)(HANDLE, u8_t);							    //Writes one byte data to stream
	BOOL    (*dvIsWriteBusy)(HANDLE);							    //Check for write operation pending status
	s16_t	(*dvCntl)(HANDLE, u8_t, void*);			                //Device control functions
    void    (*dvOnIdle)(HANDLE hHandle);
    void    (*dvFlush)(HANDLE hHandle);
    s16_t   (*dvGetRcvBufferCount)(HANDLE hHandle);
}devsw;

/**
 * @brief Open registered communication stream and do initialization
 * @param[in] pName Registered device name
 * @param[in] pFile File stream
 * @param[in] pVoid Device initialization data pointer
 * @return Device handle on success, E_NOTFOUND if device not registered, E_INIT if initialization failed
 */
HANDLE EKfopen(LPCSTR pName, EKFILE *pFile, void *pVoid);

/**
 * @brief Read one byte data from communication stream 
 * @param[in] hHandle Device handle returned by EKfopen
 * @return E_NOTSUPPORT if device not supported, E_INVHANDLE if invalid handle, device specific return code otherwise
 */
s16_t EKfgetc(HANDLE hHandle);

/**
 * @brief Write one byte data to communication stream 
 * @param[in] hHandle Device handle returned by EKfopen
 * @param[in] ch Data byte
 * @return E_NOTSUPPORT if device not supported, E_INVHANDLE if invalid handle, device specific return code otherwise
 */
s16_t EKfputc(HANDLE hHandle, u8_t ch);

/**
 * @brief Write data stream to communication stream 
 * @param[in] hHandle Device handle returned by EKfopen
 * @param[in] pBuf Data buffer
 * @param[in] nLen Data length, -1 if NULL terminated
 * @param[out] pByteWritten Number of bytes written, can be null
 * @param[in] nTimeout Timeout interval in milliseconds
 * @return E_NOTSUPPORT if device not supported, E_INVHANDLE if invalid handle, device specific return code otherwise
 */
s16_t EKfwrite(HANDLE hHandle, LPCSTR pBuffer, s32_t nLen, s32_t *pByteWritten, s16_t nTimeout);

/**
 * @brief Edit stream data
 * @param[in] hHandle Device handle returned by EKfopen
 * @param[in] pBuffer Data buffer
 * @param[in] nLen Data length, -1 if NULL terminated
 * @param[in] editMode Edit mode
 * @return E_NOTSUPPORT if device not supported, E_INVHANDLE if invalid handle, device specific return code otherwise
 */
s16_t EKfedit(HANDLE hHandle, LPCSTR pBuffer, s32_t nLen, u8_t editMode);

/**
 * @brief Close communication stream 
 * @param[in] hHandle Device handle returned by EKfopen
 * @return E_NOTSUPPORT if device not supported, E_INVHANDLE if invalid handle, device specific return code otherwise
 */
s16_t EKfclose(HANDLE hHandle);

/**
 * @brief Read data stream from communication stream 
 * @param[in] hHandle Device handle returned by EKfopen
 * @param[out] pBuffer Data buffer
 * @param[in] nLen Data length, -1 if NULL terminated
 * @param[out] pByteRead Number of bytes read, can be null
 * @param[in] nTimeout Timeout interval in milliseconds
 * @return E_NOTSUPPORT if device not supported, E_INVHANDLE if invalid handle, device specific return code otherwise
 */
s16_t EKfread(HANDLE hHandle, LPSTR pBuffer, s32_t nLen, s32_t *pByteRead, s16_t nTimeout);

/**
 * @brief Moves the file pointer to a specified location
 * @param[in] hHandle Device handle returned by EKfopen
 * @param[in] Offset  address to seek
 * @return E_NOTSUPPORT if device not supported, E_INVHANDLE if invalid handle, device specific return code otherwise
 */
s16_t EKfseek(HANDLE hHandle, u32_t Offset);

/**
 * @brief Get file size & current Offset
 * @param[in] hHandle Device handle returned by EKfopen
 * @param[in] Size    file size
 * @param[in] Offset  file current offset
 * @return E_NOTSUPPORT if device not supported, E_INVHANDLE if invalid handle, device specific return code otherwise
 */
s16_t EKftell(HANDLE hHandle, u32_t *Size, u32_t *Offset);

/**
 * @brief Check for device write operation availability
 * @param[in] hHandle Device handle returned by EKfopen
 * @return E_NOTSUPPORT if device not supported, E_INVHANDLE if invalid handle, device specific return code otherwise
 */
s16_t EKfIsWriteBusy(HANDLE hHandle);

/**
 * @brief Read/Write device configuration
 * @param[in] hHandle Device handle returned by EKfopen
 * @param[in] nFunc Configuration function
 * @param[in] pVoid Configuration data pointer
 * @return E_SYSTEM if device not supported, E_INVPARAM if invalid handle, device specific return code otherwise
 */
s16_t EKfControl(HANDLE hHandle, u8_t nFunc, void *pVoid);

/**
 * @brief Stream idle time processing
 * @param[in] hHandle Device handle
 * @return none
 */
void  EKfOnIdle(HANDLE hHandle);

/**
 * @brief Flush stream data
 * @param[in] hHandle Device handle
 * @return none
 */
void  EKfflush(HANDLE hHandle);

/**
 * @brief Get receive buffer count
 * @param[in] hHandle Device handle
 * @return receive buffer count
 */
s16_t EKfgetRcvBufferCount(HANDLE hHandle);
#endif  // end of __STREAM_H_755E2250_0BEB_49bf_87E8_D7D59E265448

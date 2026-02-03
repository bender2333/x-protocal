#ifndef __STDUTIL_H_B6B4545A_B08B_4013_856D_909C73C8315A
#define __STDUTIL_H_B6B4545A_B08B_4013_856D_909C73C8315A

#include "EKDatatype.h"

/**
 * @brief Calculate CRC16 checksum using slow algorithm with initial value
 * @param[in] sum Initial CRC value
 * @param[in] p Data buffer
 * @param[in] len Data length in bytes
 * @return CRC16 checksum value
 */
u16_t CRC16Slow(unsigned short sum, unsigned char *p, unsigned int len);

/**
 * @brief Calculate CRC16 checksum using slow algorithm 2
 * @param[in] p Data buffer
 * @param[in] len Data length
 * @return CRC16 checksum value
 */
u16_t CRC16Slow2(BYTE* p, DWORD len);

/**
 * @brief Calculate CRC16 checksum using fast algorithm
 * @param[in] pBuf Data buffer
 * @param[in] dataLength Data length in bytes
 * @return CRC16 checksum value
 */
u16_t CRC16Fast(u8_t *pBuf, u16_t dataLength);

/**
 * @brief Calculate CRC using fast algorithm 2 with initial value
 * @param[in] pBuf Data buffer
 * @param[in] dataLength Data length in bytes
 * @param[in] crc Initial CRC value
 * @return CRC checksum value
 */
u16_t CRC16Fast2(u8_t *pBuf, u16_t dataLength, u16_t crc);

/**
 * @brief Convert binary value to hex character
 * @param[in] ch Binary value (0-15)
 * @return Hex character ('0'-'9', 'A'-'F')
 */
u8_t BIN2HexChar(u8_t ch);

/**
 * @brief Convert hex character to binary value
 * @param[in] ch Hex character ('0'-'9', 'A'-'F', 'a'-'f')
 * @return Binary value (0-15), 0xFF on error
 */
u8_t HexChar2Bin(u8_t ch);

/**
 * @brief Add N characters to string
 * @param[in,out] pBuf String buffer
 * @param[in] cnt Number of characters to add
 * @param[in] ch Character to add
 * @return none
 */
void StrAddNChar(u8_t *pBuf, u8_t cnt, u8_t ch);

/**
 * @brief Delete character at specified position
 * @param[in,out] pBuf String buffer
 * @param[in] pos Position to delete character
 * @return none
 */
void DeleteChar(u8_t *pBuf, u8_t pos);

/**
 * @brief Convert byte value to string
 * @param[out] pBuf Output buffer for string
 * @param[in] value Byte value to convert
 * @param[in] bPadZero TRUE to pad with leading zeros
 * @return none
 */
void Byte2Str(u8_t* pBuf, u8_t value, BOOL bPadZero);

/**
 * @brief Convert binary value to hex string
 * @param[out] pBuf Output buffer for hex string
 * @param[in] ch Binary value to convert
 * @return none
 */
void BIN2Hex2Str(u8_t *pBuf, u8_t ch);

/**
 * @brief Convert IP address to string
 * @param[out] pBuf Output buffer for string
 * @param[in] IPAddress IP address array
 * @param[in] bPadZero TRUE to pad with leading zeros
 * @return none
 */
void IPAddress2Str(u8_t* pBuf, u8_t* IPAddress, BOOL bPadZero);

/**
 * @brief Convert 32-bit unsigned integer to string
 * @param[in] value Value to convert
 * @param[in] bPadZero TRUE to pad with leading zeros
 * @param[out] pBuf Output buffer for string
 * @return none
 */
void INT32UtoStr(u32_t value, BOOL bPadZero, u8_t *pBuf);

/**
 * @brief Convert byte value to hex string
 * @param[out] pBuf Output buffer for hex string
 * @param[in] value Byte value to convert
 * @param[in] bAppend TRUE to append to existing string
 * @return none
 */
void Byte2HexStr(u8_t *pBuf, u8_t value, BOOL bAppend);

/**
 * @brief Convert MAC address to string
 * @param[out] pBuf Output buffer for string
 * @param[in] pMac MAC address array
 * @return none
 */
void MACAddress2Str(u8_t *pBuf, u8_t *pMac);

/**
 * @brief Convert IP string to bytes
 * @param[in] pBuf IP string buffer
 * @param[out] pIP IP address array
 * @return TRUE on success, FALSE on failure
 */
BOOL IPStr2Bytes(u8_t *pBuf, u8_t *pData);

/**
 * @brief Convert MAC string to bytes
 * @param[out] pBuf Output buffer for MAC bytes
 * @param[in] pMac MAC address string
 * @return none
 */
void MACStr2Bytes(u8_t *pBuf, u8_t *pMac);

/**
 * @brief Convert BCD to binary
 * @param[in] bcd BCD value
 * @return Binary value
 */
u8_t BCD2BIN(u8_t bcd);

/**
 * @brief Convert binary to BCD
 * @param[in] bin Binary value
 * @return BCD value
 */
u8_t BIN2BCD(u8_t bin);

/**
 * @brief Get index of string in option array
 * @param[in] pOption Array of option strings
 * @param[in] pSelectStr String to search for
 * @return Index of string, -1 if not found
 */
s16_t GetIndex(const u8_t* const pOption[], const u8_t* pSelectStr);

/**
 * @brief Calculate 2 to the power of num
 * @param[in] num Exponent
 * @return 2^num
 */
u16_t pow2(u16_t num);

/**
 * @brief Convert version number to string
 * @param[in] ver Version number
 * @param[out] pbuf Output buffer for version string
 * @return none
 */
void Version2String(u16_t ver, u8_t *pbuf);

/**
 * @brief Encrypt simple password
 * @param[in,out] data Password data to encrypt
 * @param[in] bBinary TRUE if binary data, FALSE if string
 * @return none
 */
void EncryptSimplePassword(u8_t* data, BOOL bBinary);

/**
 * @brief Decrypt simple password
 * @param[in,out] data Password data to decrypt
 * @param[in] bBinary TRUE if binary data, FALSE if string
 * @return none
 */
void DecryptSimplePassword(u8_t* data, BOOL bBinary);

/**
 * @brief Verify simple password
 * @param[in] ptrPassword Password to verify
 * @param[in] data Encrypted password data
 * @return TRUE if password matches, FALSE otherwise
 */
BOOL VerifySimplePassword(u8_t* ptrPassword, u8_t *data);

/**
 * @brief Convert hex string to long integer
 * @param[in] pBuf Hex string buffer
 * @return Long integer value
 */
u32_t HexStr2Long(u8_t *pBuf);

/**
 * @brief Convert MAC hex string to bytes
 * @param[out] pBuf Output buffer for MAC bytes
 * @param[in] pMac MAC address hex string
 * @return none
 */
void MACHexStr2Bytes(u8_t *pBuf, u8_t *pMac);

/**
 * @brief Convert hex string to short integer
 * @param[in] pBuf Hex string buffer
 * @return Short integer value
 */
u16_t HexStr2Short(u8_t *pBuf);

/**
 * @brief Convert big-endian bytes to little-endian long
 * @param[in] buf Byte buffer
 * @return Little-endian long value
 */
u32_t BigEndByte2LittleEndLong(u8_t *buf);

/**
 * @brief return a NaN (Not a Number) value
 * @param[in] none
 * @return NaN float value
 */
float toNAN(void);

/**
 * @brief Calculate UTF-8 string length
 * @param[in] s UTF-8 string
 * @return String length in characters
 */
u16_t strlen_utf8(char *s);//2018-11-29 Sam

#endif	/* end of __STDUTIL_H_B6B4545A_B08B_4013_856D_909C73C8315A */

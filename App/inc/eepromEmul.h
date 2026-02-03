#ifndef __EEEPROM_EMU_H
#define __EEEPROM_EMU_H

#ifdef __cplusplus
 extern "C" {
#endif

////define EEPROM emulator config
#define EEPROM_DATA_SIZE          0x20000
#define EEPROM_FLASH_PAGE_NUM     0x1000 //4K Flash pages, 8 EEPROM pages （4MB)
#define FLASH_PAGE_SIZE           256 
#define EEPROM_BACKUP_START_ADDR  0
#define FLASH_PAGES_PER_EEPROM_PAGE   512

typedef enum
{
	EEPROM_OK,		
	EEPROM_ERROR,
	EEPROM_EMPTY,
}EEPROMSTATUS;

/**
 * @brief   EEPROM Initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void EEPROMInit(void);

/**
 * @brief    EEPROM object processing loop.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void EEPROMObj(void);

/**
 * @brief   Get EEPROM status, refer EEPROMSTATUS.
 *
 * @param[in]  none
 * @param[out] none
 * @return     EEPROMSTATUS
 */
EEPROMSTATUS GetEEPROMStatus(void);

/**
 * @brief   Read data from EEPROM at specified address.
 *
 * @param[in]  Addr    EEPROM address to read from.
 * @param[out] pData   Pointer to store read data.
 * @param[in]  nLen    Number of bytes to read.
 *
 * @return     TRUE if read successful, FALSE otherwise.
 */
BOOL EEPROMRead(u32_t Addr, void* pData, uint32_t nLen);

/**
 * @brief   Write data to EEPROM at specified address.
 *
 * @param[in]  Addr    EEPROM address to write to.
 * @param[in]  pData   Pointer to data to write.
 * @param[in]  nLen    Number of bytes to write.
 * @param[out] none
 * @return     TRUE if write successful, FALSE otherwise.
 */
BOOL EEPROMWrite(u32_t Addr, void* pData, u32_t nLen);

/**
 * @brief   Erase data from EEPROM.
 *
 * @param[in]  Addr    Starting address in EEPROM.
 * @param[in]  nLen    Number of bytes to erase.
 * @param[out] none
 * @return     TRUE if erase successful, FALSE otherwise.
 */
BOOL EEPROMErase(u32_t Addr, u32_t nLen);

/**
 * @brief   Write EEPROM signature to indicate the EEPROM is valid and not empty.
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if write successful, FALSE otherwise.
 */
BOOL EEPROMWriteSignature(void);

/**
 * @brief   Check if EEPROM is busy with write operations.
 *
 * @param[in]  none
 * @param[out] none
 * @return     TRUE if busy, FALSE if free.
 */
BOOL EEPROMBusy(void);

/**
 * @brief   Get EEPROM page size,fixed to 64Bytes as we makes it compatible with 24LC256 
 *
 * @param[in]  none
 * @param[out] none
 * @return     EEPROM page size in bytes.
 */
u8_t EEPROMGetPageSize(void);

/**
 * @brief   Set EEPROM to empty state.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void EEPROMSetEmpty(void);

/* Test function */
void test_eepromemul_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* __EEEPROM_EMU_H */



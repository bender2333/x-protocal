#ifndef __OTA_H__
#define __OTA_H__

typedef enum
{
  OTA_PARTITION_ERASE = 1,
  OTA_PROGRAM_BLOCK = 2,
  OTA_CRC_CHECK = 3,
  OTA_ERROR = 255
}OTA_PROCESS_STATUS;

#define APP_START_ADDR 0x08080000
#define APP2_START_ADDR 0x08200000
#define FLASH_END_ADDR 0x083FFFFF

/**
 * @brief   OTA Upgrade firmware through modbus private function code
 *
 * @param[in]  pdata    Pointer to the OTA data buffer.
 * @param[out] none
 * @return     OTA_PROCESS_STATUS     
 */
BYTE FWOTAUpgrade(BYTE* pdata);

#endif //__OTA_H__
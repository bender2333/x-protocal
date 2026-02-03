#ifndef __DEV_W25Q128_H_
#define __DEV_W25Q128_H_

#include "MyDevice.h"
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

#define  SPI_FLASH_BLOCK_SIZE      0x10000U
#define  SPI_FLASH_PAGE_SIZE       0x100U
#define  SPI_FLASH_CS_LOW()        gpio_bit_reset(GPIOB, GPIO_PIN_8)
#define  SPI_FLASH_CS_HIGH()       gpio_bit_set(GPIOB, GPIO_PIN_8)

/**
 * @brief   SPI GPIO and parameters for W25Q128 flash memory initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void spi_flash_init(void);

/**
 * @brief   Erase the specified flash sector.
 *
 * @param[in]  sector_addr   Address of the sector to erase.
 * @param[out] none
 * @return     none
 */
void spi_flash_sector_erase(uint32_t sector_addr);

/**
 * @brief   Erase the specified flash block (32KB unit).
 *
 * @param[in]  block_addr   Address of the block to erase.
 * @param[out] none
 * @return     none
 */
void spi_flash_block_erase(uint32_t block_addr);

/**
 * @brief   Erase the specified flash block (64KB unit).
 *
 * @param[in]  block_addr   Address of the block to erase.
 * @param[out] none
 * @return     none
 */
void spi_flash_block64_erase(uint32_t block_addr);

/**
 * @brief   Erase the entire flash memory.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void spi_flash_bulk_erase(void);

/**
 * @brief   Write multiple bytes to the flash memory.
 *
 * @param[in]  pbuffer              Pointer to the buffer containing data to write.
 * @param[in]  write_addr           Flash internal address to write to.
 * @param[in]  num_byte_to_write   Number of bytes to write to the flash.
 * @param[out] none
 * @return     none
 */
void spi_flash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint16_t num_byte_to_write);

/**
 * @brief   Write block of data to the flash memory with page boundary handling.
 *
 * @param[in]  pbuffer              Pointer to the buffer containing data to write.
 * @param[in]  write_addr           Flash internal address to write to.
 * @param[in]  num_byte_to_write   Number of bytes to write to the flash.
 * @param[out] none
 * @return     none
 */
void spi_flash_buffer_write(uint8_t *pbuffer, uint32_t write_addr, uint16_t num_byte_to_write);

/**
 * @brief   Read a block of data from the flash memory.
 *
 * @param[in]  pbuffer             Pointer to the buffer that receives the data read from the flash.
 * @param[in]  read_addr           Flash internal address to read from.
 * @param[in]  num_byte_to_read   Number of bytes to read from the flash.
 * @param[out] none
 * @return     none
 */
void spi_flash_buffer_read(uint8_t *pbuffer, uint32_t read_addr, uint16_t num_byte_to_read);

/**
 * @brief   Read flash identification (JEDEC ID).
 *
 * @param[in]  none
 * @param[out] none
 * @return     Flash identification value (24-bit JEDEC ID).
 */
uint32_t spi_flash_read_id(void);

/**
 * @brief   Start a read data sequence from the flash memory.
 *
 * @param[in]  read_addr   Flash internal address to read from.
 * @param[out] none
 * @return     none
 */
void spi_flash_start_read_sequence(uint32_t read_addr);

/**
 * @brief   Read a byte from the SPI flash memory.
 *
 * @param[in]  none
 * @param[out] none
 * @return     Byte read from the SPI flash.
 */
uint8_t spi_flash_read_byte(void);

/**
 * @brief   Send a byte through the SPI interface and return the byte received from the SPI bus.
 *
 * @param[in]  byte   Byte to send.
 * @param[out] none
 * @return     The value of the received byte.
 */
uint8_t spi_flash_send_byte(uint8_t byte);

/**
 * @brief   Send a half word through the SPI interface and return the half word received from the SPI bus.
 *
 * @param[in]  half_word   Half word to send.
 * @param[out] none
 * @return     The value of the received half word.
 */
uint16_t spi_flash_send_halfword(uint16_t half_word);

/**
 * @brief   Enable the write access to the flash memory.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void spi_flash_write_enable(void);

/**
 * @brief   Poll the status of the write in progress (WIP) flag in the flash's status register.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void spi_flash_wait_for_write_end(void);

/**
 * @brief   Enable the flash quad mode for enhanced performance.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void spi_quad_flash_quad_enable(void);

/**
 * @brief   Write block of data to the flash using SPI quad mode.
 *
 * @param[in]  pbuffer              Pointer to the buffer containing data to write.
 * @param[in]  write_addr           Flash internal address to write to.
 * @param[in]  num_byte_to_write   Number of bytes to write to the flash.
 * @param[out] none
 * @return     none
 */
bool spi_quad_flash_buffer_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t num_byte_to_write);

/**
 * @brief   Read a block of data from the flash using SPI quad mode.
 *
 * @param[in]  pbuffer             Pointer to the buffer that receives the data read from the flash.
 * @param[in]  read_addr           Flash internal address to read from.
 * @param[in]  num_byte_to_read   Number of bytes to read from the flash.
 * @param[out] none
 * @return     none
 */
void spi_quad_flash_buffer_read(uint8_t *pbuffer, uint32_t read_addr, uint32_t num_byte_to_read);

/**
 * @brief   Write multiple bytes to the flash using SPI quad mode.
 *
 * @param[in]  pbuffer              Pointer to the buffer containing data to write.
 * @param[in]  write_addr           Flash internal address to write to.
 * @param[in]  num_byte_to_write   Number of bytes to write to the flash.
 * @param[out] none
 * @return     none
 */
bool spi_quad_flash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint16_t num_byte_to_write);

/**
 * @brief   Test task for W25Q128 flash memory operations.
 *
 * @param[in]  pvParameters   Task parameters (unused).
 * @param[out] none
 * @return     none
 */
void test_w25q128_task(void *pvParameters);

#endif	// end of __DEV_W25Q128_H_

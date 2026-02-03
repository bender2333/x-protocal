#ifndef DEV_SDRAM_H
#define DEV_SDRAM_H

#include "MyDevice.h"
#include "EKStdLib.h"

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

#define SDRAM_DEVICE0_ADDR                         ((uint32_t)0xC0000000)
#define SDRAM_DEVICE1_ADDR                         ((uint32_t)0xD0000000)

extern WORD SDRAMTestStatus;
extern DWORD SDRAMSpeed;

/**
 * @brief SDRAM peripheral and GPIO pins initialization
 * 
 * @param[in] sdram_device Specify the SDRAM device
 * 
 * @return none
 */
void exmc_synchronous_dynamic_ram_init(uint32_t sdram_device);

/**
 * @brief Fill the buffer with specified value
 * 
 * @param[in] pbuffer Pointer on the buffer to fill
 * @param[in] buffer_lengh Size of the buffer to fill
 * @param[in] value Value to fill on the buffer
 * 
 * @return none
 */
void fill_buffer(uint8_t *pbuffer, uint16_t buffer_lengh, uint16_t offset);

/**
 * @brief Write a byte buffer(data is 8 bits) to the EXMC SDRAM memory
 * 
 * @param[in] sdram_device Specify which a SDRAM memory block is written
 * @param[in] pbuffer Pointer to buffer
 * @param[in] writeaddr SDRAM memory internal address from which the data will be written
 * @param[in] numbytetowrite Number of bytes to write
 * 
 * @return none
 */
void sdram_writebuffer_8(uint32_t sdram_device, uint8_t *pbuffer, uint32_t writeaddr, uint32_t numbytetowrite);

/**
 * @brief Read a block of 8-bit data from the EXMC SDRAM memory
 * 
 * @param[in] sdram_device Specify which a SDRAM memory block is written
 * @param[in] pbuffer Pointer to buffer
 * @param[in] readaddr SDRAM memory internal address to read from
 * @param[in] numbytetoread Number of bytes to read
 * 
 * @return none
 */
void sdram_readbuffer_8(uint32_t sdram_device, uint8_t *pbuffer, uint32_t readaddr, uint32_t numbytetoread);

/**
 * @brief Test SDRAM task for read/write verification
 * 
 * @param[in] pvParameters Task parameters (unused)
 * 
 * @return none
 */
void test_sdram_task(void *pvParameters);

#endif /* DEV_SDRAM_H */

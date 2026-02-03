#ifndef __DEV_FLASH_H__
#define __DEV_FLASH_H__

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

/**
 * @brief Write 32-bit data to flash
 * 
 * @param[in] addr Flash address to write to
 * @param[in] data 32-bit data to write
 * 
 * @return BOOL TRUE on success, FALSE on failure
 */
BOOL dev_flashWriteU32(u32_t addr, u32_t data);

/**
 * @brief Erase a flash page
 * 
 * @param[in] nPage Page number to erase
 * 
 * @return none
 */
bool dev_flashPageErase(BYTE nPage);

/**
 * @brief Enable or disable flash read protection
 * 
 * @param[in] en TRUE to enable protection, FALSE to disable
 * 
 * @return none
 */
void dev_flashReadProtect(BOOL en);
#endif    //__DEV_FLASH_H__
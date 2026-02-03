#ifndef __DEV_UART_H_57EC2A4B_E74F_4009_93E7_BF2FC17886BF
#define __DEV_UART_H_57EC2A4B_E74F_4009_93E7_BF2FC17886BF

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

#ifdef ewarm
#pragma pack(1)
#endif
typedef struct
{
    rcu_periph_enum       ucPortClk;
    uint32_t              ulBase;
    UARTCALLBACK          **ppCallback;
    tGPIOInfo             uartTXDInfo;
    tGPIOInfo             uartRXDInfo;
    tGPIOInfo             uartTXENInfo;
}
PACKEDSTRUCT tUARTInfo;
#ifdef ewarm
#pragma pack()
#endif

#define DEV_UART_MAX            4
#define DEV_TOTAL_RS485         2

/**
 * @brief UART peripheral and GPIO pins initialization, can be called by EKfOpen
 * 
 * @param[in] hHandle UART handle number
 * @param[in] pCfg UART configuration structure
 * 
 * @return BOOL TRUE if successful, FALSE otherwise
 */
BOOL dev_uart_Init(HANDLE hHandle, UART_CONFIG *pCfg);

/**
 * @brief Check if UART FIFO is full
 * 
 * @param[in] hHandle UART handle number
 * 
 * @return BOOL TRUE if FIFO is full, FALSE otherwise
 */
BOOL dev_uart_FifoFull(HANDLE hHandle);

/**
 * @brief Send one byte data to UART
 * 
 * @param[in] hHandle UART handle number
 * @param[in] ch Data byte to send
 * 
 * @return BOOL Always TRUE
 */
BOOL dev_uart_putc(HANDLE hHandle, u8_t ch);

/**
 * @brief Check if UART is ready for sending data
 * 
 * @param[in] hHandle UART handle number
 * 
 * @return BOOL TRUE if ready, FALSE otherwise
 */
BOOL dev_uart_IsTxEmpty(HANDLE hHandle);

/**
 * @brief Start asynchronous sending
 * 
 * @param[in] hHandle UART handle number
 * 
 * @return none
 */
void dev_uart_StartAsynSend(HANDLE hHandle);

/**
 * @brief Stop asynchronous sending
 * 
 * @param[in] hHandle UART handle number
 * 
 * @return none
 */
void dev_uart_StopAsynSend(HANDLE hHandle);

/**
 * @brief Enable or disable UART transmitter
 * 
 * @param[in] hHandle UART handle number
 * @param[in] bHalfDuplex TRUE for half duplex mode, FALSE for full duplex
 * @param[in] bEnable TRUE to enable, FALSE to disable
 * 
 * @return none
 */
void dev_uart_EnableTx(HANDLE hHandle, BOOL bHalfDuplex, BOOL bEnable);

/**
 * @brief Switch UART to receive mode in half duplex
 * 
 * @param[in] hHandle UART handle number
 * 
 * @return BOOL Always returns TRUE
 */
BOOL dev_uart_SwitchRx(HANDLE hHandle);

/**
 * @brief Set UART callback function
 * 
 * @param[in] hHandle UART handle number
 * @param[in] pCallback Pointer to callback function
 * 
 * @return none
 */
void dev_uart_SetCallBack(HANDLE hHandle, UARTCALLBACK *pCallback);

/**
 * @brief Read/Write UART communication state (reserved, not used)
 * 
 * @param[in] hHandle UART handle number
 * @param[in] nFunc Function code, 0=read, 1=write
 * @param[in] pVoid Pointer to data
 * 
 * @return none
 */
void dev_uart_SetCommState(HANDLE hHandle, u8_t nFunc, void *pVoid);

#endif      // end of __UARTLPC22xx_H_57EC2A4B_E74F_4009_93E7_BF2FC17886BF

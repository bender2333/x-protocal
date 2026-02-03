#ifndef __FIFO_H_A823A9E2_68DA_4e59_9D3D_B90CFA7E20D3
#define __FIFO_H_A823A9E2_68DA_4e59_9D3D_B90CFA7E20D3

#include "EKDataType.h"

typedef struct
{
	u8_t    *Buffer; //buffer
	u16_t  nHead;   //Head pointer
	u16_t  nTail;   //Tail pointer
	u16_t  nSize;   //size of buffer
}FIFO;

/**
 * @brief Add a character to the FIFO buffer
 * @param[in,out] pfifo Pointer to the FIFO structure
 * @param[in] data Data byte to add to the FIFO
 * @return 0 on success, -1 if buffer is full
 */
s16_t fifo_add(FIFO* pfifo, u8_t data);

/**
 * @brief Get a character from the FIFO buffer
 * @param[in,out] pfifo Pointer to the FIFO structure
 * @return -1 if buffer is empty, otherwise the data byte
 */
s16_t fifo_get(FIFO* pfifo);           

/**
 * @brief Peek at the next character in the FIFO without removing it
 * @param[in] pfifo Pointer to the FIFO structure
 * @return -1 if buffer is empty, otherwise the data byte
 */
s16_t fifo_peek(FIFO* pfifo);          
/**
 * @brief Check if the FIFO buffer is empty
 * @param[in] pfifo Pointer to the FIFO structure
 * @return TRUE if buffer is empty, FALSE if data is available
 */
BOOL fifo_empty(FIFO* pfifo);     

/**
 * @brief Check if the FIFO buffer is full
 * @param[in] pfifo Pointer to the FIFO structure
 * @return TRUE if buffer is full, FALSE if buffer is not full
 */
BOOL fifo_full(FIFO* pfifo);          

/**
 * @brief Get the number of bytes currently in the FIFO buffer
 * @param[in] pfifo Pointer to the FIFO structure
 * @return Number of data bytes in the FIFO
 */
u16_t fifo_count(FIFO* pfifo);         

/**
 * @brief Flush the FIFO buffer
 * @param[in,out] pfifo Pointer to the FIFO structure
 * @return none
 */
void fifo_flush(FIFO *fifo);             

#endif	/* end of __FIFO_H_A823A9E2_68DA_4e59_9D3D_B90CFA7E20D3 */

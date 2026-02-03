#ifndef __MODBUSTCP_H_9D9FD8A6_B81E_4b93_91E0_5AC8E93E32A9
#define __MODBUSTCP_H_9D9FD8A6_B81E_4b93_91E0_5AC8E93E32A9

#include "App.h"

#define MODBUSTCP_BUFFER_SIZE 600

#pragma pack(1)
typedef struct
{
	u16_t TransactionIdentifier;
	u16_t ProtocolIdentifier;
	u16_t Length;
	u8_t  UnitIdentifier;
}MBAPHDR;
#pragma pack()

typedef struct
{
    MBAPHDR mbapHdr;
    u8_t    Buffer[MODBUSTCP_BUFFER_SIZE];
    //u32_t   childSock;
    u16_t   port;
    u16_t   dataLen;
    u16_t   KeepAliveCount;
    BOOL    lock;
}ModbusTCPState;

void ModbusTCPInit();

#endif	/* end of __MODBUSTCP_H_9D9FD8A6_B81E_4b93_91E0_5AC8E93E32A9 */

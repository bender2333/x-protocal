#ifndef __MODBUSREGS_H_DF59A18D_8C88_4b98_968D_067EEA488A6A
#define __MODBUSREGS_H_DF59A18D_8C88_4b98_968D_067EEA488A6A
#include "App.h"
#include "ModbusOpts.h"
#include "ModbusOptsCal.h"
#include "ModbusServer.h"

#include "ModbusSerial.h"

extern const BYTE* const DiscreteInputPointer[TOTALDISCRETEINPUTPOINTERS];
extern const BYTE* const CoilOutputPointer[TOTALCOILOUTPUTPOINTERS];
extern const WORD* const InputRegisterPointer[TOTALINPUTREGS];
extern const WORD* const HoldingRegisterPointer[TOTALHOLDINGREGS];
extern const BITREGS DiscreteInputRegs[TOTALDISCRETEINPUTREGS];
extern const BITREGS CoilOutputRegs[TOTALCOILOUTPUTREGS];
#endif	/* end of __MODBUSOPTS_H_08E53B9C_92A4_429a_A592_963B21B0CBB9 */


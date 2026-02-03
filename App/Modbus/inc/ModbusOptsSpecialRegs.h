#ifndef __MODBUSOPTSSPECIALREGS_H_
#define __MODBUSOPTSSPECIALREGS_H_

///define Modbus utility/special register start address
#define MODBUSSPECIALREGSSTARTADDR    60000

///define Modbus utility/special register total registers
#define TOTALSPECIALDISCRETEINPUTREGS 4
#define TOTALSPECIALDISCRETEINPUTPOINTERS 2
#define TOTALSPECIALCOILOUTPUTREGS 2
#define TOTALSPECIALCOILOUTPUTPOINTERS 2
#define TOTALSPECIALINPUTREGS 17
#define TOTALSPECIALHOLDINGREGS 10


extern const BITREGS DiscreteInputSpecialRegs[TOTALSPECIALDISCRETEINPUTREGS];
extern const BYTE* const DiscreteInputSpecialPointer[TOTALSPECIALDISCRETEINPUTPOINTERS];
extern const BITREGS CoilOutputSpecialRegs[TOTALSPECIALCOILOUTPUTREGS];
extern const BYTE* const CoilOutputSpecialPointer[TOTALSPECIALCOILOUTPUTPOINTERS];
extern const WORD* const InputRegisterSpecialPointer[TOTALSPECIALINPUTREGS];
extern const WORD* const HoldingRegisterSpecialPointer[TOTALSPECIALHOLDINGREGS];

#endif	/* end of __MODBUSOPTSSPECIALREGS_H_ */



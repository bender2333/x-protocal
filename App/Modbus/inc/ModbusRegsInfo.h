#ifndef __MODBUSREGSINFO_H_9D9FD8A6_B81E_4b93_91E0_5AC8E93E32A8
#define __MODBUSREGSINFO_H_9D9FD8A6_B81E_4b93_91E0_5AC8E93E32A8

#include "App.h"
#include "ModbusOpts.h"


#define MODBUS_REGINT       0
#define MODBUS_REGLONG      1
#define MODBUS_REGFLOAT     2
#define MODBUS_REGSTRING    3
#define MODBUS_REGBIT       4
#define MODBUS_REGSINT      5
#define MODBUS_REGNONE      6

#define BACNETTYPE_NONE		0
#define BACNETTYPE_BVRW		1
#define BACNETTYPE_AVRW		2
#define BACNETTYPE_MSVRW	3
#define BACNETTYPE_BV		4
#define BACNETTYPE_AV		5
#define BACNETTYPE_MSV		6

typedef struct
{
    const u32_t val;
    const char *pText;

}OPTIONSEL;

typedef struct
{
    u16_t  id;
    u8_t type;
    OPTIONSEL *pOptionSel;
}OPTION;

#define OPTION_TEXT			0
#define OPTION_EDIT			1
#define OPTION_BUTTON		2
#define OPTION_COMBO		3
#define OPTION_CHECK		4
#define OPTION_RADIO		5
#define OPTION_LEDGREEN     6
#define OPTION_LEDRED		7
#define OPTION_LEDBTN		8
#define OPTION_COMBONUMBER	9
#define OPTION_BINDDINPUT	10
#define OPTION_BINDDOUTPUT	11
#define OPTION_BINDAINPUT	12
#define OPTION_BINDAOUTPUT	13

typedef struct
{
    u8_t    *regName;
    u8_t    *regDesc;
    u8_t    *regDefault;
    u8_t    regType;
    u8_t    regPosSize;
    u8_t    *regUnit;
    u8_t    regOption;
    u32_t  regMin;
    u32_t  regMax;
    u8_t    *regGroup;
	u8_t	regSubGroup;
    u8_t	ControlType;
    BOOL    Bindable;
	u8_t	BacnetType;
    u8_t    SelectType;
}MODBUSREGINFO;
extern const MODBUSREGINFO ModbusHoldingInfo[TOTALHOLDINGREGS];
extern const MODBUSREGINFO ModbusCoilInfo[TOTALCOILOUTPUTREGS];
extern const MODBUSREGINFO ModbusDiscreteInfo[TOTALDISCRETEINPUTREGS];
extern const MODBUSREGINFO ModbusInputInfo[TOTALINPUTREGS];
#if BINDALLPORT
extern const INT16U ModbusHoldingBindInfo[TOTALMODBUSHOLDINGBINDINFO];
extern const INT16U ModbusCoilBindInfo[TOTALMODBUSCOILBINDINFO];
#endif
extern const OPTIONSEL OptionSel[];
extern const OPTION Option[];
#endif	/* end of __MODBUSREGSINFO_H_9D9FD8A6_B81E_4b93_91E0_5AC8E93E32A8 */

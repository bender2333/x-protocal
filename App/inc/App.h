#ifndef __APP_H_
#define __APP_H_

#include "FreeRTOS.h"
#include "task.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>


#include "EKStdLib.h"
#include "MyDevice.h"


#include "FuncBlock.h"

#include "AppConfig.h"
#include "AutomationOpts.h"
#include "Calibration.h"
#include "PCF8563.h"


#include "AnalogueInput.h"
#include "AnalogueOutput.h"
#include "CalibrateTable.h"
#include "DigitalInput.h"
#include "DigitalOutput.h"
#include "NTP.h"
#include "OTA.h"
#include "PulseAccum.h"
#include "UIO.h"


#include "BacnetApp.h"
#include "BacnetCOV.h"
#include "BacnetIPServer.h"
#include "BacnetMSTPServer.h"
#include "apdu.h"
#include "bvlc.h"
#include "dcc.h"
#include "device.h"
#include "handlers.h"
#include "mstpRS485.h"
#include "tsm.h"


#if defined(INTRINSIC_REPORTING)
#include "address.h"
#include "nc.h"
#endif /* defined(INTRINSIC_REPORTING) */

#include "ModbusRegs.h"
#include "SerialPort.h"


#include "ModbusRegsCal.h"

#include "ModbusSerial.h"
#include "ModbusSerial2.h"
#include "ModbusUSBSerial.h"

#include "ModbusExpRegs.h"
#include "ModbusOptsSpecialRegs.h"
#include "ModbusServer.h"


#include "ModbusClient.h"
#include "ModbusConnector.h"
#include "ModbusMaster.h"
#include "ModbusTCP.h"


#include "netconf.h"

#include "CommMon.h"
#include "Func.h"
#include "HostComm.h"
#include "OEM.h"
#include "RegistersExport.h"
#include "fmath.h"
#include "hvac.h"
#include "logic.h"
#include "priority.h"
#include "schedule.h"
#include "timing.h"
#include "types.h"


#include "eepromEmul.h"

#include "../src/bootutil_priv.h"
#include "Platform/APP/app_region.h"
#include "Source/IBL_Source/ibl_export.h"
#include "bootutil/image.h"
#include "flash_hal/flash_layout.h"
#include "include/mcuboot_config/mcuboot_config.h"
#include "include/storage/flash_map.h"


#define ISNAN isnan
#define ISFINITE isfinite

#define MAXFUNCBLOCK 512
#define MAXFUNCBLOCKMEMORY 20480

// #define DEBUG_PULSEACC
// #define DEBUG_AI_RESPONSE_TIME

#ifdef DEBUG_PULSEACC
#undef DEBUG_AI_RESPONSE_TIME
#endif

#endif

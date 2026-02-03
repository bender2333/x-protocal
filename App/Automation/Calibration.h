#ifndef __CALIBRATION_H_563FF4CD_0A0B_4346_9120_98A9AB3E7BA0
#define __CALIBRATION_H_563FF4CD_0A0B_4346_9120_98A9AB3E7BA0

#include "App.h"
/* IO Calibration Object Initialization */
void CalibrationInit(void);
/* Check if IO Calibration is enable, return CalibrationState */
BOOL IsCalibrationEnable(void);
/* set this true to enable IO calibration */
extern BYTE CalibrationState;
extern WORD CalibrationEnablePW; //Reserved, not used
#endif	/* end of __CALIBRATION_H_563FF4CD_0A0B_4346_9120_98A9AB3E7BA0 */


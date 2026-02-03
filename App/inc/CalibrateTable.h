#ifndef __CALIBRATETABLE_H_
#define __CALIBRATETABLE_H_

#include "App.h"

#define TOTALTEMPTABLE			5
#define TOTALTEMPTABLEENTRY		30

typedef struct 
{
    float   AIRawConvertionM;
    float   AIRawConvertionC;
} AnalogueInputRawConvertElement;

extern const float TemperatureTableROM[TOTALTEMPTABLE][TOTALTEMPTABLEENTRY][2];
extern float TemperatureTable[TOTALTEMPTABLE][TOTALTEMPTABLEENTRY][2];
extern const AnalogueInputRawConvertElement AnalogueInputResCalibrateTableROM[TOTALAIRESISTANCETYPE][TOTALANALOGUEINPUTS];
extern const u16_t AnalogueInputVoltageCalibrateTableROM[TOTALANALOGUEINPUTS][TOTALAIVOLTAGETTYPE][3];
extern const u16_t AnalogueInputCurrentCalibrateTableROM[TOTALANALOGUEINPUTS][TOTALAICURRENTTYPE][4];
extern const u16_t AnalogueOutputCalibrateTableROM[TOTALANALOGUEOUTPUTS][3];
extern const u16_t UIOAOCalibrateTableROM[TOTALUIO][2];
extern const AnalogueInputRawConvertElement UIOUIResCalibrateTableROM[TOTALAIRESISTANCETYPE][TOTALUIO];
extern const u16_t UIOUICurrentCalibrateTableROM[TOTALUIO][TOTALAICURRENTTYPE][4];
extern const u16_t UIOUIVoltageCalibrateTableROM[TOTALUIO ][TOTALAIVOLTAGETTYPE][3];

extern u16_t AnalogueOutputCalibrate[TOTALANALOGUEOUTPUTS][3];
extern u16_t UIOAOCalibrate[TOTALUIO][2];
extern AnalogueInputRawConvertElement AnalogueInputResCalibrate[TOTALAIRESISTANCETYPE][TOTALANALOGUEINPUTS];
extern u16_t AnalogueInputCurrentCalibrate[TOTALANALOGUEINPUTS][TOTALAICURRENTTYPE][4];
extern u16_t AnalogueInputVoltageCalibrate[TOTALANALOGUEINPUTS][TOTALAIVOLTAGETTYPE][3];
extern u16_t UIOUICurrentCalibrate[TOTALUIO][TOTALAICURRENTTYPE][4];
extern u16_t UIOUIVoltageCalibrate[TOTALUIO][TOTALAIVOLTAGETTYPE][3];
extern AnalogueInputRawConvertElement UIOUIResCalibrate[TOTALAIRESISTANCETYPE][TOTALUIO];

#endif
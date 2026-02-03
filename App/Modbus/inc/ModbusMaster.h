#ifndef __MODBUSMASTER_H_
#define __MODBUSMASTER_H_

#include "App.h"

/// @brief Modbus slave device model code ///
#define XE106_485_CODE		0x0511
#define XE4U4_485_CODE		0x0512
#define XE16D_485_CODE		0x0513

/// @brief Modbus slave device IO count ///
#define XE106_AI 5
#define XE106_DI (5 + XE106_AI)
#define XE106_AO 3
#define XE106_DO 3

#define XE4U4_AI  4
#define XE4U4_DI XE4U4_AI
#define XE4U4_UIO 4

#define XE16D_DI 16

typedef enum
{
	DI_PROP_OUTOFSERVICE = 0, //Coil Output
	DI_PROP_USERSETSTATE,
	DO_PROP_OUTOFSERVICE,
	DO_PROP_SETSTATE,		
	AI_PROP_OUTOFSERVICE,	
	AO_PROP_OUTOFSERVICE,

	DI_PROP_STATE,	 		//Discrete Input
	DO_PROP_STATE,

	AI_PROP_UIO_TYPE,		//Holding Regsister
	AI_PROP_TEMPTABLE,
	AI_PROP_DECIMALPOINT,
	AI_PROP_USERSETVALUE,
	AI_PROP_SCALEHIGH,
	AI_PROP_SCALELOW,
	AO_PROP_TYPE,
	AO_PROP_SETVALUE,
	AO_PROP_SCALEHIGH,
	AO_PROP_SCALELOW,

	AI_PROP_VALUE,			//Input Regsister
	AI_PROP_RAW,
	AI_PROP_RELIABILITY,
	AO_PROP_VALUE,
	AO_PROP_RAW,
} IOPROPERTYTYPE;

/// @brief Modbus slave device's point information ///
typedef struct
{
	IOPROPERTYTYPE		IOPropreties;		
	BYTE				Type;
	uint16_t			StartingAddress;
	uint8_t				Count;
	BYTE				DataType;
}PointData;

extern const PointData XE106[];
extern const PointData XE4U4[];
extern const PointData XE16D[];

/**
 * @brief   Add a Modbus slave device with predefined point configuration.
 *
 * @param[in]  index    Device index in the master array.
 * @param[in]  devIndex Device type index (1 = XE106, 2 = XE4U4, 3 = XE16D).
 * @param[in]  devID    Device ID (slave address).
 * @param[out] none
 * @return     TRUE if device added successfully, FALSE otherwise.
 */
BOOL ModbusMasterAddDevice(uint16_t index, uint8_t devIndex, uint8_t devID);

/**
 * @brief   Modbus master initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ModbusMasterInit();

/**
 * @brief   Modbus master object loop.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void ModbusMasterObj();

#endif
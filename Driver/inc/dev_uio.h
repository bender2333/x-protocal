#ifndef __DEV_UIO_H_
#define __DEV_UIO_H_

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
    uint8_t               ucChannel;
    tGPIOInfo             Pin;
}
PACKEDSTRUCT tAIInfo;
#ifdef ewarm
#pragma pack()
#endif

#define DEV_ADC_MAX             2
#define DEV_AI_TOTAL_UI         8
#define DEV_AI_TOTAL_UIO        4
#define DEV_AI_TOTAL_UO         6
#define DEV_AI_TOTAL            (DEV_AI_TOTAL_UI + DEV_AI_TOTAL_UIO)

#define DEV_ADC2_AI_START_CH    6

#define	ADC_MAX_VALUE		    4096.000
#define ADC_REF                 2.500
#define ADC_RES_PULLUP          31600.000

#define DEV_AI_DAMPINGFACTOR    256
#define DEV_AI_DAMPING_SHIFT    8

typedef enum
{
    DEV_AI_VOLTAGE,
    DEV_AI_CURRENT, 
    DEV_AI_RESDI,
    DEV_AO_V,
    DEV_DO
} DEV_UIO_PINTYPE;

typedef enum 
{
    DEV_AI_IDLE = 0,
    DEV_AI_WAIT_CH_STABLE,
    DEV_AI_WAIT_SAMPLE,
    DEV_AI_SAMPLE_TIMEOUT,
} DEV_AI_STATE;

typedef void (DEVAIEOCCALLBACK)(void);

extern s32_t   dev_ai_cvalue[DEV_AI_TOTAL]; // damped ADC mean value
extern s32_t   dev_ai_rvalue[DEV_AI_TOTAL]; // sampled ADC raw value

/**
 * @brief   ADC peripheral and GPIO pins initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void dev_adc_init(void);

/**
 * @brief   AI peripheral and GPIO pins initialization.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void dev_ai_init(void);

/**
 * @brief   Get the raw ADC value for a specific AI channel.
 *
 * @param[in]  ch   AI channel number.
 * @param[out] none
 * @return     Raw ADC value for the specified channel.
 */
s32_t dev_ai_getRaw(u8_t ch);

/**
 * @brief   Get the damped ADC mean value for a specific AI channel.
 *
 * @param[in]  ch   AI channel number.
 * @param[out] none
 * @return     Damped ADC mean value for the specified channel.
 */
s32_t dev_ai_getValue(u8_t ch);

/**
 * @brief   Set the gain of the PGA for a specific AI channel.
 *
 * @param[in]  ch    AI channel number.
 * @param[in]  gain  Gain value (1-16).
 * @param[out] none
 * @return     none
 */
void dev_ai_setGain(u8_t ch, u8_t gain);

/**
 * @brief   Get the current gain of the PGA for a specific AI channel.
 *
 * @param[in]  ch   AI channel number.
 * @param[out] none
 * @return     Current gain value (1-16).
 */
u8_t dev_ai_getGain(u8_t ch);

/**
 * @brief   Set the end-of-conversion callback function for ADC operations.
 *
 * @param[in]  cb   Pointer to the callback function.
 * @param[out] none
 * @return     none
 */
void dev_ai_SetEOCCallback(DEVAIEOCCALLBACK* cb);

/**
 * @brief   Set the type for a specific AI channel.
 *
 * @param[in]  ch    AI channel number.
 * @param[in]  type  Channel type (V010, I020, R, DI).
 * @param[out] none
 * @return     none
 */
void dev_ai_type_set(BYTE ch, BYTE type);

/**
 * @brief   Set the type for a specific UIO channel.
 *
 * @param[in]  ch    UIO channel number.
 * @param[in]  type  Channel type (V010, I020, RD, VO, DO).
 * @param[out] none
 * @return     none
 */
void dev_uio_type_set(BYTE ch, BYTE type);

/**
 * @brief   Set the type for a specific UO channel.
 *
 * @param[in]  ch    UO channel number.
 * @param[in]  type  Channel type (V010, I020).
 * @param[out] none
 * @return     none
 */
void dev_uo_type_set(BYTE ch, BYTE type);

/**
 * @brief   Set the LED state for a specific UI channel.
 *
 * @param[in]  ch      UI channel number.
 * @param[in]  bOnOff  TRUE to turn on, FALSE to turn off.
 * @param[out] none
 * @return     none
 */
void dev_ui_led_set(BYTE ch, BOOL bOnOff);

/**
 * @brief   Set the LED state for a specific UIO channel.
 *
 * @param[in]  ch      UIO channel number.
 * @param[in]  bOnOff  TRUE to turn on, FALSE to turn off.
 * @param[out] none
 * @return     none
 */
void dev_uio_led_set(BYTE ch, BOOL bOnOff);
#endif

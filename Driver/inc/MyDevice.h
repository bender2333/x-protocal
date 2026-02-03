#ifndef __DEVICE_H_A23C3F65_8BCC_46c9_B135_FDFA199BA6DF
#define __DEVICE_H_A23C3F65_8BCC_46c9_B135_FDFA199BA6DF

#include <string.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "dev_systick.h"
#include "dev_stream.h"
#include "dev_gpio.h"
#include "dev_led.h"
#include "dev_flash.h"
#include "dev_util.h"
#include "dev_svc.h"
#include "dev_softi2c.h"
#include "dev_uart.h"
#include "dev_do.h"
#include "dev_pwm.h"
#include "dev_dac.h"
#include "dev_uio.h"
#include "dev_ssi.h"
#include "dev_w25q128.h"
#include "dev_sdram.h"
#include "dev_di.h"
#include "dev_pga116.h"
#include "dev_ms5262d.h"
#include "dev_PulseAccumTimer.h"
#include "dev_usbd.h"
#include "dev_sn74595.h"

#define ENABLE_INTERRUPT()	taskEXIT_CRITICAL()
#define DISABLE_INTERRUPT()   taskENTER_CRITICAL()

// Interrupt priority//////////////////////////////////////
#define USART0_6_INT_PREMEPTPRIORITY		0   
#define USART1_5_INT_PREMEPTPRIORITY		2   

#define TIM5_INT_PREMEPTPRIORITY		2   // TIM5
#define ADC1_INT_PREMEPTPRIORITY		3

#define EXT0_INT_PREMEPTPRIORITY		4
#define EXT1_INT_PREMEPTPRIORITY		5
#define EXTI15_10_INT_PREMEPTPRIORITY	        6

////////////////////////////////////////////////////////////

#endif // end of __DEVICE_H_A23C3F65_8BCC_46c9_B135_FDFA199BA6DF

